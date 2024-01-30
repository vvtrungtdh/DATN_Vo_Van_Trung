#ifndef UDS_BOOT_H_
#define UDS_BOOT_H_

#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#include <CAN_standard.h>

/*RETURN STATUS*/
#define R_OK 0
#define R_WRONG 1
#define R_ERROR 2
#define R_EXIT -1
/*SESSION CONTROL*/
#define DEFAULT_SESSION 0x03
#define CUSTOM_SESSION 0x04
/*SECURITY*/
#define LOCK 0x05
#define UNLOCK 0x06
/*NRC STATUS*/
#define NRC_SID 0x11
#define NRC_SUBFUNCTION 0x12
#define NRC_LENGTH 0x13
#define NRC_CONDITON 0x22
#define NRC_INVALID 0x31
#define NRC_KEY 0x35
#define SIZE_REQUEST 7
/* Variable */
extern uint8_t request[7]; // Du lieu nhan duoc tu client

/* Supported Service */
typedef enum
{
    DIAGNOSTIC_SESSION_CONTROL = 0x10,
    ECU_RESET = 0x11,
    SECURITY_ACCESS = 0x27,
    TRANSFER_DATA = 0x36,
    REQUEST_TRANSFER_EXIT = 0x37,
    //...
    NUM_UDS_SERVICES

} UDS_Service;

// Link
typedef struct
{
    uint8_t check_security;
    uint8_t session_control;
    uint16_t Key;
    uint32_t memoryAddress;
    uint32_t memorySize;
    uint8_t maxNumberOfBlockLength;
} Comm;
extern Comm comm;

/*S11*/
typedef struct
{
    uint8_t SID;
    uint8_t SUB;
    uint8_t Empty[5];
} service11;

/*S27*/
typedef struct
{
    uint8_t SID;           // byte 1 response SID
    uint8_t SUB;           // byte 2 response SUP == request Seed or send Key
    uint8_t highbyte_seed; // neu SUB == 0x02 thi seed = 0x00
    uint8_t lowbyte_seed;
    uint8_t Empty[3]; // byte k su dung
} service27;

/*S36*/
typedef struct
{
    uint8_t SID;
    uint8_t blockSequenceCounter;
    uint8_t Empty[5];
} service36;

/*S37*/
typedef struct
{
    uint8_t SID;
    uint8_t Empty[6];
} service37;

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
        service11 service11;
        service27 service27;
        service36 service36;
        service37 service37;
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
int securityAccessHandler(Comm *comm, uint8_t *request);
int transferDataHandler(Comm *comm, uint8_t *request);
int requestTransferExitHandler(Comm *comm, uint8_t *request);
extern int (*serviceHandlers[NUM_UDS_SERVICES])(Comm *comm, uint8_t *);

/*API function*/
uint16_t generateSeed();
uint16_t maHoa(uint8_t byte1, uint8_t byte2);
#endif /* UDS_BOOT_H_ */