#include "anaros.h"

#include <iostream>
#include <yaml-cpp/yaml.h>

struct TestStruct {
    int a;
    std::string b;
};

int main() {
    YAML::Node yaml_dict = YAML::Load("{a: 42, b: 'Hello'}");

    auto result = anaros::parse_from_yaml<TestStruct>(yaml_dict);

    if (result) {
        std::cout << "Parsed successfully: a = " << result->a << ", b = " << result->b << std::endl;
    } else {
        std::cerr << "Error: " << result.error() << std::endl;
    }
}