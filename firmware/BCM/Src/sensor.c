#include "sensor.h"
#include "common.h"

/**
 * @brief Initialize the adc config for the input sensors
 */
void ECU_Sensor_Init() {
    if (HAL_ADC_Start_DMA(&hadc1, (uint32_t)sensorBuffer, ECU_ADC_NUM_CHANNELS) != HAL_OK) {
        Error_Handler(); 
    }
}

/**
 * @brief Reads the value of the sensor using the id which defines what kind of 
 * mapping to use for each sensor
 */
float ECU_Sensor_Read(ECU_SensorIdTypeDef id) {
    uint8_t sensorKind = id >> 4;
    uint16_t adcValue = ECU_Sensor_Read_Raw(id);
    float mappedValue = 0;

    switch (sensorKind) {
        case ECU_SENSOR_PREASURE:
            mappedValue = fmap(
                adcValue,
                ECU_SENSOR_PREASURE_MIN_VREF_VALUE,
                ECU_SENSOR_PREASURE_MAX_VREF_VALUE,
                ECU_SENSOR_PREASURE_MIN_UNIT_VALUE,
                ECU_SENSOR_PREASURE_MAX_UNIT_VALUE
            );
            break;
        case ECU_SENSOR_TEMPERATURE: 
            mappedValue = fmap(
                adcValue,
                ECU_SENSOR_TEMPERATURE_MIN_VREF_VALUE,
                ECU_SENSOR_TEMPERATURE_MAX_VREF_VALUE,
                ECU_SENSOR_TEMPERATURE_MIN_UNIT_VALUE,
                ECU_SENSOR_TEMPERATURE_MAX_UNIT_VALUE
            );
    }

    return mappedValue;
}
/**
 * @brief Read from memory the value of each sensor using the 
 * id as reference taking the last 4 bits
 */
uint16_t ECU_Sensor_Read_Raw(ECU_SensorIdTypeDef id) {
    return sensorBuffer[id & 8];
}