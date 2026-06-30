# gflow

Drive a gRPC server by writing **Lua scripts**. gflow loads a `.proto` at
runtime via protobuf reflection (no codegen), exposes every message as a Lua
constructor and every RPC as a Lua function, and runs your scripts against a
live server. See [DESIGN.md](DESIGN.md) for the full design.

## Status

Early setup. The build, dependency, and CI scaffolding are in place; the engine
(loader, bridge, dispatcher, streaming, type emitter) is being built per the
milestones in `DESIGN.md`.

## Building

Requirements: Windows, Visual Studio 2022 (MSVC), CMake ≥ 3.25, Ninja. The
native dependencies (gRPC, protobuf, Lua) are committed prebuilt under
`third_party/`, so no dependency compilation is needed.

```
cmake --preset vs2022
cmake --build --preset vs2022
ctest --preset vs2022
```

`vs2022` works from any shell. For a Ninja build (used by CI) from a developer
command prompt, use the `windows-msvc-debug` preset instead.

## License

Copyright (C) 2026 Vladislav Milovanov

gflow is free software: you can redistribute it and/or modify it under the terms
of the **GNU Affero General Public License, version 3** as published by the Free
Software Foundation. See [LICENSE](LICENSE).

This program is distributed in the hope that it will be useful, but WITHOUT ANY
WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A
PARTICULAR PURPOSE. See the GNU AGPL for more details.

The AGPL requires that anyone who distributes gflow, or makes a modified version
available over a network, also makes the corresponding source available under
the AGPL.

### Commercial license

gflow is also available under a separate **commercial license** for use without
the AGPL's copyleft obligations (e.g. embedding in proprietary software or a
closed network service). Contact **Vladislav Milovanov &lt;vladocrat@gmail.com&gt;**.

Third-party components bundled under `third_party/` keep their own licenses; see
[THIRD-PARTY-NOTICES.md](THIRD-PARTY-NOTICES.md).
