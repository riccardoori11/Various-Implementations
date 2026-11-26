#include <catch2/catch_test_macros.hpp>

#include "shared_ptr.hpp"
#include "test_shared_ptr_incomplete.hpp"
#include "variant.hpp"

#include <atomic>
#include <functional>
#include <thread>
#include <type_traits>
#include <unordered_set>
#include <utility>
#include <variant>  // std::monostate
#include <vector>

// These tests describe what ricc::shared_ptr / ricc::weak_ptr need so the
// compiler project (~/compiler) can use them instead of std::shared_ptr /
// std::weak_ptr. Both classes live in shared_ptr.hpp (weak_ptr needs access to
// shared_ptr's control block, so keep them together, like <memory> does).
//
// Many of them will NOT COMPILE until the matching feature exists — that
// compile error is the failing test. Work through the sections in order and
// read only the FIRST compiler error each time.
//
// Design hint: weak_ptr forces a real control block.
//
//     struct control_block {
//         std::atomic<long> strong;   // shared_ptrs alive
//         std::atomic<long> weak;     // weak_ptrs alive (+1 while strong > 0)
//         virtual void destroy_object() noexcept = 0;  // strong hit 0
//         virtual void destroy_self() noexcept = 0;    // weak hit 0
//     };
//
// The object dies when `strong` reaches 0; the control block dies when `weak`
// reaches 0. make_shared puts the object and the block in ONE allocation.

namespace {

// Counts live objects, so tests can check that objects are deleted exactly once.
struct Counted {
    static inline int alive = 0;
    int value;

    explicit Counted(int v = 0) : value(v) { ++alive; }
    Counted(const Counted&) = delete;
    Counted& operator=(const Counted&) = delete;
    ~Counted() { --alive; }
};

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

// NO virtual destructor. std::shared_ptr still destroys a Leaf correctly through
// a shared_ptr<PlainBase>, because the deleter is captured with the REAL type
// at construction time (inside the control block). unique_ptr cannot do this.
struct PlainBase {
    int tag = 0;
};

struct Leaf : PlainBase {
    static inline int leaf_destroyed = 0;
    ~Leaf() { ++leaf_destroyed; }
};

struct TwoArgs {
    int a;
    double b;
    TwoArgs(int a, double b) : a(a), b(b) {}
};

void reset_counters() {
    Counted::alive = 0;
    Base::base_destroyed = 0;
    Derived::derived_destroyed = 0;
    Leaf::leaf_destroyed = 0;
}

}  // namespace

// ---------------------------------------------------------------------------
// 1. Basics (most of these should already pass)
// ---------------------------------------------------------------------------

TEST_CASE("default shared_ptr is null and owns nothing", "[shared_ptr]") {
    ricc::shared_ptr<int> p;
    REQUIRE(p.get() == nullptr);
    REQUIRE(p.use_count() == 0);
}

TEST_CASE("shared_ptr deletes its object exactly once", "[shared_ptr][lifetime]") {
    reset_counters();
    {
        ricc::shared_ptr<Counted> p(new Counted(5));
        REQUIRE(Counted::alive == 1);
        REQUIRE(p->value == 5);
        REQUIRE((*p).value == 5);
    }
    REQUIRE(Counted::alive == 0);
}

TEST_CASE("copies share ownership", "[shared_ptr][copy]") {
    reset_counters();
    {
        ricc::shared_ptr<Counted> a(new Counted(1));
        {
            ricc::shared_ptr<Counted> b(a);
            REQUIRE(a.get() == b.get());
            REQUIRE(a.use_count() == 2);
            REQUIRE(b.use_count() == 2);
        }
        REQUIRE(a.use_count() == 1);
        REQUIRE(Counted::alive == 1);   // b going away must NOT delete
    }
    REQUIRE(Counted::alive == 0);
}

TEST_CASE("move construction steals without touching the count", "[shared_ptr][move]") {
    reset_counters();
    {
        ricc::shared_ptr<Counted> a(new Counted(1));
        ricc::shared_ptr<Counted> b(std::move(a));
        REQUIRE(a.get() == nullptr);
        REQUIRE(a.use_count() == 0);
        REQUIRE(b.use_count() == 1);
        REQUIRE(Counted::alive == 1);
    }
    REQUIRE(Counted::alive == 0);
}

TEST_CASE("copy assignment releases the old object", "[shared_ptr][copy][lifetime]") {
    reset_counters();
    {
        ricc::shared_ptr<Counted> a(new Counted(1));
        ricc::shared_ptr<Counted> b(new Counted(2));
        b = a;
        REQUIRE(Counted::alive == 1);   // Counted(2) was deleted
        REQUIRE(b->value == 1);
        REQUIRE(a.use_count() == 2);
    }
    REQUIRE(Counted::alive == 0);
}

TEST_CASE("move assignment releases the old object", "[shared_ptr][move][lifetime]") {
    reset_counters();
    {
        ricc::shared_ptr<Counted> a(new Counted(1));
        ricc::shared_ptr<Counted> b(new Counted(2));
        b = std::move(a);
        REQUIRE(Counted::alive == 1);
        REQUIRE(a.get() == nullptr);
        REQUIRE(b->value == 1);
        REQUIRE(b.use_count() == 1);
    }
    REQUIRE(Counted::alive == 0);
}

TEST_CASE("self assignment keeps the object", "[shared_ptr][copy]") {
    reset_counters();
    {
        ricc::shared_ptr<Counted> p(new Counted(7));
        auto& alias = p;
        p = alias;
        REQUIRE(p.use_count() == 1);
        p = std::move(alias);
        REQUIRE(p.get() != nullptr);
        REQUIRE(p.use_count() == 1);
        REQUIRE(Counted::alive == 1);
    }
    REQUIRE(Counted::alive == 0);
}

TEST_CASE("swap exchanges pointers and counts", "[shared_ptr]") {
    ricc::shared_ptr<int> a(new int(1));
    ricc::shared_ptr<int> a2 = a;
    ricc::shared_ptr<int> b(new int(2));
    a.swap(b);
    REQUIRE(*a == 2);
    REQUIRE(*b == 1);
    REQUIRE(a.use_count() == 1);
    REQUIRE(b.use_count() == 2);
}

TEST_CASE("copy and move are noexcept", "[shared_ptr][traits]") {
    STATIC_REQUIRE(std::is_nothrow_copy_constructible_v<ricc::shared_ptr<int>>);
    STATIC_REQUIRE(std::is_nothrow_move_constructible_v<ricc::shared_ptr<int>>);
    STATIC_REQUIRE(std::is_nothrow_move_assignable_v<ricc::shared_ptr<int>>);
}

// ---------------------------------------------------------------------------
// 2. Interface details (std::shared_ptr behaviour)
// ---------------------------------------------------------------------------

TEST_CASE("operator bool is explicit", "[shared_ptr][bool][traits]") {
    ricc::shared_ptr<int> empty;
    ricc::shared_ptr<int> full(new int(1));
    REQUIRE_FALSE(empty);
    REQUIRE(full);
    // implicit conversion would let `int x = p;` compile
    STATIC_REQUIRE_FALSE(std::is_convertible_v<ricc::shared_ptr<int>, bool>);
}

TEST_CASE("raw-pointer constructor is explicit", "[shared_ptr][traits]") {
    // otherwise `void f(shared_ptr<int>); f(raw);` silently takes ownership
    STATIC_REQUIRE(std::is_constructible_v<ricc::shared_ptr<int>, int*>);
    STATIC_REQUIRE_FALSE(std::is_convertible_v<int*, ricc::shared_ptr<int>>);
}

TEST_CASE("constness is shallow: a const shared_ptr still gives a non-const T", "[shared_ptr][access]") {
    const ricc::shared_ptr<int> p(new int(1));
    STATIC_REQUIRE(std::is_same_v<decltype(p.get()), int*>);
    STATIC_REQUIRE(std::is_same_v<decltype(*p), int&>);
    *p = 2;
    REQUIRE(*p == 2);
}

TEST_CASE("shared_ptr<const T> works", "[shared_ptr][access]") {
    ricc::shared_ptr<const int> p(new int(3));
    STATIC_REQUIRE(std::is_same_v<decltype(*p), const int&>);
    REQUIRE(*p == 3);
}

TEST_CASE("equality compares the stored pointers", "[shared_ptr][compare]") {
    ricc::shared_ptr<int> a(new int(1));
    ricc::shared_ptr<int> b = a;
    ricc::shared_ptr<int> c(new int(1));
    REQUIRE(a == b);
    REQUIRE(a != c);   // same value, different object
}

// ---------------------------------------------------------------------------
// 3. reset
// ---------------------------------------------------------------------------

TEST_CASE("reset() drops ownership", "[shared_ptr][reset]") {
    reset_counters();
    ricc::shared_ptr<Counted> a(new Counted(1));
    ricc::shared_ptr<Counted> b = a;
    a.reset();
    REQUIRE(a.get() == nullptr);
    REQUIRE(Counted::alive == 1);   // b still owns it
    b.reset();
    REQUIRE(Counted::alive == 0);
}

TEST_CASE("reset(p) replaces the object", "[shared_ptr][reset]") {
    reset_counters();
    ricc::shared_ptr<Counted> a(new Counted(1));
    a.reset(new Counted(2));
    REQUIRE(Counted::alive == 1);
    REQUIRE(a->value == 2);
    REQUIRE(a.use_count() == 1);
    a.reset();
    REQUIRE(Counted::alive == 0);
}

// ---------------------------------------------------------------------------
// 4. nullptr  (Environment.hpp: `explicit EnvironmentFrame(shared_ptr<...> parent = nullptr)`)
// ---------------------------------------------------------------------------

TEST_CASE("shared_ptr converts implicitly from nullptr", "[shared_ptr][nullptr]") {
    ricc::shared_ptr<int> p = nullptr;
    REQUIRE(p.get() == nullptr);
    REQUIRE(p.use_count() == 0);
    STATIC_REQUIRE(std::is_convertible_v<std::nullptr_t, ricc::shared_ptr<int>>);
}

TEST_CASE("nullptr as a default argument (Environment pattern)", "[shared_ptr][nullptr]") {
    auto depth = [](ricc::shared_ptr<int> parent = nullptr) { return parent ? 1 : 0; };
    REQUIRE(depth() == 0);
    REQUIRE(depth(ricc::shared_ptr<int>(new int(0))) == 1);
}

TEST_CASE("comparisons with nullptr work in both directions", "[shared_ptr][nullptr][compare]") {
    ricc::shared_ptr<int> empty;
    ricc::shared_ptr<int> full(new int(1));
    REQUIRE(empty == nullptr);
    REQUIRE(nullptr == empty);
    REQUIRE(full != nullptr);
    REQUIRE(nullptr != full);
}

TEST_CASE("assigning nullptr releases the object", "[shared_ptr][nullptr][lifetime]") {
    reset_counters();
    ricc::shared_ptr<Counted> p(new Counted(1));
    p = nullptr;
    REQUIRE(p.get() == nullptr);
    REQUIRE(Counted::alive == 0);
}

// ---------------------------------------------------------------------------
// 5. make_shared  (used ~20 times in the compiler)
// ---------------------------------------------------------------------------

TEST_CASE("make_shared forwards constructor arguments", "[shared_ptr][make_shared]") {
    auto p = ricc::make_shared<TwoArgs>(3, 2.5);
    STATIC_REQUIRE(std::is_same_v<decltype(p), ricc::shared_ptr<TwoArgs>>);
    REQUIRE(p->a == 3);
    REQUIRE(p->b == 2.5);
    REQUIRE(p.use_count() == 1);
}

TEST_CASE("make_shared objects are deleted", "[shared_ptr][make_shared][lifetime]") {
    reset_counters();
    {
        auto a = ricc::make_shared<Counted>(1);
        auto b = a;
        REQUIRE(Counted::alive == 1);
        REQUIRE(a.use_count() == 2);
    }
    REQUIRE(Counted::alive == 0);
}

TEST_CASE("make_shared perfect-forwards move-only arguments", "[shared_ptr][make_shared]") {
    struct Holder {
        ricc::shared_ptr<int> inner;
        explicit Holder(ricc::shared_ptr<int>&& p) : inner(std::move(p)) {}
    };
    ricc::shared_ptr<int> p(new int(9));
    auto h = ricc::make_shared<Holder>(std::move(p));
    REQUIRE(p.get() == nullptr);
    REQUIRE(*h->inner == 9);
}

// ---------------------------------------------------------------------------
// 6. Derived -> Base conversions
// ---------------------------------------------------------------------------

TEST_CASE("shared_ptr<Derived> copies into shared_ptr<Base>", "[shared_ptr][convert]") {
    reset_counters();
    {
        auto d = ricc::make_shared<Derived>(4);
        ricc::shared_ptr<Base> b = d;
        REQUIRE(b->id() == 2);
        REQUIRE(d.use_count() == 2);   // same control block
        REQUIRE(b.use_count() == 2);
    }
    REQUIRE(Derived::derived_destroyed == 1);
    REQUIRE(Base::base_destroyed == 1);
}

TEST_CASE("shared_ptr<Derived> moves into shared_ptr<Base>", "[shared_ptr][convert]") {
    reset_counters();
    {
        ricc::shared_ptr<Base> b = ricc::make_shared<Derived>(4);
        REQUIRE(b->id() == 2);
        REQUIRE(b.use_count() == 1);
    }
    REQUIRE(Derived::derived_destroyed == 1);
}

TEST_CASE("converting assignment", "[shared_ptr][convert]") {
    reset_counters();
    ricc::shared_ptr<Base> b(new Base);
    auto d = ricc::make_shared<Derived>(1);
    b = d;
    REQUIRE(Base::base_destroyed == 1);   // the old plain Base
    REQUIRE(b->id() == 2);
    b = std::move(d);
    REQUIRE(b.use_count() == 1);
}

TEST_CASE("only valid pointer conversions are allowed", "[shared_ptr][convert][traits]") {
    STATIC_REQUIRE(std::is_convertible_v<ricc::shared_ptr<Derived>, ricc::shared_ptr<Base>>);
    STATIC_REQUIRE_FALSE(std::is_convertible_v<ricc::shared_ptr<Base>, ricc::shared_ptr<Derived>>);
    STATIC_REQUIRE_FALSE(std::is_convertible_v<ricc::shared_ptr<int>, ricc::shared_ptr<double>>);
}

TEST_CASE("the REAL type is destroyed even without a virtual destructor", "[shared_ptr][convert][deleter]") {
    // Hint: the control block must remember how to delete the pointer it was
    // created with (Leaf*), not the pointer the shared_ptr currently holds.
    reset_counters();
    {
        ricc::shared_ptr<PlainBase> p(new Leaf);
    }
    REQUIRE(Leaf::leaf_destroyed == 1);
    {
        ricc::shared_ptr<PlainBase> p = ricc::make_shared<Leaf>();
    }
    REQUIRE(Leaf::leaf_destroyed == 2);
}

// ---------------------------------------------------------------------------
// 7. Thread safety of the reference count
// ---------------------------------------------------------------------------

TEST_CASE("concurrent copies and destructions keep the count exact", "[shared_ptr][threads]") {
    reset_counters();
    auto p = ricc::make_shared<Counted>(1);
    {
        std::vector<std::thread> threads;
        for (int t = 0; t < 8; ++t) {
            threads.emplace_back([p] {   // copy into the thread
                for (int i = 0; i < 10'000; ++i) {
                    ricc::shared_ptr<Counted> local = p;
                    ricc::shared_ptr<Counted> moved = std::move(local);
                }
            });
        }
        for (auto& th : threads) th.join();
    }
    REQUIRE(p.use_count() == 1);
    REQUIRE(Counted::alive == 1);
    p.reset();
    REQUIRE(Counted::alive == 0);
}

TEST_CASE("the last owner on another thread deletes exactly once", "[shared_ptr][threads][lifetime]") {
    reset_counters();
    for (int round = 0; round < 200; ++round) {
        auto p = ricc::make_shared<Counted>(round);
        std::thread a([q = p] () mutable { q.reset(); });
        std::thread b([q = p] () mutable { q.reset(); });
        p.reset();
        a.join();
        b.join();
        REQUIRE(Counted::alive == 0);
    }
}

// ---------------------------------------------------------------------------
// 8. weak_ptr
// ---------------------------------------------------------------------------

TEST_CASE("default weak_ptr is expired", "[weak_ptr]") {
    ricc::weak_ptr<int> w;
    REQUIRE(w.expired());
    REQUIRE(w.use_count() == 0);
    REQUIRE(w.lock().get() == nullptr);
}

TEST_CASE("weak_ptr observes without owning", "[weak_ptr]") {
    auto p = ricc::make_shared<int>(5);
    ricc::weak_ptr<int> w = p;
    REQUIRE(p.use_count() == 1);   // weak does NOT bump the strong count
    REQUIRE(w.use_count() == 1);
    REQUIRE_FALSE(w.expired());
}

TEST_CASE("lock() returns a new owner while the object lives", "[weak_ptr][lock]") {
    auto p = ricc::make_shared<int>(5);
    ricc::weak_ptr<int> w = p;
    {
        ricc::shared_ptr<int> locked = w.lock();
        REQUIRE(locked.get() == p.get());
        REQUIRE(*locked == 5);
        REQUIRE(p.use_count() == 2);
    }
    REQUIRE(p.use_count() == 1);
}

TEST_CASE("weak_ptr expires when the last shared_ptr goes away", "[weak_ptr][lifetime]") {
    reset_counters();
    ricc::weak_ptr<Counted> w;
    {
        auto p = ricc::make_shared<Counted>(1);
        w = p;
        REQUIRE_FALSE(w.expired());
    }
    REQUIRE(Counted::alive == 0);   // object destroyed even though w is alive
    REQUIRE(w.expired());
    REQUIRE(w.use_count() == 0);
    REQUIRE(w.lock() == nullptr);
}

TEST_CASE("control block outlives the object while weak_ptrs exist", "[weak_ptr][lifetime]") {
    // Run under ASan: freeing the control block when strong hits 0 makes
    // w.expired() a use-after-free; never freeing it is a leak.
    reset_counters();
    ricc::weak_ptr<Counted> w1;
    ricc::weak_ptr<Counted> w2;
    {
        ricc::shared_ptr<Counted> p(new Counted(1));   // separate allocations
        w1 = p;
        w2 = w1;
    }
    REQUIRE(w1.expired());
    w1.reset();
    REQUIRE(w2.expired());
}   // w2 frees the block here

TEST_CASE("weak_ptr copy and move", "[weak_ptr]") {
    auto p = ricc::make_shared<int>(1);
    ricc::weak_ptr<int> a = p;
    ricc::weak_ptr<int> b(a);
    REQUIRE(b.lock() == p);
    ricc::weak_ptr<int> c(std::move(a));
    REQUIRE(c.lock() == p);
    REQUIRE(a.expired());   // moved-from is empty
    ricc::weak_ptr<int> d;
    d = c;
    d = std::move(b);
    REQUIRE(d.lock() == p);
    REQUIRE(p.use_count() == 1);
}

TEST_CASE("weak_ptr reset() stops observing", "[weak_ptr][reset]") {
    auto p = ricc::make_shared<int>(1);
    ricc::weak_ptr<int> w = p;
    w.reset();
    REQUIRE(w.expired());
    REQUIRE(p.use_count() == 1);
}

TEST_CASE("weak_ptr<Base> from shared_ptr<Derived>", "[weak_ptr][convert]") {
    auto d = ricc::make_shared<Derived>(1);
    ricc::weak_ptr<Base> w = d;
    auto b = w.lock();
    STATIC_REQUIRE(std::is_same_v<decltype(b), ricc::shared_ptr<Base>>);
    REQUIRE(b->id() == 2);
}

TEST_CASE("shared_ptr can be built from a weak_ptr", "[weak_ptr][lock]") {
    auto p = ricc::make_shared<int>(1);
    ricc::weak_ptr<int> w = p;
    ricc::shared_ptr<int> q(w);   // std throws bad_weak_ptr if expired
    REQUIRE(q == p);
    p.reset();
    q.reset();
    REQUIRE_THROWS_AS(ricc::shared_ptr<int>(w), ricc::bad_weak_ptr);
}

TEST_CASE("lock() racing the last reset never resurrects the object", "[weak_ptr][threads]") {
    // Hint: lock() must increment strong ONLY IF it is not already 0
    // (compare_exchange loop), not blindly fetch_add.
    reset_counters();
    for (int round = 0; round < 500; ++round) {
        auto p = ricc::make_shared<Counted>(round);
        ricc::weak_ptr<Counted> w = p;
        std::atomic<bool> saw_dead_object{false};
        std::thread t([&w, &saw_dead_object] {
            if (auto locked = w.lock()) {
                if (Counted::alive != 1 || locked->value < 0) saw_dead_object = true;
            }
        });
        p.reset();
        t.join();
        REQUIRE_FALSE(saw_dead_object);
        REQUIRE(Counted::alive == 0);
        REQUIRE(w.expired());
    }
}

// ---------------------------------------------------------------------------
// 9. Compiler patterns
// ---------------------------------------------------------------------------

namespace {

// Environment.hpp
struct EnvironmentFrame {
    static inline int destroyed = 0;
    const ricc::shared_ptr<EnvironmentFrame> parent;
    explicit EnvironmentFrame(ricc::shared_ptr<EnvironmentFrame> parent = nullptr)
        : parent(std::move(parent)) {}
    ~EnvironmentFrame() { ++destroyed; }
};

// object.hpp: a closure only weakly references its defining environment
// (a strong ref would form a cycle frame -> value -> closure -> frame).
struct FunctionObj {
    ricc::weak_ptr<EnvironmentFrame> env;
};

}  // namespace

TEST_CASE("environment chain is freed from the leaf up", "[shared_ptr][compiler]") {
    EnvironmentFrame::destroyed = 0;
    {
        auto global = ricc::make_shared<EnvironmentFrame>();
        auto inner = ricc::make_shared<EnvironmentFrame>(global);
        auto innermost = ricc::make_shared<EnvironmentFrame>(inner);
        REQUIRE(innermost->parent->parent == global);
        REQUIRE(global.use_count() == 2);
        global.reset();
        inner.reset();
        REQUIRE(EnvironmentFrame::destroyed == 0);   // innermost keeps them alive
    }
    REQUIRE(EnvironmentFrame::destroyed == 3);
}

TEST_CASE("closure's weak env locks while the frame lives (evaluator pattern)", "[weak_ptr][compiler]") {
    auto fn = ricc::make_shared<FunctionObj>();
    {
        auto frame = ricc::make_shared<EnvironmentFrame>();
        fn->env = frame;
        auto defining_env = fn->env.lock();   // evaluator.hpp:343
        REQUIRE(defining_env == frame);
    }
    REQUIRE(fn->env.lock() == nullptr);
}

TEST_CASE("shared_ptr from a released unique pointer (parser pattern)", "[shared_ptr][compiler]") {
    // parser.hpp: shared_ptr<BlockStatements>(parseBlockStatement().release())
    reset_counters();
    {
        Counted* raw = new Counted(3);
        auto body = ricc::shared_ptr<Counted>(raw);
        REQUIRE(body.get() == raw);
    }
    REQUIRE(Counted::alive == 0);
}

TEST_CASE("shared_ptr works as an unordered_set key (captured_environments)", "[shared_ptr][hash][compiler]") {
    // needs: template <class T> struct std::hash<ricc::shared_ptr<T>>
    auto a = ricc::make_shared<EnvironmentFrame>();
    auto b = ricc::make_shared<EnvironmentFrame>();
    std::unordered_set<ricc::shared_ptr<EnvironmentFrame>> captured;
    captured.insert(a);
    captured.insert(a);
    captured.insert(b);
    REQUIRE(captured.size() == 2);
    REQUIRE(captured.count(a) == 1);
    REQUIRE(std::hash<ricc::shared_ptr<EnvironmentFrame>>{}(a) ==
            std::hash<EnvironmentFrame*>{}(a.get()));
}

// ---------------------------------------------------------------------------
// Incomplete types (object.hpp forward-declares EnvironmentFrame)
//
// The helpers live in test_shared_ptr_incomplete.cpp, where Opaque is only
// forward-declared. If THAT file fails to compile, shared_ptr / weak_ptr need
// the full type somewhere they shouldn't (sizeof(T), `delete ptr`, ...).
// ---------------------------------------------------------------------------

// Defined here, at global scope, to match `struct Opaque;` in the helper header.
struct Opaque {
    static inline int alive = 0;
    int value;
    explicit Opaque(int v) : value(v) { ++alive; }
    ~Opaque() { --alive; }
};

TEST_CASE("an empty holder of an incomplete type is empty", "[shared_ptr][weak_ptr][incomplete][compiler]") {
    REQUIRE(holder_is_empty(Holder{}));
    REQUIRE(lock_holder(Holder{}) == nullptr);
}

TEST_CASE("pointers to an incomplete type can be copied, moved and destroyed", "[shared_ptr][weak_ptr][incomplete][compiler]") {
    Opaque::alive = 0;
    {
        auto obj = ricc::make_shared<Opaque>(5);
        Holder h{obj, obj};
        REQUIRE(obj.use_count() == 2);

        Holder copy = copy_holder(h);
        REQUIRE(obj.use_count() == 3);
        REQUIRE(lock_holder(copy) == obj);

        Holder moved = move_holder(std::move(copy));
        REQUIRE(holder_is_empty(copy));   // moved-from pointers are empty
        REQUIRE(obj.use_count() == 3);    // obj, h, moved

        Holder other;
        assign_holder(other, moved);
        REQUIRE(obj.use_count() == 4);

        auto* heap = new Holder{obj, obj};
        REQUIRE(obj.use_count() == 5);
        destroy_holder(heap);
        REQUIRE(obj.use_count() == 4);

        reset_holder(other);
        REQUIRE(holder_is_empty(other));
        REQUIRE(obj.use_count() == 3);
        REQUIRE(Opaque::alive == 1);
    }
    REQUIRE(Opaque::alive == 0);
}

TEST_CASE("the last owner can die where the type is incomplete", "[shared_ptr][incomplete][compiler]") {
    // `delete ptr` on an incomplete type compiles (GCC only warns) but skips
    // the destructor. The control block, built by make_shared where Opaque
    // was complete, has to do the deleting.
    Opaque::alive = 0;
    Holder h{ricc::make_shared<Opaque>(2), {}};
    REQUIRE(Opaque::alive == 1);
    reset_holder(h);                      // the only owner, released in the other file
    REQUIRE(Opaque::alive == 0);

    auto* heap = new Holder{ricc::make_shared<Opaque>(3), {}};
    destroy_holder(heap);                 // destructor of the only owner
    REQUIRE(Opaque::alive == 0);
}

TEST_CASE("a weak_ptr to an incomplete type expires when the object dies", "[weak_ptr][incomplete][compiler]") {
    Opaque::alive = 0;
    Holder h;
    {
        auto obj = ricc::make_shared<Opaque>(1);
        h.weak = obj;
        REQUIRE(lock_holder(h) == obj);
    }
    REQUIRE(Opaque::alive == 0);
    REQUIRE(lock_holder(h) == nullptr);
    REQUIRE(holder_is_empty(h));
}

// ---------------------------------------------------------------------------
// shared_ptr inside ricc::variant (object.hpp's Value)
// ---------------------------------------------------------------------------

namespace {
using FnValue = ricc::variant<std::monostate, int, ricc::shared_ptr<FunctionObj>>;
using FnPtr = ricc::shared_ptr<FunctionObj>;
}

TEST_CASE("a Value holds a function pointer", "[shared_ptr][variant][compiler]") {
    // evaluator.hpp: Value(std::make_shared<FunctionObj>(std::move(func)))
    FunctionObj func;
    FnValue v{ricc::make_shared<FunctionObj>(std::move(func))};
    REQUIRE(ricc::holds_alternative<FnPtr>(v));
    const auto* callable = ricc::get_if<FnPtr>(&v);
    REQUIRE(callable != nullptr);
    REQUIRE(*callable);
    REQUIRE(callable->use_count() == 1);
}

TEST_CASE("copying a Value shares the function, moving does not add an owner", "[shared_ptr][variant][compiler]") {
    auto fn = ricc::make_shared<FunctionObj>();
    {
        FnValue a{fn};
        REQUIRE(fn.use_count() == 2);
        FnValue b{a};
        REQUIRE(fn.use_count() == 3);
        FnValue c{std::move(b)};
        REQUIRE(fn.use_count() == 3);
        a = FnValue{1};                   // switching away releases it
        REQUIRE(fn.use_count() == 2);
        b = c;
        REQUIRE(fn.use_count() == 3);
    }
    REQUIRE(fn.use_count() == 1);
}

TEST_CASE("Values holding functions compare by pointer", "[shared_ptr][variant][compare][compiler]") {
    auto fn = ricc::make_shared<FunctionObj>();
    const FnValue a{fn};
    const FnValue b{fn};
    const FnValue other{ricc::make_shared<FunctionObj>()};
    REQUIRE(a == b);
    REQUIRE(a != other);
    REQUIRE(a != FnValue{});
}

TEST_CASE("the function is freed when the last Value holding it goes away", "[shared_ptr][variant][lifetime][compiler]") {
    ricc::weak_ptr<FunctionObj> watch;
    {
        FnValue v{ricc::make_shared<FunctionObj>()};
        watch = ricc::get<FnPtr>(v);
        FnValue copy{v};
        v = FnValue{};
        REQUIRE_FALSE(watch.expired());   // copy still owns it
    }
    REQUIRE(watch.expired());
}

TEST_CASE("calling a Value like the evaluator does", "[shared_ptr][weak_ptr][variant][compiler]") {
    // evaluator.hpp: get_if the callee, copy the pointer, check it, lock env
    auto frame = ricc::make_shared<EnvironmentFrame>();
    auto fn = ricc::make_shared<FunctionObj>();
    fn->env = frame;
    const FnValue callee{fn};

    const auto* callable = ricc::get_if<FnPtr>(&callee);
    REQUIRE(callable != nullptr);
    auto function = *callable;
    REQUIRE(function);
    REQUIRE(fn.use_count() == 3);         // fn, callee, function
    auto defining_env = function->env.lock();
    REQUIRE(defining_env == frame);

    frame.reset();
    defining_env.reset();
    REQUIRE_FALSE(function->env.lock());  // "Function environment has expired"
}

// ---------------------------------------------------------------------------
// Environment.hpp: parent ? parent->frame : nullptr
// ---------------------------------------------------------------------------

TEST_CASE("choosing between a shared_ptr and nullptr with ?:", "[shared_ptr][nullptr][compiler]") {
    struct Env {
        ricc::shared_ptr<EnvironmentFrame> frame;
    };
    auto parent_frame = [](Env* parent) -> ricc::shared_ptr<EnvironmentFrame> {
        return parent ? parent->frame : nullptr;
    };
    Env outer{ricc::make_shared<EnvironmentFrame>()};
    REQUIRE(parent_frame(&outer) == outer.frame);
    REQUIRE(outer.frame.use_count() == 1);   // the returned copy is gone again
    REQUIRE(parent_frame(nullptr) == nullptr);
}
