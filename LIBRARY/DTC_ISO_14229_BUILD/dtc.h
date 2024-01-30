
/*

Group of DTC Definition (Table D.1 ISO 14229-1)

0x000000 - 0x0000FF:  This range of values is reserved for future legislative requirements.
-------------------------------------------------------------------------------------------
0x0000FF - 0xFFFEFF:  To be determined by vehicle manufacturer
0x000100 - 0x01FFFF:  Control Group: engine control system.
0x020000 - 0x03FFFF:  Powertrain Group: engine and transmission
0x040000 - 0x05FFFF:  Chassis Group:
0x060000 - 0x07FFFF:  Body Group:
0x080000 - 0x09FFFF:  Network Communication Group
0x0A0000 - 0x0BFFFF:
0x0C0000 - 0x0DFFFF:
0x0E0000 - 0x0FFFFF:

-------------------------------------------------------------------------------------------
0xFFFF00 - 0xFFFFFE:  Functional Group Identifier
        0x00 - 0x32:   ISO/SAE reserved
        0x33       :   Emissions-system group
        0x34 - 0xCF:   ISO/SAE reserved
        0xD0       :   Safety-system group
        0xD1 - 0xDF:   Legislative system group
        0xE0 - 0xFD:   ISO/SAE reserved
        0xFE       :   VOBD system
        0xFF       :   All functional system groups
-------------------------------------------------------------------------------------------
DTCs Definition: Defines the DTC that the ECU supports
Byte 1 (0x01): Control Group: engine control system.
Byte 2 (0x01): engine cooling system.
DTCx: 0x01010x
Byte 3 (0x00): Engine cooling control valve problem.
Byte 3 (0x01): Engine cooling thermostat Failure.
Byte 3 (0x02): Engine cooling Coolant Leak.
Byte 3 (0x03): Engine cooling Fan Failure.
Byte 3 (0x04): Engine cooling Water Pump Failure.
Byte 3 (0x05): Engine cooling Sensor Issues.
Byte 3 (0x06): ...
Byte 3 (0x07): ...
Byte 3 (0x08): ...
Byte 3 (0x09): ...
*/

#ifndef DTC_H_
#define DTC_H_

#include <stdint.h>
#include <stdbool.h>

extern struct DTCInformation *HistoryDTC; // LÆ°u trá»¯ cac DTC Ä‘Æ°á»£c xĂ¡c nháº­n.
extern struct DTCInformation *DTCRecent;  // LÆ°u trá»¯ cĂ¡c DTC gáº§n Ä‘Ă¢y sá»­ dá»¥ng cho viá»‡c cáº­p nháº­t tráº¡ng thĂ¡i thanh ghi StatusOfDTC
extern struct DTCInformation *DTCCurrent; // LÆ°u trá»¯ cĂ¡c DTC hiá»‡n táº¡i
// Dá»¯ liá»‡u áº£nh chá»¥p DTC táº¡i thá»�i Ä‘iá»ƒm xĂ¡c nháº­n
struct DTCSnapshot
{
    uint32_t DTCConfirmationTime; // Thá»�i gian xáº£y ra lá»—i Ä‘Æ°á»£c xĂ¡c nháº­n.
    int sensorValueConfirmed;     // GiĂ¡ trá»‹ cáº£m biáº¿n Ä‘o Ä‘Æ°á»£c.
};
// Ä�á»‹nh nghÄ©a struct DTCInformation Ä‘á»ƒ lÆ°u trá»¯ thĂ´ng tin DTC
struct DTCInformation
{
    uint8_t DTCHighByte;   // DTCAndStatusRecord
    uint8_t DTCMiddleByte; // DTCAndStatusRecord
    uint8_t DTCLowByte;    //  DTCAndStatusRecord
    uint8_t StatusOfDTC;   // DTCAndStatusRecord
    struct DTCSnapshot snapshotRecord;
    struct DTCInformation *next;
};

// GiĂ¡ trá»‹ cá»� tráº¡ng thĂ¡i chu ká»³
typedef enum
{
    FALSE,
    TRUE,
    STOP,
    START,
    INITIAL_MONITOR,
    AGING_MONITOR
} flag;

// Káº¿t quáº£ kiá»ƒm tra láº¥y máº«u cáº£m biáº¿n Ä‘Æ°á»£c xĂ¡c nháº­n
// DTCFaultDetectionCounter operation implementation
enum TestResult
{
    PASS,
    FAILED,
    NO_RESULT
};

// Dá»¯ liá»‡u liĂªn quan Ä‘áº¿n cáº£m biáº¿n
struct SensorTest
{
    char name[12];                 // TĂªn cá»§a cáº£m biáº¿n kiá»ƒm tra
    struct DTCInformation dtcInfo; // DTC identifier
    bool pending;                  // Hoáº¡t Ä‘á»™ng kiá»ƒm tra cá»§a cáº£m biáº¿n Ä‘ang chá»� Ä‘Æ°á»£c xá»­ lĂ½
    enum TestResult result;        // Káº¿t quáº£ kiá»ƒm tra tráº£ vá»�.
};
// Khá»Ÿi táº¡o danh sĂ¡ch lÆ°u trá»¯ DTC
extern uint8_t timebase;
extern bool firstCall;                // Sá»­ dá»¥ng trong hĂ m checkSensorsStatusChange Ä‘á»ƒ khá»Ÿi táº¡o 1 láº§n cho thanh ghi lÆ°u trá»¯ lá»‹ch sá»­.
extern int pendingNumber;             // Biáº¿n lÆ°u trá»¯ sá»‘ lÆ°á»£ng cĂ¡c cáº£m biáº¿n trang chá»� Ä‘Æ°á»£c xá»­ lĂ½
extern struct SensorTest sensors[10]; // Má»—i pháº§n tá»­ trong máº£ng lÆ°u trá»¯ káº¿t quáº£ test cá»§a 1 Sensor
extern flag initializationFlag_TF;
extern flag initializationFlag_TFTOC;
extern flag initializationFlag_PDTC;
extern flag initializationFlag_CDTC;
extern flag initializationFlag_TNCSLC;
extern flag initializationFlag_TFSLC;
extern flag initializationFlag_TNCTOC;
extern flag initializationFlag_WIR;
extern flag lastOperationCycle;    // Cá»� bĂ¡o hiá»‡u chu ká»³
extern flag currentOperationCycle; // Cá»� bĂ¡o hiá»‡u chu ká»³
extern flag confirmStage;          // Sá»­ dá»¥ng Ä‘á»ƒ chuyá»ƒn giáº£i Ä‘oáº¡n cho bit sá»‘ confirmedDTC
extern int agingStatus;            // Tráº¡ng thĂ¡i lĂ£o hĂ³a quyáº¿t Ä‘á»‹nh DTC nĂ y Ä‘Æ°á»£c confirm hay khĂ´ng
extern uint8_t temperature;
extern uint16_t ErrorScanningCycle;
// Khá»Ÿi táº¡o cá»� ban Ä‘áº§u cho thanh ghi StatusOfDTC (Table D.2 - ISO 14229-1)
int InitNewCycle();

// Khá»Ÿi táº¡o tráº¡ng thĂ¡i ban Ä‘áº§u cho cĂ¡c cáº£m biáº¿n.
int InitSensors();

void setBit(uint8_t *reg, int bitPosition, int value);

int getBit(uint8_t reg, int bitPosition);

// HĂ m phá»¥ Ä‘á»ƒ kiá»ƒm tra vĂ  debug
void printBits(struct DTCInformation *current);

// ThĂªm 1 node vá»›i thĂ´ng tin dtc vĂ o cuá»‘i danh sĂ¡ch
void addDTC(struct DTCInformation **head, struct DTCInformation dtc);

// HĂ m phá»¥ Ä‘á»ƒ kiá»ƒm tra vĂ  debug
void printBit(uint8_t *reg);

void printAllDTC(struct DTCInformation *head); // HĂ m in ra toĂ n bá»™ node DTC trong danh sĂ¡ch

void freeAllDTC(struct DTCInformation *head);

// Kiá»ƒm tra xem thĂ´ng tin dtc nĂ y Ä‘Ă£ cĂ³ trong list DTCCurrent hay chÆ°a
int repeatCurrentDTC(struct DTCInformation dtc, struct DTCInformation *DTCCurrent);

// Kiá»ƒm tra xem node cuá»‘i cĂ¹ng cá»§a DTCCurrent cĂ³ trĂ¹ng vá»›i báº¥t cá»© node nĂ o trong DTCRecent.
int repeatRecentDTC(const struct DTCInformation *DTCRecent, const struct DTCInformation *DTCCurrent, const struct DTCInformation *copydtc);

// Ä�á»�c giĂ¡ trá»‹ cá»§a cáº£m biáº¿n theo D.6 DTCFaultDetectionCounter operation implementation example ISO 14229-1
int scanSystemError(struct SensorTest *sensor, int value);

// Kiá»ƒm tra sá»± thay Ä‘á»•i á»Ÿ vá»‹ trĂ­ sensor Ä‘Æ°á»£c há»— trá»£, tráº£ vá»� vá»‹ trĂ­ thay Ä‘á»•i káº¿t quáº£ test sensor.
int checkSensorsStatsChange(struct SensorTest *sensors);

// HĂ m táº¡o vĂ  thĂªm DTC vĂ o danh sĂ¡ch liĂªn káº¿t dá»±a trĂªn vá»‹ trĂ­ thay Ä‘á»•i káº¿t quáº£ test. Náº¿u vá»‹ trĂ­ nĂ y Ä‘Ă£ tá»“n táº¡i DTC liĂªn quant thĂ¬ bá»� qua.
struct DTCInformation generateAndAddDTC(int sensorNumber, struct DTCInformation **DTCCurrent);

// HĂ m cáº­p nháº­t tráº¡ng thĂ¡i thanh ghi StatusOfDTC tÆ°Æ¡ng á»©ng vá»›i vá»‹ trĂ­ gĂ¢y ra lá»—i vĂ  dtc cá»¥ thá»ƒ cá»§a nĂ³. (D.3 - D.9 ISO 14229-1)
void UpdateDTCStatus(struct DTCInformation **DTCCurrent, struct DTCInformation **DTCRecent, struct DTCInformation dtc, int sensornumber);

// HĂ m lÆ°u láº¡i danh sĂ¡ch DTC gáº§n Ä‘Ă¢y vĂ  lock cĂ¡c DTC Ä‘Æ°á»£c confirm vĂ o list HistoryDTC
void saveDTC(struct DTCInformation **DTCRecent, struct DTCInformation **DTCHistory, struct DTCInformation **DTCCurrent);

struct DTCInformation getDTC(int sensornumber);

int GetSensorNumberFromDTCNode(uint8_t DTCHighByte, uint8_t DTCMiddleByte, uint8_t DTCLowByte);

void UpdateAllDTCStatus(struct DTCInformation **DTCCurrent, struct DTCInformation **DTCRecent);

void sampleAndUpdateDTC(struct DTCInformation **DTCCurrent, struct DTCInformation **DTCRecent);

#endif /* DTC_H_ */
