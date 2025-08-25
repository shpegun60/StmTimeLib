/*
 * VTimeBase.h
 *
 *  Created on: Jul 29, 2025
 *      Author: admin
 */

#ifndef STM32_TOOLS_TIME_VIRTUAL_VTIMEBASE_H_
#define STM32_TOOLS_TIME_VIRTUAL_VTIMEBASE_H_

#include "StackVTimer.h"

//------------------------------------------------------------------------------
// VTimeBase<Interval, Policy>
//  - adapter over StackVTimer that pulls time from Policy::now()
//------------------------------------------------------------------------------

template<auto Interval, class Policy>
class VTimeBase : public StackVTimer<Interval, typename Policy::type_t>
{
    using type_t = typename Policy::type_t;
    using Base = StackVTimer<Interval, type_t>;

    static_assert(std::is_integral_v<type_t>, "VTimeBase: Policy::type_t must be integral");


    // Detect: Policy::now() exists and is convertible to type_t (optionally noexcept)
    template<class P>
    static constexpr auto has_now_impl(int) -> std::bool_constant<
        std::is_convertible_v<decltype(P::now()), type_t>
        // enable if you want to enforce noexcept:
         && noexcept(P::now())
    >;
    template<class> static constexpr std::false_type has_now_impl(...);

    static_assert(decltype(has_now_impl<Policy>(0))::value,
                  "VTimeBase: Policy must provide static now() convertible to type_t (optionally noexcept)");

public:
    using value_type = type_t;

    // Dynamic mode ctor: user provides initial interval 'iv'
    template<auto I = Interval, typename U = value_type,
             std::enable_if_t<(I == U{0}), int> = 0>
    constexpr explicit VTimeBase(const U iv = U{}) noexcept
        : Base(iv) { next(); } // auto-start

    // Static mode ctor: no parameter; initialize VTimer with compile-time Interval
    template<auto I = Interval, typename U = value_type,
             std::enable_if_t<(I != U{0}), int> = 0>
    constexpr VTimeBase() noexcept
        : Base() { next(); } // auto-start


    // forbid accidental assignment-from-time
    VTimeBase& operator=(const value_type) = delete;

    /*
     * StackVTimer interface
     */

    // Restart from now
    constexpr void next() noexcept(noexcept(Policy::now())) {
        Base::next(Policy::now());
    }

    // next(now, interval) replacement: restart+set interval (only for dynamic)
    // If static interval, this will static_assert at compile time.
    constexpr void next(value_type newInterval) noexcept(noexcept(Policy::now())) {
		 if constexpr (Base::is_static_interval) {
			static_assert(!Base::is_static_interval,
						  "ITimeBase::next(interval): cannot set interval on static timer");
		} else {
			Base::next(Policy::now(), newInterval);
		}
    }

    // Elapsed since last reset/next
    [[nodiscard]]
    constexpr value_type elapsed() const noexcept(noexcept(Policy::now())) {
        return Base::elapsed(Policy::now());
    }
};

#endif /* STM32_TOOLS_TIME_VIRTUAL_VTIMEBASE_H_ */
