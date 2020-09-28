#include "sensor.h"
#include "common.h"

uint16_t pData[5];

/**
 * @brief ADC DMA initialization
 */
HAL_StatusTypeDef ECU_ADC_Start_DMA(ADC_HandleTypeDef *hadc, uint32_t *pData, uint32_t Length)
{
  HAL_StatusTypeDef tmp_hal_status = HAL_OK;

  if (ADC_MULTIMODE_IS_ENABLE(hadc) == RESET)
  {
    __HAL_LOCK(hadc);
    /* Process locked */
    tmp_hal_status = ADC_Enable(hadc);
    /* Enable the ADC peripheral */
    if (tmp_hal_status == HAL_OK)
    {
      /* Set ADC state                                                        */
      /* - Clear state bitfield related to regular group conversion results   */
      /* - Set state bitfield related to regular operation                    */
      ADC_STATE_CLR_SET(hadc->State,
                        HAL_ADC_STATE_READY | HAL_ADC_STATE_REG_EOC | HAL_ADC_STATE_REG_OVR | HAL_ADC_STATE_REG_EOSMP,
                        HAL_ADC_STATE_REG_BUSY);
      /* Set group injected state (from auto-injection) and multimode state     */
      /* for all cases of multimode: independent mode, multimode ADC master     */
      /* or multimode ADC slave (for devices with several ADCs):                */
      if (ADC_NONMULTIMODE_OR_MULTIMODEMASTER(hadc))
      {
        /* Set ADC state (ADC independent or master) */
        CLEAR_BIT(hadc->State, HAL_ADC_STATE_MULTIMODE_SLAVE);

        /* If conversions on group regular are also triggering group injected,  */
        /* update ADC state.                                                    */
        if (READ_BIT(hadc->Instance->CR1, ADC_CR1_JAUTO) != RESET)
        {
          ADC_STATE_CLR_SET(hadc->State, HAL_ADC_STATE_INJ_EOC, HAL_ADC_STATE_INJ_BUSY);
        }
      }
      else
      {
        __HAL_UNLOCK(hadc);
      }
      /* State machine update: Check if an injected conversion is ongoing */
      if (HAL_IS_BIT_SET(hadc->State, HAL_ADC_STATE_INJ_BUSY))
      {
        /* Reset ADC error code fields related to conversions on group regular */
        CLEAR_BIT(hadc->ErrorCode, (HAL_ADC_ERROR_OVR | HAL_ADC_ERROR_DMA));
      }
      else
      {
        /* Reset ADC all error code fields */
        ADC_CLEAR_ERRORCODE(hadc);
      }
      /* Process unlocked */
      /* Unlock before starting ADC conversions: in case of potential         */
      /* interruption, to let the process to ADC IRQ Handler.                 */
      __HAL_UNLOCK(hadc);

      /* Set the DMA transfer complete callback */
      hadc->DMA_Handle->XferCpltCallback = ADC_DMAConvCplt;

      /* Set the DMA half transfer complete callback */
      hadc->DMA_Handle->XferHalfCpltCallback = ADC_DMAHalfConvCplt;

      /* Set the DMA error callback */
      hadc->DMA_Handle->XferErrorCallback = ADC_DMAError;

      /* Manage ADC and DMA start: ADC overrun interruption, DMA start, ADC   */
      /* start (in case of SW start):                                         */

      /* Clear regular group conversion flag and overrun flag */
      /* (To ensure of no unknown state from potential previous ADC           */
      /* operations)                                                          */
      __HAL_ADC_CLEAR_FLAG(hadc, ADC_FLAG_EOC);

      /* Enable ADC DMA mode */
      SET_BIT(hadc->Instance->CR2, ADC_CR2_DMA);

      /* Start the DMA channel */
      HAL_DMA_Start_IT(hadc->DMA_Handle, (uint32_t)&hadc->Instance->DR, (uint32_t)pData, Length);

      /* Start ADC conversion on regular group with SW start */
      SET_BIT(hadc->Instance->CR2, (ADC_CR2_SWSTART));
    }
  }
  else
  {
    __HAL_UNLOCK(hadc);
  }
  return tmp_hal_status;
}

/**
 * @brief Initialize the adc config for the input sensors
 */
void ECU_Sensor_Init()
{
  if (HAL_DMA_Start(&hadc1, (uint32_t *)pData, ECU_ADC_NUM_CHANNELS) != HAL_OK)
  {
    Error_Handler();
  }

  /*if (HAL_ADCEx_Calibration_Start(&hadc1) != HAL_OK)
  {
    Error_Handler();
  }*/
}

/**
 * @brief Reads the value of the sensor using the id which defines what kind of 
 * mapping to use for each sensor
 */
float ECU_Sensor_Read(ECU_SensorIdTypeDef id)
{
  uint8_t sensorKind = id >> 4;
  uint16_t adcValue = ECU_Sensor_Read_Raw(id);
  float mappedValue = 0;

  switch (sensorKind)
  {
  case ECU_SENSOR_PRESURE:
    mappedValue = fmap(
        adcValue,
        ECU_SENSOR_PRESURE_MIN_VREF_VALUE,
        ECU_SENSOR_PRESURE_MAX_VREF_VALUE,
        ECU_SENSOR_PRESURE_MIN_UNIT_VALUE,
        ECU_SENSOR_PRESURE_MAX_UNIT_VALUE);
    break;
  case ECU_SENSOR_TEMPERATURE:
    mappedValue = fmap(
        adcValue,
        ECU_SENSOR_TEMPERATURE_MIN_VREF_VALUE,
        ECU_SENSOR_TEMPERATURE_MAX_VREF_VALUE,
        ECU_SENSOR_TEMPERATURE_MIN_UNIT_VALUE,
        ECU_SENSOR_TEMPERATURE_MAX_UNIT_VALUE);
  }

  return mappedValue;
}
/**
 * @brief Read from memory the value of each sensor using the 
 * id as reference taking the last 4 bits
 */
uint16_t ECU_Sensor_Read_Raw(ECU_SensorIdTypeDef id)
{
  return pData[id & 8];
}

