#include <catch2/catch_test_macros.hpp>

#include "unique_ptr.hpp"
#include "vector.hpp"

#include <string>
#include <type_traits>
#include <utility>

// These tests describe what ricc::unique_ptr needs so the compiler project
// (~/compiler) can use it instead of std::unique_ptr.
//
// Many of them will NOT COMPILE until the matching feature exists — that
// compile error is the failing test. Work through the sections in order and
// read only the FIRST compiler error each time.

namespace {

// Counts live objects, so tests can check that unique_ptr deletes exactly once.
struct Counted {
    static inline int alive = 0;
    int value;

    explicit Counted(int v = 0) : value(v) { ++alive; }
    Counted(const Counted&) = delete;
    Counted& operator=(const Counted&) = delete;
    ~Counted() { --alive; }
};

// Base/Derived pair, like the compiler's Expression / Integer_Liter.
// Base has a VIRTUAL destructor — required to delete a Derived through a Base*.
struct Base {
    static inline int base_destroyed = 0;

    virtual ~Base() { ++base_destroyed; }
    virtual int id() const { return 1; }
};

struct Derived : Base {
    static inline int derived_destroyed = 0;
    int extra;

    explicit Derived(int e = 0) : extra(e) {}
    ~Derived() override { ++derived_destroyed; }
    int id() const override { return 2; }
};

void reset_counters() {
    Counted::alive = 0;
    Base::base_destroyed = 0;
    Derived::derived_destroyed = 0;
}

// The parser pattern: build a Derived, return it as a unique_ptr<Base>.
ricc::unique_ptr<Base> make_expression(int value) {
    return ricc::make_unique<Derived>(value);
}

}  // namespace

// ---------------------------------------------------------------------------
// 1. Basics (should already pass)
// ---------------------------------------------------------------------------

TEST_CASE("default unique_ptr is null", "[unique_ptr]") {
    ricc::unique_ptr<int> p;
    REQUIRE(p.get() == nullptr);
    REQUIRE(p == nullptr);
}

TEST_CASE("unique_ptr deletes its object exactly once", "[unique_ptr][lifetime]") {
    reset_counters();
    {
        ricc::unique_ptr<Counted> p(new Counted(5));
        REQUIRE(Counted::alive == 1);
        REQUIRE((*p).value == 5);
    }
    REQUIRE(Counted::alive == 0);
}

TEST_CASE("unique_ptr cannot be copied", "[unique_ptr][traits]") {
    STATIC_REQUIRE_FALSE(std::is_copy_constructible_v<ricc::unique_ptr<int>>);
    STATIC_REQUIRE_FALSE(std::is_copy_assignable_v<ricc::unique_ptr<int>>);
}

TEST_CASE("move construction transfers ownership", "[unique_ptr]") {
    reset_counters();
    {
        ricc::unique_ptr<Counted> a(new Counted(1));
        ricc::unique_ptr<Counted> b(std::move(a));
        REQUIRE(a.get() == nullptr);
        REQUIRE(b.get() != nullptr);
        REQUIRE(Counted::alive == 1);
    }
    REQUIRE(Counted::alive == 0);
}

TEST_CASE("move assignment deletes the old object", "[unique_ptr][lifetime]") {
    reset_counters();
    {
        ricc::unique_ptr<Counted> a(new Counted(1));
        ricc::unique_ptr<Counted> b(new Counted(2));
        b = std::move(a);
        REQUIRE(Counted::alive == 1);   // Counted(2) was deleted
        REQUIRE(a.get() == nullptr);
        REQUIRE((*b).value == 1);
    }
    REQUIRE(Counted::alive == 0);
}

TEST_CASE("self move-assignment keeps the object", "[unique_ptr]") {
    reset_counters();
    {
        ricc::unique_ptr<Counted> p(new Counted(7));
        auto& alias = p;
        p = std::move(alias);
        REQUIRE(p.get() != nullptr);
        REQUIRE(Counted::alive == 1);
    }
    REQUIRE(Counted::alive == 0);
}

TEST_CASE("swap exchanges the pointers", "[unique_ptr]") {
    ricc::unique_ptr<int> a(new int(1));
    ricc::unique_ptr<int> b(new int(2));
    a.swap(b);
    REQUIRE(*a == 2);
    REQUIRE(*b == 1);
}

// ---------------------------------------------------------------------------
// 2. operator-> and const access
// ---------------------------------------------------------------------------

TEST_CASE("operator-> reaches members", "[unique_ptr][access]") {
    ricc::unique_ptr<std::string> p(new std::string("hello"));
    REQUIRE(p->size() == 5);
    p->push_back('!');
    REQUIRE(*p == "hello!");
}

TEST_CASE("operator* and operator-> work on a const unique_ptr", "[unique_ptr][access]") {
    const ricc::unique_ptr<std::string> p(new std::string("abc"));
    REQUIRE(*p == "abc");
    REQUIRE(p->size() == 3);
}

TEST_CASE("constness is shallow: a const unique_ptr still gives a non-const T", "[unique_ptr][access]") {
    // Like std::unique_ptr: the POINTER is const, not the object it points to.
    // The compiler passes `const unique_ptr<Expression>&` and calls non-const methods.
    const ricc::unique_ptr<std::string> p(new std::string("abc"));
    STATIC_REQUIRE(std::is_same_v<decltype(p.get()), std::string*>);
    STATIC_REQUIRE(std::is_same_v<decltype(*p), std::string&>);
    p->push_back('d');
    REQUIRE(*p == "abcd");
}

// ---------------------------------------------------------------------------
// 3. explicit operator bool and explicit raw-pointer constructor
// ---------------------------------------------------------------------------

TEST_CASE("unique_ptr tests as a bool", "[unique_ptr][bool]") {
    ricc::unique_ptr<int> empty;
    ricc::unique_ptr<int> full(new int(1));
    REQUIRE_FALSE(empty);
    REQUIRE(full);
    if (full) {
        SUCCEED();
    } else {
        FAIL("non-null unique_ptr converted to false");
    }
}

TEST_CASE("operator bool is explicit", "[unique_ptr][bool][traits]") {
    // `if (p)` must work, but `bool b = p;` or `int x = p + 1;` must not compile.
    STATIC_REQUIRE(std::is_constructible_v<bool, ricc::unique_ptr<int>>);
    STATIC_REQUIRE_FALSE(std::is_convertible_v<ricc::unique_ptr<int>, bool>);
}

TEST_CASE("raw-pointer constructor is explicit", "[unique_ptr][traits]") {
    // Prevents a raw pointer from silently becoming owned (and later double-deleted).
    STATIC_REQUIRE(std::is_constructible_v<ricc::unique_ptr<int>, int*>);
    STATIC_REQUIRE_FALSE(std::is_convertible_v<int*, ricc::unique_ptr<int>>);
}

// ---------------------------------------------------------------------------
// 4. release() and reset()
// ---------------------------------------------------------------------------

TEST_CASE("release gives up ownership without deleting", "[unique_ptr][release]") {
    reset_counters();
    ricc::unique_ptr<Counted> p(new Counted(3));
    Counted* raw = p.release();

    REQUIRE(p.get() == nullptr);
    REQUIRE(raw != nullptr);
    REQUIRE(Counted::alive == 1);   // still alive: we own it now

    delete raw;
    REQUIRE(Counted::alive == 0);
}

TEST_CASE("reset replaces and deletes the old object", "[unique_ptr][reset]") {
    reset_counters();
    {
        ricc::unique_ptr<Counted> p(new Counted(1));
        p.reset(new Counted(2));
        REQUIRE(Counted::alive == 1);
        REQUIRE(p->value == 2);

        p.reset();
        REQUIRE(p.get() == nullptr);
        REQUIRE(Counted::alive == 0);
    }
    REQUIRE(Counted::alive == 0);
}

// ---------------------------------------------------------------------------
// 5. make_unique
// ---------------------------------------------------------------------------

TEST_CASE("make_unique forwards constructor arguments", "[unique_ptr][make_unique]") {
    auto s = ricc::make_unique<std::string>(3, 'x');
    STATIC_REQUIRE(std::is_same_v<decltype(s), ricc::unique_ptr<std::string>>);
    REQUIRE(*s == "xxx");
}

TEST_CASE("make_unique objects are deleted", "[unique_ptr][make_unique][lifetime]") {
    reset_counters();
    {
        auto p = ricc::make_unique<Counted>(9);
        REQUIRE(Counted::alive == 1);
        REQUIRE(p->value == 9);
    }
    REQUIRE(Counted::alive == 0);
}

// ---------------------------------------------------------------------------
// 6. Converting move: unique_ptr<Derived> -> unique_ptr<Base>
// ---------------------------------------------------------------------------

TEST_CASE("unique_ptr<Derived> converts to unique_ptr<Base>", "[unique_ptr][convert]") {
    reset_counters();
    {
        ricc::unique_ptr<Derived> d(new Derived(4));
        ricc::unique_ptr<Base> b(std::move(d));

        REQUIRE(d.get() == nullptr);
        REQUIRE(b->id() == 2);      // virtual dispatch reaches Derived
    }
    // deleting through Base* must run Derived's destructor too
    REQUIRE(Derived::derived_destroyed == 1);
    REQUIRE(Base::base_destroyed == 1);
}

TEST_CASE("converting move assignment", "[unique_ptr][convert]") {
    reset_counters();
    {
        ricc::unique_ptr<Base> b(new Base);
        ricc::unique_ptr<Derived> d(new Derived(1));
        b = std::move(d);           // old Base deleted, now owns the Derived

        REQUIRE(Base::base_destroyed == 1);
        REQUIRE(b->id() == 2);
        REQUIRE(d.get() == nullptr);
    }
    REQUIRE(Derived::derived_destroyed == 1);
    REQUIRE(Base::base_destroyed == 2);
}

TEST_CASE("returning make_unique<Derived> as unique_ptr<Base> (parser pattern)", "[unique_ptr][convert]") {
    reset_counters();
    {
        ricc::unique_ptr<Base> expr = make_expression(42);
        REQUIRE(expr->id() == 2);
    }
    REQUIRE(Derived::derived_destroyed == 1);
}

TEST_CASE("only valid pointer conversions are allowed", "[unique_ptr][convert][traits]") {
    // Derived -> Base is fine, the other direction and unrelated types are not.
    STATIC_REQUIRE(std::is_constructible_v<ricc::unique_ptr<Base>, ricc::unique_ptr<Derived>&&>);
    STATIC_REQUIRE_FALSE(std::is_constructible_v<ricc::unique_ptr<Derived>, ricc::unique_ptr<Base>&&>);
    STATIC_REQUIRE_FALSE(std::is_constructible_v<ricc::unique_ptr<int>, ricc::unique_ptr<double>&&>);
}

// ---------------------------------------------------------------------------
// 7. Working inside ricc::Vector (the compiler's AST stores Vector<unique_ptr<...>>)
// ---------------------------------------------------------------------------

TEST_CASE("unique_ptr moves are noexcept", "[unique_ptr][traits]") {
    // ricc::Vector only MOVES elements during reallocation if the move is noexcept;
    // otherwise it tries to copy, and unique_ptr cannot be copied.
    STATIC_REQUIRE(std::is_nothrow_move_constructible_v<ricc::unique_ptr<int>>);
    STATIC_REQUIRE(std::is_nothrow_move_assignable_v<ricc::unique_ptr<int>>);
}

TEST_CASE("Vector<unique_ptr<int>> keeps values across reallocations", "[unique_ptr][vector]") {
    ricc::Vector<ricc::unique_ptr<int>> v;
    for (int i = 0; i < 100; ++i) {
        v.push_back(ricc::make_unique<int>(i));
    }
    REQUIRE(v.size() == 100);
    for (int i = 0; i < 100; ++i) {
        REQUIRE(*v[static_cast<std::size_t>(i)] == i);
    }
}

TEST_CASE("Vector<unique_ptr<Base>> holds Derived objects and deletes them", "[unique_ptr][vector][lifetime]") {
    reset_counters();
    {
        ricc::Vector<ricc::unique_ptr<Base>> nodes;
        for (int i = 0; i < 10; ++i) {
            nodes.push_back(ricc::make_unique<Derived>(i));
        }
        nodes.emplace_back(new Base);
        REQUIRE(nodes[0]->id() == 2);
        REQUIRE(nodes[10]->id() == 1);
    }
    REQUIRE(Derived::derived_destroyed == 10);
    REQUIRE(Base::base_destroyed == 11);
}

// ---------------------------------------------------------------------------
// 8. nullptr: the compiler writes `return nullptr;` and `ptr = nullptr;`
// ---------------------------------------------------------------------------

namespace {
// Like parser.hpp's parseIfStatement(): bail out with `return nullptr;`
ricc::unique_ptr<Base> parse_or_fail(bool ok) {
    if (!ok) {
        return nullptr;
    }
    return ricc::make_unique<Derived>(1);
}
} // namespace

TEST_CASE("unique_ptr can be constructed from nullptr", "[unique_ptr][nullptr]") {
    ricc::unique_ptr<int> a(nullptr);
    ricc::unique_ptr<int> b = nullptr;   // implicit: an empty pointer is always safe
    REQUIRE(a.get() == nullptr);
    REQUIRE(b.get() == nullptr);
}

TEST_CASE("nullptr converts implicitly, raw pointers still do not", "[unique_ptr][nullptr][traits]") {
    STATIC_REQUIRE(std::is_convertible_v<std::nullptr_t, ricc::unique_ptr<int>>);
    STATIC_REQUIRE_FALSE(std::is_convertible_v<int*, ricc::unique_ptr<int>>);
}

TEST_CASE("a function returning unique_ptr can return nullptr (parser pattern)", "[unique_ptr][nullptr]") {
    reset_counters();
    {
        auto failed = parse_or_fail(false);
        REQUIRE(failed == nullptr);

        auto parsed = parse_or_fail(true);
        REQUIRE(parsed != nullptr);
        REQUIRE(parsed->id() == 2);
    }
    REQUIRE(Derived::derived_destroyed == 1);
}

TEST_CASE("assigning nullptr deletes the object and empties the pointer", "[unique_ptr][nullptr][lifetime]") {
    reset_counters();
    ricc::unique_ptr<Counted> p(new Counted(1));
    REQUIRE(Counted::alive == 1);

    p = nullptr;
    REQUIRE(Counted::alive == 0);
    REQUIRE(p.get() == nullptr);
    REQUIRE_FALSE(p);
}

TEST_CASE("assigning nullptr returns *this and is noexcept", "[unique_ptr][nullptr][traits]") {
    ricc::unique_ptr<int> p(new int(5));
    REQUIRE(&(p = nullptr) == &p);
    STATIC_REQUIRE(noexcept(p = nullptr));
}

TEST_CASE("comparisons with nullptr work in both directions", "[unique_ptr][nullptr]") {
    // The compiler writes `x != nullptr` and the tests write `nullptr == x`.
    ricc::unique_ptr<int> empty;
    ricc::unique_ptr<int> full(new int(1));
    REQUIRE(empty == nullptr);
    REQUIRE(nullptr == empty);
    REQUIRE(full != nullptr);
    REQUIRE(nullptr != full);
}
