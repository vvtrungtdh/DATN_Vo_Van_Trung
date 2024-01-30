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
#include "clockMan1.h"
#include "canCom1.h"
#include "dmaController1.h"
#include "pin_mux.h"
#include "lpTmr1.h"
#include "Flash1.h"

#if CPU_INIT_CONFIG
#include "Init_Config.h"
#endif

#include <stdint.h>
#include <stdbool.h>

#include "CAN_standard.h"
#include "CANTP.h"
#include "UDS_ECU.h"
#include "DTC.h"

volatile int exit_code = 0;
#define NUMBER_OF_SAMPLE 200  //Each Operation cycles 10s/50ms = 200 sample
/* Variable */
uint8_t TxBuffer[4095];  // Buffer truyen
uint8_t RxBuffer[4095];  // Buffer nhan
uint8_t max_sample = 0;
/* Function prototypes
 *
 ******************************************************************************/
void buttonISR(void);
void GPIO_Init(void);
/*Function */
/* LPTMR IRQ handler */
void lptmrISR(void) {
	/* Clear compare flag */
	LPTMR_DRV_ClearCompareFlag(INST_LPTMR1);
	/*Implement*/
	static uint32_t reset_session = 0;
	sample_time++;
	max_sample = 0;
	static uint8_t warn = 0;
	reset_session ++;
	warn++;
	if (reset_session >= 10000) {
		reset_session = 0;
		comm.check_security = LOCK;
		comm.session_control = DEFAULT_SESSION;
	}
	if (warn >= 20) {
		warn = 0;
		DTCCurrent[0].Sensor.value -= 2;
		DTCCurrent[1].Sensor.value -= 2;
	}
	if (DTCCurrent[0].Sensor.value <= 20) {
		DTCCurrent[0].Sensor.value = 20;
	} else if (DTCCurrent[0].Sensor.value < DTCCurrent[0].Sensor.limit) {

		PINS_DRV_SetPins(GPIO_PORT, (1 << LED1));
	} else {
		PINS_DRV_TogglePins(GPIO_PORT, 1 << LED1);
	}

	if (DTCCurrent[1].Sensor.value <= 20) {
		DTCCurrent[1].Sensor.value = 20;
	}

	if (sample_time == 1) {
		lastOperationCycle = STOP;
		currentOperationCycle = START;
	} else if (sample_time == 2) {
		lastOperationCycle = START;
		currentOperationCycle = START;
	} else if (sample_time == (NUMBER_OF_SAMPLE - 1)) {
		lastOperationCycle = STOP;
		currentOperationCycle = STOP;
	} else if (sample_time >= NUMBER_OF_SAMPLE) {
		sample_time = 0;
		DTCCurrent[0].Snapshot(&DTCCurrent[0], &DTCHistory[0]);
		DTCCurrent[0].Sensor.Pretest = 0;
		DTCCurrent[0].Sensor.result = NORESULT;
		DTCCurrent[1].Snapshot(&DTCCurrent[1], &DTCHistory[1]);
		DTCCurrent[1].Sensor.Pretest = 0;
		DTCCurrent[1].Sensor.result = NORESULT;
	}
}

/**
 * Button interrupt handler
 */
void buttonISR(void) {
	/* Check if one of the buttons was pressed */
	uint32_t buttonsPressed = PINS_DRV_GetPortIntFlag(BTN_PORT)
			& ((1 << BTN1_PIN) | (1 << BTN2_PIN));
	uint32_t buttons = PINS_DRV_ReadPins(BTN_GPIO);
	if (buttonsPressed != 0) {
		switch (buttonsPressed) {
		case (1 << BTN1_PIN):
			if (buttons & ((1 << BTN2_PIN))) {
				// Nham 2 nut => Mo khoa
				PINS_DRV_TogglePins(GPIO_PORT, (1 << LED2));
			} else {
				// Nhan Nut Phai => Tang nhiet do
				DTCCurrent[0].Sensor.value += 10;
			}
			/* Clear interrupt flag */
			PINS_DRV_ClearPinIntFlagCmd(BTN_PORT, BTN1_PIN);
			break;
		case (1 << BTN2_PIN):
			// Nhan Nut Trai => Giam nhiet do
			DTCCurrent[1].Sensor.value += 10;
			/* Clear interrupt flag */
			PINS_DRV_ClearPinIntFlagCmd(BTN_PORT, BTN2_PIN);
			break;
		default:
			PINS_DRV_ClearPortIntFlagCmd(BTN_PORT);
			break;
		}
	}
}

void GPIO_Init(void) {
	/* Output direction for LEDs */
	PINS_DRV_SetPinsDirection(GPIO_PORT,
			(1 << LED2) | (1 << LED1) | (1 << LED0));

	/* Set Output value LEDs */
	PINS_DRV_ClearPins(GPIO_PORT, 1 << LED1);
//	PINS_DRV_SetPins(GPIO_PORT, 1 << LED2);

	/* Setup button pin */
	PINS_DRV_SetPinsDirection(BTN_GPIO, ~((1 << BTN1_PIN) | (1 << BTN2_PIN)));

	/* Setup button pins interrupt */
	PINS_DRV_SetPinIntSel(BTN_PORT, BTN1_PIN, PORT_INT_RISING_EDGE);
	PINS_DRV_SetPinIntSel(BTN_PORT, BTN2_PIN, PORT_INT_RISING_EDGE);

	/* Install buttons ISR */
	INT_SYS_InstallHandler(BTN_PORT_IRQn, &buttonISR, NULL);

	/* Enable buttons interrupt */
	INT_SYS_EnableIRQ(BTN_PORT_IRQn);
}

/* User includes (#include below this line is not maintained by Processor Expert) */

/*! 
 \brief The main function for the project.
 \details The startup initialization sequence is the following:
 * - startup asm routine
 * - main()
 */
int main(void) {
	/* Write your local variable definition here */

	/*** Processor Expert internal initialization. DON'T REMOVE THIS CODE!!! ***/
#ifdef PEX_RTOS_INIT
	PEX_RTOS_INIT(); /* Initialization of the selected RTOS. Macro is defined by the RTOS component. */
#endif
	/*** End of Processor Expert internal initialization.                    ***/
	CLOCK_SYS_Init(g_clockManConfigsArr, CLOCK_MANAGER_CONFIG_CNT,
			g_clockManCallbacksArr, CLOCK_MANAGER_CALLBACK_CNT);
	CLOCK_SYS_UpdateConfiguration(0U, CLOCK_MANAGER_POLICY_AGREEMENT);

	//Flash
	FLASH_DRV_Init(&Flash1_InitConfig0, &flashSsdConfig);
// LPTMR
	PINS_DRV_Init(NUM_OF_CONFIGURED_PINS, g_pin_mux_InitConfigArr);
	LPTMR_DRV_Init(INST_LPTMR1, &lpTmr1_config0, false);
	/* Install IRQ handler for LPTMR interrupt */
	INT_SYS_InstallHandler(LPTMR0_IRQn, &lptmrISR, (isr_t *) 0);
	/* Enable IRQ for LPTMR */
	INT_SYS_EnableIRQ(LPTMR0_IRQn);

	/* Start LPTMR counter */
	LPTMR_DRV_StartCounter(INST_LPTMR1);
	CANInit();
	GPIO_Init();
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

	comm.Key = 0xFFFF;
	comm.check_security = LOCK;
	comm.session_control = DEFAULT_SESSION;
	comm.maxNumberOfBlockLength = 25; // 100 Kb = 20 x 4096 byte
	// Tao 1 Buffer
	for (int i = 0; i < 4000; i++) {
		TxBuffer[i] = 0x01;
		RxBuffer[i] = 0x41;
	}

	/*DTC*/
	DTCCurrent[0].confirmStage = INITIAL_MONITOR;
	DTCCurrent[0].agingStatus = 0;
	DTCCurrent[1].confirmStage = INITIAL_MONITOR;
	DTCCurrent[1].agingStatus = 0;
	while (1) {
		processUDSService(&comm, request);
		if (max_sample < sample_time) {
			DTCStatus(&DTCCurrent[0], &DTCData_Recent[0]);
			DTCStatus(&DTCCurrent[1], &DTCData_Recent[1]);
			max_sample = NUMBER_OF_SAMPLE;
		}
	}
	/* Write your code here */
	/* For example: for(;;) { } */

	/*** Don't write any code pass this line, or it will be deleted during code generation. ***/
	/*** RTOS startup code. Macro PEX_RTOS_START is defined by the RTOS component. DON'T MODIFY THIS CODE!!! ***/
#ifdef PEX_RTOS_START
	PEX_RTOS_START(); /* Startup of the selected RTOS. Macro is defined by the RTOS component. */
#endif
	/*** End of RTOS startup code.  ***/
	/*** Processor Expert end of main routine. DON'T MODIFY THIS CODE!!! ***/
	for (;;) {
		if (exit_code != 0) {
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

/*
 ** ###################################################################
 **
 **     This file was created by Processor Expert 10.1 [05.21]
 **     for the NXP S32K series of microcontrollers.
 **
 ** ###################################################################
 */
