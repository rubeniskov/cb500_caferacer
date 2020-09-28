#include "cooling.h"
#include "sensor.h"
#include "kalman.h"
#include "common.h"

KalmanTypeDef kdColantTemp;

void ECU_Cooling_Init() {
    KalmanInit(
        &kdColantTemp,
        ECU_COOLING_TEMPERATURE_KF_ERROR_MEASUREMENT,
        ECU_COOLING_TEMPERATURE_KF_ERROR_ESTIMATE,
        ECU_COOLING_TEMPERATURE_KF_PROCESS_NOISE
    );
    if (HAL_TIM_PWM_Start(&htim3, ECU_COOLING_FAN_TIMMER_CHANNEL) != HAL_OK) { 
        Error_Handler(); 
    }
}

void ECU_Cooling_Check() {
    // Check the temperature of radiator
    float temp = ECU_Cooling_Read_Temp(); 
    if (temp > ECU_COOLING_TEMPERATURE_RANGE_HIGH) {
        ECU_Cooling_Set_Fan_Speed(ECU_COOLING_FAN_HIGH_SPEED);
    } else if (temp > ECU_COOLING_TEMPERATURE_RANGE_MEDIUM) {
        ECU_Cooling_Set_Fan_Speed(ECU_COOLING_FAN_MEDIUM_SPEED);
    } else if (temp > ECU_COOLING_TEMPERATURE_RANGE_LOW) {
        ECU_Cooling_Set_Fan_Speed(ECU_COOLING_FAN_LOW_SPEED);
    } else {
        ECU_Cooling_Set_Fan_Speed(0);
    }
}

void ECU_Cooling_Set_Fan_Speed(int8_t speed) {
    // Set duty cicle 
    __HAL_TIM_SET_COMPARE(&htim3, ECU_COOLING_FAN_TIMMER_CHANNEL, speed);
}

float ECU_Cooling_Read_Temp() {
    float value = ECU_Sensor_Read(ECU_SENSOR_TEMPERATURE_COOLANT);
    return KalmanUpdateEstimate(&kdColantTemp, value);
}
