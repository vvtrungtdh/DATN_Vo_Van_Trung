#ifndef CAN_STANDARD_H_
#define CAN_STANDARD_H_

/* Including needed modules to compile this module/procedure */
#include "Cpu.h"
#include "clockMan1.h"
#include "canCom1.h"
#include "dmaController1.h"
#include "pin_mux.h"
#if CPU_INIT_CONFIG
#include "Init_Config.h"
#endif

#include "CANTP.h"
#include <stdint.h>
#include <stdbool.h>
#include "UDS_ECU.h"
/******************************************************************************
 * Definitions
 ******************************************************************************/

/* This example is setup to work by default with EVB. To use it with other boards
   please comment the following line
*/

#define EVB

#ifdef EVB
#define LED_PORT PORTD
#define GPIO_PORT PTD
#define PCC_INDEX PCC_PORTD_INDEX
#define LED0 15U
#define LED1 16U
#define LED2 0U

#define BTN_GPIO PTC
#define BTN1_PIN 13U
#define BTN2_PIN 12U
#define BTN_PORT PORTC
#define BTN_PORT_IRQn PORTC_IRQn
#else
#define LED_PORT PORTC
#define GPIO_PORT PTC
#define PCC_INDEX PCC_PORTC_INDEX
#define LED0 0U
#define LED1 1U
#define LED2 2U

#define BTN_GPIO PTC
#define BTN1_PIN 13U
#define BTN2_PIN 12U
#define BTN_PORT PORTC
#define BTN_PORT_IRQn PORTC_IRQn
#endif

/* Use this define to specify if the application runs as master or slave */
#define MASTER
/*#define MASTER SLAVE*/

/* Definition of the TX and RX message buffers depending on the bus role */
#if defined(MASTER)
#define TX_MAILBOX (1UL)
#define TX_MSG_ID (1UL)    //ID NODE muĂ¡Â»â€˜n gĂ¡Â»Â­i dĂ¡ÂºÂ¿n
#define RX_MAILBOX (0UL)
#define RX_MSG_ID (2UL)    //ID NODE nÄ‚Â y
#elif defined(SLAVE)
#define TX_MAILBOX (0UL)
#define TX_MSG_ID (2UL)
#define RX_MAILBOX (1UL)
#define RX_MSG_ID (1UL)
#endif
typedef enum
{
    LED0_CHANGE_REQUESTED = 0x00U,
    LED1_CHANGE_REQUESTED = 0x01U
} can_commands_list;
extern 	uint8_t convert_Buff[8];
extern uint8_t callback_test;
extern uint8_t ledRequested;
/* Define receive buffer */
extern flexcan_msgbuff_t recvBuff;
extern flexcan_data_info_t dataInfo;

/******************************************************************************
 * Function prototypes
 ******************************************************************************/
void SendCANData(uint32_t mailbox, uint32_t messageId, uint8_t *data, uint32_t len);
void CANInit(void);
void can0_Callback(uint8_t instance, flexcan_event_type_t eventType,
                       uint32_t buffIdx, flexcan_state_t *flexcanState);
void can0_ErrorCallback(uint8_t instance, flexcan_event_type_t eventType,
                            flexcan_state_t *flexcanState);
int implementCode ();
#endif /*CAN_STANDARD_H_*/
