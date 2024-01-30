#include "CAN_standard.h"
#include "CANTP.h"
#include <stdint.h>
uint8_t convert_Buff[8];
uint8_t callback_test = 0;
uint8_t ledRequested = (uint8_t) LED0_CHANGE_REQUESTED;
flexcan_msgbuff_t recvBuff;
flexcan_data_info_t dataInfo;
flexcan_data_info_t dataInfo = { .data_length = 8U, .msg_id_type =
		FLEXCAN_MSG_ID_STD, .enable_brs = true, .fd_enable = true, .fd_padding =
		0U };

/******************************************************************************
 * Functions
 ******************************************************************************/

void can0_Callback(uint8_t instance, flexcan_event_type_t eventType,
		uint32_t buffIdx, flexcan_state_t *flexcanState) {

	(void) flexcanState;
	(void) instance;

	switch (eventType) {
	case FLEXCAN_EVENT_RX_COMPLETE:
		callback_test |= 0x1; // set bit0 to to evidence RX was complete
		if (buffIdx == RX_MAILBOX) {
			// Implement code
			implementCode();
			/* Start receiving data in RX_MAILBOX again. */
			FLEXCAN_DRV_Receive(INST_CANCOM1, RX_MAILBOX, &recvBuff);
		}

		break;
	case FLEXCAN_EVENT_RXFIFO_COMPLETE:
		break;
	case FLEXCAN_EVENT_DMA_COMPLETE:
		break;
	case FLEXCAN_EVENT_TX_COMPLETE:
		callback_test |= 0x2; // set bit1 to to evidence TX was complete
		PINS_DRV_SetPins(GPIO_PORT, 1 << LED2);
		break;
	default:
		break;
	}
}

void can0_ErrorCallback(uint8_t instance, flexcan_event_type_t eventType,
		flexcan_state_t *flexcanState) {
	volatile uint32_t error;

	(void) flexcanState;
	(void) instance;

	switch (eventType) {
	case FLEXCAN_EVENT_ERROR:
		callback_test |= 0x4; // set bit2 to to evidence error ISR hit

		error = FLEXCAN_DRV_GetErrorStatus(INST_CANCOM1);

		if (error & 0x4) // if BOFFINT was set
				{
			callback_test |= 0x8; // set bit3 to to evidence bus off ISR hit

			// abort TX MB, after bus off recovery message is not send
			FLEXCAN_DRV_AbortTransfer(INST_CANCOM1, TX_MAILBOX);

			PINS_DRV_ClearPins(GPIO_PORT, 1 << LED2);
		}

		break;

	default:
		break;
	}
}

void SendCANData(uint32_t mailbox, uint32_t messageId, uint8_t *data,
		uint32_t len) {
	flexcan_data_info_t dataInfo = { .data_length = len, .msg_id_type =
			FLEXCAN_MSG_ID_STD, .enable_brs = true, .fd_enable = true,
			.fd_padding = 0U };

	/* Configure TX message buffer with index TX_MSG_ID and TX_MAILBOX*/
	FLEXCAN_DRV_ConfigTxMb(INST_CANCOM1, mailbox, &dataInfo, messageId);

	/* Execute send non-blocking */
	FLEXCAN_DRV_Send(INST_CANCOM1, mailbox, &dataInfo, messageId, data);
}

void CANInit(void) {
	CLOCK_SYS_Init(g_clockManConfigsArr, CLOCK_MANAGER_CONFIG_CNT,
			g_clockManCallbacksArr, CLOCK_MANAGER_CALLBACK_CNT);
	CLOCK_SYS_UpdateConfiguration(0U, CLOCK_MANAGER_POLICY_FORCIBLE);

	/* Initialize pins
	 *  -   Init FlexCAN and GPIO pins
	 *  -   See PinSettings component for more info
	 */
	PINS_DRV_Init(NUM_OF_CONFIGURED_PINS, g_pin_mux_InitConfigArr);
	FLEXCAN_DRV_Init(INST_CANCOM1, &canCom1_State, &canCom1_InitConfig0);
	FLEXCAN_DRV_InstallEventCallback(INST_CANCOM1,
			(flexcan_callback_t) can0_Callback, (void *) NULL);
	FLEXCAN_DRV_InstallErrorCallback(INST_CANCOM1,
			(flexcan_error_callback_t) can0_ErrorCallback, (void *) NULL);
}

int implementCode() {
	for (uint8_t i = 0; i < 8; i++) {
		convert_Buff[i] = recvBuff.data[i];
	}
	can_message(&link, convert_Buff);
	PINS_DRV_TogglePins(GPIO_PORT, (1 << LED0));
	return 0;
}
