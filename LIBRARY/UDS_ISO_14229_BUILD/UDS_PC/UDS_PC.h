#ifndef UDS_PC_H_
#define UDS_PC_H_

#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

// #include "CAN_standard.h"

// Example define send frame
#define TX_MAILBOX 0
#define TX_MSG_ID 0
void SendCANData(uint8_t a, uint8_t b, uint8_t *request, uint8_t c);

// End example

/*RETURN STATUS*/
#define R_OK 0
#define R_WRONG 1
#define R_ERROR 2
/*NRC STATUS*/
#define WRONG_SUBFUNCTION 0x12
#define WRONG_LENGTH 0x13
#define WRONG_CONDITON 0x22
#define WRONG_KEY 0x35
/*DID SUB*/
#define BootSoftwareIdentificationDataIdentifier 0xF180
#define applicationSoftwareIdentificationDataIdentifier 0xF181

extern uint8_t request[7];

typedef enum
{
    ECU_RESET = 0x51,
    READ_DATA_BY_IDENTIFIER = 0x62,
    WRITE_DATA_BY_IDENTIFIER = 0x6E,
    SECURITY_ACCESS = 0x67,
    REQUEST_DOWNLOAD = 0x74,
    TRANSFER_DATA = 0x76,
    REQUEST_TRANSFER_EXIT = 0x77,
    NEGATIVE = 0x7F,
    NUM_UDS_SERVICES

} UDS_Service;

/*S11*/
typedef struct
{
    uint8_t SID;
    uint8_t SUB;
    uint8_t Empty[5];
} service11;

/*S22*/
typedef struct
{
    uint8_t SID;
    uint8_t MSB;
    uint8_t LSB;
    uint8_t Empty[4];
} service22;

/*S2E*/
typedef struct
{
    uint8_t SID;
    uint8_t MSB;
    uint8_t LSB;
    uint8_t dataRecord1;
    uint8_t dataRecord2;
    uint8_t dataRecord3;
    uint8_t dataRecord4;
} service2E;

/*S27*/
typedef struct
{
    uint8_t SID;          // 0x27
    uint8_t SUB;          // 0x01  == request Seed or send Key
    uint8_t highbyte_key; // neu SUB == 0x01 thi key = 0x00
    uint8_t lowbyte_key;
    uint8_t Empty[3]; // 0x00
} service27;

/*S34*/
typedef struct
{
    uint8_t SID;
    uint8_t memoryAddress1;
    uint8_t memoryAddress2;
    uint8_t memoryAddress3;
    uint8_t memorySize1;
    uint8_t memorySize2;
    uint8_t memorySize3;
} service34;

/*S37*/
typedef struct
{
    uint8_t SID;
    uint8_t Empty[6];
} service37;

// Message Data single frame CAN
typedef struct
{
    uint8_t ptr[7];
} dataArray;

// ISO
typedef struct
{
    union
    {
        service11 service11;
        service22 service22;
        service2E service2E;
        service27 service27;
        service34 service34;
        service37 service37;
        dataArray data;
    } as;
} ISO;

///////////////////////////////////////////////////////
//////////////////PROCESS FUNCTION/////////////////////
///////////////////////////////////////////////////////
/*ECU Processing*/
int processUDSService(uint8_t request[7]);

/*Service Function*/
int ecuResetHandler(uint8_t *request);
int readDataByIdentifierHandler(uint8_t *request);
int writeDataByIdentifierHandler(uint8_t *request);
int securityAccessHandler(uint8_t *request);
int requestDownloadHandler(uint8_t *request);
int transferDataHandler(uint8_t *request);
int requestTransferExitHandler(uint8_t *request);
int negativeResponse(uint8_t *request);
extern int (*serviceHandlers[NUM_UDS_SERVICES])(uint8_t *);

/*API Function*/
uint16_t maHoa(uint8_t byte1, uint8_t byte2);
uint16_t DIDBootSoftwareIdentifier();
uint16_t DIDApplicationSoftwareIdentifier();

/*Send Function*/
int send_s10_st1();
int send_s11_st1();
int send_s22_st1();
int send_s2E_st1(uint16_t (*getDataIdentifier)(), uint32_t dataRecord);
int send_s27_st1();
int send_s34_st1();
int send_s37_st1();
#endif /* UDS_PC_H_ */
