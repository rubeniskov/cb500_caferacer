#include "oled.h"

//static unsigned char CHR_X, CHR_Y;
// static unsigned char CHR_X;

void UCM_OLED_Send_Cmd(uint8_t c)
{
  HAL_GPIO_WritePin(GPIOB, GPIO_PIN_0, GPIO_PIN_RESET); //dc
  HAL_GPIO_WritePin(GPIOB, GPIO_PIN_1, GPIO_PIN_RESET); //cs
  UCM_OLED_Send_Data(c);
  HAL_GPIO_WritePin(GPIOB, GPIO_PIN_1, GPIO_PIN_SET); //cs
}

void UCM_OLED_Send_Data(uint8_t c)
{
  if (HAL_SPI_Transmit(&hspi1, (uint8_t *)&c, sizeof(c), 1000) != HAL_OK)
  {
    Error_Handler();
  }

  // while ((&hspi1)->State != HAL_SPI_STATE_READY)
  // {
  // }
}

void UCM_OLED_Init()
{
  // Wait GPIOA is enabled
  while (!__HAL_RCC_GPIOA_IS_CLK_ENABLED())
    ;
  // Send Reset
  HAL_GPIO_WritePin(GPIOA, GPIO_PIN_6, GPIO_PIN_RESET);
  HAL_GPIO_WritePin(GPIOA, GPIO_PIN_6, GPIO_PIN_SET);

  UCM_OLED_Send_Cmd(CMD_DISPLAY_OFF);            //Display Off
  UCM_OLED_Send_Cmd(CMD_SET_CONTRAST_A);         //Set contrast for color A
  UCM_OLED_Send_Cmd(0x91);                       //145 (0x91)
  UCM_OLED_Send_Cmd(CMD_SET_CONTRAST_B);         //Set contrast for color B
  UCM_OLED_Send_Cmd(0x50);                       //80 (0x50)
  UCM_OLED_Send_Cmd(CMD_SET_CONTRAST_C);         //Set contrast for color C
  UCM_OLED_Send_Cmd(0x7D);                       //125 (0x7D)
  UCM_OLED_Send_Cmd(CMD_MASTER_CURRENT_CONTROL); //master current control
  UCM_OLED_Send_Cmd(0x06);                       //6
  UCM_OLED_Send_Cmd(CMD_SET_PRECHARGE_SPEED_A);  //Set Second Pre-change Speed For ColorA
  UCM_OLED_Send_Cmd(0x64);                       //100
  UCM_OLED_Send_Cmd(CMD_SET_PRECHARGE_SPEED_B);  //Set Second Pre-change Speed For ColorB
  UCM_OLED_Send_Cmd(0x78);                       //120
  UCM_OLED_Send_Cmd(CMD_SET_PRECHARGE_SPEED_C);  //Set Second Pre-change Speed For ColorC
  UCM_OLED_Send_Cmd(0x64);                       //100
  UCM_OLED_Send_Cmd(CMD_SET_REMAP);              //set remap & data format
  UCM_OLED_Send_Cmd(0x72);                       //0x72
  UCM_OLED_Send_Cmd(CMD_SET_DISPLAY_START_LINE); //Set display Start Line
  UCM_OLED_Send_Cmd(0x0);
  UCM_OLED_Send_Cmd(CMD_SET_DISPLAY_OFFSET); //Set display offset
  UCM_OLED_Send_Cmd(0x0);
  UCM_OLED_Send_Cmd(CMD_NORMAL_DISPLAY);      //Set display mode
  UCM_OLED_Send_Cmd(CMD_SET_MULTIPLEX_RATIO); //Set multiplex ratio
  UCM_OLED_Send_Cmd(0x3F);
  UCM_OLED_Send_Cmd(CMD_SET_MASTER_CONFIGURE); //Set master configuration
  UCM_OLED_Send_Cmd(0x8E);
  UCM_OLED_Send_Cmd(CMD_POWER_SAVE_MODE);         //Set Power Save Mode
  UCM_OLED_Send_Cmd(0x00);                        //0x00
  UCM_OLED_Send_Cmd(CMD_PHASE_PERIOD_ADJUSTMENT); //phase 1 and 2 period adjustment
  UCM_OLED_Send_Cmd(0x31);                        //0x31
  UCM_OLED_Send_Cmd(CMD_DISPLAY_CLOCK_DIV);       //display clock divider/oscillator frequency
  UCM_OLED_Send_Cmd(0xF0);
  UCM_OLED_Send_Cmd(CMD_SET_PRECHARGE_VOLTAGE); //Set Pre-Change Level
  UCM_OLED_Send_Cmd(0x3A);
  UCM_OLED_Send_Cmd(CMD_SET_V_VOLTAGE); //Set vcomH
  UCM_OLED_Send_Cmd(0x3E);
  UCM_OLED_Send_Cmd(CMD_DEACTIVE_SCROLLING);           //disable scrolling
  UCM_OLED_Send_Cmd(CMD_NORMAL_BRIGHTNESS_DISPLAY_ON); //set display on
}

void UCM_OLED_Draw_Pixel(uint16_t x, uint16_t y, uint16_t color)
{
  if ((x < 0) || (x >= RGB_OLED_WIDTH) || (y < 0) || (y >= RGB_OLED_HEIGHT))
    return;
  //set column point
  UCM_OLED_Send_Cmd(CMD_SET_COLUMN_ADDRESS);
  UCM_OLED_Send_Cmd(x);
  UCM_OLED_Send_Cmd(RGB_OLED_WIDTH - 1);
  //set row point
  UCM_OLED_Send_Cmd(CMD_SET_ROW_ADDRESS);
  UCM_OLED_Send_Cmd(y);
  UCM_OLED_Send_Cmd(RGB_OLED_HEIGHT - 1);

  HAL_GPIO_WritePin(GPIOB, GPIO_PIN_0, GPIO_PIN_SET);   //dc
  HAL_GPIO_WritePin(GPIOB, GPIO_PIN_1, GPIO_PIN_RESET); //cs

  UCM_OLED_Send_Data(color >> 8);
  UCM_OLED_Send_Data(color);

  HAL_GPIO_WritePin(GPIOB, GPIO_PIN_1, GPIO_PIN_SET); //cs
}

void UCM_OLED_Draw_Line(uint16_t x0, uint16_t y0, uint16_t x1, uint16_t y1, uint16_t color)
{
  if ((x0 < 0) || (y0 < 0) || (x1 < 0) || (y1 < 0))
    return;

  if (x0 >= RGB_OLED_WIDTH)
    x0 = RGB_OLED_WIDTH - 1;
  if (y0 >= RGB_OLED_HEIGHT)
    y0 = RGB_OLED_HEIGHT - 1;
  if (x1 >= RGB_OLED_WIDTH)
    x1 = RGB_OLED_WIDTH - 1;
  if (y1 >= RGB_OLED_HEIGHT)
    y1 = RGB_OLED_HEIGHT - 1;

  UCM_OLED_Send_Cmd(CMD_DRAW_LINE);                   //draw line
  UCM_OLED_Send_Cmd(x0);                              //start column
  UCM_OLED_Send_Cmd(y0);                              //start row
  UCM_OLED_Send_Cmd(x1);                              //end column
  UCM_OLED_Send_Cmd(y1);                              //end row
  UCM_OLED_Send_Cmd((uint8_t)((color >> 11) & 0x1F)); //R
  UCM_OLED_Send_Cmd((uint8_t)((color >> 5) & 0x3F));  //G
  UCM_OLED_Send_Cmd((uint8_t)(color & 0x1F));         //B
}

void UCM_OLED_Draw_Frame(uint16_t x0, uint16_t y0, uint16_t x1, uint16_t y1, uint16_t outColor, uint16_t fillColor)
{
  if ((x0 < 0) || (y0 < 0) || (x1 < 0) || (y1 < 0))
    return;

  if (x0 >= RGB_OLED_WIDTH)
    x0 = RGB_OLED_WIDTH - 1;
  if (y0 >= RGB_OLED_HEIGHT)
    y0 = RGB_OLED_HEIGHT - 1;
  if (x1 >= RGB_OLED_WIDTH)
    x1 = RGB_OLED_WIDTH - 1;
  if (y1 >= RGB_OLED_HEIGHT)
    y1 = RGB_OLED_HEIGHT - 1;

  UCM_OLED_Send_Cmd(CMD_FILL_WINDOW); //fill window
  UCM_OLED_Send_Cmd(ENABLE_FILL);
  UCM_OLED_Send_Cmd(CMD_DRAW_RECTANGLE);                  //draw rectangle
  UCM_OLED_Send_Cmd(x0);                                  //start column
  UCM_OLED_Send_Cmd(y0);                                  //start row
  UCM_OLED_Send_Cmd(x1);                                  //end column
  UCM_OLED_Send_Cmd(y1);                                  //end row
  UCM_OLED_Send_Cmd((uint8_t)((outColor >> 11) & 0x1F));  //R
  UCM_OLED_Send_Cmd((uint8_t)((outColor >> 5) & 0x3F));   //G
  UCM_OLED_Send_Cmd((uint8_t)(outColor & 0x1F));          //B
  UCM_OLED_Send_Cmd((uint8_t)((fillColor >> 11) & 0x1F)); //R
  UCM_OLED_Send_Cmd((uint8_t)((fillColor >> 5) & 0x3F));  //G
  UCM_OLED_Send_Cmd((uint8_t)(fillColor & 0x1F));         //B
}

void UCM_OLED_Draw_Circle(uint16_t x, uint16_t y, uint16_t radius, uint16_t color)
{
  signed char xc = 0;
  signed char yc = 0;
  signed char p = 0;

  // Out of range
  if (x >= RGB_OLED_WIDTH || y >= RGB_OLED_HEIGHT)
    return;

  yc = radius;
  p = 3 - (radius << 1);
  while (xc <= yc)
  {
    UCM_OLED_Draw_Pixel(x + xc, y + yc, color);
    UCM_OLED_Draw_Pixel(x + xc, y - yc, color);
    UCM_OLED_Draw_Pixel(x - xc, y + yc, color);
    UCM_OLED_Draw_Pixel(x - xc, y - yc, color);
    UCM_OLED_Draw_Pixel(x + yc, y + xc, color);
    UCM_OLED_Draw_Pixel(x + yc, y - xc, color);
    UCM_OLED_Draw_Pixel(x - yc, y + xc, color);
    UCM_OLED_Draw_Pixel(x - yc, y - xc, color);
    if (p < 0)
      p += (xc++ << 2) + 6;
    else
      p += ((xc++ - yc--) << 2) + 10;
  }
}

void UCM_OLED_Clear()
{
  HAL_Delay(50);
  UCM_OLED_Draw_Frame(0, 0, 96, 64, COLOR_BLACK, COLOR_BLACK);
  HAL_Delay(50);
}

void UCM_OLED_Draw()
{
  uint8_t i;
  UCM_OLED_Clear();
  // for (i = 0; i < 24; i++)
  // {
  //   UCM_OLED_Draw_Line(0, 0, i * 4, 64, RGB(i * 10, 0, 0));
  //   UCM_OLED_Draw_Line(96, 0, i * 4, 64, RGB(i * 10, 0, 0));
  //   HAL_Delay(20);
  // }
  UCM_OLED_Clear();
  for (i = 0; i < 300; i++)
  {
    UCM_OLED_Draw_Circle(48, 32, 5, i + COLOR_BLUE);
    HAL_Delay(5);
    UCM_OLED_Draw_Circle(48, 32, 10, i + COLOR_GOLDEN);
    HAL_Delay(5);
    UCM_OLED_Draw_Circle(48, 32, 15, i + COLOR_BLUE);
    HAL_Delay(5);
    UCM_OLED_Draw_Circle(48, 32, 20, i + COLOR_GOLDEN);
    HAL_Delay(5);
    UCM_OLED_Draw_Circle(48, 32, 25, i + COLOR_BLUE);
    HAL_Delay(5);
    UCM_OLED_Draw_Circle(48, 32, 30, i + COLOR_GOLDEN);
    HAL_Delay(100);
  }
  UCM_OLED_Clear();
}