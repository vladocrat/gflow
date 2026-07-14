# Third-Party Notices

gflow bundles and/or links the following third-party components. Each is the
property of its respective authors and is provided under its own license. These
licenses are all permissive and compatible with gflow's AGPL-3.0 license.

| Component | Version | License |
|---|---|---|
| gRPC | 1.76.0 | Apache-2.0 |
| Protocol Buffers (protobuf) | 6.33.4 | BSD-3-Clause |
| Abseil | (bundled with gRPC 1.76.0) | Apache-2.0 |
| BoringSSL | (bundled with gRPC 1.76.0) | OpenSSL / ISC / MIT-style |
| c-ares | (bundled with gRPC 1.76.0) | MIT |
| RE2 | (bundled with gRPC 1.76.0) | BSD-3-Clause |
| zlib | (bundled with gRPC 1.76.0) | Zlib |
| Lua | 5.4.8 | MIT |
| sol2 | 3.5.0 | MIT |
| spdlog | 1.15.3 | MIT |
| fmt | (bundled with spdlog 1.15.3) | MIT |
| CLI11 | 2.6.2 | BSD-3-Clause |
| GoogleTest (tests only) | 1.15.2 | BSD-3-Clause |

The heavy native components (gRPC, protobuf, Lua) are acquired via vcpkg (see
`vcpkg.json`); full license texts are available in each component's upstream
repository and in the corresponding vcpkg port.
