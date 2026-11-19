#include <catch2/catch_message.hpp>
#include <catch2/catch_test_macros.hpp>

#include "string.hpp"

#include <algorithm>
#include <cstddef>
#include <span>
#include <sstream>
#include <stdexcept>
#include <string>
#include <string_view>
#include <utility>

namespace {

constexpr std::size_t inline_capacity = 23;
constexpr std::size_t first_heap_capacity = 46;

void require_contents(const ricc::String& value, std::string_view expected) {
    REQUIRE_THROWS_AS(value[expected.size()], std::out_of_range);
    std::ostringstream output;
    output << value;
    REQUIRE(output.str() == expected);
}

void build_prefix(ricc::String& value, std::string_view prefix) {
    for (std::size_t i = 0; i < prefix.size(); ++i) {
        CAPTURE(i);
        value.pushBack(prefix[i]);
        // Stop setup if an append already produced an incorrect length.
        REQUIRE_THROWS_AS(std::as_const(value)[i + 1], std::out_of_range);
    }
    require_contents(value, prefix);
}

}  // namespace

TEST_CASE("first String pushBack adds exactly one character", "[string]") {
    ricc::String value;
    value.pushBack('x');
    require_contents(value, "x");
}

TEST_CASE("String pushBack keeps characters inline through length 23", "[string][sso]") {
    ricc::String value;
    const auto object_bytes = std::as_bytes(std::span{&value, 1});

    for (std::size_t length = 1; length <= inline_capacity; ++length) {
        CAPTURE(length);
        value.pushBack('x');

        const void* first_character = &value.front();
        REQUIRE(std::ranges::any_of(object_bytes, [first_character](const std::byte& byte) {
            return static_cast<const void*>(&byte) == first_character;
        }));
    }
}

TEST_CASE("String pushBack keeps the character that crosses inline capacity", "[string][sso]") {
    constexpr std::string_view prefix = "abcdefghijklmnopqrstuvw";
    static_assert(prefix.size() == inline_capacity);
    ricc::String value;
    build_prefix(value, prefix);

    value.pushBack('!');

    require_contents(value, std::string(prefix) + '!');
}

TEST_CASE("String pushBack can grow past the first heap allocation", "[string]") {
    std::string expected(first_heap_capacity, 'a');
    ricc::String value;
    build_prefix(value, expected);

    for (char ch : {'!', '?'}) {
        value.pushBack(ch);
        expected.push_back(ch);
        require_contents(value, expected);
    }
}

TEST_CASE("String accepts pushBack after its heap buffer has been moved", "[string]") {
    const std::string prefix(inline_capacity + 1, 'a');
    ricc::String source;
    build_prefix(source, prefix);
    ricc::String destination(std::move(source));
    REQUIRE_THROWS_AS(std::as_const(source)[0], std::out_of_range);

    source.pushBack('x');

    require_contents(source, "x");
    require_contents(destination, prefix);
}

TEST_CASE("String pushBack grows repeatedly", "[string]") {
    std::string expected;
    ricc::String value;
    for (int i = 0; i < 500; ++i) {
        const char ch = static_cast<char>('a' + i % 26);
        value.pushBack(ch);
        expected.push_back(ch);
    }
    require_contents(value, expected);
}

TEST_CASE("moved-from String can grow again", "[string]") {
    ricc::String source;
    build_prefix(source, std::string(inline_capacity + 1, 'a'));
    ricc::String destination(std::move(source));

    std::string expected;
    for (int i = 0; i < 100; ++i) {
        source.pushBack('x');
        expected.push_back('x');
    }
    require_contents(source, expected);
}
