/*
 * StackTimer.h
 *
 * Created on: Jul 25, 2025
 * Author: admin
 *
 * Static stack timer with fixed or dynamic interval
 */
#ifndef STM32_TOOLS_TIME_INTERVAL_STACKTIMER_H_
#define STM32_TOOLS_TIME_INTERVAL_STACKTIMER_H_

#include "time/interval_policy.h"
#include <utility>

//------------------------------------------------------------------------------
// Unified StackITimer:
//   - StackITimer<>            => dynamic (interval set at runtime)
//   - StackITimer<100>         => static 100 (compile-time)
//   - Default interval type    => unsigned int (reg)
//------------------------------------------------------------------------------

template<auto Interval = 0u, typename T = reg>
class StackITimer : public std::conditional_t<(Interval == T{0}),
                                              DynamicIntervalPolicy<T>,
                                              StaticIntervalPolicy<T, Interval>>
{
    using Policy = std::conditional_t<(Interval == T{0}),
                                      DynamicIntervalPolicy<T>,
                                      StaticIntervalPolicy<T, Interval>>;

    static_assert(std::is_integral_v<T>, "StackITimer requires integral T");
    static_assert(std::is_unsigned_v<T>, "StackITimer: T must be unsigned");

    // Trait: does Policy provide a .setInterval(T) member?
    template<class P, class X, class = void>
    struct has_set_interval : std::false_type {};
    template<class P, class X>
    struct has_set_interval<P, X, std::void_t<decltype(std::declval<P&>().setInterval(std::declval<X>()))>>
        : std::true_type {};
    static constexpr bool policy_can_set_interval = has_set_interval<Policy, T>::value;

public:
    using value_type = T;
    static constexpr bool is_static_interval  = (Interval != T{0});
    static constexpr bool is_dynamic_interval = !is_static_interval;

    // inherit policy constructors (dynamic policy may accept initial interval)
    using Policy::Policy;

    // bring getInterval into scope (works for both static and dynamic policies)
    using Policy::getInterval;
    //using Policy::setInterval; // commit this else --> compile error

    [[nodiscard]] constexpr bool isExpired(const value_type now) const noexcept {
        return (now - _lastTime) >= Policy::getInterval();
    }

    [[nodiscard]] constexpr value_type timeLeft(const value_type now) const noexcept {
        const value_type elapsed  = now - _lastTime;
        const value_type interval = Policy::getInterval();
        return (elapsed >= interval) ? value_type{0} : (interval - elapsed);
    }

    // Restart from now
    constexpr void next(const value_type now) noexcept { _lastTime = now; }

    // Restart with new interval:
    //  - static mode -> hard error
    //  - dynamic mode:
    //      - if policy has setInterval(T) -> use it
    //      - else -> hard error with clear message
    constexpr void next(const value_type now, const value_type interval) noexcept {
        if constexpr (is_static_interval) {
            static_assert(!is_static_interval,
                          "StackITimer: cannot set interval on a static timer");
        } else {
            if constexpr (policy_can_set_interval) {
            	_lastTime = now;
                // call only when the policy actually has this member
                Policy::setInterval(interval);
            } else {
                static_assert(policy_can_set_interval,
                              "StackITimer: selected policy does not provide setInterval(T)");
            }
        }
    }

    [[nodiscard]] constexpr value_type elapsed(const value_type now) const noexcept {
        return now - _lastTime;
    }

    // Assignment operator resets timer
    constexpr StackITimer& operator=(const value_type now) noexcept {
        next(now);
        return *this;
    }

    [[nodiscard]] static constexpr bool isAvailable() noexcept { return true; }

private:
    value_type _lastTime = 0;
};


#endif /* STM32_TOOLS_TIME_INTERVAL_STACKTIMER_H_ */
