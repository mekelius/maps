#ifndef __DEFERRED_BOOL_HH
#define __DEFERRED_BOOL_HH

namespace Maps {

// class for booleans we may or may not know yet
enum class DeferredBool {
    true_,
    false_,
    maybe_,
};

constexpr DeferredBool db_true = DeferredBool::true_;
constexpr DeferredBool db_false = DeferredBool::false_;
constexpr DeferredBool db_maybe = DeferredBool::maybe_;

} // namespace Maps

#endif