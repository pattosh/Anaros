#pragma once

#include <expected>
#include <meta>
#include <optional>
#include <string>
#include <type_traits>
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
struct yaml_traits; // primary left undefined — specialize to opt in

template <typename T>
concept HasYamlTraits = requires(const YAML::Node& n) {
    { yaml_traits<T>::parse(n) } -> std::same_as<std::expected<T, std::string>>;
};

namespace detail {

template <typename>
struct is_optional : std::false_type {};

template <typename U>
struct is_optional<std::optional<U>> : std::true_type {
    using value_type = U;
};

template <typename U>
std::expected<U, std::string> parse_value(const YAML::Node& n) {
    if constexpr (HasYamlTraits<U>) {
        return yaml_traits<U>::parse(n);
    } else {
        try {
            return n.template as<U>();
        } catch (const YAML::Exception& e) {
            return std::unexpected(std::string(e.what()));
        }
    }
}

} // namespace detail

template <typename T>
std::expected<T, std::string> parse_from_yaml(const YAML::Node& yaml_dict) {
    if constexpr (HasYamlTraits<T>) {
        return yaml_traits<T>::parse(yaml_dict);
    } else {
        T result{};

        template for (constexpr auto field : get_fields<T>()) {
            constexpr auto name = std::meta::identifier_of(field);
            using FieldType = typename[:std::meta::type_of(field):];

            auto sub = yaml_dict[name];
            if (!sub) {
                if constexpr (detail::is_optional<FieldType>::value) {
                    continue;
                } else if constexpr (std::meta::has_default_member_initializer(field)) {
                    continue;
                } else {
                    return std::unexpected(std::string("Missing field: ") + std::string(name));
                }
            } else if constexpr (detail::is_optional<FieldType>::value) {
                using Inner = typename detail::is_optional<FieldType>::value_type;
                auto r = detail::parse_value<Inner>(sub);
                if (!r) {
                    return std::unexpected(std::string("Failed to parse field '") +
                                           std::string(name) + "': " + r.error());
                }
                result.[:field:] = *std::move(r);
            } else {
                auto r = detail::parse_value<FieldType>(sub);
                if (!r) {
                    return std::unexpected(std::string("Failed to parse field '") +
                                           std::string(name) + "': " + r.error());
                }
                result.[:field:] = *std::move(r);
            }
        }

        return result;
    }
}

} // namespace anaros
