#include <memory>
#include <string>

#include <gtest/gtest.h>

#include <google/protobuf/compiler/importer.h>
#include <google/protobuf/descriptor.h>
#include <google/protobuf/dynamic_message.h>

#include "EchoServer.hpp"
#include "GRPCClient.hpp"

namespace {

class EchoIntegrationTest : public ::testing::Test
{
protected:
    void SetUp() override
    {
        m_server = std::make_unique<gflow::demo::EchoServer>("localhost:0");
        ASSERT_NE(m_server->port(), 0) << "echo server failed to bind";

        m_client = std::make_unique<gflow::GRPCClient>("localhost:" + std::to_string(m_server->port()));

        m_sourceTree.MapPath("", GFLOW_PROTO_DIR);
        m_importer = std::make_unique<google::protobuf::compiler::Importer>(&m_sourceTree, nullptr);
        m_file = m_importer->Import("echo.proto");
        ASSERT_NE(m_file, nullptr) << "failed to import echo.proto";
    }

    const google::protobuf::MethodDescriptor* echoMethod() const
    {
        return m_file->FindServiceByName("EchoService")->FindMethodByName("Echo");
    }

    google::protobuf::DynamicMessageFactory m_factory;
    google::protobuf::compiler::DiskSourceTree m_sourceTree;
    std::unique_ptr<google::protobuf::compiler::Importer> m_importer;
    const google::protobuf::FileDescriptor* m_file = nullptr;
    std::unique_ptr<gflow::demo::EchoServer> m_server;
    std::unique_ptr<gflow::GRPCClient> m_client;
};

TEST_F(EchoIntegrationTest, EchoesMessageAndComputesLength)
{
    const google::protobuf::MethodDescriptor* method = echoMethod();
    const google::protobuf::Descriptor* inputType = method->input_type();
    const google::protobuf::Descriptor* outputType = method->output_type();

    std::unique_ptr<google::protobuf::Message> request(m_factory.GetPrototype(inputType)->New());
    std::unique_ptr<google::protobuf::Message> response(m_factory.GetPrototype(outputType)->New());

    request->GetReflection()->SetString(request.get(), inputType->FindFieldByName("message"), "hello");

    const grpc::Status status = m_client->unaryCall(method, *request, response.get());
    ASSERT_TRUE(status.ok()) << status.error_message();

    const google::protobuf::Reflection* out = response->GetReflection();
    EXPECT_EQ(out->GetString(*response, outputType->FindFieldByName("message")), "hello");
    EXPECT_EQ(out->GetInt32(*response, outputType->FindFieldByName("length")), 5);
}

TEST_F(EchoIntegrationTest, EchoesEnumByName)
{
    const google::protobuf::MethodDescriptor* method = echoMethod();
    const google::protobuf::Descriptor* inputType = method->input_type();
    const google::protobuf::Descriptor* outputType = method->output_type();

    std::unique_ptr<google::protobuf::Message> request(m_factory.GetPrototype(inputType)->New());
    std::unique_ptr<google::protobuf::Message> response(m_factory.GetPrototype(outputType)->New());

    const google::protobuf::FieldDescriptor* colorField = inputType->FindFieldByName("color");
    const google::protobuf::EnumValueDescriptor* blue = colorField->enum_type()->FindValueByName("BLUE");
    request->GetReflection()->SetEnum(request.get(), colorField, blue);

    const grpc::Status status = m_client->unaryCall(method, *request, response.get());
    ASSERT_TRUE(status.ok()) << status.error_message();

    const google::protobuf::Reflection* out = response->GetReflection();
    const google::protobuf::EnumValueDescriptor* echoed =
        out->GetEnum(*response, outputType->FindFieldByName("color"));
    EXPECT_EQ(echoed->name(), "BLUE");
}

TEST_F(EchoIntegrationTest, EmptyMessageHasZeroLength)
{
    const google::protobuf::MethodDescriptor* method = echoMethod();
    std::unique_ptr<google::protobuf::Message> request(m_factory.GetPrototype(method->input_type())->New());
    std::unique_ptr<google::protobuf::Message> response(m_factory.GetPrototype(method->output_type())->New());

    const grpc::Status status = m_client->unaryCall(method, *request, response.get());
    ASSERT_TRUE(status.ok()) << status.error_message();

    const google::protobuf::Reflection* out = response->GetReflection();
    EXPECT_EQ(out->GetInt32(*response, method->output_type()->FindFieldByName("length")), 0);
}

}
