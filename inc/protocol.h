/*
Details from http://en.wikipedia.org/wiki/OBD-II_PIDs

Supported PID's

- PID => Process identifier
- DBR => Data bytes returned

| MODE | PID           | DBR | Description                  | Min value | Max value | Units    | Formula     |
|------|---------------|-----|------------------------------|-----------|-----------|----------|-------------|
|                                      SAE Standard PID's                                                    |
|------|---------------|-----|------------------------------|-----------|-----------|----------|-------------|
|  01  | 0x00 /    0   |  4  | PIDs supported [01 - 20]     |           |           |  °C      |    encoded  |
|  01  | 0x05 /    5   |  3  | Engine coolant temperature   |           |           |  °C      |    A-40     |
|  01  | 0x0C /   12   |  2  | Engine speed                 |      0    | 16,383.75 |  rpm     | (256A+B)/4  |
|  01  | 0x0D /   13   |  1  | Vehicle speed                |      0    |   255     |  km/h    |     A       |
|  01  | 0x1F /   31   |  2  | Run time since engine start  |      0    | 65,535    |  seconds |     A       |
|  01  | 0x20 /   32   |  4  | PIDs supported [21 - 40]     |           |           |          |  encoded    |
|  01  | 0x2F /   47   |  1  | Fuel Tank Level Input        |      0    |    100    |     %    | (100/255)*A |
|  01  | 0x40 /   64   |  4  | PIDs supported [41 - 60]     |           |           |          |  encoded    |
|  01  | 0x42 /   66   |  1  | Control module voltage	      |    0      |   65.535  |  °C      |(256A+B/1000)|
|  01  | 0x46 /   70   |  1  | Ambient air temperature	  |    -40    |    215    |  °C      |    A-40     |
|  01  | 0x51 /   81   |  1  | Fuel Type                    |           |           |          |     1       |
|  01  | 0x5C /   92   |  1  | Engine oil temperature       |    -40    |    210    |  °C      |   256A+B    |
|  01  | 0x60 /   96   |  4  | PIDs supported [61 - 80]     |           |           |          |  encoded    |
|  01  | 0x7F /  127   |  13 | Engine run time              |           |           |  seconds |     A       |
|  01  | 0x80 /  128   |  4  | PIDs supported [81 - A0]     |           |           |          |  encoded    |
|------|---------------|-----|------------------------------|-----------|-----------|----------|-------------|
|                                     Custom Standard PID's                                                 |
|------|---------------|-----|------------------------------|-----------|-----------|----------|-------------|
|  01  | 0xC5 /   197  |  3  | Engine oil pressure          |      1    |    200    |    PSI   |     A       |
|  01  | 0xC6 /   198  |  3  | Engine coolant pressure      |      1    |    200    |    PSI   |     A       |
|  01  | 0xC7 /   199  |  1  | Gearbox current gear         |      0    |    6      |          |     A       |
|  01  | 0x7e0 / 2016  |  1  | User control PID's           |           |           |          |   encoded   |
|------------------------------------------------------------------------------------------------------------|
*/

#ifndef CBCR_PROTOCOL_H
#define CBCR_PROTOCOL_H

#define CBCR_PROTOCOL_CANSPEED 500000 // CAN speed at 500 kbps

#define CBCR_PROTOCOL_PID_REQUEST 0x7DF
#define CBCR_PROTOCOL_PID_REPLY 0x7E8

// SAE Standard PID's
#define CBCR_PROTOCOL_PID_SUPPORTED_0_20 0x00
#define CBCR_PROTOCOL_PID_SUPPORTED_0_20_INFO "Supported PIDs [0 - 20]"
#define CBCR_PROTOCOL_ENGINE_COOLANT_TEMP 0x05
#define CBCR_PROTOCOL_ENGINE_COOLANT_TEMP_INFO "Engine coolant temperature"
#define CBCR_PROTOCOL_ENGINE_RPM 0x0C
#define CBCR_PROTOCOL_ENGINE_RPM_INFO "Engine RPM"
#define CBCR_PROTOCOL_VEHICLE_SPEED 0x0D
#define CBCR_PROTOCOL_VEHICLE_SPEED_INFO "Vehicle Speed"
#define CBCR_PROTOCOL_ENGINE_RUNTIME 0x1F
#define CBCR_PROTOCOL_ENGINE_RUNTIME_INFO "Run Time Since Engine Started"

#define CBCR_PROTOCOL_PID_SUPPORTED_21_40 0x20
#define CBCR_PROTOCOL_PID_SUPPORTED_21_40_INFO "Supported PIDs [21 - 40]"
#define CBCR_PROTOCOL_FUEL_LEVEL 0x2F
#define CBCR_PROTOCOL_FUEL_LEVEL_INFO "Fuel Level Input"

#define CBCR_PROTOCOL_PID_41_60 0x40
#define CBCR_PROTOCOL_PID_41_60_INFO "Supported PIDs [41 - 60]"
#define CBCR_PROTOCOL_ECU_VOLTAGE 0x42
#define CBCR_PROTOCOL_ECU_VOLTAGE_INFO "Control Module Voltage"
#define CBCR_PROTOCOL_AMB_AIR_TEMP 0x46
#define CBCR_PROTOCOL_AMB_AIR_TEMP_INFO "Ambient Air Temperature"
#define CBCR_PROTOCOL_FUEL_TYPE 0x51
#define CBCR_PROTOCOL_FUEL_TYPE_INFO "Fuel Type"
#define CBCR_PROTOCOL_ENGINE_OIL_TEMP 0x5C
#define CBCR_PROTOCOL_ENGINE_OIL_TEMP_INFO "Engine Oil Temperature"

#define CBCR_PROTOCOL_PID_61_80 0x60
#define CBCR_PROTOCOL_PID_61_80_INFO "Supported PIDs [61 - 80]"
#define CBCR_PROTOCOL_ENGINE_RUN_TIME 0x7F
#define CBCR_PROTOCOL_ENGINE_RUN_TIME_INFO "Engine run time"

#define CBCR_PROTOCOL_PID_61_80 0x80
#define CBCR_PROTOCOL_PID_61_80_INFO "Supported PIDs [81 - A0]"

// Custom PID's
#define CBCR_PROTOCOL_ENGINE_OIL_PREASURE 0xC5
#define CBCR_PROTOCOL_ENGINE_OIL_PREASURE_INFO "Engine oil preasure"
#define CBCR_PROTOCOL_ENGINE_COOLANT_PREASURE 0xC6
#define CBCR_PROTOCOL_ENGINE_COOLANT_PREASURE "Engine coolant preasure"
#define CBCR_PROTOCOL_GEARBOX_CURRENT_GEAR 0xC7
#define CBCR_PROTOCOL_GEARBOX_CURRENT_GEAR_INFO "Gearbox current gear"
#define CBCR_PROTOCOL_PID_USER_CONTROL 0x7e0
#define CBCR_PROTOCOL_PID_USER_CONTROL_INFO "User control PID's"

#endif