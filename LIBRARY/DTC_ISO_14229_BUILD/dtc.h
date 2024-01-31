#ifndef DTC_H_
#define DTC_H_
#include <stdint.h>

#define DTC_NUMBER 2
/*TEST RESULT*/
#define TEST_OK 0
#define TEST_FAILED 1
#define PASS 0
#define FAILED 1
#define NORESULT 2

typedef enum
{
    FALSE,
    TRUE,
    STOP,
    START,
    INITIAL_MONITOR,
    AGING_MONITOR
} flag;

typedef struct
{
    uint8_t high;
    uint8_t mid;
    uint8_t low;
} DTCtype;

typedef struct
{
    uint16_t value; // Gia tri cam bien
    uint16_t limit; // Gioi han cam biwn
    int Pretest;    // Nguong kiem tra
    uint8_t result; // Ket qua
} Sensor;

typedef struct DTCData DTCData;
struct DTCData
{
    Sensor Sensor;
    DTCtype DTC;    // (DTC.high. DTC.mid. DTC.low)
    uint8_t Status; // Thanh ghi trang thai
    int Condition;  // Dieu kien dat nguong (Pretest == Condition)
    int (*DTCHandler)(struct DTCData *data);
    void (*Snapshot)(struct DTCData *data, struct DTCData *DTCHistory);
};

extern flag lastOperationCycle;
extern flag currentOperationCycle;
extern flag confirmStage;
extern int agingStatus;
extern uint32_t sample_time;
extern DTCData DTCHistory[2];
extern DTCData DTCCurrent[2];

void takeSnapshot(DTCData *data, DTCData *DTCHistory);
int checkSensor(DTCData *data);
void updateSensorPretest(DTCData *data, int checkResult);
void updateSensorResult(DTCData *data);
int scanFaultDetection(DTCData *data);
int getBit(uint8_t reg, int bitPosition);
void setBit(uint8_t *reg, int bitPosition, int value);
void copyDataToRecent(DTCData *data, DTCData *DTCData_Recent, int size);
int compareStatusBit2(DTCData *data, DTCData *DTCData_Recent);
int DTCStatus(DTCData *data);
#endif /* DTC_H_ */
