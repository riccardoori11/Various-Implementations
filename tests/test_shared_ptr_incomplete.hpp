#pragma once

// Shared between test_shared_ptr.cpp (where Opaque is defined) and
// test_shared_ptr_incomplete.cpp (where it is not). See the comment at the top
// of test_shared_ptr_incomplete.cpp.

#include "shared_ptr.hpp"

struct Opaque;   // incomplete here, like `struct EnvironmentFrame;` in object.hpp

// Shaped like the compiler's FunctionObj, which holds weak_ptr<EnvironmentFrame>.
struct Holder {
    ricc::shared_ptr<Opaque> strong;
    ricc::weak_ptr<Opaque> weak;
};

// Defined in test_shared_ptr_incomplete.cpp, where Opaque is incomplete.
Holder copy_holder(const Holder& h);
Holder move_holder(Holder&& h);
void assign_holder(Holder& to, const Holder& from);
void destroy_holder(Holder* h);
bool holder_is_empty(const Holder& h);
ricc::shared_ptr<Opaque> lock_holder(const Holder& h);
void reset_holder(Holder& h);
