// Copyright (C) 2026 Vladislav Milovanov
// SPDX-License-Identifier: AGPL-3.0-only

#include "CliParser.hpp"

#include <CLI/CLI.hpp>

struct CliParser::impl_t
{
    CLI::App app {"Updates binaries of an application", "Updater"};
    Command parsedCommand {Command::None};

    impl_t()
    {
        app.require_option(1);

        auto* update = app.add_subcommand("update", "Update the application of the given pid");
        update->add_option("--patch-location", "Root directory of the new binaries")
            ->required()
            ->check(CLI::ExistingDirectory);
        update->add_flag("--zip", "Decides wether the binaries need to be unpacked before copying")->default_val(false);
        update->callback([this]() {
            parsedCommand = Command::Update;
        });
    }
};

CliParser::CliParser()
{
    createImpl();
}

CliParser::~CliParser() = default;

int CliParser::parse(int argc, char** argv)
{
    CLI11_PARSE(impl().app, argc, argv);
    return 0;
}

Command CliParser::selected() const
{
    return impl().parsedCommand;
}
