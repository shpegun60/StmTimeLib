/*
 * Dwt.cpp
 *
 *  Created on: Jul 29, 2025
 *      Author: admin
 */

#include "Dwt.h"

#if defined(DWT) && defined(DWT_BASE)

/**
 * @brief DWT initialization (called once).
 */
__STATIC_INLINE void DWT_Init(void)
{
	// Enable core debug timers
	SET_BIT(CoreDebug->DEMCR, CoreDebug_DEMCR_TRCENA_Msk);

	// Unlock write access to DWT registers (required for STM32H7)
#ifdef STM32H7
	DWT->LAR = 0xC5ACCE55; // Unlock access for STM32H7
#endif

	// Reset and enable the cycle counter
	DWT->CYCCNT = 0U;
	SET_BIT(DWT->CTRL, DWT_CTRL_CYCCNTENA_Msk);
}

Dwt::Dwt()
{
	DWT_Init();
}

bool Dwt::isAvailable() noexcept
{
	return (READ_BIT(CoreDebug->DEMCR, CoreDebug_DEMCR_TRCENA_Msk) &&
			READ_BIT(DWT->CTRL, DWT_CTRL_CYCCNTENA_Msk));
}

#endif /* DWT is exists */
