#ifndef UDS_ECU_H_
#define UDS_ECU_H_

#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#include "CAN_standard.h"
#include "Flash1.h"
#include "DTC.h"

/*RETURN STATUS*/
#define R_OK 0
#define R_WRONG 1
#define R_ERROR 2
/*SESSION CONTROL*/
#define DEFAULT_SESSION 0x03
#define CUSTOM_SESSION 0x04
/*SECURITY*/
#define LOCK 0x05
#define UNLOCK 0x06
/*NRC STATUS*/
#define NRC_SUBFUNCTION 0x12   // SUB KHONG DUOC HO TRO
#define NRC_LENGTH 0x13        // DO DAI KHONG HOP LE
#define NRC_CONDITION 0x22      // KHONG THOA MAN DIEU KIEN THUC HIEN
#define NRC_INVALID 0x31       // KHONG HOP LE
#define NRC_KEY 0x35           // SAI KEY
/*DID SUB*/
#define BootSoftwareIdentificationDataIdentifier 0xF180
#define applicationSoftwareIdentificationDataIdentifier 0xF181

/*SERVICE SIZE*/
#define SIZE_REQUEST 7
#define NULL_DATA 0x00
#define TIME_MIN_HB 0x00 // min time out = 50ms
#define TIME_MIN_LB 0x32
#define TIME_MAX_HB 0x01 // max time out = 5000ms;
#define TIME_MAX_LB 0xF4
#define MAX_NUMBER_OF_BLOCK 0x19  // 100kb = 25 block x 4095 byte
#define DOWNLOAD_SUCCESS_ADDR 0x19000
#define UPDATE_SUCCESS_ADDR 0x19100
#define DID_ADDRESS 0x19000
/* Variable */
extern uint8_t request[7]; // Du lieu nhan duoc tu client
extern flash_ssd_config_t flashSsdConfig;

/* Supported Service */
typedef enum
{
    DIAGNOSTIC_SESSION_CONTROL = 0x10,
    ECU_RESET = 0x11,
    CLEAR_DIAGNOSTIC_INFO = 0x14,
    READ_DTC_INFO = 0x19,
    READ_DATA_BY_IDENTIFIER = 0x22,
    WRITE_DATA_BY_IDENTIFIER = 0x2E,
    SECURITY_ACCESS = 0x27,
    REQUEST_DOWNLOAD = 0x34,
    //...
    NUM_UDS_SERVICES

} UDS_Service;

// Link
typedef struct
{
	uint8_t ID[4];
    uint8_t check_security;
    uint8_t session_control;
    uint16_t Key;
    uint32_t memoryAddress;
    uint32_t memorySize;
    uint8_t maxNumberOfBlockLength;
} Comm;
extern Comm comm;

/*S10*/
typedef struct
{
    uint8_t SID;
    uint8_t SUB;
    uint8_t time_min_highbyte;
    uint8_t time_min_lowbyte;
    uint8_t time_max_highbyte;
    uint8_t time_max_lowbyte;
    uint8_t Empty;
} service10;

/*S11*/
typedef struct
{
    uint8_t SID;
    uint8_t SUB;
    uint8_t Empty[5];
} service11;

/*S19*/
typedef struct
{
uint8_t SID;
uint8_t SUB;
uint8_t NUM;
uint8_t byte_mid;
uint8_t byte_low;
uint8_t byte_status;
uint8_t Empty;
}service19;

/*S22*/
typedef struct
{
    uint8_t SID;
    uint8_t MSB;
    uint8_t LSB;
    uint8_t dataRecord1;
    uint8_t dataRecord2;
    uint8_t dataRecord3;
    uint8_t dataRecord4;
} service22;

/*S2E*/
typedef struct
{
    uint8_t SID;
    uint8_t MSB;
    uint8_t LSB;
    uint8_t Empty[4];
} service2E;

/*S27*/
typedef struct
{
    uint8_t SID;           // byte 1 response SID
    uint8_t SUB;           // byte 2 response SUP == request Seed or send Key
    uint8_t highbyte_seed; // neu SUB == 0x02 thi seed = 0x00
    uint8_t lowbyte_seed;
    uint8_t Empty[3]; // byte k su dung
} service27;

/*S34*/
typedef struct
{
    uint8_t SID;
    uint8_t lengthFormatIdentifier;
    uint8_t maxNumberOfBlockLength1;
    uint8_t Empty[4];
} service34;

/*NEGATIVE*/
typedef struct
{
    uint8_t negative;
    uint8_t SID;
    uint8_t responseCode;
    uint8_t Empty[4];
} Negative;

/*Send frame*/
typedef struct
{
    uint8_t ptr[7];
} dataArray;

// ISO
typedef struct
{
    union
    {
        service10 service10;
        service11 service11;
        service19 service19;
        service22 service22;
        service2E service2E;
        service27 service27;
        service34 service34;
        Negative Negative;
        dataArray data;
    } as;
} ISO;

//////////////////PROCESS FUNCTION/////////////////////
/*ECU processing*/
int processUDSService(Comm *comm, uint8_t request[7]);

/*Service function*/
int diagnosticSessionControlHandler(Comm *comm, uint8_t *request);
int ecuResetHandler(Comm *comm, uint8_t *request);
int clearDiagnosticInfoHandler(Comm *comm, uint8_t *request);
int readDtcInfoHandler(Comm *comm, uint8_t *request);
int readDataByIdentifierHandler(Comm *comm, uint8_t *request);
int writeDataByIdentifierHandler(Comm *comm, uint8_t *request);
int securityAccessHandler(Comm *comm, uint8_t *request);
int requestDownloadHandler(Comm *comm, uint8_t *request);
extern int (*serviceHandlers[NUM_UDS_SERVICES])(Comm *comm, uint8_t *);

/*API function*/
uint16_t generateSeed();
uint16_t maHoa(uint8_t byte1, uint8_t byte2);
void writeDataToDFlash(uint32_t address, uint8_t dataLength, uint8_t data[]);
#endif /* UDS_ECU_H_ */
