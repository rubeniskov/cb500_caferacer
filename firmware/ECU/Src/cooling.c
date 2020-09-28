#include "cooling.h"
#include "sensor.h"
#include "kalman.h"
#include "common.h"

KalmanTypeDef kdCoolantTemp = { 0.1, 0.1, 0.2 };
KalmanTypeDef kdColantPresure = { 0.1, 0.1, 0.2 };

void ECU_Cooling_Init() {
    if (HAL_TIM_PWM_Start(&htim3, ECU_COOLING_FAN_TIMMER_CHANNEL) != HAL_OK) { 
        Error_Handler(); 
    }
}

void ECU_Cooling_Check() {
    // Check the coolant temperature
    float temp = ECU_Cooling_Read_Temperature(); 
    // Check the coolant PRESURE
    float press = ECU_Cooling_Read_Presure(); 
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

void ECU_Cooling_Set_Fan_Speed(int32_t speed) {
    // Set duty cicle 
    __HAL_TIM_SET_COMPARE(&htim3, ECU_COOLING_FAN_TIMMER_CHANNEL, speed);
}

float ECU_Cooling_Read_Temperature() {
    uint32_t value = ECU_Sensor_Read(ECU_SENSOR_TEMPERATURE_COOLANT);
    return KalmanUpdateEstimate(&kdCoolantTemp, value);
}

float ECU_Cooling_Read_Presure() {
    uint32_t value = ECU_Sensor_Read(ECU_SENSOR_PRESURE_COOLANT);
    return KalmanUpdateEstimate(&kdCoolantTemp, value); 
}
