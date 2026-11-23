// Tests for ricc::variant, covering what the compiler project needs to replace
// std::variant in `using Value = std::variant<std::monostate, int, float,
// double, bool, std::shared_ptr<FunctionObj>>;`
//
// Every test here also passes with std::variant / std::get / std::get_if /
// std::holds_alternative / std::visit / std::bad_variant_access in place of the
// ricc:: names, so a failure means ricc::variant behaves differently.
//
// Names the tests expect in namespace ricc:
//   variant<Ts...>, v.index(), holds_alternative<T>(v), get<T>(v),
//   get_if<T>(&v), visit(f, v) and visit(f, v1, v2), bad_variant_access
//
// Sections:
//   1. Construction (default, from each alternative)
//   2. index / holds_alternative
//   3. get<T> and bad_variant_access
//   4. get_if<T>
//   5. Copy and move (construction and assignment)
//   6. Lifetime: destructor and switching alternatives
//   7. visit (one variant, two variants, return types)
//   8. Compiler-shaped usage (Value, optional<Value>, Vector<Value>)

#include <catch2/catch_test_macros.hpp>

#include "variant.hpp"
#include "optional.hpp"
#include "vector.hpp"

#include <cstddef>
#include <cstdint>
#include <exception>
#include <memory>
#include <stdexcept>
#include <string>
#include <string_view>
#include <type_traits>
#include <utility>
#include <variant>  // std::monostate

namespace {

// Counts live objects so tests can see missing or extra destructor calls.
struct Tracked {
    static inline int alive = 0;
    int value;

    explicit Tracked(int v = 0) : value(v) { ++alive; }
    Tracked(const Tracked& other) : value(other.value) { ++alive; }
    Tracked(Tracked&& other) noexcept : value(std::exchange(other.value, -1)) { ++alive; }
    Tracked& operator=(const Tracked& other) { value = other.value; return *this; }
    Tracked& operator=(Tracked&& other) noexcept { value = std::exchange(other.value, -1); return *this; }
    ~Tracked() { --alive; }
};

// For visit with several lambdas.
template <typename... Fs>
struct overloaded : Fs... {
    using Fs::operator()...;
};

// Shaped like the compiler's object.hpp.
struct FunctionObj {
    int arity = 0;
};
using Value = ricc::variant<std::monostate, int, float, double, bool, std::shared_ptr<FunctionObj>>;

template <typename T>
constexpr bool is_numeric = std::is_arithmetic_v<T> && !std::is_same_v<T, bool>;

// Shaped like evaluator.hpp's evalInfixExpression: binary visit returning Value.
Value add(const Value& left, const Value& right) {
    return ricc::visit([](const auto& a, const auto& b) -> Value {
        using A = std::remove_cvref_t<decltype(a)>;
        using B = std::remove_cvref_t<decltype(b)>;
        if constexpr (is_numeric<A> && is_numeric<B>) {
            return Value(a + b);
        } else {
            throw std::runtime_error("Invalid numeric value for infix expression");
        }
    }, left, right);
}

// Shaped like object.cpp's type().
std::string_view type_name(const Value& value) {
    return ricc::visit([](const auto& item) -> std::string_view {
        using T = std::remove_cvref_t<decltype(item)>;
        if constexpr (std::is_same_v<T, std::monostate>) return "NULL";
        else if constexpr (std::is_same_v<T, int>) return "int";
        else if constexpr (std::is_same_v<T, float>) return "float";
        else if constexpr (std::is_same_v<T, double>) return "double";
        else if constexpr (std::is_same_v<T, bool>) return "bool";
        else return "function";
    }, value);
}

}  // namespace

// ---------------------------------------------------------------------------
// 1. Construction
// ---------------------------------------------------------------------------

TEST_CASE("default variant holds a value-initialized first alternative", "[variant][ctor]") {
    ricc::variant<int, double> v;
    REQUIRE(v.index() == 0);
    REQUIRE(ricc::get<int>(v) == 0);
}

TEST_CASE("default Value holds std::monostate", "[variant][ctor]") {
    Value v;
    REQUIRE(v.index() == 0);
    REQUIRE(ricc::holds_alternative<std::monostate>(v));
}

TEST_CASE("variant is built from each alternative type", "[variant][ctor]") {
    REQUIRE(Value{std::monostate{}}.index() == 0);
    REQUIRE(Value{1}.index() == 1);
    REQUIRE(Value{1.5f}.index() == 2);
    REQUIRE(Value{2.5}.index() == 3);
    REQUIRE(Value{true}.index() == 4);
    REQUIRE(Value{std::make_shared<FunctionObj>()}.index() == 5);
}

TEST_CASE("variant stores the value it was built from", "[variant][ctor]") {
    REQUIRE(ricc::get<int>(Value{42}) == 42);
    REQUIRE(ricc::get<float>(Value{1.5f}) == 1.5f);
    REQUIRE(ricc::get<double>(Value{2.25}) == 2.25);
    REQUIRE(ricc::get<bool>(Value{false}) == false);
}

TEST_CASE("variant is built from an lvalue, a const lvalue and an rvalue", "[variant][ctor]") {
    int x = 3;
    const double y = 4.5;
    ricc::variant<int, double> from_lvalue{x};
    ricc::variant<int, double> from_const{y};
    ricc::variant<int, double> from_rvalue{7};

    REQUIRE(ricc::get<int>(from_lvalue) == 3);
    REQUIRE(ricc::get<double>(from_const) == 4.5);
    REQUIRE(ricc::get<int>(from_rvalue) == 7);
    REQUIRE(x == 3);  // copying from an lvalue leaves it alone
}

TEST_CASE("variant converts implicitly from an alternative", "[variant][ctor]") {
    // The compiler writes `Value value{std::monostate{}};` and returns plain
    // ints/bools from functions declared to return Value.
    STATIC_REQUIRE(std::is_convertible_v<int, Value>);
    STATIC_REQUIRE(std::is_convertible_v<bool, Value>);
    STATIC_REQUIRE(std::is_convertible_v<std::shared_ptr<FunctionObj>, Value>);

    auto make = []() -> Value { return 10; };
    REQUIRE(ricc::get<int>(make()) == 10);
}

TEST_CASE("variant holding a std::string keeps its contents", "[variant][ctor]") {
    ricc::variant<int, std::string> v{std::string(100, 'x')};
    REQUIRE(v.index() == 1);
    REQUIRE(ricc::get<std::string>(v) == std::string(100, 'x'));
}

TEST_CASE("variant moves a shared_ptr in instead of copying it", "[variant][ctor]") {
    auto fn = std::make_shared<FunctionObj>();
    std::weak_ptr<FunctionObj> watch = fn;
    Value v{std::move(fn)};
    REQUIRE(fn == nullptr);
    REQUIRE(watch.use_count() == 1);
}

TEST_CASE("variant can hold an alignment-sensitive type", "[variant][ctor]") {
    struct alignas(32) Wide { double d[4]; };
    ricc::variant<char, Wide> v{Wide{{1, 2, 3, 4}}};
    const auto address = reinterpret_cast<std::uintptr_t>(&ricc::get<Wide>(v));
    REQUIRE(address % 32 == 0);
    REQUIRE(ricc::get<Wide>(v).d[3] == 4);
}

// ---------------------------------------------------------------------------
// 2. index / holds_alternative
// ---------------------------------------------------------------------------

TEST_CASE("index and holds_alternative agree for every alternative", "[variant][index]") {
    const Value values[] = {Value{}, Value{1}, Value{1.0f}, Value{1.0}, Value{true},
                            Value{std::make_shared<FunctionObj>()}};
    for (std::size_t i = 0; i < 6; ++i) {
        CAPTURE(i);
        const Value& v = values[i];
        REQUIRE(v.index() == i);
        REQUIRE(ricc::holds_alternative<std::monostate>(v) == (i == 0));
        REQUIRE(ricc::holds_alternative<int>(v) == (i == 1));
        REQUIRE(ricc::holds_alternative<float>(v) == (i == 2));
        REQUIRE(ricc::holds_alternative<double>(v) == (i == 3));
        REQUIRE(ricc::holds_alternative<bool>(v) == (i == 4));
        REQUIRE(ricc::holds_alternative<std::shared_ptr<FunctionObj>>(v) == (i == 5));
    }
}

TEST_CASE("index() is a const size_t member", "[variant][index]") {
    const ricc::variant<int, double> v{1.0};
    STATIC_REQUIRE(std::is_same_v<decltype(v.index()), std::size_t>);
    REQUIRE(v.index() == 1);
}

// ---------------------------------------------------------------------------
// 3. get<T> and bad_variant_access
// ---------------------------------------------------------------------------

TEST_CASE("get returns a reference to the stored value", "[variant][get]") {
    ricc::variant<int, double> v{5};
    STATIC_REQUIRE(std::is_same_v<decltype(ricc::get<int>(v)), int&>);
    ricc::get<int>(v) = 9;
    REQUIRE(ricc::get<int>(v) == 9);
}

TEST_CASE("get on a const variant returns a const reference", "[variant][get]") {
    const ricc::variant<int, double> v{2.5};
    STATIC_REQUIRE(std::is_same_v<decltype(ricc::get<double>(v)), const double&>);
    REQUIRE(ricc::get<double>(v) == 2.5);
}

TEST_CASE("get with the wrong type throws bad_variant_access", "[variant][get]") {
    ricc::variant<int, double> v{5};
    REQUIRE_THROWS_AS(ricc::get<double>(v), ricc::bad_variant_access);
}

TEST_CASE("const get with the wrong type throws bad_variant_access", "[variant][get]") {
    // Easy to miss: writing `bad_variant_access();` without `throw`.
    const ricc::variant<int, double> v{5};
    REQUIRE_THROWS_AS(ricc::get<double>(v), ricc::bad_variant_access);
}

TEST_CASE("bad_variant_access is a std::exception but not a std::runtime_error", "[variant][get]") {
    // error_regression_tests relies on it not being caught as a runtime_error.
    STATIC_REQUIRE(std::is_base_of_v<std::exception, ricc::bad_variant_access>);
    STATIC_REQUIRE_FALSE(std::is_base_of_v<std::runtime_error, ricc::bad_variant_access>);
}

TEST_CASE("get on a temporary variant", "[variant][get]") {
    REQUIRE(ricc::get<int>(ricc::variant<int, double>{11}) == 11);
}

// ---------------------------------------------------------------------------
// 4. get_if<T>
// ---------------------------------------------------------------------------

TEST_CASE("get_if returns a pointer to the value when the type matches", "[variant][get_if]") {
    ricc::variant<int, double> v{5};
    int* p = ricc::get_if<int>(&v);
    REQUIRE(p != nullptr);
    REQUIRE(*p == 5);
    *p = 6;
    REQUIRE(ricc::get<int>(v) == 6);
}

TEST_CASE("get_if returns nullptr when the type does not match", "[variant][get_if]") {
    ricc::variant<int, double> v{5};
    REQUIRE(ricc::get_if<double>(&v) == nullptr);
}

TEST_CASE("get_if on a const variant returns a pointer to const", "[variant][get_if]") {
    const Value v{3};
    STATIC_REQUIRE(std::is_same_v<decltype(ricc::get_if<int>(&v)), const int*>);
    const int* p = ricc::get_if<int>(&v);
    REQUIRE(p != nullptr);
    REQUIRE(*p == 3);
    REQUIRE(ricc::get_if<bool>(&v) == nullptr);
}

TEST_CASE("get_if finds a shared_ptr alternative like the call evaluator", "[variant][get_if]") {
    // evaluator.hpp: const auto* callable = std::get_if<std::shared_ptr<FunctionObj>>(&callee);
    auto fn = std::make_shared<FunctionObj>();
    fn->arity = 2;
    const Value callee{fn};
    const auto* callable = ricc::get_if<std::shared_ptr<FunctionObj>>(&callee);
    REQUIRE(callable != nullptr);
    REQUIRE((*callable)->arity == 2);

    const Value not_callable{1};
    REQUIRE(ricc::get_if<std::shared_ptr<FunctionObj>>(&not_callable) == nullptr);
}

// ---------------------------------------------------------------------------
// 5. Copy and move
// ---------------------------------------------------------------------------

TEST_CASE("copy construction copies the active alternative", "[variant][copy]") {
    ricc::variant<int, std::string> original{std::string("hello")};
    ricc::variant<int, std::string> copy{original};
    REQUIRE(copy.index() == 1);
    REQUIRE(ricc::get<std::string>(copy) == "hello");

    ricc::get<std::string>(copy) += " world";
    REQUIRE(ricc::get<std::string>(original) == "hello");
}

TEST_CASE("copy construction works for every alternative index", "[variant][copy]") {
    // Catches a copy table that only handles the first one or two types.
    const Value values[] = {Value{}, Value{1}, Value{1.5f}, Value{2.5}, Value{true},
                            Value{std::make_shared<FunctionObj>()}};
    for (std::size_t i = 0; i < 6; ++i) {
        CAPTURE(i);
        Value copy{values[i]};
        REQUIRE(copy.index() == i);
    }
    REQUIRE(ricc::get<float>(Value{values[2]}) == 1.5f);
    REQUIRE(ricc::get<double>(Value{values[3]}) == 2.5);
    REQUIRE(ricc::get<bool>(Value{values[4]}) == true);
}

TEST_CASE("copying a shared_ptr alternative shares ownership", "[variant][copy]") {
    Value a{std::make_shared<FunctionObj>()};
    Value b{a};
    const auto& pa = ricc::get<std::shared_ptr<FunctionObj>>(a);
    const auto& pb = ricc::get<std::shared_ptr<FunctionObj>>(b);
    REQUIRE(pa == pb);
    REQUIRE(pa.use_count() == 2);
}

TEST_CASE("move construction takes the active alternative", "[variant][move]") {
    auto fn = std::make_shared<FunctionObj>();
    std::weak_ptr<FunctionObj> watch = fn;
    Value a{std::move(fn)};
    Value b{std::move(a)};
    REQUIRE(b.index() == 5);
    REQUIRE(watch.use_count() == 1);  // moved, not copied
}

TEST_CASE("move construction keeps the index for every alternative", "[variant][move]") {
    for (std::size_t i = 0; i < 6; ++i) {
        CAPTURE(i);
        Value values[] = {Value{}, Value{1}, Value{1.5f}, Value{2.5}, Value{true},
                          Value{std::make_shared<FunctionObj>()}};
        Value moved{std::move(values[i])};
        REQUIRE(moved.index() == i);
    }
    Value source{2.5};
    Value target{std::move(source)};
    REQUIRE(ricc::get<double>(target) == 2.5);
}

TEST_CASE("a moved-from variant can be assigned and used again", "[variant][move]") {
    ricc::variant<int, std::string> a{std::string(50, 'a')};
    ricc::variant<int, std::string> b{std::move(a)};
    a = 7;
    REQUIRE(ricc::get<int>(a) == 7);
    REQUIRE(ricc::get<std::string>(b) == std::string(50, 'a'));
}

TEST_CASE("Value moves are noexcept", "[variant][move]") {
    // Without this, ricc::Vector<Value> copies every element when it grows.
    STATIC_REQUIRE(std::is_nothrow_move_constructible_v<Value>);
    STATIC_REQUIRE(std::is_nothrow_move_assignable_v<Value>);
}

TEST_CASE("copy assignment to the same alternative", "[variant][assign]") {
    ricc::variant<int, std::string> a{std::string("old")};
    const ricc::variant<int, std::string> b{std::string("new")};
    a = b;
    REQUIRE(ricc::get<std::string>(a) == "new");
    REQUIRE(ricc::get<std::string>(b) == "new");
}

TEST_CASE("copy assignment to a different alternative", "[variant][assign]") {
    ricc::variant<int, std::string> a{5};
    const ricc::variant<int, std::string> b{std::string("text")};
    a = b;
    REQUIRE(a.index() == 1);
    REQUIRE(ricc::get<std::string>(a) == "text");

    const ricc::variant<int, std::string> c{9};
    a = c;
    REQUIRE(a.index() == 0);
    REQUIRE(ricc::get<int>(a) == 9);
}

TEST_CASE("copy assignment returns *this", "[variant][assign]") {
    ricc::variant<int, double> a{1};
    const ricc::variant<int, double> b{2.0};
    auto& result = (a = b);
    REQUIRE(&result == &a);
}

TEST_CASE("self copy assignment keeps the value", "[variant][assign]") {
    ricc::variant<int, std::string> a{std::string(40, 'z')};
    auto& same = a;
    a = same;
    REQUIRE(ricc::get<std::string>(a) == std::string(40, 'z'));
}

TEST_CASE("move assignment to the same and to a different alternative", "[variant][assign]") {
    ricc::variant<int, std::string> a{1};
    a = ricc::variant<int, std::string>{std::string("moved in")};
    REQUIRE(ricc::get<std::string>(a) == "moved in");

    a = ricc::variant<int, std::string>{std::string("again")};
    REQUIRE(ricc::get<std::string>(a) == "again");

    a = ricc::variant<int, std::string>{3};
    REQUIRE(ricc::get<int>(a) == 3);
}

TEST_CASE("assigning a plain alternative value", "[variant][assign]") {
    // evaluator.hpp: var.value = eval(...); and Value v; v = 5;
    Value v;
    v = 5;
    REQUIRE(ricc::get<int>(v) == 5);
    v = true;
    REQUIRE(ricc::get<bool>(v) == true);
    v = std::make_shared<FunctionObj>();
    REQUIRE(v.index() == 5);
    v = std::monostate{};
    REQUIRE(v.index() == 0);
}

// ---------------------------------------------------------------------------
// 6. Lifetime
// ---------------------------------------------------------------------------

TEST_CASE("destroying a variant destroys the active alternative", "[variant][lifetime]") {
    Tracked::alive = 0;
    {
        ricc::variant<int, Tracked> v{Tracked{1}};
        REQUIRE(Tracked::alive == 1);
    }
    REQUIRE(Tracked::alive == 0);
}

TEST_CASE("destroying a variant releases a shared_ptr", "[variant][lifetime]") {
    std::weak_ptr<FunctionObj> watch;
    {
        Value v{std::make_shared<FunctionObj>()};
        watch = ricc::get<std::shared_ptr<FunctionObj>>(v);
        REQUIRE_FALSE(watch.expired());
    }
    REQUIRE(watch.expired());
}

TEST_CASE("switching alternatives destroys the old one", "[variant][lifetime]") {
    Tracked::alive = 0;
    {
        ricc::variant<int, Tracked> v{Tracked{1}};
        v = 5;
        REQUIRE(Tracked::alive == 0);
        v = Tracked{2};
        REQUIRE(Tracked::alive == 1);
        REQUIRE(ricc::get<Tracked>(v).value == 2);
    }
    REQUIRE(Tracked::alive == 0);
}

TEST_CASE("replacing a shared_ptr alternative with an int releases it", "[variant][lifetime]") {
    auto fn = std::make_shared<FunctionObj>();
    std::weak_ptr<FunctionObj> watch = fn;
    Value v{std::move(fn)};
    v = 1;
    REQUIRE(watch.expired());
}

TEST_CASE("copies and moves leave no extra objects alive", "[variant][lifetime]") {
    Tracked::alive = 0;
    {
        ricc::variant<int, Tracked> a{Tracked{1}};
        ricc::variant<int, Tracked> b{a};
        ricc::variant<int, Tracked> c{std::move(a)};
        REQUIRE(Tracked::alive == 3);  // a still holds a moved-from Tracked
        b = c;
        a = 0;
        REQUIRE(Tracked::alive == 2);
        REQUIRE(ricc::get<Tracked>(b).value == 1);
    }
    REQUIRE(Tracked::alive == 0);
}

// ---------------------------------------------------------------------------
// 7. visit
// ---------------------------------------------------------------------------

TEST_CASE("visit calls the function with the active alternative", "[variant][visit]") {
    ricc::variant<int, double> v{2.5};
    double seen = 0;
    ricc::visit([&](auto x) { seen = static_cast<double>(x); }, v);
    REQUIRE(seen == 2.5);
}

TEST_CASE("visit returns what the function returns", "[variant][visit]") {
    const ricc::variant<int, double> v{4};
    auto doubled = ricc::visit([](auto x) -> double { return x * 2.0; }, v);
    STATIC_REQUIRE(std::is_same_v<decltype(doubled), double>);
    REQUIRE(doubled == 8.0);
}

TEST_CASE("visit can modify the active alternative", "[variant][visit]") {
    ricc::variant<int, std::string> v{std::string("ab")};
    ricc::visit([](auto& x) {
        if constexpr (std::is_same_v<std::remove_cvref_t<decltype(x)>, std::string>) x += "c";
        else x += 1;
    }, v);
    REQUIRE(ricc::get<std::string>(v) == "abc");
}

TEST_CASE("visit passes a const reference for a const variant", "[variant][visit]") {
    const ricc::variant<int, std::string> v{std::string("const")};
    bool was_const = ricc::visit([](auto& x) {
        return std::is_const_v<std::remove_reference_t<decltype(x)>>;
    }, v);
    REQUIRE(was_const);
}

TEST_CASE("visit with an overloaded set of lambdas", "[variant][visit]") {
    auto describe = overloaded{
        [](int) { return std::string("int"); },
        [](double) { return std::string("double"); },
        [](const std::string& s) { return "string:" + s; },
    };
    REQUIRE(ricc::visit(describe, ricc::variant<int, double, std::string>{1}) == "int");
    REQUIRE(ricc::visit(describe, ricc::variant<int, double, std::string>{1.0}) == "double");
    REQUIRE(ricc::visit(describe, ricc::variant<int, double, std::string>{std::string("x")}) == "string:x");
}

TEST_CASE("visit reaches every alternative of Value", "[variant][visit]") {
    REQUIRE(type_name(Value{}) == "NULL");
    REQUIRE(type_name(Value{1}) == "int");
    REQUIRE(type_name(Value{1.0f}) == "float");
    REQUIRE(type_name(Value{1.0}) == "double");
    REQUIRE(type_name(Value{true}) == "bool");
    REQUIRE(type_name(Value{std::make_shared<FunctionObj>()}) == "function");
}

TEST_CASE("visit lets exceptions from the function propagate", "[variant][visit]") {
    ricc::variant<int, double> v{1};
    REQUIRE_THROWS_AS(
        ricc::visit([](auto) -> int { throw std::runtime_error("boom"); }, v),
        std::runtime_error);
}

TEST_CASE("visit with two variants sees both active alternatives", "[variant][visit][binary]") {
    ricc::variant<int, double> a{2};
    ricc::variant<int, double> b{0.5};
    auto kinds = ricc::visit([](auto x, auto y) {
        return std::string(std::is_same_v<decltype(x), int> ? "i" : "d") +
               (std::is_same_v<decltype(y), int> ? "i" : "d");
    }, a, b);
    REQUIRE(kinds == "id");
}

TEST_CASE("binary visit covers every pair of alternatives", "[variant][visit][binary]") {
    const ricc::variant<int, double, bool> values[] = {1, 2.0, true};
    for (std::size_t i = 0; i < 3; ++i) {
        for (std::size_t j = 0; j < 3; ++j) {
            CAPTURE(i, j);
            auto pair = ricc::visit([](auto x, auto y) {
                auto index_of = [](auto v) -> std::size_t {
                    using T = decltype(v);
                    return std::is_same_v<T, int> ? 0 : std::is_same_v<T, double> ? 1 : 2;
                };
                return std::pair{index_of(x), index_of(y)};
            }, values[i], values[j]);
            REQUIRE(pair.first == i);
            REQUIRE(pair.second == j);
        }
    }
}

TEST_CASE("binary visit returning Value like evalInfixExpression", "[variant][visit][binary]") {
    Value sum = add(Value{2}, Value{3});
    REQUIRE(ricc::get<int>(sum) == 5);

    Value mixed = add(Value{2}, Value{0.5});
    REQUIRE(ricc::holds_alternative<double>(mixed));
    REQUIRE(ricc::get<double>(mixed) == 2.5);

    Value single = add(Value{1.5f}, Value{1.5f});
    REQUIRE(ricc::get<float>(single) == 3.0f);

    REQUIRE_THROWS_AS(add(Value{true}, Value{1}), std::runtime_error);
    REQUIRE_THROWS_AS(add(Value{}, Value{1}), std::runtime_error);
}

// ---------------------------------------------------------------------------
// 8. Compiler-shaped usage
// ---------------------------------------------------------------------------

TEST_CASE("ricc::optional<Value> stores, copies and resets a Value", "[variant][usage]") {
    // Environment.hpp: Binding holds ricc::optional<Value>.
    std::weak_ptr<FunctionObj> watch;
    {
        ricc::optional<Value> slot;
        REQUIRE_FALSE(slot.has_value());

        slot = Value{5};
        REQUIRE(ricc::get<int>(slot.value()) == 5);

        auto fn = std::make_shared<FunctionObj>();
        watch = fn;
        slot = Value{std::move(fn)};
        ricc::optional<Value> copy{slot};
        REQUIRE(watch.use_count() == 2);

        slot.reset();
        REQUIRE(watch.use_count() == 1);
    }
    REQUIRE(watch.expired());
}

TEST_CASE("ricc::Vector<Value> keeps every alternative through growth", "[variant][usage]") {
    ricc::Vector<Value> values;
    for (int i = 0; i < 60; ++i) {
        switch (i % 4) {
            case 0: values.push_back(Value{i}); break;
            case 1: values.push_back(Value{static_cast<double>(i)}); break;
            case 2: values.push_back(Value{i % 3 == 0}); break;
            case 3: values.push_back(Value{std::make_shared<FunctionObj>(FunctionObj{i})}); break;
        }
    }
    REQUIRE(values.size() == 60);
    for (std::size_t i = 0; i < 60; ++i) {
        CAPTURE(i);
        const int n = static_cast<int>(i);
        switch (i % 4) {
            case 0: REQUIRE(ricc::get<int>(values[i]) == n); break;
            case 1: REQUIRE(ricc::get<double>(values[i]) == n); break;
            case 2: REQUIRE(ricc::get<bool>(values[i]) == (n % 3 == 0)); break;
            case 3: REQUIRE(ricc::get<std::shared_ptr<FunctionObj>>(values[i])->arity == n); break;
        }
    }
}

TEST_CASE("checking and reading a Value like the unary evaluator", "[variant][usage]") {
    // evaluator.hpp: if (!std::holds_alternative<int>(right)) throw ...;
    //                int val = std::get<int>(right);
    auto negate = [](const Value& right) -> Value {
        if (!ricc::holds_alternative<int>(right)) {
            throw std::runtime_error("Invalid right expression for MINUS");
        }
        return Value(-ricc::get<int>(right));
    };
    REQUIRE(ricc::get<int>(negate(Value{4})) == -4);
    REQUIRE_THROWS_AS(negate(Value{true}), std::runtime_error);
}
