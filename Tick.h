/*
 * Time.h
 *
 *  Created on: Jul 27, 2025
 *      Author: admin
 */

#ifndef STM32_TOOLS_TIME_TICK_H_
#define STM32_TOOLS_TIME_TICK_H_

#include "interval_depency.h"

//------------------------------------------------------------------------------
// Time utility class for accessing the system tick counter (uwTick)
//------------------------------------------------------------------------------

extern "C" __IO uint32_t uwTick;

// Mark class as static-only using macro (e.g., delete constructor, etc.)
class Tick
{
    STATIC_CLASS(Tick); // Disallow instantiation (likely deletes ctor/copy)

public:
    using type_t = u32;

    // Return current system tick count (from SysTick)
    static inline type_t now() noexcept { return uwTick; }
    static constexpr inline bool isAvailable() noexcept { return true; }
};

// interval ----------------------------
#include "interval/ITimeBase.h"
template<auto Interval = 0u>
using TickITimer = ITimeBase<Interval, Tick>;

#include "interval/OneShotIBase.h"
template<auto Interval = 0u>
using OneShotITick = OneShotIBase<Interval, Tick>;

// virtual ---------------------------------
#include "virtual/VTimeBase.h"
template<auto Interval = 0u>
using TickVTimer = VTimeBase<Interval, Tick>;

#include "virtual/OneShotVBase.h"
template<auto Interval = 0u>
using OneShotVTick = OneShotVBase<Interval, Tick>;


#endif /* STM32_TOOLS_TIME_TICK_H_ */
