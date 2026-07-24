// Copyright (C) 2026 Vladislav Milovanov
// SPDX-License-Identifier: AGPL-3.0-only

#include "ProtoModel.hpp"

#include <format>

#include <google/protobuf/compiler/importer.h>
#include <google/protobuf/descriptor.h>
#include <google/protobuf/dynamic_message.h>

#include "Utils.hpp"

namespace gflow
{

namespace
{

struct CollectingErrors final : public google::protobuf::compiler::MultiFileErrorCollector
{
public:
    void RecordError(absl::string_view filename, int line, int column, absl::string_view message) override
    {
        text = std::format("[{} {}:{}]: {} ", filename, line, column, message);
    }

    bool hasErrors() const noexcept
    {
        return !text.empty();
    }

    std::string text;
};

} // namespace

struct ProtoModel::impl_t
{
    google::protobuf::compiler::DiskSourceTree tree;
    CollectingErrors errors;
    std::unique_ptr<google::protobuf::compiler::Importer> importer;
    google::protobuf::DynamicMessageFactory factory;
    const google::protobuf::FileDescriptor* file {nullptr};

    void registerPaths(const std::filesystem::path& entryPoint, const std::vector<std::filesystem::path>& importPaths)
    {
        tree.MapPath("", entryPoint.parent_path().string());

        for (const auto& path : importPaths) {
            tree.MapPath("", path.string());
        }

        importer = std::make_unique<google::protobuf::compiler::Importer>(&tree, &errors);
        file     = importer->Import(entryPoint.filename().string());
    }
};

ProtoModel::ProtoModel()
{
    createImpl();
};

ProtoModel::~ProtoModel() = default;

bool ProtoModel::load(
    const std::filesystem::path& protoFile, const std::vector<std::filesystem::path>& importPaths, std::string* error
)
{
    _impl->registerPaths(protoFile, importPaths);

    if (_impl->file != nullptr) {
        return true;
    }

    if (error != nullptr) {
        *error =
            !_impl->errors.hasErrors() ? _impl->errors.text : std::format("Failed to import {}", protoFile.string());
    }

    return false;
}

std::string ProtoModel::name() const
{
    return std::filesystem::path(std::string(_impl->file->name())).stem().string();
}

std::vector<const google::protobuf::EnumDescriptor*> ProtoModel::enums() const
{
    return std::views::iota(0, _impl->file->enum_type_count()) | std::views::transform([this](int i) {
        return _impl->file->enum_type(i);
    }) | to<std::vector<const google::protobuf::EnumDescriptor*>>();
}

std::vector<const google::protobuf::Descriptor*> ProtoModel::messages() const
{
    return std::views::iota(0, _impl->file->message_type_count()) | std::views::transform([this](int i) {
        return _impl->file->message_type(i);
    }) | to<std::vector<const google::protobuf::Descriptor*>>();
}

std::vector<const google::protobuf::ServiceDescriptor*> ProtoModel::services() const
{
    return std::views::iota(0, _impl->file->service_count()) | std::views::transform([this](int i) {
        return _impl->file->service(i);
    }) | to<std::vector<const google::protobuf::ServiceDescriptor*>>();
}

std::unique_ptr<google::protobuf::Message> ProtoModel::newMessage(const google::protobuf::Descriptor* descriptor) const
{
    return std::unique_ptr<google::protobuf::Message>(_impl->factory.GetPrototype(descriptor)->New());
}

} // namespace gflow
