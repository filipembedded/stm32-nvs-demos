/**
  ******************************************************************************************************************************
  * File Name          : fram_mem_table.h
  * Description        : This file contains adresses in FRAM for particular parameters
  ******************************************************************************************************************************
  *
  * COPYRIGHT(c) 2018 Meter and Control doo.
  *
  ******************************************************************************************************************************
  */
	
#ifndef LEGACY_FRAM_MEM_TABLE_H
#define LEGACY_FRAM_MEM_TABLE_H

//power quality
#define FADDR_IMAGE_ACTIVATE_FLAG																			0UL // len = 1

//measurement
#define VOLTAGE_RESISTOR_RATIO_WITH_CRC_LENGHT												4 		// Length of voltage divider configuration array with two bytes added for CRC

#if (FADDR_IMAGE_ACTIVATE_FLAG != 0)
	#error "Change this address in bootloader project!!!"
#endif
#define FADDR_VOLTAGE_LEVEL1_COUNTER_L1																(FADDR_IMAGE_ACTIVATE_FLAG + 1)
#define FADDR_VOLTAGE_LEVEL2_COUNTER_L1																(FADDR_VOLTAGE_LEVEL1_COUNTER_L1 + 4)
#define FADDR_VOLTAGE_LEVEL3_COUNTER_L1																(FADDR_VOLTAGE_LEVEL2_COUNTER_L1 + 4)
#define FADDR_VOLTAGE_LEVEL4_COUNTER_L1																(FADDR_VOLTAGE_LEVEL3_COUNTER_L1 + 4)
#define FADDR_VOLTAGE_LEVEL5_COUNTER_L1																(FADDR_VOLTAGE_LEVEL4_COUNTER_L1 + 4)
#define FADDR_VOLTAGE_LEVEL6_COUNTER_L1																(FADDR_VOLTAGE_LEVEL5_COUNTER_L1 + 4)
#define FADDR_VOLTAGE_LEVEL7_COUNTER_L1																(FADDR_VOLTAGE_LEVEL6_COUNTER_L1 + 4)

#define FADDR_VOLTAGE_LEVEL1_COUNTER_L2																(FADDR_VOLTAGE_LEVEL7_COUNTER_L1 + 4)
#define FADDR_VOLTAGE_LEVEL2_COUNTER_L2																(FADDR_VOLTAGE_LEVEL1_COUNTER_L2 + 4)
#define FADDR_VOLTAGE_LEVEL3_COUNTER_L2																(FADDR_VOLTAGE_LEVEL2_COUNTER_L2 + 4)
#define FADDR_VOLTAGE_LEVEL4_COUNTER_L2																(FADDR_VOLTAGE_LEVEL3_COUNTER_L2 + 4)
#define FADDR_VOLTAGE_LEVEL5_COUNTER_L2																(FADDR_VOLTAGE_LEVEL4_COUNTER_L2 + 4)
#define FADDR_VOLTAGE_LEVEL6_COUNTER_L2																(FADDR_VOLTAGE_LEVEL5_COUNTER_L2 + 4)
#define FADDR_VOLTAGE_LEVEL7_COUNTER_L2																(FADDR_VOLTAGE_LEVEL6_COUNTER_L2 + 4)

#define FADDR_VOLTAGE_LEVEL1_COUNTER_L3																(FADDR_VOLTAGE_LEVEL7_COUNTER_L2 + 4)
#define FADDR_VOLTAGE_LEVEL2_COUNTER_L3																(FADDR_VOLTAGE_LEVEL1_COUNTER_L3 + 4)
#define FADDR_VOLTAGE_LEVEL3_COUNTER_L3																(FADDR_VOLTAGE_LEVEL2_COUNTER_L3 + 4)
#define FADDR_VOLTAGE_LEVEL4_COUNTER_L3																(FADDR_VOLTAGE_LEVEL3_COUNTER_L3 + 4)
#define FADDR_VOLTAGE_LEVEL5_COUNTER_L3																(FADDR_VOLTAGE_LEVEL4_COUNTER_L3 + 4)
#define FADDR_VOLTAGE_LEVEL6_COUNTER_L3																(FADDR_VOLTAGE_LEVEL5_COUNTER_L3 + 4)
#define FADDR_VOLTAGE_LEVEL7_COUNTER_L3																(FADDR_VOLTAGE_LEVEL6_COUNTER_L3 + 4)

#endif // LEGACY_FRAM_MEM_TABLE_H