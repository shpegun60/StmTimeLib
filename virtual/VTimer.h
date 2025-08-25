/**
 * @file VTimer.h
 * @brief Declaration of the VirtualTimer class for managing system timers.
 *
 * The VirtualTimer class provides a simple timer mechanism that relies on a global
 * container of timer objects. Each timer is decremented in the SysTick interrupt.
 * Use the provided start(), stop(), erase(), and emplace() functions to control the timer.
 *
 * @note Ensure that HAL_SYSTICK_Callback() is called from your SysTick interrupt handler.
 *
 * @author Shpegun60
 * @date
 */

#ifndef __TOOLS_SYS_VTIMER_H__
#define __TOOLS_SYS_VTIMER_H__

#include "time/interval_depency.h"
#include <vector>
#include <utility>

class VTimer
{
public:
    using value_type = reg;
    static_assert(sizeof(value_type) <= sizeof(reg), "counter write must be single-copy atomic");

    /**
     * @brief Constructor with an initial delay.
     *
     * Constructs a VirtualTimer object with the counter set to the provided delay
     * and registers it in the global timer list.
     *
     * @param delay Initial delay value for the timer.
     */
    VTimer(const value_type delay = 0);

    /**
     * @brief Destructor.
     *
     * Removes the timer from the global timer list.
     */
    virtual ~VTimer();

    /**
     * @brief Checks if the timer has expired.
     *
     * A timer is considered expired if its counter is zero.
     *
     * @return true if the timer is expired, false otherwise.
     */
    bool isExpired() const { return m_counter == 0; }
    value_type timeLeft() const { return m_counter; }

    /**
     * @brief Starts the timer with a specified delay.
     *
     * Sets the timer counter to the provided delay value.
     *
     * @param delay The delay to set for the timer.
     */
    void next(const value_type delay) { m_counter = delay; }

    /**
     * @brief Stops the timer.
     *
     * Sets the timer counter to zero.
     */
    void stop();

    /**
     * @brief Erases the timer from the global timer list.
     *
     * Removes the current timer object from the global container.
     */
    void erase();

    /**
     * @brief Adds (emplaces) the timer into the global timer list.
     *
     * If the timer is not already present, it is added to the global container.
     */
    void emplace();

    void reserve(const reg n = 5);

private:
    /**
     * @brief Decrements the counter for each registered timer.
     *
     * This static function is called from the SysTick interrupt callback
     * to update the timers.
     */
    static inline void proceed() {
        for (auto* const timer : std::as_const(m_timers)) {
            value_type _counter = timer->m_counter;

            if (_counter) {
            	--_counter;
                timer->m_counter = _counter;
            }
        }
    }

    // Grant access to HAL_SYSTICK_Callback so it can call proceed()
    friend void HAL_SYSTICK_Callback(void);

private:
    volatile value_type m_counter = 0;                      ///< Timer counter. When zero, the timer is expired.
    static inline std::vector<VTimer*> m_timers = {};///< Global list of registered VirtualTimer objects.
};

void HAL_SYSTICK_Callback(void);

#endif /* __TOOLS_SYS_VTIMER_H__ */
