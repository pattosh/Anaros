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

    static YAML::Node dump(const Vec3& v) {
        YAML::Node n(YAML::NodeType::Map);
        n["x"] = v.x;
        n["y"] = v.y;
        n["z"] = v.z;
        return n;
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

TEST(SerializeToYaml, HappyPath) {
    TestStruct value{42, "Hello"};
    YAML::Node node = anaros::to_yaml(value);

    EXPECT_EQ(node["a"].as<int>(), 42);
    EXPECT_EQ(node["b"].as<std::string>(), "Hello");
}

TEST(SerializeToYaml, CustomTrait) {
    Transform value{Vec3{1.0f, 2.0f, 3.0f}, "origin"};
    YAML::Node node = anaros::to_yaml(value);

    ASSERT_TRUE(node["position"]);
    EXPECT_FLOAT_EQ(node["position"]["x"].as<float>(), 1.0f);
    EXPECT_FLOAT_EQ(node["position"]["y"].as<float>(), 2.0f);
    EXPECT_FLOAT_EQ(node["position"]["z"].as<float>(), 3.0f);
    EXPECT_EQ(node["name"].as<std::string>(), "origin");
}

TEST(SerializeToYaml, OptionalField_Present) {
    OptionalConfig value{"localhost", 9000};
    YAML::Node node = anaros::to_yaml(value);

    EXPECT_EQ(node["host"].as<std::string>(), "localhost");
    ASSERT_TRUE(node["port"]);
    EXPECT_EQ(node["port"].as<int>(), 9000);
}

TEST(SerializeToYaml, OptionalField_Absent) {
    OptionalConfig value{"localhost", std::nullopt};
    YAML::Node node = anaros::to_yaml(value);

    EXPECT_EQ(node["host"].as<std::string>(), "localhost");
    EXPECT_FALSE(node["port"]);
}

TEST(RoundTrip, CustomTrait) {
    Transform original{Vec3{1.5f, -2.0f, 3.25f}, "node"};
    YAML::Node node = anaros::to_yaml(original);
    auto result = anaros::parse_from_yaml<Transform>(node);

    ASSERT_TRUE(result.has_value()) << result.error();
    EXPECT_FLOAT_EQ(result->position.x, original.position.x);
    EXPECT_FLOAT_EQ(result->position.y, original.position.y);
    EXPECT_FLOAT_EQ(result->position.z, original.position.z);
    EXPECT_EQ(result->name, original.name);
}

// A type with no default constructor: only constructible via its user-provided
// constructor. yaml_traits::parse returns by value, so it never needs to
// default-construct then mutate.
struct Celsius {
    explicit Celsius(double d) : degrees(d) {}
    double degrees;
};

template <>
struct anaros::yaml_traits<Celsius> {
    static std::expected<Celsius, std::string> parse(const YAML::Node& n) {
        if (!n.IsScalar()) {
            return std::unexpected("Celsius needs a scalar");
        }
        return Celsius{n.as<double>()};
    }

    static YAML::Node dump(const Celsius& c) { return YAML::Node(c.degrees); }
};

TEST(RoundTrip, CustomConstructorNoDefault) {
    Celsius original{21.5};
    YAML::Node node = anaros::to_yaml(original);
    EXPECT_DOUBLE_EQ(node.as<double>(), 21.5);

    auto result = anaros::parse_from_yaml<Celsius>(node);
    ASSERT_TRUE(result.has_value()) << result.error();
    EXPECT_DOUBLE_EQ(result->degrees, 21.5);
}
