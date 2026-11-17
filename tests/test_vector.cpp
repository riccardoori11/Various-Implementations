#include <catch2/catch_test_macros.hpp>

#include "vector.hpp"

#include <stdexcept>
#include <string>
#include <utility>
#include <vector>

TEST_CASE("default-constructed Vector is empty", "[vector]") {
    ricc::Vector<int> v;
    REQUIRE(v.empty());
    REQUIRE(v.size() == 0);
    REQUIRE(v.capacity() == 0);
}


TEST_CASE("initializer list with front and back", "[vector]") {
    ricc::Vector<int> v{1, 23, 4, 44444};
    REQUIRE(v.size() == 4);
    REQUIRE(v.front() == 1);
    REQUIRE(v.back() == 44444);
}

TEST_CASE("push_back keeps elements across reallocations", "[vector]") {
    ricc::Vector<int> v;
    std::vector<int> ref;
    for (int i = 0; i < 100; ++i) {
        v.push_back(i);
        ref.push_back(i);
    }
    REQUIRE(v.size() == ref.size());
    for (std::size_t i = 0; i < ref.size(); ++i) {
        REQUIRE(v[i] == ref[i]);
    }
}

TEST_CASE("push_back works with non-trivial types", "[vector]") {
    ricc::Vector<std::string> v;
    for (int i = 0; i < 20; ++i) {
        v.push_back(std::string(30, static_cast<char>('a' + i)));
    }
    REQUIRE(v.size() == 20);
    REQUIRE(v[0] == std::string(30, 'a'));
    REQUIRE(v[19] == std::string(30, 't'));
}

TEST_CASE("copy constructor and copy assignment", "[vector]") {
    ricc::Vector<int> a{1, 2, 3};

    ricc::Vector<int> b{a};
    REQUIRE(b.size() == 3);
    REQUIRE(b[2] == 3);

    ricc::Vector<int> c;
    c = a;
    REQUIRE(c.size() == 3);
    REQUIRE(c[0] == 1);
}

TEST_CASE("move constructor and move assignment", "[vector]") {
    ricc::Vector<int> a{1, 2, 3};

    ricc::Vector<int> b{std::move(a)};
    REQUIRE(b.size() == 3);
    REQUIRE(a.size() == 0);
    REQUIRE(a.capacity() == 0);

    ricc::Vector<int> c;
    c = std::move(b);
    REQUIRE(c.size() == 3);
    REQUIRE(c[1] == 2);
}

TEST_CASE("pop_back removes the last element", "[vector]") {
    ricc::Vector<int> v{1, 2};
    v.pop_back();
    REQUIRE(v.size() == 1);
    REQUIRE(v.back() == 1);
    v.pop_back();
    REQUIRE(v.empty());
}

TEST_CASE("at() throws on out-of-range index", "[vector]") {
    ricc::Vector<int> v{1, 2, 3};
    REQUIRE(v.at(2) == 3);
    REQUIRE_THROWS_AS(v.at(3), std::out_of_range);
}

TEST_CASE("emplace_back returns a reference to the new element", "[vector]") {
    ricc::Vector<std::string> v;
    v.reserve(4);
    std::string& s = v.emplace_back(5, 'x');
    REQUIRE(&s == &v.back());
    REQUIRE(s == "xxxxx");
}

TEST_CASE("shrink_to_fit reduces capacity to size", "[vector]") {
    ricc::Vector<int> v;
    v.reserve(10);
    v.push_back(1);
    v.shrink_to_fit();
    REQUIRE(v.capacity() == 1);
    REQUIRE(v[0] == 1);
}
