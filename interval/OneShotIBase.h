/*
 * OneShotIBase.h
 *
 *  Created on: Jul 30, 2025
 *      Author: admin
 */

#ifndef STM32_TOOLS_TIME_INTERVAL_ONESHOTIBASE_H_
#define STM32_TOOLS_TIME_INTERVAL_ONESHOTIBASE_H_

#include "OneShotITimer.h"     // unified StackITimer template
#include <utility>

//------------------------------------------------------------------------------
// ITimeBase<Interval, Policy>:
//------------------------------------------------------------------------------
template<auto Interval, class Policy>
class OneShotIBase : public OneShotITimer<Interval, typename Policy::type_t>
{
    using type_t = typename Policy::type_t;
    using Base = OneShotITimer<Interval, type_t>;

    static_assert(std::is_integral_v<type_t>,
                  "OneShotIBase: Policy::type_t must be integral");

    // Policy::now() must exist and be convertible to type_t
    template<class P>
    static constexpr auto has_now_impl(int) -> std::bool_constant<
        std::is_convertible_v<decltype(P::now()), type_t>
    	// if you want to require noexcept, uncomment:
         && noexcept(P::now())
    >;
    template<class>
    static constexpr std::false_type has_now_impl(...);
    static_assert(decltype(has_now_impl<Policy>(0))::value,
                  "OneShotIBase: Policy must provide static now() convertible to type_t (or optional noexcept)");

public:
    // expose type to users
    using value_type = type_t;

    // Inherit constructor: in dynamic mode, initial interval can be passed; ignored in static mode
    using Base::Base;

    // forbid accidental assignment-from-time
    OneShotIBase& operator=(const value_type) = delete;

    /*
     * OneShotITimer interface
     */

    // Return true only once when expired (Base mutates internal state)
    [[nodiscard]]
    constexpr bool isExpired() noexcept(noexcept(Policy::now())) {
        return Base::isExpired(Policy::now());
    }

    // Restart from now (doesn't auto-start)
    constexpr void next() noexcept(noexcept(Policy::now())) {
        Base::next(Policy::now());
    }

    // Restart + set interval (delegates to Base to keep its compile-time checks)
    constexpr void next(const value_type interval) noexcept(noexcept(Policy::now())) {
        Base::next(Policy::now(), interval);
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
     * StackITimer interface
     */

    // Time left / elapsed with now()
	[[nodiscard]]
	constexpr value_type timeLeft() const noexcept(noexcept(Policy::now())) {
		return Base::timeLeft(Policy::now());
	}


    // Return elapsed ticks since last reset/next
    [[nodiscard]]
    constexpr value_type elapsed() const noexcept(noexcept(Policy::now())) {
        return Base::elapsed(Policy::now());
    }
};

#endif /* STM32_TOOLS_TIME_INTERVAL_ONESHOTIBASE_H_ */
