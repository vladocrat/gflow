#include <memory>

#include <google/protobuf/descriptor.h>
#include <google/protobuf/dynamic_message.h>
#include <google/protobuf/compiler/importer.h>

#include <spdlog/spdlog.h>

#include "gflow-sdk/GRPCClient.hpp"

int main()
{
    google::protobuf::compiler::DiskSourceTree sourceTree;
    sourceTree.MapPath("", GFLOW_PROTO_DIR);

    google::protobuf::compiler::Importer importer(&sourceTree, nullptr);
    const google::protobuf::FileDescriptor* file = importer.Import("echo.proto");
    if (!file) {
        spdlog::error("gflow: failed to load echo.proto from {}", GFLOW_PROTO_DIR);
        return 1;
    }

    const google::protobuf::ServiceDescriptor* service = file->FindServiceByName("EchoService");
    const google::protobuf::MethodDescriptor* method = service->FindMethodByName("Echo");
    const google::protobuf::Descriptor* inputType = method->input_type();
    const google::protobuf::Descriptor* outputType = method->output_type();

    google::protobuf::DynamicMessageFactory factory;
    std::unique_ptr<google::protobuf::Message> request(factory.GetPrototype(inputType)->New());
    std::unique_ptr<google::protobuf::Message> response(factory.GetPrototype(outputType)->New());

    const google::protobuf::Reflection* reflection = request->GetReflection();
    reflection->SetString(request.get(), inputType->FindFieldByName("message"), "hello");
    reflection->SetInt32(request.get(), inputType->FindFieldByName("count"), 3);

    gflow::GRPCClient client("localhost:50051");
    const grpc::Status status = client.unaryCall(method, *request, response.get());

    if (!status.ok()) {
        spdlog::error("gflow: Echo failed: {} {}", static_cast<int>(status.error_code()), status.error_message());
        return 1;
    }

    spdlog::info("gflow: Echo OK\n{}", response->DebugString());
    return 0;
}
