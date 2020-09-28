#include "stm32f1xx_hal.h"
/* GPIO pins define ----------------------------------------------------------*/
#define LIGHT_0                    ((uint16_t)0x001)  // Front daytime
#define LIGHT_1                    ((uint16_t)0x002)  // Front low beam
#define LIGHT_2                    ((uint16_t)0x004)  // Front hight beam
#define LIGHT_3                    ((uint16_t)0x008)  // Front left indicator
#define LIGHT_4                    ((uint16_t)0x010)  // Front right indicator
#define LIGHT_5                    ((uint16_t)0x020)  // Rear left indicator
#define LIGHT_6                    ((uint16_t)0x040)  // Rear right indicator
#define LIGHT_7                    ((uint16_t)0x080)  // Rear position
#define LIGHT_8                    ((uint16_t)0x100)  // Rear brake
#define LIGHT_9                    ((uint16_t)0x200)  // Rear tag

#define LIGHT_FRONT_INDICATORS      = LIGHT_3 | LIGHT_4;
#define LIGHT_REAR_INDICATORS       = LIGHT_5 | LIGHT_6;
#define LIGHT_LEFT_INDICATORS       = LIGHT_3 | LIGHT_5;
#define LIGHT_RIGHT_INDICATORS      = LIGHT_4 | LIGHT_6;
#define LIGHT_ALL_INDICATORS        = LIGHT_FRONT_INDICATORS | LIGHT_REAR_INDICATORS;

#define INDICATOR_DIRECTION_LEFT    = 0; 
#define INDICATOR_DIRECTION_RIGHT   = 1;

typedef struct LightMappingTypeDef {
    uint16_t LIGHT_IDX;
    GPIO_TypeDef* GPIO_Group;
    uint16_t GPIO_Pin;
} LightMapping;


/* Light Functions -----------------------------------------------------------*/
int setLightState(uint16_t mask, float intensity);