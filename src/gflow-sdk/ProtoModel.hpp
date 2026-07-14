#pragma once

#include <filesystem>
#include <memory>
#include <string>
#include <vector>

#include <utils-cpp/pimpl.h>

namespace google::protobuf
{
class Descriptor;
class ServiceDescriptor;
class Message;
class EnumDescriptor;
} // namespace google::protobuf

namespace gflow
{

class ProtoModel final
{
public:
    ProtoModel();
    ~ProtoModel();

    ProtoModel(const ProtoModel&)            = delete;
    ProtoModel& operator=(const ProtoModel&) = delete;
    ProtoModel(ProtoModel&&)                 = delete;
    ProtoModel& operator=(ProtoModel&&)      = delete;

    bool load(
        const std::filesystem::path& protoFile, const std::vector<std::filesystem::path>& importPaths,
        std::string* error = nullptr
    );

    std::string name() const;

    std::vector<const google::protobuf::EnumDescriptor*> enums() const;
    std::vector<const google::protobuf::Descriptor*> messages() const;
    std::vector<const google::protobuf::ServiceDescriptor*> services() const;

    std::unique_ptr<google::protobuf::Message> newMessage(const google::protobuf::Descriptor* descriptor) const;

private:
    DECLARE_PIMPL
};

} // namespace gflow
