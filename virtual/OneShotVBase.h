/*
 * OneShotVBase.h
 *
 *  Created on: Jul 30, 2025
 *      Author: admin
 */

#ifndef STM32_TOOLS_TIME_VIRTUAL_ONESHOTVBASE_H_
#define STM32_TOOLS_TIME_VIRTUAL_ONESHOTVBASE_H_

#include "OneShotVTimer.h"

//------------------------------------------------------------------------------
// OneShotVBase<Interval, Policy>
//  - adapter over OneShotVTimer that pulls time from Policy::now()
//------------------------------------------------------------------------------

template<auto Interval, class Policy>
class OneShotVBase : public OneShotVTimer<Interval, typename Policy::type_t>
{
    using type_t = typename Policy::type_t;
    using Base = OneShotVTimer<Interval, type_t>;

    static_assert(std::is_integral_v<type_t>,
                  "OneShotVBase: Policy::type_t must be integral");

    // Detect: Policy::now() exists and is convertible to type_t (optionally noexcept)
    template<class P>
    static constexpr auto has_now_impl(int) -> std::bool_constant<
        std::is_convertible_v<decltype(P::now()), type_t>
        // enable if you want to enforce noexcept:
         && noexcept(P::now())
    >;
    template<class> static constexpr std::false_type has_now_impl(...);

    static_assert(decltype(has_now_impl<Policy>(0))::value,
                  "OneShotVBase: Policy must provide static now() convertible to type_t (optionally noexcept)");

public:
    using value_type = type_t;

    // Inherit constructor (for dynamic mode can pass iv, ignored if static)
    using Base::Base;

    // forbid accidental assignment-from-time
    OneShotVBase& operator=(const type_t) = delete;

    /*
     * OneShotVTimer interface
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


    // Explicit start
    constexpr void start() noexcept(noexcept(Policy::now())) {
        Base::start(Policy::now());
    }

    // Explicit start + set interval
    constexpr void start(const value_type interval) noexcept(noexcept(Policy::now())) {
        Base::start(Policy::now(), interval);
    }

    /*
     * StackVTimer interface
     */

    // Elapsed since last reset/next
    [[nodiscard]]
    constexpr value_type elapsed() const noexcept(noexcept(Policy::now())) {
        return Base::elapsed(Policy::now());
    }
};

#endif /* STM32_TOOLS_TIME_VIRTUAL_ONESHOTVBASE_H_ */
