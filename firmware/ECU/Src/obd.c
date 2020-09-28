// On Board Diagnostics
#include "obd.h"
#include "can.h"
#include "delay.h"

CAN_RxHeaderTypeDef rxHeader; //CAN Bus Transmit Header
CAN_TxHeaderTypeDef txHeader; //CAN Bus Receive Header
uint8_t canRX[8] = {0,0,0,0,0,0,0,0};  //CAN Bus Receive Buffer
CAN_FilterTypeDef canfil; //CAN Bus Filter
uint32_t canMailbox; //CAN Bus Mail box variable

void ECU_OBD_Init() {
    //HAL_CAN_Stop(&hcan);
    /*if (HAL_CAN_Receive_IT(&hcan, CAN_FIFO0) != HAL_OK) {
        Error_Handler();
    }*/
    canfil.FilterBank = 0;
    canfil.FilterMode = CAN_FILTERMODE_IDMASK;
    canfil.FilterFIFOAssignment = CAN_RX_FIFO0;
    canfil.FilterIdHigh = 0;
    canfil.FilterIdLow = 0;
    canfil.FilterMaskIdHigh = 0;
    canfil.FilterMaskIdLow = 0;
    canfil.FilterScale = CAN_FILTERSCALE_32BIT;
    canfil.FilterActivation = ENABLE;
    canfil.SlaveStartFilterBank = 14;

    txHeader.DLC = 8; // Number of bites to be transmitted max- 8
    txHeader.IDE = CAN_ID_STD;
    txHeader.RTR = CAN_RTR_DATA;
    txHeader.StdId = 0x030;
    txHeader.ExtId = 0x02;
    txHeader.TransmitGlobalTime = DISABLE;

    HAL_CAN_ConfigFilter(&hcan,&canfil); //Initialize CAN Filter
    HAL_CAN_Start(&hcan); //Initialize CAN Bus
    HAL_CAN_ActivateNotification(&hcan, CAN_IT_RX_FIFO0_MSG_PENDING);// Initi
}

void ECU_OBD_Check() {
    ECU_OBD_Transmit();
}




void ECU_OBD_Transmit() {
    uint8_t csend[] = {0x01,0x02,0x03,0x04,0x05,0x06,0x07,0x08};
    if (HAL_CAN_AddTxMessage(&hcan,&txHeader,csend,&canMailbox) != HAL_OK) {
        Error_Handler();
    }
}

void HAL_CAN_RxFifo0MsgPendingCallback(CAN_HandleTypeDef *hcan1) {
	HAL_CAN_GetRxMessage(hcan1, CAN_RX_FIFO0, &rxHeader, canRX); //Receive CAN bus message to canRX buffer
    HAL_GPIO_WritePin(GPIOC, GPIO_PIN_13, 1);
}

//USB_LP_CAN_RX0_IRQHandler
// void CAN_RX0_IRQHandler() {
//     uint32_t RxMessage[10];
//     &hcan->
    
//   /*if ((CanHandle->pRxMsg->StdId == 0x321)&&(CanHandle->pRxMsg->IDE == CAN_ID_STD) && (CanHandle->pRxMsg->DLC == 2))
//   {
//     LED_Display(CanHandle->pRxMsg->Data[0]);
//     ubKeyNumber = CanHandle->pRxMsg->Data[0];
//   }
  
//   // Receive 
//   if(HAL_CAN_Receive_IT(CanHandle, CAN_FIFO0) != HAL_OK)
//   {
//     // Reception Error 
//     Error_Handler();
//   }*/
// }

// https://www.csselectronics.com/screen/page/simple-intro-obd2-explained
// https://en.wikipedia.org/wiki/OBD-II_PIDs
// [PIDs supported [21 - 40]](https://en.wikipedia.org/wiki/OBD-II_PIDs#Service_01_PID_00)


/*

- PID => Process identifier
- DBR => Data bytes returned

| MODE | PID         | DBR | Description                  | Min value | Max value | Units    | Formula     |
|------|-------------|-----|------------------------------|-----------|-----------|----------|-------------|
|                                    SAE Standard PID's                                                    |
|------|-------------|-----|------------------------------|-----------|-----------|----------|-------------|
|  01  | 0x00 /  0   |  4  | PIDs supported [01 - 20]     |           |           |  °C      |    encoded  |
|  01  | 0x05 /  5   |  3  | Engine coolant temperature   |           |           |  °C      |    A-40     |
|  01  | 0x0C / 12   |  2  | Engine speed                 |      0    | 16,383.75 |  rpm     | (256A+B)/4  |
|  01  | 0x0D / 13   |  1  | Vehicle speed                |      0    |   255     |  km/h    |     A       |
|  01  | 0x1F / 31   |  2  | Run time since engine start  |      0    | 65,535    |  seconds |     A       |
|  01  | 0x20 / 32   |  4  | PIDs supported [21 - 40]     |           |           |          |  encoded    |
|  01  | 0x2F / 47   |  1  | Fuel Tank Level Input        |      0    |    100    |     %    | (100/255)*A |
|  01  | 0x40 / 64   |  4  | PIDs supported [41 - 60]     |           |           |          |  encoded    |
|  01  | 0x46 / 70	 |  1  | Ambient air temperature	  |    -40    |    215    |  °C      |    A-40     |
|  01  | 0x51 / 81   |  1  | Fuel Type                    |           |           |          |     1       |
|  01  | 0x5C / 92   |  1  | Engine oil temperature       |    -40    |    210    |  °C      |   256A+B    |
|  01  | 0x60 / 96   |  4  | PIDs supported [61 - 80]     |           |           |          |  encoded    |
|  01  | 0x7F / 127  |  13 | Engine run time              |           |           |  seconds |     A       |
|  01  | 0x80 / 128  |  4  | PIDs supported [81 - A0]     |           |           |          |  encoded    |
|------|-------------|-----|------------------------------|-----------|-----------|----------|-------------|
|                                    Custom Standard PID's                                                 |
|------|-------------|-----|------------------------------|-----------|-----------|----------|-------------|
|  01  | 0xC4 / 196  |  4  | Engine oil pressure          |           |           |          |  encoded    |
|  01  | 0xC4 / 196  |  4  | Engine coolant pressure      |           |           |          |  encoded    |
|----------------------------------------------------------------------------------------------------------|
*/

