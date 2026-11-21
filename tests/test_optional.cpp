#include <catch2/catch_test_macros.hpp>

#include "optional.hpp"
#include "unique_ptr.hpp"
#include "vector.hpp"

#include <string>
#include <type_traits>
#include <utility>

// These tests describe what ricc::optional needs so the compiler project
// (~/compiler) can use it instead of std::optional. The compiler uses it for:
//
//   * parser results that may fail:
//       std::optional<Vector<unique_ptr<Expression>>> parseCallExpressionArguments()
//       { ... return std::nullopt; ... return std::optional<...>(std::move(args)); }
//   * environment lookups:
//       std::optional<Binding> Get(name) const { ... return it->second; ... return std::nullopt; }
//   * values that may be uninitialised:  struct Binding { TokenType type; std::optional<Value> value; };
//
// Needed in include/optional.hpp, namespace ricc:
//   optional<T>, nullopt_t, and a constant `nullopt`.
//
// Like the unique_ptr tests: this won't compile until the features exist.
// Work section by section and read only the FIRST compiler error.

namespace {

// Counts constructions/destructions, so tests can prove that an empty optional
// never constructs a T and that every T is destroyed exactly once.
struct Tracked {
    static inline int alive = 0;
    static inline int copies = 0;
    static inline int moves = 0;
    int value;

    explicit Tracked(int v) : value(v) { ++alive; }
    Tracked(const Tracked& other) : value(other.value) { ++alive; ++copies; }
    Tracked(Tracked&& other) noexcept : value(other.value) { ++alive; ++moves; }
    Tracked& operator=(const Tracked& other) { value = other.value; ++copies; return *this; }
    Tracked& operator=(Tracked&& other) noexcept { value = other.value; ++moves; return *this; }
    ~Tracked() { --alive; }
};

void reset_tracked() {
    Tracked::alive = 0;
    Tracked::copies = 0;
    Tracked::moves = 0;
}

// No default constructor: an empty optional<NoDefault> must still be possible,
// so optional cannot simply hold a `T value;` member.
struct NoDefault {
    int x;
    explicit NoDefault(int v) : x(v) {}
    NoDefault() = delete;
};

// Like the compiler's Binding: { TokenType type; std::optional<Value> value; }
struct Binding {
    int type;
    ricc::optional<int> value;
};

// Like Environment::Get: returns a copy of the found value, or nullopt.
ricc::optional<Binding> lookup(const Binding& stored, bool found) {
    if (!found) {
        return ricc::nullopt;
    }
    return stored;            // implicit conversion from const T&
}

// Like parseCallExpressionArguments: fails with nullopt, succeeds by moving a Vector in.
ricc::optional<ricc::Vector<ricc::unique_ptr<int>>> parse_args(bool ok) {
    if (!ok) {
        return ricc::nullopt;
    }
    ricc::Vector<ricc::unique_ptr<int>> args;
    args.push_back(ricc::make_unique<int>(1));
    args.push_back(ricc::make_unique<int>(2));
    return ricc::optional<ricc::Vector<ricc::unique_ptr<int>>>(std::move(args));
}

} // namespace

// ---------------------------------------------------------------------------
// 1. Empty and engaged
// ---------------------------------------------------------------------------

TEST_CASE("default optional is empty", "[optional]") {
    ricc::optional<int> o;
    REQUIRE_FALSE(o.has_value());
    REQUIRE_FALSE(o);
}

TEST_CASE("optional constructed from nullopt is empty", "[optional]") {
    ricc::optional<int> a(ricc::nullopt);
    ricc::optional<int> b = ricc::nullopt;
    REQUIRE_FALSE(a.has_value());
    REQUIRE_FALSE(b.has_value());
}

TEST_CASE("optional constructed from a value holds it", "[optional]") {
    ricc::optional<int> o(42);
    REQUIRE(o.has_value());
    REQUIRE(o);
    REQUIRE(*o == 42);
}

TEST_CASE("an empty optional never constructs a T", "[optional][lifetime]") {
    reset_tracked();
    {
        ricc::optional<Tracked> o;
        REQUIRE(Tracked::alive == 0);
    }
    REQUIRE(Tracked::alive == 0);
}

TEST_CASE("optional works with a type that has no default constructor", "[optional]") {
    ricc::optional<NoDefault> empty;
    REQUIRE_FALSE(empty);

    ricc::optional<NoDefault> full(NoDefault(7));
    REQUIRE(full->x == 7);
}

TEST_CASE("the held value is destroyed exactly once", "[optional][lifetime]") {
    reset_tracked();
    {
        ricc::optional<Tracked> o(Tracked(1));
        REQUIRE(Tracked::alive == 1);   // the temporary is gone, only the copy inside remains
    }
    REQUIRE(Tracked::alive == 0);
}

// ---------------------------------------------------------------------------
// 2. Access: *, ->, value()
// ---------------------------------------------------------------------------

TEST_CASE("operator* and operator-> reach the value", "[optional][access]") {
    ricc::optional<std::string> o(std::string("abc"));
    REQUIRE(*o == "abc");
    REQUIRE(o->size() == 3);

    o->push_back('d');           // non-const access can modify
    REQUIRE(*o == "abcd");
}

TEST_CASE("const optional gives const access", "[optional][access][traits]") {
    // Unlike unique_ptr, optional CONTAINS its value — constness is deep, like Vector.
    const ricc::optional<std::string> o(std::string("abc"));
    REQUIRE(*o == "abc");
    REQUIRE(o->size() == 3);
    STATIC_REQUIRE(std::is_same_v<decltype(*o), const std::string&>);
    STATIC_REQUIRE(std::is_same_v<decltype(o.operator->()), const std::string*>);
}

TEST_CASE("value() returns the value when engaged", "[optional][access]") {
    ricc::optional<int> o(5);
    REQUIRE(o.value() == 5);
    o.value() = 6;
    REQUIRE(*o == 6);

    const ricc::optional<int> c(9);
    REQUIRE(c.value() == 9);
}

TEST_CASE("value() throws when empty", "[optional][access]") {
    // Throw std::bad_optional_access or your own type derived from std::exception.
    ricc::optional<int> o;
    REQUIRE_THROWS_AS(o.value(), std::exception);

    const ricc::optional<int> c;
    REQUIRE_THROWS_AS(c.value(), std::exception);
}

// ---------------------------------------------------------------------------
// 3. explicit operator bool
// ---------------------------------------------------------------------------

TEST_CASE("operator bool is explicit", "[optional][bool][traits]") {
    STATIC_REQUIRE(std::is_constructible_v<bool, ricc::optional<int>>);
    STATIC_REQUIRE_FALSE(std::is_convertible_v<ricc::optional<int>, bool>);
}

TEST_CASE("operator bool works on a const optional", "[optional][bool]") {
    const ricc::optional<int> empty;
    const ricc::optional<int> full(1);
    REQUIRE_FALSE(empty);
    REQUIRE(full);
    REQUIRE(!empty);
}

// ---------------------------------------------------------------------------
// 4. Copy and move
// ---------------------------------------------------------------------------

TEST_CASE("copying an engaged optional copies the value", "[optional][copy]") {
    reset_tracked();
    {
        ricc::optional<Tracked> a(Tracked(3));
        ricc::optional<Tracked> b(a);
        REQUIRE(a->value == 3);
        REQUIRE(b->value == 3);
        REQUIRE(Tracked::alive == 2);
    }
    REQUIRE(Tracked::alive == 0);
}

TEST_CASE("copying an empty optional gives an empty optional", "[optional][copy]") {
    reset_tracked();
    ricc::optional<Tracked> a;
    ricc::optional<Tracked> b(a);
    REQUIRE_FALSE(b);
    REQUIRE(Tracked::alive == 0);
}

TEST_CASE("moving an optional moves the value, not copies it", "[optional][move]") {
    reset_tracked();
    {
        ricc::optional<Tracked> a(Tracked(4));
        Tracked::copies = 0;
        ricc::optional<Tracked> b(std::move(a));
        REQUIRE(b->value == 4);
        REQUIRE(Tracked::copies == 0);
    }
    REQUIRE(Tracked::alive == 0);
}

TEST_CASE("copy assignment covers all four empty/engaged combinations", "[optional][copy][lifetime]") {
    reset_tracked();
    {
        ricc::optional<Tracked> full(Tracked(1));
        ricc::optional<Tracked> empty;

        SECTION("engaged = engaged") {
            ricc::optional<Tracked> target(Tracked(2));
            target = full;
            REQUIRE(target->value == 1);
            REQUIRE(Tracked::alive == 2);
        }
        SECTION("empty = engaged constructs a value") {
            ricc::optional<Tracked> target;
            target = full;
            REQUIRE(target->value == 1);
            REQUIRE(Tracked::alive == 2);
        }
        SECTION("engaged = empty destroys the value") {
            ricc::optional<Tracked> target(Tracked(2));
            target = empty;
            REQUIRE_FALSE(target);
            REQUIRE(Tracked::alive == 1);
        }
        SECTION("empty = empty stays empty") {
            ricc::optional<Tracked> target;
            target = empty;
            REQUIRE_FALSE(target);
            REQUIRE(Tracked::alive == 1);
        }
    }
    REQUIRE(Tracked::alive == 0);
}

TEST_CASE("move assignment from an engaged optional", "[optional][move][lifetime]") {
    reset_tracked();
    {
        ricc::optional<Tracked> source(Tracked(5));
        ricc::optional<Tracked> target;
        Tracked::copies = 0;
        target = std::move(source);
        REQUIRE(target->value == 5);
        REQUIRE(Tracked::copies == 0);
    }
    REQUIRE(Tracked::alive == 0);
}

TEST_CASE("self copy-assignment keeps the value", "[optional][copy]") {
    reset_tracked();
    {
        ricc::optional<Tracked> o(Tracked(8));
        auto& alias = o;
        o = alias;
        REQUIRE(o->value == 8);
        REQUIRE(Tracked::alive == 1);
    }
    REQUIRE(Tracked::alive == 0);
}

TEST_CASE("moves are noexcept when T's moves are", "[optional][move][traits]") {
    // So Vector<optional<T>> moves instead of copies when it grows.
    STATIC_REQUIRE(std::is_nothrow_move_constructible_v<ricc::optional<int>>);
    STATIC_REQUIRE(std::is_nothrow_move_constructible_v<ricc::optional<Tracked>>);
}

TEST_CASE("optional of a move-only type is movable but not copyable", "[optional][move][traits]") {
    using Opt = ricc::optional<ricc::unique_ptr<int>>;
    STATIC_REQUIRE(std::is_move_constructible_v<Opt>);
    STATIC_REQUIRE_FALSE(std::is_copy_constructible_v<Opt>);

    Opt a(ricc::make_unique<int>(3));
    Opt b(std::move(a));
    REQUIRE(**b == 3);
}

// ---------------------------------------------------------------------------
// 5. Assigning values and nullopt, reset, emplace
// ---------------------------------------------------------------------------

TEST_CASE("assigning a value to an empty optional engages it", "[optional][assign]") {
    ricc::optional<std::string> o;
    o = std::string("hi");
    REQUIRE(o);
    REQUIRE(*o == "hi");

    o = std::string("bye");       // engaged = value
    REQUIRE(*o == "bye");
}

TEST_CASE("assigning nullopt destroys the value", "[optional][assign][lifetime]") {
    reset_tracked();
    ricc::optional<Tracked> o(Tracked(1));
    REQUIRE(Tracked::alive == 1);
    o = ricc::nullopt;
    REQUIRE_FALSE(o);
    REQUIRE(Tracked::alive == 0);
}

TEST_CASE("reset destroys the value and empties the optional", "[optional][reset][lifetime]") {
    reset_tracked();
    ricc::optional<Tracked> o(Tracked(1));
    o.reset();
    REQUIRE_FALSE(o);
    REQUIRE(Tracked::alive == 0);

    o.reset();                     // resetting an empty optional is fine
    REQUIRE_FALSE(o);
}

TEST_CASE("emplace constructs the value in place", "[optional][emplace]") {
    reset_tracked();
    {
        ricc::optional<Tracked> o;
        Tracked& ref = o.emplace(9);
        REQUIRE(o->value == 9);
        REQUIRE(&ref == &*o);
        REQUIRE(Tracked::copies == 0);
        REQUIRE(Tracked::moves == 0);
    }
    REQUIRE(Tracked::alive == 0);
}

TEST_CASE("emplace on an engaged optional replaces the old value", "[optional][emplace][lifetime]") {
    reset_tracked();
    {
        ricc::optional<Tracked> o(Tracked(1));
        o.emplace(2);
        REQUIRE(o->value == 2);
        REQUIRE(Tracked::alive == 1);   // the old one was destroyed
    }
    REQUIRE(Tracked::alive == 0);
}

TEST_CASE("emplace with no arguments default-constructs (compiler test pattern)", "[optional][emplace]") {
    // evaluator_operator_tests.cpp:  call->arguments.emplace();
    ricc::optional<ricc::Vector<int>> args;
    args.emplace();
    REQUIRE(args);
    REQUIRE(args->empty());
    args->push_back(1);
    REQUIRE(args->size() == 1);
}

// ---------------------------------------------------------------------------
// 6. The compiler's patterns
// ---------------------------------------------------------------------------

TEST_CASE("environment lookup: return a copy or nullopt", "[optional][compiler]") {
    const Binding stored{1, ricc::optional<int>(10)};

    auto missing = lookup(stored, false);
    REQUIRE_FALSE(missing);

    const auto found = lookup(stored, true);
    REQUIRE(found);
    REQUIRE(found->type == 1);
    REQUIRE(found->value);
    REQUIRE(*found->value == 10);
    REQUIRE(found->value.value() == 10);
}

TEST_CASE("binding with an uninitialised value", "[optional][compiler]") {
    Binding b{2, ricc::nullopt};    // like `int x;` with no initializer
    REQUIRE_FALSE(b.value);
    b.value = 5;
    REQUIRE(*b.value == 5);
}

TEST_CASE("parser result: optional<Vector<unique_ptr<T>>>", "[optional][compiler]") {
    auto failed = parse_args(false);
    REQUIRE_FALSE(failed);

    auto parsed = parse_args(true);
    REQUIRE(parsed);
    REQUIRE(parsed->size() == 2);
    REQUIRE(*parsed->at(0) == 1);
    REQUIRE(*parsed->at(1) == 2);

    for (const auto& arg : *parsed) {
        REQUIRE(arg != nullptr);
    }
}

TEST_CASE("assigning a parser result into an existing optional member", "[optional][compiler]") {
    // parser.hpp:  declaration->arguments = parseCallExpressionArguments();
    ricc::optional<ricc::Vector<ricc::unique_ptr<int>>> arguments;
    arguments = parse_args(true);
    REQUIRE(arguments->size() == 2);
    arguments = parse_args(false);
    REQUIRE_FALSE(arguments);
}
