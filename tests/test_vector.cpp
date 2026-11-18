#include <catch2/catch_test_macros.hpp>

#include "vector.hpp"

#include <set>
#include <stdexcept>
#include <string>
#include <type_traits>
#include <utility>
#include <vector>

namespace {

// Counts how many objects are alive, so tests can check that every
// constructed element is destroyed exactly once (no leaks, no double frees).
// It has NO noexcept move constructor, so Vector must use its copy path.
struct Tracked {
    static inline int alive = 0;
    int value;

    Tracked(int v = 0) : value(v) { ++alive; }
    Tracked(const Tracked& other) : value(other.value) { ++alive; }
    Tracked& operator=(const Tracked&) = default;
    ~Tracked() { --alive; }
};

// Same idea, but with a noexcept move constructor, so Vector uses its move path.
struct TrackedMovable {
    static inline int alive = 0;
    static inline int copies = 0;
    static inline int moves = 0;
    int value;

    TrackedMovable(int v = 0) : value(v) { ++alive; }
    TrackedMovable(const TrackedMovable& other) : value(other.value) {
        ++alive;
        ++copies;
    }
    TrackedMovable(TrackedMovable&& other) noexcept : value(other.value) {
        ++alive;
        ++moves;
    }
    TrackedMovable& operator=(const TrackedMovable&) = default;
    ~TrackedMovable() { --alive; }
};

// Throws on the Nth copy, to test that a failed construction cleans up.
// It remembers the address of every live object, so destroying the wrong
// object (one that was never constructed) is detected, not just miscounted.
struct ThrowOnCopy {
    static inline std::set<const void*> live;
    static inline int bad_destroys = 0;          // destructor ran on a non-object
    static inline int copies_until_throw = -1;   // -1 = never throw

    static void reset() {
        live.clear();
        bad_destroys = 0;
        copies_until_throw = -1;
    }

    ThrowOnCopy() { live.insert(this); }
    ThrowOnCopy(const ThrowOnCopy&) {
        if (copies_until_throw == 0) {
            throw std::runtime_error("copy failed");
        }
        if (copies_until_throw > 0) {
            --copies_until_throw;
        }
        live.insert(this);
    }
    ~ThrowOnCopy() {
        if (live.erase(this) == 0) {
            ++bad_destroys;
        }
    }
};

}  // namespace

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

TEST_CASE("appending an existing string preserves both values during growth", "[vector][append]") {
    const std::string original = "first element";
    ricc::Vector<std::string> v{original};
    while (v.size() < v.capacity()) {
        v.emplace_back("filler");
    }
    const auto old_size = v.size();

    SECTION("push_back") {
        v.push_back(v.front());
        REQUIRE(v.front() == original);
    }
    SECTION("emplace_back") {
        v.emplace_back(v.front());
        REQUIRE(v.front() == original);
    }
    SECTION("push_back with move") {
        v.push_back(std::move(v.front()));
        // front() is now moved-from (unspecified state), so only back() is checked
    }

    REQUIRE(v.size() == old_size + 1);
    REQUIRE(v.back() == original);
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

// ---------------------------------------------------------------------------
// Object lifetime: every constructed element must be destroyed exactly once
// ---------------------------------------------------------------------------

TEST_CASE("destructor destroys all elements", "[vector][lifetime]") {
    Tracked::alive = 0;
    {
        ricc::Vector<Tracked> v{Tracked{1}, Tracked{2}, Tracked{3}};
        REQUIRE(Tracked::alive == 3);
    }
    REQUIRE(Tracked::alive == 0);
}

TEST_CASE("pop_back destroys the removed element", "[vector][lifetime]") {
    Tracked::alive = 0;
    {
        ricc::Vector<Tracked> v{Tracked{1}, Tracked{2}};
        v.pop_back();
        REQUIRE(Tracked::alive == 1);
    }
    REQUIRE(Tracked::alive == 0);
}

TEST_CASE("reallocation destroys old elements (copy path)", "[vector][lifetime]") {
    Tracked::alive = 0;
    {
        ricc::Vector<Tracked> v;
        for (int i = 0; i < 10; ++i) {
            v.push_back(Tracked{i});
        }
        REQUIRE(Tracked::alive == 10);
        REQUIRE(v[9].value == 9);
    }
    REQUIRE(Tracked::alive == 0);
}

TEST_CASE("reallocation destroys moved-from elements (move path)", "[vector][lifetime]") {
    TrackedMovable::alive = 0;
    {
        ricc::Vector<TrackedMovable> v;
        for (int i = 0; i < 10; ++i) {
            v.push_back(TrackedMovable{i});
        }
        TrackedMovable::moves = 0;
        TrackedMovable::copies = 0;
        v.reserve(v.capacity() + 1);

        REQUIRE(TrackedMovable::moves == 10);
        REQUIRE(TrackedMovable::copies == 0);
        REQUIRE(TrackedMovable::alive == 10);
        REQUIRE(v[9].value == 9);
    }
    REQUIRE(TrackedMovable::alive == 0);
}

TEST_CASE("shrink_to_fit does not leak elements", "[vector][lifetime]") {
    Tracked::alive = 0;
    {
        ricc::Vector<Tracked> v;
        v.reserve(10);
        v.push_back(Tracked{1});
        v.push_back(Tracked{2});
        v.shrink_to_fit();
        REQUIRE(Tracked::alive == 2);
    }
    REQUIRE(Tracked::alive == 0);
}

TEST_CASE("copy and move assignment do not leak elements", "[vector][lifetime]") {
    Tracked::alive = 0;
    {
        ricc::Vector<Tracked> a{Tracked{1}, Tracked{2}};
        ricc::Vector<Tracked> b{Tracked{3}};
        b = a;                       // b's old element must be destroyed
        REQUIRE(Tracked::alive == 4);
        ricc::Vector<Tracked> c;
        c = std::move(a);            // no new objects created
        REQUIRE(Tracked::alive == 4);
    }
    REQUIRE(Tracked::alive == 0);
}

// ---------------------------------------------------------------------------
// Exception safety
// ---------------------------------------------------------------------------

TEST_CASE("initializer list cleans up if a copy throws", "[vector][exceptions]") {
    ThrowOnCopy::reset();
    {
        std::initializer_list<ThrowOnCopy> src{ThrowOnCopy{}, ThrowOnCopy{}, ThrowOnCopy{}};
        const auto before = ThrowOnCopy::live.size();

        ThrowOnCopy::copies_until_throw = 2;  // 3rd copy throws
        REQUIRE_THROWS_AS(ricc::Vector<ThrowOnCopy>(src), std::runtime_error);
        ThrowOnCopy::copies_until_throw = -1;

        // the 2 elements that were successfully copied must have been destroyed,
        // and nothing that was never constructed may be destroyed
        REQUIRE(ThrowOnCopy::bad_destroys == 0);
        REQUIRE(ThrowOnCopy::live.size() == before);
    }
}

TEST_CASE("copy constructor cleans up if a copy throws", "[vector][exceptions]") {
    ThrowOnCopy::reset();
    {
        ricc::Vector<ThrowOnCopy> src;
        src.reserve(3);
        src.emplace_back();
        src.emplace_back();
        src.emplace_back();
        const auto before = ThrowOnCopy::live.size();

        ThrowOnCopy::copies_until_throw = 2;
        REQUIRE_THROWS_AS(ricc::Vector<ThrowOnCopy>(src), std::runtime_error);
        ThrowOnCopy::copies_until_throw = -1;

        REQUIRE(ThrowOnCopy::bad_destroys == 0);
        REQUIRE(ThrowOnCopy::live.size() == before);
    }
}

// ---------------------------------------------------------------------------
// reserve / capacity
// ---------------------------------------------------------------------------

TEST_CASE("initializer list sets capacity", "[vector][capacity]") {
    ricc::Vector<int> v{1, 2, 3};
    REQUIRE(v.capacity() >= v.size());
}

TEST_CASE("reserve grows capacity and keeps elements", "[vector][capacity]") {
    ricc::Vector<int> v{1, 2, 3};
    v.reserve(100);
    REQUIRE(v.capacity() >= 100);
    REQUIRE(v.size() == 3);
    REQUIRE(v[0] == 1);
    REQUIRE(v[2] == 3);
}

TEST_CASE("reserve with a smaller capacity does nothing", "[vector][capacity]") {
    ricc::Vector<int> v;
    v.reserve(10);
    v.push_back(1);
    const auto* before = v.begin();

    REQUIRE_NOTHROW(v.reserve(5));
    REQUIRE_NOTHROW(v.reserve(10));
    REQUIRE(v.capacity() == 10);
    REQUIRE(v.begin() == before);    // no reallocation happened
}

TEST_CASE("shrink_to_fit on an empty vector", "[vector][capacity]") {
    ricc::Vector<int> v;
    v.reserve(10);
    v.shrink_to_fit();
    REQUIRE(v.capacity() == 0);
    REQUIRE(v.empty());
}

TEST_CASE("emplace_back returns a reference to back() when the vector grows", "[vector]") {
    ricc::Vector<std::string> v;
    for (int i = 0; i < 5; ++i) {
        std::string& s = v.emplace_back(40, static_cast<char>('a' + i));  // forces reallocations
        REQUIRE(&s == &v.back());
        REQUIRE(s == std::string(40, static_cast<char>('a' + i)));
    }
}

// ---------------------------------------------------------------------------
// Copies are deep and independent
// ---------------------------------------------------------------------------

TEST_CASE("copy is independent of the original", "[vector]") {
    ricc::Vector<std::string> a{"one", "two"};
    ricc::Vector<std::string> b{a};
    b[0] = "changed";
    REQUIRE(a[0] == "one");
    REQUIRE(b[0] == "changed");
}

TEST_CASE("self copy-assignment keeps the contents", "[vector]") {
    ricc::Vector<int> v{1, 2, 3};
    auto& alias = v;
    v = alias;
    REQUIRE(v.size() == 3);
    REQUIRE(v[2] == 3);
}

// ---------------------------------------------------------------------------
// const-correctness
// ---------------------------------------------------------------------------

TEST_CASE("at() on a const vector returns a const reference", "[vector][const]") {
    const ricc::Vector<int> v{1, 2, 3};
    REQUIRE(v.at(1) == 2);
    REQUIRE(std::is_same_v<decltype(v.at(0)), const int&>);
}

// ---------------------------------------------------------------------------
// Fill constructor: Vector(n, value)
// (note: use parentheses — braces would pick the initializer-list constructor)
// ---------------------------------------------------------------------------

TEST_CASE("fill constructor", "[vector][fill]") {
    ricc::Vector<int> v(5, 7);
    REQUIRE(v.size() == 5);
    REQUIRE(v.capacity() >= 5);
    for (std::size_t i = 0; i < v.size(); ++i) {
        REQUIRE(v[i] == 7);
    }
}

TEST_CASE("fill constructor with zero elements", "[vector][fill]") {
    ricc::Vector<std::string> v(0, "x");
    REQUIRE(v.empty());
}

TEST_CASE("fill constructor cleans up if a copy throws", "[vector][fill][exceptions]") {
    ThrowOnCopy::reset();
    {
        ThrowOnCopy proto;
        const auto before = ThrowOnCopy::live.size();

        ThrowOnCopy::copies_until_throw = 3;  // 4th copy throws
        REQUIRE_THROWS_AS(ricc::Vector<ThrowOnCopy>(5, proto), std::runtime_error);
        ThrowOnCopy::copies_until_throw = -1;

        REQUIRE(ThrowOnCopy::bad_destroys == 0);
        REQUIRE(ThrowOnCopy::live.size() == before);
    }
}
