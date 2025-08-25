/*
 * StackVTimer.h
 *
 *  Created on: Jul 29, 2025
 *      Author: admin
 */

#ifndef STM32_TOOLS_TIME_VIRTUAL_STACKVTIMER_H_
#define STM32_TOOLS_TIME_VIRTUAL_STACKVTIMER_H_

#include "time/interval_policy.h"
#include "VTimer.h"
#include <limits>

//------------------------------------------------------------------------------
// Unified StackVTimer:
//   - StackVTimer<>         => dynamic (interval set at runtime, ctor takes iv)
//   - StackVTimer<100u>     => static (compile-time constant interval, no-arg ctor)
//   - Default interval type  => unsigned int (reg)
//------------------------------------------------------------------------------

template<auto Interval = 0u, class T = reg>
class StackVTimer
    : public VTimer,
      public std::conditional_t< Interval == T{0},
                                    DynamicIntervalPolicy<T>,
                                    StaticIntervalPolicy<T, Interval> >
{
    using Policy = std::conditional_t< Interval == T{0},
                                        DynamicIntervalPolicy<T>,
                                        StaticIntervalPolicy<T, Interval> >;
    using Base = VTimer;


    static_assert(std::is_integral_v<T>,      "StackVTimer: T must be integral");
    static_assert(std::is_unsigned_v<T>,      "StackVTimer: T must be unsigned");

    // Trait: does Policy provide a .setInterval(T) member?
    template<class P, class X, class = void>
    struct has_set_interval : std::false_type {};
    template<class P, class X>
    struct has_set_interval<P, X, std::void_t<decltype(std::declval<P&>().setInterval(std::declval<X>()))>>
        : std::true_type {};
    static constexpr bool policy_can_set_interval = has_set_interval<Policy, T>::value;


    static constexpr bool is_static_interval  = (Interval != T{0});
    static constexpr bool is_dynamic_interval = !is_static_interval;

    // Ensure static Interval fits into T
    static_assert(!is_static_interval
                  || static_cast<unsigned long long>(Interval)
                         <= static_cast<unsigned long long>(std::numeric_limits<T>::max()),
                  "StackVTimer: Interval does not fit into T");

public:

    // expose type to users
    using value_type = T;

    // Dynamic mode ctor: user provides initial interval 'iv'
    template<auto I = Interval, typename U = value_type,
             std::enable_if_t<(I == U{0}), int> = 0>
    constexpr explicit StackVTimer(const U iv = U{}) noexcept
        : Base(iv), Policy(iv) {}

    // Static mode ctor: no parameter; initialize VTimer with compile-time Interval
    template<auto I = Interval, typename U = value_type,
             std::enable_if_t<(I != U{0}), int> = 0>
    constexpr StackVTimer() noexcept
        : Base(static_cast<U>(I)), Policy() {}

    /*
     * VTimer-facing adapter
     */

    // Restart countdown using policy interval, stamp 'now' for elapsed()
    constexpr void next(const value_type now) noexcept {
    	 _lastTime = now;
        Base::next(Policy::getInterval());
    }

    // Restart with new interval:
    //  - static mode -> hard error
    //  - dynamic mode:
    //      - if policy has setInterval(T) -> use it
    //      - else -> hard error with clear message
    constexpr void next(const value_type now, const value_type interval) noexcept {
        if constexpr (is_static_interval) {
            static_assert(!is_static_interval,
                          "StackVTimer: cannot set interval on a static timer");
        } else {
            if constexpr (policy_can_set_interval) {
            	_lastTime = now;
                // call only when the policy actually has this member
                Policy::setInterval(interval);
                Base::next(interval);
            } else {
                static_assert(policy_can_set_interval,
                              "StackVTimer: selected policy does not provide setInterval(T)");
            }
        }
    }

    /*
     * StackVTimer interface
     */

    // Elapsed ticks since last reset
    [[nodiscard]] constexpr value_type elapsed(const value_type now) const noexcept {
        return now - _lastTime; // unsigned wrap-around semantics
    }

    // Convenience assignment = next(now)
    constexpr StackVTimer& operator=(const value_type now) noexcept {
        next(now);
        return *this;
    }

    [[nodiscard]] static constexpr bool isAvailable() noexcept { return true; }

private:
    value_type _lastTime = value_type{};
};

#endif /* STM32_TOOLS_TIME_VIRTUAL_STACKVTIMER_H_ */

