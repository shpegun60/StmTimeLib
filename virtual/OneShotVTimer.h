/*
 * OneShotVTimer.h
 *
 *  Created on: Jul 30, 2025
 *      Author: admin
 */

#ifndef STM32_TOOLS_TIME_VIRTUAL_ONESHOTVTIMER_H_
#define STM32_TOOLS_TIME_VIRTUAL_ONESHOTVTIMER_H_

#include "StackVTimer.h"
#include "irq/IRQGuard.h"

//------------------------------------------------------------------------------
// OneShotVTimer:
//   - OneShotVTimer<>         => dynamic (interval set at runtime)
//   - OneShotVTimer<100u>     => static interval = 100
//   - Default interval type   => unsigned int
//------------------------------------------------------------------------------

template<auto Interval = 0u, typename T = reg>
class OneShotVTimer
    : public StackVTimer<Interval, T>
{
    using Base = StackVTimer<Interval, T>;

public:
    // expose type to users
    using value_type = T;

    // Inherit constructor (for dynamic mode can pass iv, ignored if static)
    using Base::Base;

    /*
     * VTimer interface
     */

    /// @brief Returns true only once after expiration
    [[nodiscard]]
	 constexpr bool isExpired() noexcept {
        if (!_started || _expired) {
            return false;
        }
        if (Base::isExpired()) {
            _expired = true;
            return true;
        }
        return false;
    }

    /*
     * StackVTimer interface
     */
    constexpr void next(const value_type now) noexcept {
		Base::next(now);
		_expired = false;
    }

    // Restart + set interval — compile-time error for static interval.
    constexpr void next(const value_type now, const value_type interval) noexcept {
        if constexpr (Base::is_static_interval) {
            static_assert(!Base::is_static_interval,
                          "Cannot set interval on a static OneShotVTimer");
        }
        Base::next(now, interval);
        _expired = false;
    }

    // Convenience assignment
    constexpr OneShotVTimer& operator=(const value_type now) noexcept {
        next(now);
        return *this;
    }

    /*
     * OneShotVTimer interface
     */

    /// @brief Start the one-shot timer from given time
    constexpr void start(const value_type now) noexcept {
        if (Base::isExpired()) {
            Base::next(now);
        } else {
            IRQGuard guard;
            Base::next(now);
        }

        _started = true;
        _expired = false;
    }

    // Start + set interval (compile-time error for static interval)
    constexpr void start(const value_type now, const value_type interval) noexcept {
        if constexpr (Base::is_static_interval) {
            static_assert(!Base::is_static_interval,
                          "Cannot set interval on a static OneShotVTimer");
        }

        if (Base::isExpired()) {
        	Base::next(now, interval);
        } else {
            IRQGuard guard;
            Base::next(now, interval);
        }

        _expired = false;
        _started = true;
    }

    /// @brief Stop the timer (disable further expiration until start())
    constexpr void stop() noexcept {
        _started = false;
        _expired = true;
        Base::stop();  // Assumes StackVTimer provides stop()
    }

private:
    bool _started = false;
    bool _expired = true;
};

#endif /* STM32_TOOLS_TIME_VIRTUAL_ONESHOTVTIMER_H_ */
