#include "Registrar.hpp"

#include <functional>
#include <memory>
#include <stdexcept>
#include <string>

#include <google/protobuf/descriptor.h>
#include <google/protobuf/message.h>

#include <spdlog/spdlog.h>

#include "gflow-sdk/Bridge.hpp"
#include "gflow-sdk/GRPCClient.hpp"
#include "gflow-sdk/ProtoModel.hpp"
#include "gflow-sdk/ServerStream.hpp"

namespace gflow
{

namespace
{

void registerMessages(sol::table& module, ProtoModel& model)
{
    for (const auto* descriptor : model.messages()) {
        module[std::string(descriptor->name())] = [](sol::table fields) {
            return fields;
        };
    }
}

void registerEnums(sol::table& module, sol::state& lua, ProtoModel& model)
{
    for (const auto* descriptor : model.enums()) {
        auto values = lua.create_table();
        for (int i = 0; i < descriptor->value_count(); ++i) {
            const std::string name(descriptor->value(i)->name());
            values[name] = name;
        }
        module[std::string(descriptor->name())] = values;
    }
}

sol::table registerServiceMethods(
    sol::state& lua, ProtoModel& model, const google::protobuf::ServiceDescriptor* const service, GRPCClient& client
)
{
    auto serviceTable = lua.create_table();

    for (int i = 0; i < service->method_count(); ++i) {
        const auto* method = service->method(i);
        const std::string methodName(method->name());

        if (method->client_streaming()) {
            continue;
        }

        if (method->server_streaming()) {
            serviceTable[methodName] =
                [method, &model, &client](sol::this_state state, const sol::table& request) -> sol::object {
                const sol::state_view lua(state);

                const auto requestMessage = model.newMessage(method->input_type());
                luaTableToMessage(request, requestMessage.get());

                std::shared_ptr<ServerStream> stream = client.serverStreamingCall(method, *requestMessage);

                std::function<sol::object(sol::this_state)> iterator =
                    [stream, method, &model](sol::this_state innerState) -> sol::object {
                    const sol::state_view innerLua(innerState);
                    const auto responseMessage = model.newMessage(method->output_type());

                    if (stream->read(responseMessage.get())) {
                        return sol::object(messageToLuaTable(*responseMessage, innerLua));
                    }

                    const grpc::Status status = stream->finish();
                    if (!status.ok()) {
                        throw std::runtime_error(
                            "stream " + std::string(method->full_name()) + " failed: " + status.error_message()
                        );
                    }

                    return sol::lua_nil;
                };

                return sol::make_object(lua, std::move(iterator));
            };
            continue;
        }

        serviceTable[methodName] =
            [method, &model, &client](sol::this_state state, const sol::table& request) -> sol::table {
            const sol::state_view lua(state);

            const auto requestMessage  = model.newMessage(method->input_type());
            const auto responseMessage = model.newMessage(method->output_type());

            luaTableToMessage(request, requestMessage.get());

            const grpc::Status status = client.unaryCall(method, *requestMessage, responseMessage.get());

            if (!status.ok()) {
                spdlog::error("request failed");
            }

            return messageToLuaTable(*responseMessage, lua);
        };
    }

    return serviceTable;
}

void registerServices(sol::table& module, sol::state& lua, ProtoModel& model, GRPCClient& client)
{
    for (const auto* service : model.services()) {
        module[std::string(service->name())] = registerServiceMethods(lua, model, service, client);
    }
}

} // namespace

void registerBindings(sol::state& lua, ProtoModel& model, GRPCClient& client)
{
    auto module = lua.create_table();

    registerMessages(module, model);
    registerEnums(module, lua, model);
    registerServices(module, lua, model, client);

    lua["package"]["loaded"][model.name()] = module;
}

} // namespace gflow
