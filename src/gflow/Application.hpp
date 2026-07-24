// Copyright (C) 2026 Vladislav Milovanov
// SPDX-License-Identifier: AGPL-3.0-only

#pragma once

#include <utils-cpp/pimpl.h>

namespace gflow
{

class Application final
{
public:
    Application();
    ~Application();

    Application(const Application&)            = delete;
    Application& operator=(const Application&) = delete;
    Application(Application&&)                 = delete;
    Application& operator=(Application&&)      = delete;

    int run(int argc, char** argv);

private:
    DECLARE_PIMPL
};

} // namespace gflow
