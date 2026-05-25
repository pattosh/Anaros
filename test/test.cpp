#include "anaros.h"

#include <gtest/gtest.h>
#include <optional>
#include <string>
#include <yaml-cpp/yaml.h>

struct TestStruct {
    int a;
    std::string b;
};

TEST(ParseFromYaml, HappyPath) {
    YAML::Node yaml = YAML::Load("{a: 42, b: 'Hello'}");
    auto result = anaros::parse_from_yaml<TestStruct>(yaml);

    ASSERT_TRUE(result.has_value()) << result.error();
    EXPECT_EQ(result->a, 42);
    EXPECT_EQ(result->b, "Hello");
}

TEST(ParseFromYaml, MissingField) {
    YAML::Node yaml = YAML::Load("{a: 42}");
    auto result = anaros::parse_from_yaml<TestStruct>(yaml);

    ASSERT_FALSE(result.has_value());
    EXPECT_NE(result.error().find("Missing field"), std::string::npos);
    EXPECT_NE(result.error().find("b"), std::string::npos);
}

TEST(ParseFromYaml, TypeMismatch) {
    YAML::Node yaml = YAML::Load("{a: 'not-an-int', b: 'Hello'}");
    auto result = anaros::parse_from_yaml<TestStruct>(yaml);

    ASSERT_FALSE(result.has_value());
    EXPECT_NE(result.error().find("Failed to parse field"), std::string::npos);
    EXPECT_NE(result.error().find("a"), std::string::npos);
}

struct Vec3 {
    float x;
    float y;
    float z;
};

template <>
struct anaros::yaml_traits<Vec3> {
    static std::expected<Vec3, std::string> parse(const YAML::Node& n) {
        if (!n["x"] || !n["y"] || !n["z"]) {
            return std::unexpected("Vec3 needs x, y, z");
        }
        return Vec3{n["x"].as<float>(), n["y"].as<float>(), n["z"].as<float>()};
    }
};

struct Transform {
    Vec3 position;
    std::string name;
};

TEST(ParseFromYaml, CustomTrait) {
    YAML::Node yaml = YAML::Load("{position: {x: 1.0, y: 2.0, z: 3.0}, name: 'origin'}");
    auto result = anaros::parse_from_yaml<Transform>(yaml);

    ASSERT_TRUE(result.has_value()) << result.error();
    EXPECT_FLOAT_EQ(result->position.x, 1.0f);
    EXPECT_FLOAT_EQ(result->position.y, 2.0f);
    EXPECT_FLOAT_EQ(result->position.z, 3.0f);
    EXPECT_EQ(result->name, "origin");
}

struct OptionalConfig {
    std::string host;
    std::optional<int> port;
};

TEST(ParseFromYaml, OptionalField_Present) {
    YAML::Node yaml = YAML::Load("{host: 'localhost', port: 9000}");
    auto result = anaros::parse_from_yaml<OptionalConfig>(yaml);

    ASSERT_TRUE(result.has_value()) << result.error();
    EXPECT_EQ(result->host, "localhost");
    ASSERT_TRUE(result->port.has_value());
    EXPECT_EQ(*result->port, 9000);
}

TEST(ParseFromYaml, OptionalField_Absent) {
    YAML::Node yaml = YAML::Load("{host: 'localhost'}");
    auto result = anaros::parse_from_yaml<OptionalConfig>(yaml);

    ASSERT_TRUE(result.has_value()) << result.error();
    EXPECT_EQ(result->host, "localhost");
    EXPECT_FALSE(result->port.has_value());
}

inline int compute_default_workers() { return 7; }

struct DefaultedConfig {
    std::string host;
    int port = 8080;
    int workers = compute_default_workers();
};

TEST(ParseFromYaml, DefaultInitializer_AppliedWhenMissing) {
    YAML::Node yaml = YAML::Load("{host: 'localhost'}");
    auto result = anaros::parse_from_yaml<DefaultedConfig>(yaml);

    ASSERT_TRUE(result.has_value()) << result.error();
    EXPECT_EQ(result->host, "localhost");
    EXPECT_EQ(result->port, 8080);
    EXPECT_EQ(result->workers, 7);
}

TEST(ParseFromYaml, DefaultInitializer_OverriddenWhenPresent) {
    YAML::Node yaml = YAML::Load("{host: 'localhost', port: 9000}");
    auto result = anaros::parse_from_yaml<DefaultedConfig>(yaml);

    ASSERT_TRUE(result.has_value()) << result.error();
    EXPECT_EQ(result->host, "localhost");
    EXPECT_EQ(result->port, 9000);
    EXPECT_EQ(result->workers, 7);
}
