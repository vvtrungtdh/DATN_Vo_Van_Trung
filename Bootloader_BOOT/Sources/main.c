/* ###################################################################
 **     Filename    : main.c
 **     Processor   : S32K1xx
 **     Abstract    :
 **         Main module.
 **         This module contains user's application code.
 **     Settings    :
 **     Contents    :
 **         No public methods
 **
 ** ###################################################################*/
/*!
 ** @file main.c
 ** @version 01.00
 ** @brief
 **         Main module.
 **         This module contains user's application code.
 */
/*!
 **  @addtogroup main_module main module documentation
 **  @{
 */
/* MODULE main */

/* Including necessary module. Cpu.h contains other modules needed for compiling.*/
#include "Cpu.h"
#include "pin_mux.h"
#include "clockMan1.h"
#include "Flash1.h"
#include "canCom1.h"
#include "dmaController1.h"
#include "osif1.h"
#include "lpTmr1.h"
#include "CANTP.h"
#include "CAN_standard.h"

volatile int exit_code = 0;

/* User includes (#include below this line is not maintained by Processor Expert) */
#include <stdint.h>
#include <stdbool.h>

flash_ssd_config_t flashSSDConfig;

/* Data source for program operation */
#define BUFFER_SIZE         4095          /* Size of data source */

#define BOOTLOADER_START_ADDR   0x00000000
#define BOOTLOADER_END_ADDR     0x0000CFFF
#define APP_CURRENT_START_ADDR  0x0000D000
#define APP_CURRENT_END_ADDR    0x0001BFFF
#define APP_UPDATE_START_ADDR   0x0001A000
#define APP_UPDATE_END_ADDR     0x00026FFF
#define DOWNLOAD_SUCCESS_ADDR 	0x0004B000
#define UPDATE_SUCCESS_ADDR     0x0004B010

/* Variable */
uint8_t TxBuffer[20];  // Buffer truyen
uint8_t RxBuffer[BUFFER_SIZE]; //Buffer

/* Function declarations */
typedef struct {
	uint8_t dataLength;
	uint8_t recordType;
	uint8_t data[16];
	uint8_t checkSum;
} hex_struct;
typedef void (*app_entry_point_t)(void);
void writeDataToDFlash(uint32_t address, uint8_t dataLength, uint8_t data[]);
void deleteData(uint32_t startAddress, uint32_t endAddress);
void hexAnalysAndWriteToMemory(hex_struct *a);
bool checkCheckSum(hex_struct *a, int stt);
bool IsDowloadSuccess();
void bootloader_main();
void CCIF_Handler(void);
//If target is flash, insert this macro to locate callback function into RAM
START_FUNCTION_DECLARATION_RAMSECTION
void CCIF_Callback(void)
END_FUNCTION_DECLARATION_RAMSECTION

void GPIOInit(void) {
	/* Output direction for LEDs */
	PINS_DRV_SetPinsDirection(GPIO_PORT,
			(1 << LED2) | (1 << LED1) | (1 << LED0));

	/* Set Output value LEDs */
	PINS_DRV_ClearPins(GPIO_PORT, 1 << LED1);
	PINS_DRV_SetPins(GPIO_PORT, 1 << LED2);
}

bool checkCheckSum(hex_struct *a, int stt) {
	uint8_t sumDataToCheckCheckSum = 0U;
	for (int i = stt; i < a->dataLength + 4 + stt; i++) {
		sumDataToCheckCheckSum += RxBuffer[i];
	}
	uint8_t TwoComponent = (~sumDataToCheckCheckSum) + 0x01;
	if (TwoComponent == a->checkSum) {
		return true;
	} else {
		return false;
	}
}
//separe hex code to collect data and write it to DFlash
void hexAnalysAndWriteToMemory(hex_struct *a) {
	status_t ret;
	uint32_t i = 0U;
	uint32_t Address = 0U;
	static uint32_t nextAddress = 0U;
	uint8_t downloadSuccessFlag[8] = { 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08 };

	while (i < sizeof(RxBuffer)) {
		a->dataLength = RxBuffer[i];

		a->recordType = RxBuffer[i + 3];

		for (uint8_t j = 0; j < a->dataLength; j++) {
			a->data[j] = RxBuffer[4 + j + i];
		}

		a->checkSum = RxBuffer[a->dataLength + 4 + i];

		// checksum
		if (checkCheckSum(a, i) == false) {
			// stop recieve, delete app backup
			deleteData(APP_UPDATE_START_ADDR, APP_UPDATE_END_ADDR);
			return;
		}

		i = a->dataLength + 5 + i;

		if (a->recordType == 0x02 || a->recordType == 0x04 || a->recordType == 0x03 || a->recordType == 0x05) {
			continue;
		} else if (a->recordType == 0x01) {
			//set dowload success flag
			writeDataToDFlash(DOWNLOAD_SUCCESS_ADDR, sizeof(downloadSuccessFlag), downloadSuccessFlag);
			return;
		}
		Address = APP_UPDATE_START_ADDR + nextAddress;
		//write data to memory...
		writeDataToDFlash(Address, a->dataLength, a->data);
		nextAddress += 0x00000010;
	}
}

void writeDataToDFlash(uint32_t address, uint8_t dataLength, uint8_t data[]) {
	status_t ret;
	uint32_t failAddr;
	/* Program data from the source buffer to DFlash */
	ret = FLASH_DRV_Program(&flashSSDConfig, address, dataLength, data);
	DEV_ASSERT(STATUS_SUCCESS == ret);

	/* Verify the program operation at margin level value of 1, user margin */
	ret = FLASH_DRV_ProgramCheck(&flashSSDConfig, address, dataLength, data, &failAddr, 1u);
	DEV_ASSERT(STATUS_SUCCESS == ret);
}

void writeDataToAppCurent() {
	status_t ret;
	uint32_t failAddr;
	uint32_t desAddress = APP_CURRENT_START_ADDR;
	uint32_t srcAddress = APP_UPDATE_START_ADDR;
	do {
		ret = FLASH_DRV_Program(&flashSSDConfig, desAddress, 0x10, (uint8_t*) srcAddress);
		DEV_ASSERT(STATUS_SUCCESS == ret);
		/* Verify the program operation at margin level value of 1, user margin */
		ret = FLASH_DRV_ProgramCheck(&flashSSDConfig, desAddress, 0x10, (uint8_t*) srcAddress, &failAddr, 1u);
		DEV_ASSERT(STATUS_SUCCESS == ret);
		desAddress += 0x00000010;
		srcAddress += 0x00000010;
	} while (srcAddress <= APP_UPDATE_END_ADDR);
}

void deleteData(uint32_t startAddress, uint32_t endAddress)
{
	status_t ret;
	uint32_t sectorSize = FEATURE_FLS_PF_BLOCK_SECTOR_SIZE * 13;

	ret = FLASH_DRV_EraseSector(&flashSSDConfig, startAddress, sectorSize);
	DEV_ASSERT(STATUS_SUCCESS == ret);

	ret = FLASH_DRV_VerifySection(&flashSSDConfig, startAddress, sectorSize / FTFx_PHRASE_SIZE, 1u);
	DEV_ASSERT(STATUS_SUCCESS == ret);

}

uint64_t readFlag(uint32_t address) {
	uint64_t data;
	data = *(volatile uint64_t*) (address);
	return data;
}

bool IsDowloadSuccess() {
	bool stt = true;
	uint64_t Value_Flag = 0;
	Value_Flag = readFlag(DOWNLOAD_SUCCESS_ADDR);
	if (Value_Flag != 0x807060504030201) {
		stt = false;
	}
	return stt;
}

bool IsUpdateSuccess() {
	bool stt = true;
    uint64_t Update_Flag = 0;
	Update_Flag = readFlag(UPDATE_SUCCESS_ADDR);
	if (Update_Flag != 0xABABABABABABABAB) {
		stt = false;
	}
	return stt;
}

void Jump_To_Application(uint32_t userSP)
{
   void (*entry)(void);
   uint32_t pc;
   if(userSP == 0xFFFFFFFF)
   {
      return;
   }
   else
   {
   /* Set up stack pointer */
      __asm("msr msp, r0");
      __asm("msr psp, r0");
   /* Relocate vector table */
      S32_SCB->VTOR = (uint32_t)APP_CURRENT_START_ADDR;
   /* Jump to application PC */
      pc = *((volatile uint32_t *)(0x0000D000+4));
      entry = (void (*)(void))pc;
      entry();
   }
}

void bootloader_main() {
	//if recieve require update from host, clear updateSuccessFlag

	if (IsUpdateSuccess()) {
		INT_SYS_DisableIRQGlobal();
		//jump to app
		Jump_To_Application(*((uint32_t*)APP_CURRENT_START_ADDR));
	} else {
		flash_callback_t pCallBack;
		status_t ret;
		uint8_t updateSuccessValue[8] = { 0xAB, 0xAB, 0xAB, 0xAB, 0xAB, 0xAB, 0xAB, 0xAB };
		uint32_t failAddr;

		ret = FLASH_DRV_Init(&flash1_InitConfig0, &flashSSDConfig);
		DEV_ASSERT(STATUS_SUCCESS == ret);

		pCallBack = (flash_callback_t) CCIF_Callback;
		flashSSDConfig.CallBack = pCallBack;

		/* Disable Callback */
		flashSSDConfig.CallBack = NULL_CALLBACK;

		deleteData(APP_UPDATE_START_ADDR, APP_UPDATE_END_ADDR);

		//recieve data
		hex_struct rxData;
		int khai = 0;
		while (khai < 4) {
			if (link.receive_status == RECEIVE_STATUS_FULL) {
				khai++;
				//OSIF_TimeDelay(3000);
				hexAnalysAndWriteToMemory(&rxData);
				link.receive_status = RECEIVE_STATUS_IDLE;
				 memset(RxBuffer, 0, sizeof(RxBuffer));
				 PINS_DRV_TogglePins(GPIO_PORT, 1 << LED1);
			}
		}

//		hexAnalysAndWriteToMemory(&rxData);
//		deleteData(APP_UPDATE_START_ADDR, APP_UPDATE_END_ADDR);
		if (IsDowloadSuccess()) {

			//delete app current
			deleteData(APP_CURRENT_START_ADDR, APP_CURRENT_END_ADDR);

			//copy data from App update to app current
			writeDataToAppCurent();

			//set update success flag
			writeDataToDFlash(UPDATE_SUCCESS_ADDR, sizeof(updateSuccessValue), updateSuccessValue);

			//reset
			S32_SCB->AIRCR = S32_SCB_AIRCR_VECTKEY(0x5FA) | S32_SCB_AIRCR_SYSRESETREQ(1);
		}
	}
}

int main(void) {
	/* Write your local variable definition here */

	/*** Processor Expert internal initialization. DON'T REMOVE THIS CODE!!! ***/
#ifdef PEX_RTOS_INIT
	PEX_RTOS_INIT(); /* Initialization of the selected RTOS. Macro is defined by the RTOS component. */
#endif
	/*** End of Processor Expert internal initialization.                    ***/

	/* Write your code here */
	CLOCK_SYS_Init(g_clockManConfigsArr, CLOCK_MANAGER_CONFIG_CNT,
			g_clockManCallbacksArr, CLOCK_MANAGER_CALLBACK_CNT);
	CLOCK_SYS_UpdateConfiguration(0U, CLOCK_MANAGER_POLICY_AGREEMENT);

	/*Init Project*/
	PINS_DRV_Init(NUM_OF_CONFIGURED_PINS, g_pin_mux_InitConfigArr);

	CANInit();
	GPIOInit();
	FLEXCAN_DRV_ConfigRxMb(INST_CANCOM1, RX_MAILBOX, &dataInfo, RX_MSG_ID);
	FLEXCAN_DRV_Receive(INST_CANCOM1, RX_MAILBOX, &recvBuff);
	//CANTPInit
	link.send_data = TxBuffer;
	link.send_offset = 0;
	link.send_sn = 0;
	link.send_bs_remain = 0;
	link.receive_data = RxBuffer;
	link.receive_buffer_size = sizeof(RxBuffer);
	link.receive_size = 0;    // dc quy dinh boi receive first frame
	link.receive_offset = 0;  // so byte da nhan
	link.receive_sn = 0;      // so thu tu frame nhan duoc
	link.block_size = 0; // tinh bang receive first frame truyen bang block size
	link.flow_status = PCI_FLOW_STATUS_CONTINUE;
	link.receive_status = RECEIVE_STATUS_IDLE;
	link.stmin = 10;   // minumum > 10ms


#ifndef FLASH_TARGET
#ifdef S32K144_SERIES
    MSCM->OCMDR[0u] |= MSCM_OCMDR_OCM1(0x3u);
    MSCM->OCMDR[1u] |= MSCM_OCMDR_OCM1(0x3u);
#endif /* S32K144_SERIES */
#endif /* FLASH_TARGET */
	/* Install interrupt for Flash Command Complete event */
	INT_SYS_InstallHandler(FTFC_IRQn, CCIF_Handler, (isr_t*) 0);
	INT_SYS_EnableIRQ(FTFC_IRQn);

	/* Enable global interrupt */
	INT_SYS_EnableIRQGlobal();

	bootloader_main();

	/* For example: for(;;) { } */

	/*** Don't write any code pass this line, or it will be deleted during code generation. ***/
  /*** RTOS startup code. Macro PEX_RTOS_START is defined by the RTOS component. DON'T MODIFY THIS CODE!!! ***/
  #ifdef PEX_RTOS_START
    PEX_RTOS_START();                  /* Startup of the selected RTOS. Macro is defined by the RTOS component. */
  #endif
  /*** End of RTOS startup code.  ***/
  /*** Processor Expert end of main routine. DON'T MODIFY THIS CODE!!! ***/
  for(;;) {
    if(exit_code != 0) {
      break;
    }
  }
  return exit_code;
  /*** Processor Expert end of main routine. DON'T WRITE CODE BELOW!!! ***/
} /*** End of main routine. DO NOT MODIFY THIS TEXT!!! ***/

/* END main */
/*!
 ** @}
 */

/*!
 \brief Interrupt handler for Flash Command Complete event
 */
void CCIF_Handler(void) {
	/* Disable Flash Command Complete interrupt */
	FTFx_FCNFG &= (~FTFx_FCNFG_CCIE_MASK);

	return;
}

/*!
 \brief Callback function for Flash operations
 */
START_FUNCTION_DEFINITION_RAMSECTION
void CCIF_Callback(void) {
	/* Enable interrupt for Flash Command Complete */
	if ((FTFx_FCNFG & FTFx_FCNFG_CCIE_MASK) == 0u) {
		FTFx_FCNFG |= FTFx_FCNFG_CCIE_MASK;
	}
}
END_FUNCTION_DEFINITION_RAMSECTION
/*
 ** ###################################################################
 **
 **     This file was created by Processor Expert 10.1 [05.21]
 **     for the NXP S32K series of microcontrollers.
 **
 ** ###################################################################
 */
