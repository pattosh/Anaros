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
concept HasYamlParse = requires(const YAML::Node& n) {
    { yaml_traits<T>::parse(n) } -> std::same_as<std::expected<T, std::string>>;
};

template <typename T>
concept HasYamlDump = requires(const T& v) {
    { yaml_traits<T>::dump(v) } -> std::same_as<YAML::Node>;
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
    if constexpr (HasYamlParse<U>) {
        return yaml_traits<U>::parse(n);
    } else {
        try {
            return n.template as<U>();
        } catch (const YAML::Exception& e) {
            return std::unexpected(std::string(e.what()));
        }
    }
}

template <typename U>
YAML::Node emit_value(const U& v) {
    if constexpr (HasYamlDump<U>) {
        return yaml_traits<U>::dump(v);
    } else {
        return YAML::Node(v);
    }
}

} // namespace detail

template <typename T>
std::expected<T, std::string> parse_from_yaml(const YAML::Node& yaml_dict) {
    if constexpr (HasYamlParse<T>) {
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

template <typename T>
YAML::Node to_yaml(const T& value) {
    if constexpr (HasYamlDump<T>) {
        return yaml_traits<T>::dump(value);
    } else {
        YAML::Node node(YAML::NodeType::Map);

        template for (constexpr auto field : get_fields<T>()) {
            constexpr auto name = std::meta::identifier_of(field);
            using FieldType = typename[:std::meta::type_of(field):];

            if constexpr (detail::is_optional<FieldType>::value) {
                if (value.[:field:].has_value()) {
                    using Inner = typename detail::is_optional<FieldType>::value_type;
                    node[std::string(name)] = detail::emit_value<Inner>(*value.[:field:]);
                }
                // absent optional -> field omitted, symmetric with parse_from_yaml
            } else {
                node[std::string(name)] = detail::emit_value<FieldType>(value.[:field:]);
            }
        }

        return node;
    }
}

} // namespace anaros
