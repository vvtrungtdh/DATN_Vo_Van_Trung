/*
  CANTP version 5 - CANTP Driver
  - SingleFrame: Ă„ï¿½Ă¡ÂºÂ¡i diĂ¡Â»â€¡n cho khung CAN Ă„â€˜Ă†Â¡n.
  - FirstFrame: Ă„ï¿½Ă¡ÂºÂ¡i diĂ¡Â»â€¡n cho khung CAN Ă„â€˜Ă¡ÂºÂ§u tiÄ‚Âªn.
  - ConsecutiveFrame: Ă„ï¿½Ă¡ÂºÂ¡i diĂ¡Â»â€¡n cho khung CAN liÄ‚Âªn tĂ¡Â»Â¥c.
  - FlowControl: Ă„ï¿½Ă¡ÂºÂ¡i diĂ¡Â»â€¡n cho khung Ă„â€˜iĂ¡Â»ï¿½u khiĂ¡Â»Æ’n luĂ¡Â»â€œng.
  - CanMessage: Union bao gĂ¡Â»â€œm cÄ‚Â¡c loĂ¡ÂºÂ¡i khung khÄ‚Â¡c nhau.
  Enums:
  - ProtocolControlInformation: Ă„ï¿½Ă¡Â»â€¹nh nghĂ„Â©a cÄ‚Â¡c loĂ¡ÂºÂ¡i khung.
  - FlowStatus: Ă„ï¿½Ă¡Â»â€¹nh nghĂ„Â©a trĂ¡ÂºÂ¡ng thÄ‚Â¡i Ă„â€˜iĂ¡Â»ï¿½u khiĂ¡Â»Æ’n luĂ¡Â»â€œng.
  CÄ‚Â¡c chĂ¡Â»Â©c nĂ„Æ’ng:
  - GĂ¡Â»Â­i vÄ‚Â  nhĂ¡ÂºÂ­n cÄ‚Â¡c loĂ¡ÂºÂ¡i khung CAN.
  - XĂ¡Â»Â­ lÄ‚Â½ tin nhĂ¡ÂºÂ¯n CAN, thĂ¡Â»ï¿½i gian chĂ¡Â»ï¿½, gĂ¡Â»Â­i vÄ‚Â  xĂ¡Â»Â­ lÄ‚Â½ dĂ¡Â»Â¯ liĂ¡Â»â€¡u.
  TÄ‚Â¡c giĂ¡ÂºÂ£: VÄ‚Âµ VĂ„Æ’n Trung
  NgÄ‚Â y: 12/12/2023
*/

#ifndef CANTP_H_
#define CANTP_H_

#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <stdbool.h>
#include "CAN_standard.h"

#define RET_OK 0
#define RET_ERROR -1
#define RET_INPROGRESS -2
#define RET_OVERFLOW -3
#define RET_WRONG_SN -4
#define RET_NO_DATA -5
#define RET_TIMEOUT -6
#define RET_LENGTH -7
typedef struct {
	uint8_t SF_DL :4;
	uint8_t type :4;
	uint8_t data[7];
} SingleFrame;

typedef struct {
	uint8_t FF_DL_high :4;
	uint8_t type :4;
	uint8_t FF_DL_low;
	uint8_t data[6];
} FirstFrame;

typedef struct {
	uint8_t SN :4;
	uint8_t type :4;
	uint8_t data[7];
} ConsecutiveFrame;

typedef struct {
	uint8_t FS :4;
	uint8_t type :4;
	uint8_t BS;
	uint8_t STmin;
	uint8_t reserve[5];
} FlowControl;

typedef enum {
	PCI_TYPE_SINGLE = 0x0,
	PCI_TYPE_FIRST_FRAME = 0x1,
	TYPE_CONSECUTIVE_FRAME = 0x2,
	TYPE_FLOW_CONTROL_FRAME = 0x3
} ProtocolControlInformation;

typedef enum {
	PCI_FLOW_STATUS_CONTINUE = 0x0,
	PCI_FLOW_STATUS_WAIT = 0x1,
	PCI_FLOW_STATUS_OVERFLOW = 0x2
} FlowStatus;

typedef struct {
	uint8_t ptr[8];
} DataArray;

typedef struct {
	uint8_t reserve_1 :4;
	uint8_t type :4;
	uint8_t reserve_2[7];
} PciType;
typedef struct {
	union {
		PciType common;
		SingleFrame single_frame;
		FirstFrame first_frame;
		ConsecutiveFrame consecutive_frame;
		FlowControl flow_control;
		DataArray data_array;
	} as;
} CanMessage;

typedef enum {
	RECEIVE_STATUS_IDLE,       //  dang khong nhan
	RECEIVE_STATUS_INPROGRESS, // dang nhan chua xong
	RECEIVE_STATUS_FULL,       // nhan full
} ReceiveStatusType;

typedef enum {
	SEND_STATUS_IDLE,       // dang khong gui
	SEND_STATUS_INPROGRESS, // dang gui chua xong
	SEND_STATUS_ERROR,      // gui loi
} SendStatusType;

typedef struct Link {
	uint16_t *send_size;  // tro den bien size la size cua data gui di
	uint8_t *send_data;   // tro den Buffer gui di Buffer
	uint16_t send_offset; // so byte da gui di
	uint16_t send_sn;     // so thu tu block size gui di
	uint8_t send_bs_remain;

	uint16_t receive_size;
	uint16_t receive_offset;
	uint8_t *receive_data;
	uint8_t receive_sn;
	uint8_t receive_status;
	uint16_t receive_buffer_size;

	uint8_t block_size;      // so block size gui di de yeu cau sender
	uint8_t number_of_block; // so lan gui flowcontrol frame
	uint8_t remaining_block; // so luong block size con lai de gui di flow con trol trong lan tiep theo
	uint8_t stmin;           // stmin gui cho sender de gui consecutive frame
	uint8_t flow_status;     // trang thai hien tai cua thiet bi
} Link;

extern Link link;    // link

/* Cho mot khoang thoi gian cu the */
void wait(uint32_t milliseconds);

/* Gui khung du lieu don (single frame) */
int send_single_frame(Link *link, uint32_t messageId);

/* Gui khung du lieu dau tien (first frame) */
int send_first_frame(Link *link, uint32_t messageId);

/* Gui khung dieu khien luong (flow control) */
int send_flow_control(Link *link);

/* Gui khung du lieu lien tuc (consecutive frame) */
int send_consecutive_frame(Link *link);

/* Nhan khung du lieu don (single frame) */
int receive_single_frame(Link *link, CanMessage *message);

/* Nhan khung du lieu dau tien (first frame) */
int receive_first_frame(Link *link, CanMessage *message);

/* Nhan khung dieu khien luong (flow control) */
int receive_flow_control(CanMessage *message);

/* Nhan khung du lieu lien tuc (consecutive frame) */
int receive_consecutive_frame(Link *link, CanMessage *message);

/* Xu ly thong diep CAN */
int can_message(Link *link, uint8_t *data);

/* Gui du lieu */
int sendData(Link *link, uint16_t size);

#endif /*CANTP_H_*/