#include "lights.h"

// Physical light mapping to GPIO 
struct LightMappingTypeDef LIGHT_MAP_ARRAY[4] = {
    { LIGHT_0, GPIOB, GPIO_PIN_6 },
    { LIGHT_1, GPIOB, GPIO_PIN_7 },
    { LIGHT_2, GPIOB, GPIO_PIN_8 },
    { LIGHT_3, GPIOB, GPIO_PIN_9 }
};

int setLightState(uint16_t mask, float intensity) {
    size_t len = sizeof(LIGHT_MAP_ARRAY) / sizeof(LIGHT_MAP_ARRAY[0]);
    int ret = 1;
    for(int i = 0; i < len; i++) {
        LightMapping lightMap = LIGHT_MAP_ARRAY[i];
        if((mask & lightMap.LIGHT_IDX) != 0) {
            ret = 0;
            if (intensity == 0.0) {
                HAL_GPIO_WritePin(lightMap.GPIO_Group, lightMap.GPIO_Pin, 0);
            } else {
                HAL_GPIO_WritePin(lightMap.GPIO_Group, lightMap.GPIO_Pin, 1);
            }
        }
    }
    return ret;
}