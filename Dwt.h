/*
 * dwt.h
 *
 *  Created on: Jul 29, 2025
 *      Author: admin
 */

#ifndef STM32_TOOLS_TIME_DWT_H_
#define STM32_TOOLS_TIME_DWT_H_

#include "interval_depency.h"

// Ensure DWT is available
#if !(defined(DWT) && defined(DWT_BASE))
#warning "[DWT TIME]: DWT is not supported on this target device."
#else

#define DWT_TIME_IS_EXISTS 1

#include "basic_types.h" // u32 definition
#include "macro.h"

class Dwt
{
    Dwt(); // Private constructor, runs DWT_Init()
    ~Dwt() = default;

    _DELETE_COPY_MOVE(Dwt); // Disallow copy and move

public:
    using type_t = u32;

    // Return current DWT cycle count. DWT will be initialized on first call.
    static inline type_t now() noexcept {
        static Dwt _once; // One-time initialization
        return DWT->CYCCNT;
    }

	static bool isAvailable() noexcept;
};

/**
 * @brief Helper class to calculate clock cycles based on SystemCoreClock.
 */
class DwtBuilder {
	STATIC_CLASS(DwtBuilder);
public:
	using type_t = Dwt::type_t;

    /**
     * @brief Converts nanoseconds to clock cycles.
     * @param ns Time in nanoseconds.
     * @return Clock cycles (rounded up).
     */
    static inline type_t from_nano(const u64 ns) {
        return (ns * SystemCoreClock + 999'999'999ull) / 1'000'000'000ull;
    }

    /**
     * @brief Converts microseconds to clock cycles.
     * @param us Time in microseconds.
     * @return Clock cycles (rounded up).
     */
    static inline type_t from_micro(const u64 us) {
        return (us * static_cast<u64>(SystemCoreClock) + 999'999ull) / 1'000'000ull;
    }

    /**
     * @brief Converts milliseconds to clock cycles.
     * @param ms Time in milliseconds.
     * @return Clock cycles (rounded up).
     */
    static inline type_t from_milli(const u64 ms) {
        return (ms * static_cast<u64>(SystemCoreClock) + 999ull) / 1'000ull;
    }

    // Спеціалізовані перевантаження:
    static inline type_t from(const std::chrono::nanoseconds d) noexcept {
        return from_nano(static_cast<u64>(d.count()));
    }
    static inline type_t from(const std::chrono::microseconds d) noexcept {
        return from_micro(static_cast<u64>(d.count()));
    }
    static inline type_t from(const std::chrono::milliseconds d) noexcept {
        return from_milli(static_cast<u64>(d.count()));
    }

    // Загальний випадок для інших періодів:
    template<typename Rep, typename Period>
    static inline type_t from(const std::chrono::duration<Rep, Period> d) noexcept {
        // Кастимо в наносекунди й далі в цикли
        auto ns = std::chrono::duration_cast<std::chrono::nanoseconds>(d).count();
        return from_nano(static_cast<u64>(ns));
    }
};


// interval ----------------------------
#include "interval/ITimeBase.h"
template<auto Interval = 0u>
using DwtITimer = ITimeBase<Interval, Dwt>;

#include "interval/OneShotIBase.h"
template<auto Interval = 0u>
using OneShotIDwt = OneShotIBase<Interval, Dwt>;

// virtual ---------------------------------
#include "virtual/VTimeBase.h"
template<auto Interval = 0u>
using DwtVTimer = VTimeBase<Interval, Dwt>;

#include "virtual/OneShotVBase.h"
template<auto Interval = 0u>
using OneShotVDwt = OneShotVBase<Interval, Dwt>;


#endif /* DWT is exists */
#endif /* STM32_TOOLS_TIME_DWT_H_ */
