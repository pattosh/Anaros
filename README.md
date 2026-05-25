# Anaros

A C++26 header-only library that uses P2996 static reflection to automatically
serialize and deserialize structs to YAML. Requires GCC 16 with `-freflection`.

With Anaros, the struct is the schema. Want to add a new string field to your YAML schema?
Simply add a new `std::string` member to your struct and Anaros will automatically generate
serialization and de-serialization code for it.

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
