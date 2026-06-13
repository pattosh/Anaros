# Anaros

A C++26 header-only library that uses P2996 static reflection to automatically
serialize and deserialize structs to YAML. Requires GCC 16 with `-freflection`.

[![CI](https://github.com/pattosh/Anaros/actions/workflows/ci.yml/badge.svg?branch=main)](https://github.com/pattosh/Anaros/actions/workflows/ci.yml)

With Anaros, the struct is the schema. Want to add a new string field to your YAML schema?
Simply add a new `std::string` member to your struct and Anaros will automatically generate
serialization and de-serialization code for it.

## Usage

```cpp
#include "anaros.h"

struct Config {
    std::string host;
    int port = 8080;            // default applied when the key is absent
    std::optional<int> workers; // omitted from output when not set
};

// Deserialize: returns std::expected<Config, std::string>
auto result = anaros::parse_from_yaml<Config>(YAML::Load("{host: localhost, port: 9000}"));
if (result) {
    use(*result);
} else {
    std::println("parse error: {}", result.error());
}

// Serialize: returns a YAML::Node (dump it with YAML::Dump for a string)
Config cfg{.host = "localhost", .port = 9000};
std::string yaml = YAML::Dump(anaros::to_yaml(cfg));
```

Fields are matched by member name. A missing key is an error unless the field is
`std::optional` or has a default member initializer. On serialization, an empty
`std::optional` is omitted from the output.

### Custom types

For types that aren't plain aggregates — or that need validation, custom
constructors, or a bespoke YAML shape — specialize `anaros::yaml_traits<T>`.
Anaros automatically picks up `parse` when deserializing and `dump` when
serializing; you can define either or both.

```cpp
struct Vec3 { float x, y, z; };

template <>
struct anaros::yaml_traits<Vec3> {
    static std::expected<Vec3, std::string> parse(const YAML::Node& n) {
        if (!n["x"] || !n["y"] || !n["z"]) {
            return std::unexpected("Vec3 needs x, y, z");
        }
        return Vec3{n["x"].as<float>(), n["y"].as<float>(), n["z"].as<float>()};
    }

    static YAML::Node dump(const Vec3& v) {
        YAML::Node n;
        n["x"] = v.x;
        n["y"] = v.y;
        n["z"] = v.z;
        return n;
    }
};
```

Because `parse` *returns* the value rather than mutating a default-constructed
one, it supports types that have no default constructor and rich, propagating
error messages. A `Vec3` member of another struct is handled automatically — its
`parse`/`dump` are used wherever the type appears.

## Quickstart (Bazel — primary)

```sh
nix develop --command bazel test //...
```

Everything — GCC 16, yaml-cpp, gtest, clang-format — comes from the pinned
Nix store. `nix develop` drops you into the dev shell; `bazel test` builds
against the nix-wrapped cc toolchain registered in `MODULE.bazel`.

## CMake (consumer-facing)

The CMake build is preserved as a thin surface for downstream projects that
prefer `find_package`. From within the dev shell:

```sh
nix develop --command sh -c '
  cmake -S . -B build -G Ninja -DBUILD_TESTING=ON &&
  cmake --build build &&
  ctest --test-dir build --output-on-failure
'
```

To consume from a downstream CMake project, after installing:

```sh
cmake --install build --prefix /some/prefix
```

then in the downstream project:

```cmake
find_package(Anaros REQUIRED)
target_link_libraries(my_target PRIVATE Anaros::anaros)
```

## Editor integration (compile_commands.json)

```sh
nix develop --command bazel run //:refresh_compile_commands
```

writes a `compile_commands.json` at the repo root with entries pointing at the
nix-store GCC.

## Formatting

```sh
nix develop --command clang-format -i src/*.h test/*.cpp
```
