#pragma once

#include <expected>
#include <meta>
#include <string>
#include <yaml-cpp/yaml.h>

namespace anaros {

template <typename T>
consteval std::size_t field_count() {
    return std::meta::nonstatic_data_members_of(^^T, std::meta::access_context::unchecked()).size();
}

template <typename T>
consteval auto get_fields() {
    std::array<std::meta::info, field_count<T>()> arr{};
    auto members =
        std::meta::nonstatic_data_members_of(^^T, std::meta::access_context::unchecked());
    std::ranges::copy(members, arr.begin());
    return arr;
}

template <typename T>
std::expected<T, std::string> parse_from_yaml(const YAML::Node& yaml_dict) {
    T result{};

    template for (constexpr auto field : get_fields<T>()) {
        constexpr auto name = std::meta::identifier_of(field);

        // Alias the field type to avoid parser ambiguity in template argument
        using FieldType = typename[:std::meta::type_of(field):];

        if (!yaml_dict[name]) {
            return std::unexpected(std::string("Missing field: ") + std::string(name));
        }

        try {
            result.[:field:] = yaml_dict[name].template as<FieldType>();
        } catch (const YAML::Exception& e) {
            return std::unexpected(std::string("Failed to parse field '") + std::string(name) +
                                   "': " + e.what());
        }
    }

    return result;
}

} // namespace anaros