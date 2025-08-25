/*
 * OneShotITimer.h
 *
 *  Created on: Jul 30, 2025
 *      Author: admin
 */

#ifndef STM32_TOOLS_TIME_INTERVAL_ONESHOTITIMER_H_
#define STM32_TOOLS_TIME_INTERVAL_ONESHOTITIMER_H_

#include "StackITimer.h"

//------------------------------------------------------------------------------
// OneShotITimer:
//   - OneShotITimer<>          => dynamic (interval set at runtime)
//   - OneShotITimer<100u>      => static interval = 100
//   - Default interval type    => unsigned int
//------------------------------------------------------------------------------

template<auto Interval = 0u, typename T = reg>
class OneShotITimer
    : public StackITimer<Interval, T>
{
    using Base = StackITimer<Interval, T>;

public:
    // expose type to users
    using value_type = T;

    // Inherit constructor (for dynamic mode can pass iv, ignored if static)
    using Base::Base;

    /*
     * StackITimer interface
     */

    // Returns true only once, when the interval has expired.
    // Note: not const because it mutates `_expired`.
    [[nodiscard]] constexpr bool isExpired(const value_type now) noexcept {
        if (!_started || _expired) {
            return false;
        }
        if (Base::isExpired(now)) {
            _expired = true;
            return true;
        }
        return false;
    }

    // Restart from now — does not change `_started` requirement: you explicitly start.
    // This mirrors your desired behavior: timer must be started explicitly.
    constexpr void next(const value_type now) noexcept {
        Base::next(now);
        _expired = false;
    }


    // Restart + set interval — compile-time error for static interval.
    constexpr void next(const value_type now, const value_type interval) noexcept {
        if constexpr (Base::is_static_interval) {
            static_assert(!Base::is_static_interval,
                          "Cannot set interval on a static OneShotITimer");
        }
        Base::next(now, interval);
        _expired = false;
    }

    // Start (explicit): sets last time and marks started.
    constexpr void start(const value_type now) noexcept {
        Base::next(now);
        _expired = false;
        _started = true;
    }

    // Start + set interval (compile-time error for static interval)
    constexpr void start(const value_type now, const value_type interval) noexcept {
        if constexpr (Base::is_static_interval) {
            static_assert(!Base::is_static_interval,
                          "Cannot set interval on a static OneShotITimer");
        }
        Base::next(now, interval);
        _expired = false;
        _started = true;
    }

    /*
     * OneShotITimer interface
     */

    constexpr void stop() noexcept {
        _started = false;
        _expired = true;
    }

    [[nodiscard]] constexpr bool isStopped() const noexcept { return !_started; }

    // Convenience assignment (does not change started state)
     constexpr OneShotITimer& operator=(const value_type now) noexcept {
         next(now);
         return *this;
     }

private:
    bool _started = false;
    bool _expired = true;
};


#endif /* STM32_TOOLS_TIME_INTERVAL_ONESHOTITIMER_H_ */
