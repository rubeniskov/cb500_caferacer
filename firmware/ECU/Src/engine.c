#include "stm32f1xx_hal.h"
#include "delay.h"

void ECU_Engine_Init() {
    
}

void ECU_Engine_Start() {

    //HAL_GPIO_WritePin(GPIOB, GPIO_PIN_13, 0);
    // Check if kickstand is active and no neutral gear system protection 
    /*if (ECU_KICKSTAND_PARKING && !ECU_GEAR_NEUTRAL) {
        return 127;
    }*/

    // Enable CDI
    HAL_GPIO_WritePin(GPIOB, GPIO_PIN_3, 1);
    // Start startup motor
    HAL_GPIO_WritePin(GPIOB, GPIO_PIN_4, 1);
    // Waits 2 second
    DWT_Delay(2000000); 
    // Stop startup motor
    HAL_GPIO_WritePin(GPIOB, GPIO_PIN_4, 0);

    
    // Spike system will be carry 
}

void ECU_Engine_Stop() {
    // Disable CDI
    HAL_GPIO_WritePin(GPIOB, GPIO_PIN_3, 0);
}

/*
https://www.carbibles.com/high-oil-pressure/
25 PSI -> 65 PSI OK
> 65 PSI Warning
> 80 PSI Error 
*/