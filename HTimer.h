/*
 * Timer.h
 *
 *  Created on: Jul 29, 2025
 *      Author: admin
 */

#ifndef STM32_TOOLS_TIME_HTIMER_H_
#define STM32_TOOLS_TIME_HTIMER_H_

#include "interval_depency.h"

#ifdef HAL_TIM_MODULE_ENABLED

class HTimer {
    STATIC_CLASS(HTimer);
public:
    using type_t = u32;

    /**
     * @brief Gets the current timer counter value.
     * @return Current timer tick value.
     */

    // Called once with htim — stores it and starts the timer
    static bool attachTimer(TIM_HandleTypeDef* const htim);

    // Use this for all future calls (after the first one)
    static inline type_t now() noexcept { return _htim ? __HAL_TIM_GET_COUNTER(_htim) : 0; }
    /**
     * @brief Checks if the timer is available.
     * @return true if the timer is attached, false otherwise.
     */
    inline static bool isAvailable() noexcept { return _htim != nullptr; }

private:
    /**
     * @brief Starts the hardware timer.
     */
    inline static void timerStart() {
        if (_htim != nullptr) {
            HAL_TIM_Base_Start(_htim);
        }
    }

    /**
     * @brief Stops the hardware timer.
     */
    inline static void timerStop() {
        if (_htim != nullptr) {
            HAL_TIM_Base_Stop(_htim);
        }
    }

private:
    static inline TIM_HandleTypeDef* _htim = nullptr; ///< Shared timer handle for all instances.
};


// interval ----------------------------
#include "interval/ITimeBase.h"
template<auto Interval = 0u>
using HardITimer = ITimeBase<Interval, HTimer>;

#include "interval/OneShotIBase.h"
template<auto Interval = 0u>
using OneShotIHtim = OneShotIBase<Interval, HTimer>;

// virtual ---------------------------------
#include "virtual/VTimeBase.h"
template<auto Interval = 0u>
using HardVTimer = VTimeBase<Interval, HTimer>;

#include "virtual/OneShotVBase.h"
template<auto Interval = 0u>
using OneShotVHtim = OneShotVBase<Interval, HTimer>;
#else
#warning "[Hardware TIME]: Hardware time is not enabled in this device"
#endif /* HAL_TIM_MODULE_ENABLED */

#endif /* STM32_TOOLS_TIME_HTIMER_H_ */
