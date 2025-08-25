/*
 * interval_policy.h
 *
 *  Created on: Jul 29, 2025
 *      Author: admin
 */

#ifndef STM32_TOOLS_TIME_INTERVAL_POLICY_H_
#define STM32_TOOLS_TIME_INTERVAL_POLICY_H_

#include "interval_depency.h"
#include <type_traits>

//------------------------------------------------------------------------------
// Policy for static interval (Interval != 0)
//------------------------------------------------------------------------------
template<typename T, T Interval>
struct StaticIntervalPolicy {
    static_assert(std::is_integral_v<T>, "StaticIntervalPolicy requires integral T");
    static_assert(Interval > T{0}, "StaticIntervalPolicy requires Interval > 0");

    constexpr StaticIntervalPolicy() noexcept = default;
    constexpr StaticIntervalPolicy(const T) noexcept = delete;

    [[nodiscard]] static constexpr T getInterval() noexcept { return Interval; }
};

//------------------------------------------------------------------------------
// Policy for dynamic interval  (Interval == 0)
//------------------------------------------------------------------------------
template<typename T>
struct DynamicIntervalPolicy {
    static_assert(std::is_integral_v<T>, "DynamicIntervalPolicy requires integral T");

    T interval;

    constexpr DynamicIntervalPolicy(const T iv = T{}) noexcept
        : interval(normalize(iv)) {}

    constexpr void setInterval(const T iv) noexcept { interval = normalize(iv); }

    [[nodiscard]] constexpr T getInterval() const noexcept { return interval; }

private:
    static constexpr T normalize(const T iv) noexcept {
        if constexpr (std::is_signed_v<T>) {
            return (iv < T{0}) ? -iv : iv;
        } else {
            return iv;
        }
    }
};

#endif /* STM32_TOOLS_TIME_INTERVAL_POLICY_H_ */
