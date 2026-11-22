// Tests for the parts of ricc::String the compiler project needs in order to
// replace std::string in token / lexxer / ast / parser / Environment / object.
//
// Every test here also passes with std::string in place of ricc::String, so a
// failure means ricc::String behaves differently from the standard one.
//
// Sections:
//   1. Construction from C strings          String(const char*), String(const char*, size_t)
//   2. size / empty / data / c_str
//   3. operator==  (and the != C++20 generates from it)
//   4. substr
//   5. operator+= and operator+
//   6. std::hash<ricc::String> and unordered_map keys
//   7. noexcept moves and ricc::Vector<String>
//   8. Copies, assignment from literals, and compiler-shaped usage

#include <catch2/catch_test_macros.hpp>

#include "string.hpp"
#include "vector.hpp"

#include <charconv>
#include <cstddef>
#include <cstring>
#include <functional>
#include <sstream>
#include <stdexcept>
#include <string>
#include <string_view>
#include <system_error>
#include <type_traits>
#include <unordered_map>
#include <utility>
#include <vector>

namespace {

// 23 characters fit inline (short string); 24 or more go to the heap.
constexpr const char* exactly_23 = "abcdefghijklmnopqrstuvw";
constexpr const char* exactly_24 = "abcdefghijklmnopqrstuvwx";
constexpr const char* long_text  = "this string is definitely longer than twenty-three characters";

std::string_view view(const ricc::String& s) {
    return std::string_view{s.data(), s.size()};
}

std::string printed(const ricc::String& s) {
    std::ostringstream out;
    out << s;
    return out.str();
}

// Checks size, contents via data(), the null terminator via c_str(), and
// operator<< all agree with `expected`.
void require_equals(const ricc::String& s, std::string_view expected) {
    REQUIRE(s.size() == expected.size());
    REQUIRE(s.empty() == expected.empty());
    REQUIRE(view(s) == expected);
    REQUIRE(std::strlen(s.c_str()) == expected.size());
    REQUIRE(s.c_str()[expected.size()] == '\0');
    REQUIRE(printed(s) == expected);
}

// Shaped like the compiler's Token.
enum class Kind { IDENTIFIER, INT, ENDOFFILE };
struct Token {
    Kind kind;
    ricc::String text;
    Token(Kind kind, ricc::String text) : kind(kind), text(std::move(text)) {}
};

// Shaped like lexxer::readIdentifier: scans [a-z]+ starting at pos.
ricc::String read_word(const ricc::String& input, std::size_t& pos) {
    const std::size_t start = pos;
    while (pos < input.size() && input[pos] >= 'a' && input[pos] <= 'z') {
        ++pos;
    }
    return input.substr(start, pos - start);
}

}  // namespace

// ---------------------------------------------------------------------------
// 1. Construction from C strings
// ---------------------------------------------------------------------------

TEST_CASE("String from an empty literal is empty", "[string-compiler][ctor]") {
    ricc::String s{""};
    require_equals(s, "");
}

TEST_CASE("String from a short literal", "[string-compiler][ctor]") {
    ricc::String s{"int"};
    require_equals(s, "int");
    REQUIRE(s[0] == 'i');
    REQUIRE(s[2] == 't');
}

TEST_CASE("String from a 23 character literal (last size that fits inline)", "[string-compiler][ctor][sso]") {
    ricc::String s{exactly_23};
    require_equals(s, exactly_23);
}

TEST_CASE("String from a 24 character literal (first size on the heap)", "[string-compiler][ctor][sso]") {
    ricc::String s{exactly_24};
    require_equals(s, exactly_24);
}

TEST_CASE("String from a long literal", "[string-compiler][ctor]") {
    ricc::String s{long_text};
    require_equals(s, long_text);
}

TEST_CASE("String converts implicitly from a literal", "[string-compiler][ctor]") {
    // The compiler relies on this: `return "null";`, `std::string input = "";`,
    // and the keyword table `{"int", TokenType::INT}`.
    STATIC_REQUIRE(std::is_convertible_v<const char*, ricc::String>);

    ricc::String s = "return";
    require_equals(s, "return");

    auto make = []() -> ricc::String { return "null"; };
    require_equals(make(), "null");
}

TEST_CASE("String(const char*, size_t) copies exactly count characters", "[string-compiler][ctor]") {
    // Used at the boundary: tests hold a std::string and hand it to the lexer
    // as ricc::String{source.data(), source.size()}.
    const std::string source = "int x = 42;";
    ricc::String s{source.data(), source.size()};
    require_equals(s, source);
}

TEST_CASE("String(const char*, size_t) can take a prefix", "[string-compiler][ctor]") {
    ricc::String s{"identifier", 5};
    require_equals(s, "ident");
}

TEST_CASE("String(const char*, size_t) with count 0 is empty", "[string-compiler][ctor]") {
    ricc::String s{"anything", 0};
    require_equals(s, "");
}

TEST_CASE("String(const char*, size_t) builds a long string", "[string-compiler][ctor][sso]") {
    const std::string source(100, 'z');
    ricc::String s{source.data(), source.size()};
    require_equals(s, source);
}

// ---------------------------------------------------------------------------
// 2. size / empty / data / c_str
// ---------------------------------------------------------------------------

TEST_CASE("default String is empty and has a valid c_str", "[string-compiler][observers]") {
    ricc::String s;
    require_equals(s, "");
    REQUIRE(s.c_str() != nullptr);
}

TEST_CASE("size, empty, data and c_str work on a const String", "[string-compiler][observers]") {
    const ricc::String s{"bool"};
    STATIC_REQUIRE(std::is_same_v<decltype(s.size()), std::size_t>);
    STATIC_REQUIRE(std::is_same_v<decltype(s.empty()), bool>);
    STATIC_REQUIRE(std::is_same_v<decltype(s.c_str()), const char*>);
    require_equals(s, "bool");
}

TEST_CASE("data and c_str point at the same characters", "[string-compiler][observers]") {
    ricc::String small{"x"};
    ricc::String large{long_text};
    REQUIRE(small.data() == small.c_str());
    REQUIRE(large.data() == large.c_str());
}

TEST_CASE("data() and size() work with std::from_chars like the parser", "[string-compiler][observers]") {
    // parser.hpp: const char* begin = curr_tok.text.data();
    //             const char* end   = begin + curr_tok.text.size();
    ricc::String text{"12345"};
    const char* begin = text.data();
    const char* end = begin + text.size();
    int value = 0;
    auto [ptr, ec] = std::from_chars(begin, end, value);
    REQUIRE(ec == std::errc{});
    REQUIRE(ptr == end);
    REQUIRE(value == 12345);
}

TEST_CASE("c_str can be used to build a std::runtime_error", "[string-compiler][observers]") {
    ricc::String message{"Integer overflow"};
    std::runtime_error error{message.c_str()};
    REQUIRE(std::string_view{error.what()} == "Integer overflow");
}

// ---------------------------------------------------------------------------
// 3. operator==
// ---------------------------------------------------------------------------

TEST_CASE("equal Strings compare equal", "[string-compiler][compare]") {
    ricc::String a{"else"};
    ricc::String b{"else"};
    REQUIRE(a == b);
    REQUIRE_FALSE(a != b);
}

TEST_CASE("Strings with different contents are not equal", "[string-compiler][compare]") {
    REQUIRE_FALSE(ricc::String{"true"} == ricc::String{"tru3"});
    REQUIRE(ricc::String{"true"} != ricc::String{"tru3"});
}

TEST_CASE("a prefix is not equal to the longer String", "[string-compiler][compare]") {
    // Comparing only min(size) characters is the classic bug here.
    REQUIRE_FALSE(ricc::String{"if"} == ricc::String{"iff"});
    REQUIRE_FALSE(ricc::String{"iff"} == ricc::String{"if"});
    REQUIRE_FALSE(ricc::String{""} == ricc::String{"a"});
}

TEST_CASE("empty Strings compare equal", "[string-compiler][compare]") {
    REQUIRE(ricc::String{} == ricc::String{""});
    REQUIRE(ricc::String{"abc", 0} == ricc::String{});
}

TEST_CASE("long Strings compare by contents, not by pointer", "[string-compiler][compare][sso]") {
    ricc::String a{long_text};
    ricc::String b{long_text};
    REQUIRE(a.data() != b.data());
    REQUIRE(a == b);

    ricc::String c{exactly_24};
    ricc::String d{"abcdefghijklmnopqrstuvwX"};  // differs only in the last char
    REQUIRE_FALSE(c == d);
}

TEST_CASE("String compares against a literal on either side", "[string-compiler][compare]") {
    ricc::String s{"double"};
    REQUIRE(s == "double");
    REQUIRE("double" == s);
    REQUIRE(s != "doubles");
}

TEST_CASE("operator== works on const Strings", "[string-compiler][compare]") {
    const ricc::String a{"x"};
    const ricc::String b{"x"};
    REQUIRE(a == b);
}

// ---------------------------------------------------------------------------
// 4. substr
// ---------------------------------------------------------------------------

TEST_CASE("substr from the middle", "[string-compiler][substr]") {
    const ricc::String s{"int x = 42;"};
    require_equals(s.substr(4, 1), "x");
    require_equals(s.substr(8, 2), "42");
}

TEST_CASE("substr from the start and to the end", "[string-compiler][substr]") {
    const ricc::String s{"return"};
    require_equals(s.substr(0, 3), "ret");
    require_equals(s.substr(3, 3), "urn");
    require_equals(s.substr(0, 6), "return");
}

TEST_CASE("substr clamps a count that runs past the end", "[string-compiler][substr]") {
    const ricc::String s{"abc"};
    require_equals(s.substr(1, 100), "bc");
}

TEST_CASE("substr with only a position takes the rest", "[string-compiler][substr]") {
    const ricc::String s{"hello world"};
    require_equals(s.substr(6), "world");
    require_equals(s.substr(0), "hello world");
}

TEST_CASE("substr at size() is empty, past size() throws", "[string-compiler][substr]") {
    const ricc::String s{"abc"};
    require_equals(s.substr(3, 5), "");
    REQUIRE_THROWS_AS(s.substr(4, 1), std::out_of_range);
}

TEST_CASE("substr with count 0 is empty", "[string-compiler][substr]") {
    const ricc::String s{"abc"};
    require_equals(s.substr(1, 0), "");
}

TEST_CASE("substr of a long String can be short or long", "[string-compiler][substr][sso]") {
    const ricc::String s{long_text};
    const std::string_view expected{long_text};
    require_equals(s.substr(5, 6), expected.substr(5, 6));
    require_equals(s.substr(2, 40), expected.substr(2, 40));
}

TEST_CASE("substr leaves the original unchanged", "[string-compiler][substr]") {
    const ricc::String s{exactly_24};
    auto part = s.substr(0, 4);
    part = "zzzz";
    require_equals(s, exactly_24);
}

// ---------------------------------------------------------------------------
// 5. operator+= and operator+
// ---------------------------------------------------------------------------

TEST_CASE("operator+= appends another String", "[string-compiler][append]") {
    ricc::String s{"foo"};
    s += ricc::String{"bar"};
    require_equals(s, "foobar");
}

TEST_CASE("operator+= appends a literal", "[string-compiler][append]") {
    ricc::String s{"foo"};
    s += "bar";
    require_equals(s, "foobar");
}

TEST_CASE("operator+= returns the String itself", "[string-compiler][append]") {
    ricc::String s{"a"};
    ricc::String& result = (s += "b");
    REQUIRE(&result == &s);
    (s += "c") += "d";
    require_equals(s, "abcd");
}

TEST_CASE("operator+= an empty String changes nothing", "[string-compiler][append]") {
    ricc::String s{"abc"};
    s += ricc::String{};
    s += "";
    require_equals(s, "abc");
}

TEST_CASE("operator+= crosses from inline to heap", "[string-compiler][append][sso]") {
    ricc::String s{"twenty characters!!!"};  // 20
    s += "1234567890";                       // 30
    require_equals(s, "twenty characters!!!1234567890");
}

TEST_CASE("operator+= grows a heap String several times", "[string-compiler][append][sso]") {
    ricc::String s{long_text};
    std::string expected{long_text};
    for (int i = 0; i < 10; ++i) {
        s += long_text;
        expected += long_text;
    }
    require_equals(s, expected);
}

TEST_CASE("operator+= with itself", "[string-compiler][append]") {
    // Aliasing trap: if the buffer is reallocated before copying from `other`,
    // `other` (which is *this) points at freed memory.
    ricc::String small{"abc"};
    small += small;
    require_equals(small, "abcabc");

    ricc::String large{exactly_24};
    large += large;
    require_equals(large, std::string{exactly_24} + exactly_24);
}

TEST_CASE("operator+ joins two Strings and leaves them unchanged", "[string-compiler][append]") {
    // parser.hpp: Token literal{TokenType::INTEGER, op.text + curr_tok.text};
    const ricc::String op{"-"};
    const ricc::String number{"42"};
    ricc::String joined = op + number;
    require_equals(joined, "-42");
    require_equals(op, "-");
    require_equals(number, "42");
}

TEST_CASE("operator+ with a literal on the right", "[string-compiler][append]") {
    const ricc::String name{"x"};
    require_equals(name + " is undefined", "x is undefined");
}

TEST_CASE("operator+ producing a long String", "[string-compiler][append][sso]") {
    const ricc::String a{exactly_23};
    const ricc::String b{exactly_23};
    require_equals(a + b, std::string{exactly_23} + exactly_23);
}

// ---------------------------------------------------------------------------
// 6. std::hash<ricc::String> and unordered_map keys
// ---------------------------------------------------------------------------

TEST_CASE("std::hash<ricc::String> exists and returns size_t", "[string-compiler][hash]") {
    STATIC_REQUIRE(std::is_default_constructible_v<std::hash<ricc::String>>);
    STATIC_REQUIRE(std::is_same_v<std::invoke_result_t<std::hash<ricc::String>, const ricc::String&>,
                                  std::size_t>);
}

TEST_CASE("equal Strings have equal hashes", "[string-compiler][hash]") {
    std::hash<ricc::String> hash;
    REQUIRE(hash(ricc::String{"int"}) == hash(ricc::String{"int"}));
    REQUIRE(hash(ricc::String{long_text}) == hash(ricc::String{long_text}));

    // Same contents built two different ways.
    const ricc::String whole{"int x"};
    REQUIRE(hash(whole.substr(0, 3)) == hash(ricc::String{"int"}));
}

TEST_CASE("hash only looks at the characters, not the spare buffer", "[string-compiler][hash]") {
    // A String shrunk out of a bigger buffer (substr) must hash like a fresh one.
    const ricc::String big{"abcXXXXXXXXXXXXXXXXXXXXXXXXXXXXX"};
    std::hash<ricc::String> hash;
    REQUIRE(hash(big.substr(0, 3)) == hash(ricc::String{"abc"}));
}

TEST_CASE("String works as an unordered_map key like the keyword table", "[string-compiler][hash]") {
    // lexxer.hpp: static const std::unordered_map<std::string, TokenType> keywords{...}
    const std::unordered_map<ricc::String, Kind> keywords{
        {"int", Kind::INT},
        {"return", Kind::IDENTIFIER},
    };
    REQUIRE(keywords.size() == 2);

    auto it = keywords.find(ricc::String{"int"});
    REQUIRE(it != keywords.end());
    REQUIRE(it->second == Kind::INT);

    REQUIRE(keywords.find(ricc::String{"in"}) == keywords.end());
    REQUIRE(keywords.find(ricc::String{"integer"}) == keywords.end());
}

TEST_CASE("unordered_map<String, T> supports define / get / set like Environment", "[string-compiler][hash]") {
    std::unordered_map<ricc::String, int> bindings;
    bindings.emplace(ricc::String{"x"}, 1);
    bindings.emplace(ricc::String{long_text}, 2);

    REQUIRE(bindings.contains(ricc::String{"x"}));
    REQUIRE(bindings.at(ricc::String{long_text}) == 2);

    bindings[ricc::String{"x"}] = 10;
    REQUIRE(bindings.at(ricc::String{"x"}) == 10);
    REQUIRE(bindings.size() == 2);
}

// ---------------------------------------------------------------------------
// 7. noexcept moves and ricc::Vector<String>
// ---------------------------------------------------------------------------

TEST_CASE("String moves are noexcept", "[string-compiler][move]") {
    // Without this, ricc::Vector copies every String when it grows.
    STATIC_REQUIRE(std::is_nothrow_move_constructible_v<ricc::String>);
    STATIC_REQUIRE(std::is_nothrow_move_assignable_v<ricc::String>);
}

TEST_CASE("ricc::Vector<String> keeps contents through growth", "[string-compiler][move]") {
    ricc::Vector<ricc::String> words;
    std::vector<std::string> expected;
    for (int i = 0; i < 40; ++i) {
        std::string text = (i % 2 == 0) ? std::string(5, char('a' + i % 26))
                                        : std::string(30, char('a' + i % 26));
        words.push_back(ricc::String{text.data(), text.size()});
        expected.push_back(text);
    }
    REQUIRE(words.size() == expected.size());
    for (std::size_t i = 0; i < expected.size(); ++i) {
        CAPTURE(i);
        require_equals(words[i], expected[i]);
    }
}

// ---------------------------------------------------------------------------
// 8. Copies, assignment from literals, compiler-shaped usage
// ---------------------------------------------------------------------------

TEST_CASE("copies are independent", "[string-compiler][copy]") {
    ricc::String original{long_text};
    ricc::String copy = original;
    copy += "!";
    require_equals(original, long_text);
    require_equals(copy, std::string{long_text} + "!");
}

TEST_CASE("assigning a literal replaces the contents", "[string-compiler][copy]") {
    ricc::String s{long_text};
    s = "short";
    require_equals(s, "short");
    s = exactly_24;
    require_equals(s, exactly_24);
    s = "";
    require_equals(s, "");
}

TEST_CASE("Token(kind, {}) builds an empty text", "[string-compiler][usage]") {
    // lexxer.hpp: return Token(TokenType::ENDOFFILE, {});
    Token token(Kind::ENDOFFILE, {});
    require_equals(token.text, "");
}

TEST_CASE("Token text survives copying the Token", "[string-compiler][usage]") {
    Token a(Kind::IDENTIFIER, "variable_name_that_is_long");
    Token b = a;
    require_equals(b.text, "variable_name_that_is_long");
    require_equals(a.text, "variable_name_that_is_long");
}

TEST_CASE("a lexer-style scan with operator[] and substr finds keywords", "[string-compiler][usage]") {
    const std::unordered_map<ricc::String, Kind> keywords{{"int", Kind::INT}};
    const ricc::String input{"int value"};

    std::size_t pos = 0;
    ricc::String first = read_word(input, pos);
    require_equals(first, "int");
    REQUIRE(keywords.find(first) != keywords.end());

    ++pos;  // skip the space
    ricc::String second = read_word(input, pos);
    require_equals(second, "value");
    REQUIRE(keywords.find(second) == keywords.end());
    REQUIRE(pos == input.size());
}
