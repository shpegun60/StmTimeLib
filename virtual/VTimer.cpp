/**
 * @file VTimer.cpp
 * @brief Implementation of the VTimer class for managing system timers.
 *
 * This file implements the VTimer class methods which provide a simple timer mechanism.
 * Timers are updated in the SysTick interrupt handler via the HAL_SYSTICK_Callback function.
 *
 * @note Ensure that HAL_SYSTICK_Callback() is correctly called in your SysTick interrupt.
 *
 * @author Shpegun60
 * @date
 */

#include "VTimer.h"
#include "irq/IRQGuard.h"
#include <algorithm>

/**
 * @brief Destructor for VTimer.
 *
 * Removes this timer from the global timer list.
 */
VTimer::~VTimer()
{
	IRQGuard guard;
    auto it = std::find(m_timers.begin(), m_timers.end(), this);
    if (it != m_timers.end()) {
        m_timers.erase(it);
    }
}

/**
 * @brief Constructor for VTimer with an initial delay.
 *
 * Sets the timer counter to the specified delay and registers the timer.
 *
 * @param delay The initial delay for the timer.
 */
VTimer::VTimer(const VTimer::value_type delay)
{
	IRQGuard guard;
    m_counter = delay;
    m_timers.emplace_back(this);
}

/**
 * @brief Stops the timer.
 *
 * Sets the timer counter to zero, effectively stopping the timer.
 */
void VTimer::stop()
{
	IRQGuard guard;
    m_counter = 0;
}


/**
 * @brief Erases the timer from the global timer list.
 *
 * Removes this timer from the global container of timers.
 */
void VTimer::erase()
{
	IRQGuard guard;
    auto it = std::find(m_timers.begin(), m_timers.end(), this);
    if (it != m_timers.end()) {
        m_timers.erase(it);
    }
}

/**
 * @brief Emplaces the timer into the global timer list.
 *
 * If this timer is not already registered, it is added to the global container.
 */
void VTimer::emplace()
{
	IRQGuard guard;
    auto it = std::find(m_timers.begin(), m_timers.end(), this);
    if (it == m_timers.end()) {
        m_counter = 0;
        m_timers.emplace_back(this);
    }
}

void VTimer::reserve(const reg n)
{
	IRQGuard guard;
	m_timers.reserve(n);
}

/**
 * @brief SysTick callback function.
 *
 * This function should be invoked from the SysTick interrupt handler.
 * It updates all registered timers by decrementing their counters.
 */

//---------------------------- PUT INVOKING THIS FUNCTION TO SysTick() Interrupt!!!------------------------------------------------------------------------------
//---------------------------- file: stm32f1xx_it.c ------------------------------------------------------------------------------

void HAL_SYSTICK_Callback(void)
{
    VTimer::proceed();
}

#warning "[VTimer]: You must to put HAL_SYSTICK_Callback in your stm32xxxx_it.c file"


