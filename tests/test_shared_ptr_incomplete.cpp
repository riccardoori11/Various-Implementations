// Helpers for the "[incomplete]" tests in test_shared_ptr.cpp.
//
// In this file Opaque is ONLY forward-declared, like EnvironmentFrame in the
// compiler's object.hpp, where FunctionObj holds a weak_ptr<EnvironmentFrame>.
// Every function below copies, moves or destroys a shared_ptr / weak_ptr
// while Opaque is incomplete.
//
// It has to be a separate file: inside one file the compiler usually
// instantiates templates at the end, after Opaque is defined, so a
// shared_ptr that does sizeof(T) or `delete ptr` would still compile there.
//
// If this file fails to compile with "invalid use of incomplete type", the
// shared_ptr / weak_ptr code being instantiated needs the full type. It has to
// go through the control block (which was built where T was complete) instead.

#include "shared_ptr.hpp"
#include "test_shared_ptr_incomplete.hpp"

#include <utility>

Holder copy_holder(const Holder& h) {
    return h;                       // copy-constructs both pointers
}

Holder move_holder(Holder&& h) {
    return std::move(h);            // move-constructs both pointers
}

void assign_holder(Holder& to, const Holder& from) {
    to = from;                      // copy assignment releases the old values
}

void destroy_holder(Holder* h) {
    delete h;                       // runs both destructors
}

bool holder_is_empty(const Holder& h) {
    return !h.strong && h.weak.expired() && h.strong == nullptr;
}

ricc::shared_ptr<Opaque> lock_holder(const Holder& h) {
    return h.weak.lock();           // evaluator.hpp: function->env.lock()
}

void reset_holder(Holder& h) {
    h.strong.reset();
    h.weak.reset();
}
