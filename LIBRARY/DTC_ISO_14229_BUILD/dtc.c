x#include "dtc.h"

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>

bool firstCall = true; // Biáº¿n Ä‘á»ƒ kiá»ƒm tra láº§n Ä‘áº§u tiĂªn gá»�i hĂ m
int pendingNumber = 0;
struct SensorTest sensors[10];
uint8_t timebase = 0;
uint8_t temperature = 0;
uint16_t ErrorScanningCycle = 0;
// Khá»Ÿi táº¡o danh sĂ¡ch lÆ°u trá»¯ DTC
struct DTCInformation *HistoryDTC = NULL; // LÆ°u trá»¯ cac DTC Ä‘Æ°á»£c xĂ¡c nháº­n.
struct DTCInformation *DTCRecent = NULL;  // LÆ°u trá»¯ cĂ¡c DTC gáº§n Ä‘Ă¢y sá»­ dá»¥ng cho viá»‡c cáº­p nháº­t tráº¡ng thĂ¡i thanh ghi StatusOfDTC
struct DTCInformation *DTCCurrent = NULL; // LÆ°u trá»¯ cĂ¡c DTC hiá»‡n táº¡i

flag initializationFlag_TF;
flag initializationFlag_TFTOC;
flag initializationFlag_PDTC;
flag initializationFlag_CDTC;
flag initializationFlag_TNCSLC;
flag initializationFlag_TFSLC;
flag initializationFlag_TNCTOC;
flag initializationFlag_WIR;
flag lastOperationCycle;
flag currentOperationCycle;
flag confirmStage = INITIAL_MONITOR;
int agingStatus = 0;

int InitSensors()
{
    // Khá»Ÿi táº¡o giĂ¡ trá»‹ cho cĂ¡c thĂ nh pháº§n DTC cá»§a máº£ng sensors
    sensors[0].dtcInfo.DTCHighByte = 0x01;
    sensors[0].dtcInfo.DTCMiddleByte = 0x01;
    sensors[0].dtcInfo.DTCLowByte = 0x01;
    sensors[0].dtcInfo.StatusOfDTC = 0x00;

    sensors[1].dtcInfo.DTCHighByte = 0x01;
    sensors[1].dtcInfo.DTCMiddleByte = 0x01;
    sensors[1].dtcInfo.DTCLowByte = 0x02;
    sensors[1].dtcInfo.StatusOfDTC = 0x00;

    sensors[2].dtcInfo.DTCHighByte = 0x01;
    sensors[2].dtcInfo.DTCMiddleByte = 0x01;
    sensors[2].dtcInfo.DTCLowByte = 0x03;
    sensors[2].dtcInfo.StatusOfDTC = 0x00;

    sensors[3].dtcInfo.DTCHighByte = 0x01;
    sensors[3].dtcInfo.DTCMiddleByte = 0x02;
    sensors[3].dtcInfo.DTCLowByte = 0x04;
    sensors[3].dtcInfo.StatusOfDTC = 0x00;

    sensors[4].dtcInfo.DTCHighByte = 0x01;
    sensors[4].dtcInfo.DTCMiddleByte = 0x02;
    sensors[4].dtcInfo.DTCLowByte = 0x05;
    sensors[4].dtcInfo.StatusOfDTC = 0x00;

    sensors[5].dtcInfo.DTCHighByte = 0x01;
    sensors[5].dtcInfo.DTCMiddleByte = 0x02;
    sensors[5].dtcInfo.DTCLowByte = 0x06;
    sensors[5].dtcInfo.StatusOfDTC = 0x00;

    sensors[6].dtcInfo.DTCHighByte = 0x01;
    sensors[6].dtcInfo.DTCMiddleByte = 0x03;
    sensors[6].dtcInfo.DTCLowByte = 0x07;
    sensors[6].dtcInfo.StatusOfDTC = 0x00;

    sensors[7].dtcInfo.DTCHighByte = 0x01;
    sensors[7].dtcInfo.DTCMiddleByte = 0x03;
    sensors[7].dtcInfo.DTCLowByte = 0x08;
    sensors[7].dtcInfo.StatusOfDTC = 0x00;

    sensors[8].dtcInfo.DTCHighByte = 0x01;
    sensors[8].dtcInfo.DTCMiddleByte = 0x03;
    sensors[8].dtcInfo.DTCLowByte = 0x09;
    sensors[8].dtcInfo.StatusOfDTC = 0x00;

    // Khá»Ÿi táº¡o giĂ¡ trá»‹ cho cĂ¡c thĂ nh pháº§n result vĂ  pending cá»§a máº£ng sensors
    for (int i = 0; i < 10; ++i)
    {
        sensors[i].result = NO_RESULT;
        sensors[i].pending = false;
    }
    return 0;
}

int InitNewCycle()
{
    initializationFlag_TF = FALSE;
    initializationFlag_TFTOC = FALSE;
    initializationFlag_PDTC = FALSE;
    initializationFlag_CDTC = FALSE;
    initializationFlag_TNCSLC = FALSE;
    initializationFlag_TFSLC = FALSE;
    initializationFlag_TNCTOC = FALSE;
    initializationFlag_WIR = FALSE;
    lastOperationCycle = STOP;
    currentOperationCycle = STOP;
    return 0;
}

// HĂ m thiáº¿t láº­p má»™t bit cá»¥ thá»ƒ trong má»™t thanh ghi
void setBit(uint8_t *reg, int bitPosition, int value)
{
    if (value)
    {
        *reg |= (1u << bitPosition);
    }
    else
    {
        *reg &= ~(1u << bitPosition);
    }
}

// HĂ m láº¥y giĂ¡ trá»‹ cá»§a má»™t bit cá»¥ thá»ƒ trong má»™t thanh ghi
int getBit(uint8_t reg, int bitPosition)
{
    return (reg >> bitPosition) & 1;
}

// HĂ m in ra táº¥t cáº£ cĂ¡c bit trong má»™t thanh ghi
void printBits(struct DTCInformation *current)
{
    struct DTCInformation *tempCurrent = current;
    while (tempCurrent != NULL)
    {
        for (int i = 7; i >= 0; i--)
        {
            int bit = (tempCurrent->StatusOfDTC >> i) & 1;
            printf("%d", bit);
        }
        printf("\n");

        // Di chuyá»ƒn con trá»� Ä‘áº¿n node tiáº¿p theo trong danh sĂ¡ch
        tempCurrent = tempCurrent->next;
    }
}

void printBit(uint8_t *reg)
{
    for (int i = 7; i >= 0; i--)
    {
        int bit = (*reg >> i) & 1;
        printf("%d", bit);
    }
    printf("\n");
}

void printAllDTC(struct DTCInformation *head)
{
    if (head == NULL)
    {
        printf("NULL\n"); // In ra "NULL" náº¿u linked list lĂ  rá»—ng
        return;
    }

    struct DTCInformation *current = head;
    int index = 0;
    while (current != NULL)
    {
        printf("DTCStatus #%d\n", index);
        printf("DTC High Byte: 0x%02X\n", current->DTCHighByte);
        printf("DTC Middle Byte: 0x%02X\n", current->DTCMiddleByte);
        printf("DTC Low Byte: 0x%02X\n", current->DTCLowByte);
        printf("DTC StatusOfDTC: ");
        printBit(&(current->StatusOfDTC));
        printf("\n");
        current = current->next;
        index++;
    }
}

// HĂ m giáº£i phĂ³ng toĂ n bá»™ danh sĂ¡ch liĂªn káº¿t DTC
void freeAllDTC(struct DTCInformation *head)
{
    while (head != NULL)
    {
        struct DTCInformation *temp = head;
        head = head->next;
        free(temp);
    }
}

void addDTC(struct DTCInformation **head, struct DTCInformation dtc)
{
    // Táº¡o má»™t node má»›i vĂ  gĂ¡n giĂ¡ trá»‹ cho nĂ³
    struct DTCInformation *newNode = (struct DTCInformation *)malloc(sizeof(struct DTCInformation));
    newNode->DTCHighByte = dtc.DTCHighByte;
    newNode->DTCMiddleByte = dtc.DTCMiddleByte;
    newNode->DTCLowByte = dtc.DTCLowByte;
    newNode->StatusOfDTC = dtc.StatusOfDTC;
    newNode->next = NULL;

    // Náº¿u danh sĂ¡ch liĂªn káº¿t rá»—ng, thĂ¬ node má»›i trá»Ÿ thĂ nh head
    if (*head == NULL)
    {
        *head = newNode;
        return;
    }

    // TĂ¬m node cuá»‘i cĂ¹ng trong danh sĂ¡ch liĂªn káº¿t
    struct DTCInformation *current = *head;
    while (current->next != NULL)
    {
        current = current->next;
    }

    // Gáº¯n node má»›i vĂ o cuá»‘i danh sĂ¡ch liĂªn káº¿t
    current->next = newNode;
}

int repeatCurrentDTC(struct DTCInformation dtc, struct DTCInformation *DTCCurrent)
{
    if (DTCCurrent == NULL)
    {
        return 0; // Náº¿u DTCCurrent rá»—ng, tráº£ vá»� 0
    }

    struct DTCInformation *current = DTCCurrent;

    while (current != NULL)
    {
        // Kiá»ƒm tra tá»«ng node trong DTCCurrent xem cĂ³ trĂ¹ng vá»›i dtc khĂ´ng
        if (current->DTCHighByte == dtc.DTCHighByte &&
            current->DTCMiddleByte == dtc.DTCMiddleByte &&
            current->DTCLowByte == dtc.DTCLowByte)
        {
            return 1; // Náº¿u cĂ³ node trĂ¹ng, tráº£ vá»� 1
        }
        current = current->next; // Di chuyá»ƒn Ä‘áº¿n node tiáº¿p theo
    }

    return 0; // Náº¿u khĂ´ng cĂ³ node nĂ o trĂ¹ng, tráº£ vá»� 0
}
int repeatRecentDTC(const struct DTCInformation *DTCRecent, const struct DTCInformation *DTCCurrent, const struct DTCInformation *copydtc)
{
    // Kiá»ƒm tra xem cáº£ hai linked list cĂ³ tá»“n táº¡i hay khĂ´ng
    if (DTCRecent == NULL || DTCCurrent == NULL)
    {
        return 0;
    }

    // Táº¡o báº£n sao cá»§a con trá»� Ä‘á»ƒ khĂ´ng lĂ m thay Ä‘á»•i linked list ban Ä‘áº§u
    const struct DTCInformation *recentCopy = DTCRecent;
    const struct DTCInformation *currentCopy = DTCCurrent;
    uint8_t copyStatusOfDTC;
    // Láº·p qua táº¥t cáº£ cĂ¡c node trong DTCCurrent
    while (currentCopy != NULL)
    {
        // Kiá»ƒm tra náº¿u cĂ¡c giĂ¡ trá»‹ DTCHighByte, DTCMiddleByte, DTCLowByte cá»§a node hiá»‡n táº¡i trĂ¹ng vá»›i copydtc
        if (currentCopy->DTCHighByte == copydtc->DTCHighByte &&
            currentCopy->DTCMiddleByte == copydtc->DTCMiddleByte &&
            currentCopy->DTCLowByte == copydtc->DTCLowByte)
        {
            // LÆ°u láº¡i thanh ghi StatusOfDTC vĂ  thoĂ¡t khá»�i vĂ²ng láº·p
            copyStatusOfDTC = currentCopy->StatusOfDTC;
            break;
        }
        currentCopy = currentCopy->next;
    }

    // Láº·p qua táº¥t cáº£ cĂ¡c node trong DTCRecent Ä‘á»ƒ so sĂ¡nh vá»›i giĂ¡ trá»‹ Ä‘Ă£ lÆ°u á»Ÿ trĂªn
    while (recentCopy != NULL)
    {
        // So sĂ¡nh cĂ¡c giĂ¡ trá»‹ cá»§a node hiá»‡n táº¡i trong DTCRecent vá»›i copyStatusOfDTC Ä‘Ă£ lÆ°u
        if (recentCopy->DTCHighByte == copydtc->DTCHighByte &&
            recentCopy->DTCMiddleByte == copydtc->DTCMiddleByte &&
            recentCopy->DTCLowByte == copydtc->DTCLowByte &&
            (recentCopy->StatusOfDTC & 0x02) && (copyStatusOfDTC & 0x02))
        {
            return 1; // Tráº£ vá»� 1 náº¿u cĂ³ sá»± trĂ¹ng láº·p
        }
        recentCopy = recentCopy->next;
    }
    return 0; // Tráº£ vá»� 0 náº¿u khĂ´ng cĂ³ sá»± trĂ¹ng láº·p
}

// HĂ m kiá»ƒm tra vĂ  xá»­ lĂ½ lá»—i há»‡ thá»‘ng
int scanSystemError(struct SensorTest *sensor, int value)
{
    int Pretest = 0; // NgÆ°á»¡ng káº¿t quáº£ test
    int Value = value;       // GiĂ¡ trá»‹ cá»§a cáº£m biáº¿n
    int count = 6;   // Sá»‘ láº§n quĂ©t giĂ¡ trá»‹ cáº£m biáº¿n tá»‘i Ä‘a lĂ  6 láº§n. (Fault Detection Counter)

    while (count > 0)
    {
        // Thay tháº¿ báº±ng hĂ m Ä‘á»�c giĂ¡ trá»‹ cáº£m biáº¿n nhiá»‡t Ä‘á»™ trĂªn ECU
        printf("scan sensors value (*C) = ");
        // Nhiá»‡t Ä‘á»™ cho phĂ©p giáº£ sá»­ lĂ  37 Ä‘á»™
        if (Value <= 37)
        {
            Pretest--;
        }
        else
        {
            if (Pretest < 0 && Value > 37)
            {
                Pretest = 1;
            }
            else
            {
                Pretest++;
            }
        }

        if (Pretest > 2)
        {
            sensor->result = FAILED;
            return 0;
        }
        else if (Pretest < -2)
        {
            sensor->result = PASS;
            return 0;
        }
        else if (Pretest >= -2 && Pretest < 2 && count == 1)
        {
            sensor->result = NO_RESULT;
            return 0;
        }

        count--;
    }
    return 0;
}

int checkSensorsStatsChange(struct SensorTest *sensors)
{
    static struct SensorTest preSensors[10]; // Máº£ng static lÆ°u trá»¯ tráº¡ng thĂ¡i trÆ°á»›c Ä‘Ă³ cá»§a sensors
    int j = -1;
    if (pendingNumber > 0) // Náº¿u cĂ³ pending cáº£m biáº¿n
    {
        for (int i = 0; i < 10; ++i)
        {
            if (sensors[i].pending) // Kiá»ƒm tra náº¿u cáº£m biáº¿n Ä‘ang pending
            {
                sensors[i].pending = false;        // Reset pending cá»§a cáº£m biáº¿n
                pendingNumber = pendingNumber - 1; // Giáº£m sá»‘ lÆ°á»£ng pending cáº£m biáº¿n
                return i;                          // Tráº£ vá»� vá»‹ trĂ­ cá»§a cáº£m biáº¿n Ä‘ang pending
            }
        }
    }
    else // Náº¿u khĂ´ng cĂ³ cáº£m biáº¿n nĂ o pending
    {
        if (firstCall)
        {
            for (int i = 0; i < 10; ++i)
            {
                preSensors[i].result = NO_RESULT; // Khá»Ÿi táº¡o giĂ¡ trá»‹ ban Ä‘áº§u chá»‰ á»Ÿ láº§n Ä‘áº§u tiĂªn gá»�i hĂ m
            }
            firstCall = false; // Ä�Ă¡nh dáº¥u Ä‘Ă£ gá»�i hĂ m láº§n Ä‘áº§u
        }
        // Kiá»ƒm tra vĂ  so sĂ¡nh giĂ¡ trá»‹ cá»§a sensors vá»›i preSensors
        for (int i = 0; i < 10; ++i)
        {
            if (sensors[i].result != preSensors[i].result)
            {
                printf("Thay doi phan tu so: %d\n", i);
                preSensors[i].result = sensors[i].result;
                sensors[i].pending = true; // Ä�Ă¡nh dáº¥u cáº£m biáº¿n i lĂ  Ä‘ang pending
                pendingNumber++;           // TÄƒng sá»‘ lÆ°á»£ng pending cáº£m biáº¿n
                j = i;
            }
        }
        if (j >= 0)
        {
            pendingNumber--;
            sensors[j].pending = false;
            return j;
        }
    }
    printf("pendingNumber: %d\n", pendingNumber);
    pendingNumber--;
    return -1;
}

struct DTCInformation generateAndAddDTC(int sensorNumber, struct DTCInformation **DTCCurrent)
{
    struct DTCInformation dtc;

    if (sensorNumber >= 0 && sensorNumber < 10)
    {
        dtc.DTCHighByte = sensors[sensorNumber].dtcInfo.DTCHighByte;
        dtc.DTCMiddleByte = sensors[sensorNumber].dtcInfo.DTCMiddleByte;
        dtc.DTCLowByte = sensors[sensorNumber].dtcInfo.DTCLowByte;
        dtc.StatusOfDTC = 0b01010000;
    }
    else
    {
        printf("No DTC Change (-1)\n");
        dtc = (struct DTCInformation){0x00, 0x00, 0x00, 0x00};
        return dtc;
    }

    // Kiá»ƒm tra xem DTC Ä‘Ă£ tá»“n táº¡i trong linked list DTCCurrent hay chÆ°a
    if (!repeatCurrentDTC(dtc, *DTCCurrent))
    {
        addDTC(DTCCurrent, dtc);
    }

    return dtc;
}

void UpdateDTCStatus(struct DTCInformation **DTCCurrent, struct DTCInformation **DTCRecent, struct DTCInformation dtc, int sensornumber)
{
    enum TestResult mostRecentTestResult = sensors[sensornumber].result;
    // Traverse the DTC list and find the node with matching data to 'dtc'
    struct DTCInformation *current = *DTCCurrent;
    // Kiá»ƒm tra náº¿u dtc báº±ng (0x00, 0x00, 0x00, 0x00) thĂ¬ return
    if (dtc.DTCHighByte == 0x00 && dtc.DTCMiddleByte == 0x00 && dtc.DTCLowByte == 0x00 && dtc.StatusOfDTC == 0x00)
    {
        printf("NULL DTC\n");
        return;
    }

    while ((current->next != NULL))
    {
        // Kiá»ƒm tra tá»«ng node trong DTCCurrent xem cĂ³ trĂ¹ng vá»›i dtc khĂ´ng
        if (current->DTCHighByte == dtc.DTCHighByte &&
            current->DTCMiddleByte == dtc.DTCMiddleByte &&
            current->DTCLowByte == dtc.DTCLowByte)
        {
            break; // Exit the loop after performing required operations on the found node
        }
        current = current->next;
    }

    // Retrieve status bits of the current DTC
    int testFailed = getBit(current->StatusOfDTC, 0);
    int testFailedThisOperationCycle = getBit(current->StatusOfDTC, 1);
    int pendingDTC = getBit(current->StatusOfDTC, 2);
    int confirmedDTC = getBit(current->StatusOfDTC, 3);
    int testNotCompletedSinceLastClear = getBit(current->StatusOfDTC, 4);
    int testFailedSinceLastClear = getBit(current->StatusOfDTC, 5);
    int testNotCompletedThisOperationCycle = getBit(current->StatusOfDTC, 6);
    int FLAG_testNotCompletedThisOperationCycle = 0;

    // Initialize flag for testFailed if not initialized and set testFailed to 0
    if (initializationFlag_TF == FALSE)
    {
        initializationFlag_TF = TRUE;
        testFailed = 0;
    }

    // Initialize flag for testFailedThisOperationCycle if not initialized and set testFailedThisOperationCycle to 0
    if (initializationFlag_TFTOC == FALSE)
    {
        initializationFlag_TFTOC = TRUE;
        testFailedThisOperationCycle = 0;
    }

    // Initialize flag for pendingDTC if not initialized and set pendingDTC to 0
    if (initializationFlag_PDTC == FALSE)
    {
        initializationFlag_PDTC = TRUE;
        pendingDTC = 0;
    }

    // Initialize flag for confirmedDTC if not initialized and set confirmedDTC to 0
    if (initializationFlag_CDTC == FALSE)
    {
        initializationFlag_CDTC = TRUE;
        confirmedDTC = 0;
    }

    // Initialize flag for testNotCompletedSinceLastClear if not initialized and set testNotCompletedSinceLastClear to 1
    if (initializationFlag_TNCSLC == FALSE)
    {
        initializationFlag_TNCSLC = TRUE;
        testNotCompletedSinceLastClear = 1;
    }

    // Initialize flag for testFailedSinceLastClear if not initialized and set testFailedSinceLastClear to 0
    if (initializationFlag_TFSLC == FALSE)
    {
        initializationFlag_TFSLC = TRUE;
        testFailedSinceLastClear = 0;
    }

    // Initialize flag for testNotCompletedThisOperationCycle if not initialized and set testNotCompletedThisOperationCycle to 1
    if (initializationFlag_TNCTOC == FALSE)
    {
        initializationFlag_TNCTOC = TRUE;
        testNotCompletedThisOperationCycle = 1;
    }

    // Update bits based on most recent test results
    if (mostRecentTestResult == PASS)
    {
        testFailed = 0;
    }
    else if ((mostRecentTestResult == FAILED) || (mostRecentTestResult == NO_RESULT))
    {
        testFailed = 1;
    }
    setBit(&(current->StatusOfDTC), 0, testFailed);

    // Update testFailedThisOperationCycle based on operation cycle and most recent test result
    if (lastOperationCycle == STOP && currentOperationCycle == START)
    {
        testFailedThisOperationCycle = 0;
    }
    else if (mostRecentTestResult == FAILED)
    {
        testFailedThisOperationCycle = 1;
    }
    setBit(&(current->StatusOfDTC), 1, testFailedThisOperationCycle);

    // Update pendingDTC based on most recent test result and operation cycle
    if (mostRecentTestResult == FAILED)
    {
        pendingDTC = 1;
    }
    else if (currentOperationCycle == STOP && testFailedThisOperationCycle == 0)
    {
        pendingDTC = 0;
    }
    setBit(&(current->StatusOfDTC), 2, pendingDTC);

    // Determine and update confirmedDTC based on threshold and confirm stage
    int threshold = repeatRecentDTC(*DTCRecent, *DTCCurrent, &dtc);
    if (confirmStage == INITIAL_MONITOR)
    {
        // If threshold met, confirm the DTC, update aging status, and set confirm stage to AGING_MONITOR
        if (threshold)
        {
            confirmedDTC = 1;
            agingStatus = 0;
            confirmStage = AGING_MONITOR;
        }
        else
        {
            confirmedDTC = 0;
        }
    }
    else if (confirmStage == AGING_MONITOR)
    {
        // Manage aging status and confirm DTC accordingly
        if (agingStatus == 6)
        {
            confirmedDTC = 0;
            confirmStage = INITIAL_MONITOR;
            agingStatus = 0;
        }
        else if (mostRecentTestResult == FAILED)
        {
            agingStatus = 0;
        }
        else
        {
            if (!threshold && currentOperationCycle == STOP)
            {
                agingStatus++;
            }
        }
    }
    setBit(&(current->StatusOfDTC), 3, confirmedDTC);

    // Update testNotCompletedSinceLastClear based on most recent test result
    if (mostRecentTestResult == PASS || mostRecentTestResult == FAILED)
    {
        testNotCompletedSinceLastClear = 0;
    }
    setBit(&(current->StatusOfDTC), 4, testNotCompletedSinceLastClear);

    // Manage testFailedSinceLastClear based on aging status and most recent test result
    if (agingStatus == 2)
    {
        testFailedSinceLastClear = 0;
    }
    else if (mostRecentTestResult == FAILED)
    {
        testFailedSinceLastClear = 1;
    }
    setBit(&(current->StatusOfDTC), 5, testFailedSinceLastClear);

    // Manage testNotCompletedThisOperationCycle flag
    if (lastOperationCycle == STOP && currentOperationCycle == START)
    {
        testNotCompletedThisOperationCycle = 1;
    }
    else if (mostRecentTestResult == PASS || mostRecentTestResult == FAILED)
    {
        testNotCompletedThisOperationCycle = 0;
    }
    setBit(&(current->StatusOfDTC), 6, testNotCompletedThisOperationCycle);
}

void saveDTC(struct DTCInformation **DTCRecent, struct DTCInformation **DTCHistory, struct DTCInformation **DTCCurrent)
{
    // Náº¿u DTCRecent khĂ´ng rá»—ng, xĂ³a toĂ n bá»™ node trong linked list
    if (*DTCRecent != NULL)
    {
        freeAllDTC(*DTCRecent);
        *DTCRecent = NULL;
    }

    // Náº¿u DTCCurrent khĂ´ng rá»—ng, sao chĂ©p cĂ¡c node tá»« DTCCurrent sang DTCRecent vĂ  DTCHistory
    if (*DTCCurrent != NULL)
    {
        struct DTCInformation *current = *DTCCurrent;

        while (current != NULL)
        {
            struct DTCInformation *newNode = (struct DTCInformation *)malloc(sizeof(struct DTCInformation));
            if (newNode == NULL)
            {
                return;
            }

            // Sao chĂ©p dá»¯ liá»‡u tá»« DTCCurrent sang DTCRecent
            *newNode = *current;

            // ThĂªm node má»›i vĂ o linked list DTCRecent
            newNode->next = *DTCRecent;
            *DTCRecent = newNode;

            // Kiá»ƒm tra bit sá»‘ 3 trong thanh ghi StatusOfDTC Ä‘á»ƒ xĂ¡c Ä‘á»‹nh viá»‡c thĂªm vĂ o DTCHistory
            if (current->StatusOfDTC & 0x08)
            {
                // Náº¿u bit sá»‘ 3 báº±ng 1, sao chĂ©p node Ä‘Ă³ sang DTCHistory báº±ng hĂ m addDTC
                addDTC(DTCHistory, *current);
            }

            current = current->next;
        }
    }
}

struct DTCInformation getDTC(int sensornumber)
{
    struct DTCInformation dtcCopy;
    dtcCopy.DTCHighByte = sensors[sensornumber].dtcInfo.DTCHighByte;
    dtcCopy.DTCMiddleByte = sensors[sensornumber].dtcInfo.DTCMiddleByte;
    dtcCopy.DTCLowByte = sensors[sensornumber].dtcInfo.DTCLowByte;
    return dtcCopy;
}

// HĂ m trá»� Ä‘áº¿n node trong DTCCurrent vĂ  tráº£ vá»� tham sá»‘ sensornumber tÆ°Æ¡ng á»©ng
int GetSensorNumberFromDTCNode(uint8_t DTCHighByte, uint8_t DTCMiddleByte, uint8_t DTCLowByte)
{
    for (int i = 0; i < 10; ++i)
    {
        if (sensors[i].dtcInfo.DTCHighByte == DTCHighByte &&
            sensors[i].dtcInfo.DTCMiddleByte == DTCMiddleByte &&
            sensors[i].dtcInfo.DTCLowByte == DTCLowByte)
        {
            return i;
        }
    }
    return -1; // Tráº£ vá»� -1 náº¿u khĂ´ng tĂ¬m tháº¥y sensor tÆ°Æ¡ng á»©ng
}
// HĂ m trá»� Ä‘áº¿n node trong DTCCurrent vĂ  cáº­p nháº­t tráº¡ng thĂ¡i
void UpdateAllDTCStatus(struct DTCInformation **DTCCurrent, struct DTCInformation **DTCRecent)
{
    if (*DTCCurrent == NULL)
    {
        // TrÆ°á»�ng há»£p DTCCurrent lĂ  NULL, thoĂ¡t khá»�i hĂ m
        return;
    }

    struct DTCInformation *currentNode = *DTCCurrent;
    while (currentNode != NULL)
    {
        // LÆ°u giĂ¡ trá»‹ cá»§a node hiá»‡n táº¡i
        uint8_t DTCHighByteCopy = currentNode->DTCHighByte;
        uint8_t DTCMiddleByteCopy = currentNode->DTCMiddleByte;
        uint8_t DTCLowByteCopy = currentNode->DTCLowByte;

        // TĂ¬m sensorNumber tá»« cĂ¡c giĂ¡ trá»‹ node hiá»‡n táº¡i
        int sensorNumber = GetSensorNumberFromDTCNode(DTCHighByteCopy, DTCMiddleByteCopy, DTCLowByteCopy);

        // Táº¡o struct DTCInformation tá»« giĂ¡ trá»‹ node hiá»‡n táº¡i
        struct DTCInformation dtc;
        dtc.DTCHighByte = DTCHighByteCopy;
        dtc.DTCMiddleByte = DTCMiddleByteCopy;
        dtc.DTCLowByte = DTCLowByteCopy;

        // In ra thĂ´ng tin sensorNumber vĂ  dtc cá»§a node hiá»‡n táº¡i
        printf("Sensor Number: %d:  ", sensorNumber);
        printf("%d:  0x", sensors[sensorNumber].result);
        printf("%02X", dtc.DTCHighByte);
        printf("%02X", dtc.DTCMiddleByte);
        printf("%02X", dtc.DTCLowByte);
        printf("\n");

        // Cáº­p nháº­t tráº¡ng thĂ¡i cho node hiá»‡n táº¡i
        UpdateDTCStatus(DTCCurrent, DTCRecent, dtc, sensorNumber);

        // Trá»� Ä‘áº¿n node tiáº¿p theo trong DTCCurrent
        currentNode = currentNode->next;
    }
}

// HĂ m nĂ y thá»±c hiá»‡n quĂ©t cáº£m biáº¿n, táº¡o vĂ  thĂªm DTC khi cáº§n thiáº¿t
void sampleAndUpdateDTC(struct DTCInformation **DTCCurrent, struct DTCInformation **DTCRecent)
{
    do
    {
        int sensornumber = checkSensorsStatsChange(sensors);
        generateAndAddDTC(sensornumber, DTCCurrent);
    } while (pendingNumber > 0);
    UpdateAllDTCStatus(DTCCurrent, DTCRecent);
    printBits(*DTCCurrent);
}

int UnifiedDiagnosticServices()
{
    if (timebase == 0)
    {
        InitSensors();
        lastOperationCycle = STOP;
        currentOperationCycle = START;
        pendingNumber = 0;
        // Gá»�i hĂ m scanSystemError Ä‘á»ƒ kiá»ƒm tra há»‡ thá»‘ng
        scanSystemError(&sensors[1], temperature);
        sampleAndUpdateDTC(&DTCCurrent, &DTCRecent);
        lastOperationCycle = START;
        currentOperationCycle = START;
    }
    if (timebase > 0)
    {
        pendingNumber = 0;
        // Gá»�i hĂ m scanSystemError Ä‘á»ƒ kiá»ƒm tra há»‡ thá»‘ng
        scanSystemError(&sensors[1], temperature);
        sampleAndUpdateDTC(&DTCCurrent, &DTCRecent);
    }
    if (timebase >= 30)
    {
        pendingNumber = 0;
        lastOperationCycle = START;
        currentOperationCycle = STOP;
        // Gá»�i hĂ m scanSystemError Ä‘á»ƒ kiá»ƒm tra há»‡ thá»‘ng
        scanSystemError(&sensors[1], temperature);
        sampleAndUpdateDTC(&DTCCurrent, &DTCRecent);
        saveDTC(&DTCRecent, &HistoryDTC, &DTCCurrent);
        timebase = 0;
    	ErrorScanningCycle ++;
    }
    return 0;
}
