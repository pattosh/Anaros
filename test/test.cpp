#include "anaros.h"

#include <gtest/gtest.h>
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
