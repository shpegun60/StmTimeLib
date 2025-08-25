/*
 * Timer.cpp
 *
 *  Created on: Jul 29, 2025
 *      Author: admin
 */

#include "HTimer.h"

#ifdef HAL_TIM_MODULE_ENABLED

bool HTimer::attachTimer(TIM_HandleTypeDef *const htim)
{
    if (htim == nullptr || !IS_TIM_INSTANCE(htim->Instance)) {
        return false;
    }

    if(_htim != htim) {
        timerStop();
    }

    _htim = htim;
    timerStart();  // Start the timer when attached
    return true;
}

#endif /* HAL_TIM_MODULE_ENABLED */
