#include "stm32f1xx_hal.h"
#include "adc.h"

#define VREFINT_CAL_ADDR                0x1FFFF7BA  /* datasheet p. 19 */
#define VREFINT_CAL ((uint16_t*) VREFINT_CAL_ADDR)

#define ECU_ADC_NUM_CHANNELS 5
#define ECU_SENSOR_PREASURE_MIN_VREF_VALUE ((uint32_t)330)  // 0.333 Volts
#define ECU_SENSOR_PREASURE_MAX_VREF_VALUE ((uint32_t)2970) // 2.97 Volts
#define ECU_SENSOR_PREASURE_MIN_UNIT_VALUE 0
#define ECU_SENSOR_PREASURE_MAX_UNIT_VALUE 200

#define ECU_SENSOR_TEMPERATURE_MIN_VREF_VALUE ((uint32_t)330)
#define ECU_SENSOR_TEMPERATURE_MAX_VREF_VALUE ((uint32_t)2970)
#define ECU_SENSOR_TEMPERATURE_MIN_UNIT_VALUE -55
#define ECU_SENSOR_TEMPERATURE_MAX_UNIT_VALUE 200

#define ECU_SENSOR_VOLUME_MIN_VALUE VREFINT_CAL * 0.1
#define ECU_SENSOR_VOLUME_MAX_VALUE VREFINT_CAL
#define ECU_SENSOR_VOLUME_MIN_UNIT_VALUE 1
#define ECU_SENSOR_VOLUME_MAX_UNIT_VALUE 18

#define ECU_SENSOR_TYPE_MASK(__KIND__, __VALUE__) ((uint8_t)__KIND__ << 4) | (uint8_t)__VALUE__ 

uint16_t sensorBuffer[ECU_ADC_NUM_CHANNELS];

/**
  * @brief  HAL Status structures definition
  */
typedef enum
{
  ECU_SENSOR_PREASURE                  = (uint8_t)0x1,
  ECU_SENSOR_TEMPERATURE               = (uint8_t)0x2,
  ECU_SENSOR_VOLUME                    = (uint8_t)0x3
} ECU_SensorKindTypeDef;

/**
  * @brief  HAL Status structures definition
  */
typedef enum
{
  ECU_SENSOR_PREASSURE_COOLANT         = ECU_SENSOR_TYPE_MASK(ECU_SENSOR_PREASURE, 0x1),
  ECU_SENSOR_PREASSURE_OIL             = ECU_SENSOR_TYPE_MASK(ECU_SENSOR_PREASURE, 0x2),
  ECU_SENSOR_TEMPERATURE_OIL           = ECU_SENSOR_TYPE_MASK(ECU_SENSOR_TEMPERATURE, 0x3),
  ECU_SENSOR_TEMPERATURE_COOLANT       = ECU_SENSOR_TYPE_MASK(ECU_SENSOR_TEMPERATURE, 0x4),
  ECU_SENSOR_VOLUME_FUEL               = ECU_SENSOR_TYPE_MASK(ECU_SENSOR_VOLUME, 0x4)
} ECU_SensorIdTypeDef;

void ECU_Sensor_Init();
float ECU_Sensor_Read(ECU_SensorIdTypeDef id);
uint16_t ECU_Sensor_Read_Raw(ECU_SensorIdTypeDef id);