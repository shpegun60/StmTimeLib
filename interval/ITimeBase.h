/*
 * ITimeBase.h
 *
 *  Created on: Jul 29, 2025
 *      Author: admin
 */

#ifndef STM32_TOOLS_TIME_INTERVAL_ITIMEBASE_H_
#define STM32_TOOLS_TIME_INTERVAL_ITIMEBASE_H_

#include "StackITimer.h"     // unified StackITimer template

//------------------------------------------------------------------------------
// ITimeBase<Interval, Policy>
//  - thin adapter around StackITimer that calls Policy::now() internally
//  - maximized for inlining and compile-time checking
//------------------------------------------------------------------------------

template<auto Interval, class Policy>
class ITimeBase : public StackITimer<Interval, typename Policy::type_t>
{
    using type_t = typename Policy::type_t;
    using Base = StackITimer<Interval, type_t>;

    static_assert(std::is_integral_v<type_t>,
                  "ITimeBase: Policy::type_t must be an integral type");

    // Detection: Policy::now() exists, is convertible to type_t, and (optional) noexcept
    template<class P>
    static constexpr auto has_now_impl(int) -> std::bool_constant<
        std::is_convertible_v<decltype(P::now()), type_t>
        // if you want to require noexcept, uncomment:
         && noexcept(P::now())
    >;
    template<class>
    static constexpr std::false_type has_now_impl(...);

    static_assert(decltype(has_now_impl<Policy>(0))::value,
                      "ITimeBase: Policy must provide static now() convertible to type_t (or optional noexcept)");

public:
    // expose type to users
    using value_type = type_t;

    // 1) DYNAMIC (Interval == 0):
    //    - Accepts an initial interval for the runtime policy.
    //    - Immediately arms the timer by stamping the current time via Policy::now().
    //      This guarantees the instance is usable right after construction.
    template<auto I = Interval, typename U = value_type,
             std::enable_if_t<(I == U{0}), int> = 0>
    explicit ITimeBase(const U iv = U{}) noexcept(noexcept(Policy::now()))
        : Base(iv) { next(); }  // auto-start

    // 2) STATIC (Interval != 0):
    //    - Interval is a compile-time constant; nothing to configure.
    //    - Immediately arms the timer on construction, same reasoning as above.
    template<auto I = Interval, typename U = value_type,
             std::enable_if_t<(I != U{0}), int> = 0>
    ITimeBase() noexcept(noexcept(Policy::now()))
        : Base() { next(); }  // auto-start

    // Delete assignment-from-value inherited from StackITimer on purpose:
    // user must call next()/start() explicitly, not assign accidentally.
    ITimeBase& operator=(const value_type) = delete;

    /*
     * StackITimer interface
     */

    // ---- thin forwarding methods, fully constexpr/noexcept where possible ----

    // Check if the interval has expired isExpired() -> uses Policy::now() internally
    [[nodiscard]]
    constexpr bool isExpired() const noexcept(noexcept(Policy::now())) {
        return Base::isExpired(Policy::now());
    }

    // get time left timeLeft() -> uses Policy::now()
    [[nodiscard]]
    constexpr value_type timeLeft() const noexcept(noexcept(Policy::now())) {
        return Base::timeLeft(Policy::now());
    }

    // next() -> Restart the timer from now
    constexpr void next() noexcept(noexcept(Policy::now())) {
        Base::next(Policy::now());
    }

    // next(now, interval) replacement: restart+set interval (only for dynamic)
    // If static interval, this will static_assert at compile time.
    constexpr void next(const value_type newInterval) noexcept(noexcept(Policy::now())) {
        if constexpr (Base::is_static_interval) {
            static_assert(!Base::is_static_interval,
                          "ITimeBase::next(interval): cannot set interval on static timer");
        } else {
            Base::next(Policy::now(), newInterval);
        }
    }


    // elapsed ticks since last reset using Policy::now()
    [[nodiscard]]
    constexpr value_type elapsed() const noexcept(noexcept(Policy::now())) {
        return Base::elapsed(Policy::now());
    }
};

#endif /* STM32_TOOLS_TIME_INTERVAL_ITIMEBASE_H_ */
