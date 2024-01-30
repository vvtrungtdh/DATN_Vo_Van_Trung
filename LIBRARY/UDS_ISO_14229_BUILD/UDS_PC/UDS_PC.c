#include <UDS_PC.h>

uint8_t request[7] = {0x74, 0x01, 0x19, 0x00, 0x00, 0x00, 0x00};

int (*serviceHandlers[NUM_UDS_SERVICES])(uint8_t *) = {
    [ECU_RESET] = ecuResetHandler,
    [READ_DATA_BY_IDENTIFIER] = readDataByIdentifierHandler,
    [WRITE_DATA_BY_IDENTIFIER] = writeDataByIdentifierHandler,
    [SECURITY_ACCESS] = securityAccessHandler,
    [REQUEST_DOWNLOAD] = requestDownloadHandler,
    [TRANSFER_DATA] = transferDataHandler,
    [REQUEST_TRANSFER_EXIT] = requestTransferExitHandler,
    [NEGATIVE] = negativeResponse};

// define send function
void SendCANData(uint8_t a, uint8_t b, uint8_t *buffer, uint8_t c)
{
    for (int i = 0; i < 7; i++)
    {
        printf("%02X ", buffer[i]);
    }
    printf("\n");
}

//End example

int processUDSService(uint8_t request[7])
{
    UDS_Service service = (UDS_Service)request[0];

    if (service == SECURITY_ACCESS || service == ECU_RESET || service == READ_DATA_BY_IDENTIFIER || service == WRITE_DATA_BY_IDENTIFIER || service == REQUEST_DOWNLOAD || service == NEGATIVE)
    {
        serviceHandlers[service](request);
        return R_OK;
    }
    else
    {
        // NRC
        return R_WRONG;
    }
    return R_ERROR;
}

int ecuResetHandler(uint8_t *request)
{
    uint8_t ecu_reset = request[1];
    if (ecu_reset == 0x02)
    {
        printf("ECU reset OK\n");
        return R_OK;
    }
    else
    {
        // NRC
        return R_WRONG;
    }
    return R_ERROR;
}

int readDataByIdentifierHandler(uint8_t *request)
{
    uint16_t DID = ((uint16_t)request[1] << 8) | request[2];
    uint8_t ID[4];
    if (DID == BootSoftwareIdentificationDataIdentifier)
    {
        ID[0] = request[3];
        ID[1] = request[4];
        ID[2] = request[5];
        ID[3] = request[6];

        // In ra man hinh Identify 4 byte
        for (int i = 0; i < 4; i++)
        {
            printf("%02X ", ID[i]);
        }
        printf("\n");
        return R_OK;
    }
    else if (DID == applicationSoftwareIdentificationDataIdentifier)
    {
        ID[0] = request[3];
        ID[1] = request[4];
        ID[2] = request[5];
        ID[3] = request[6];

        // In ra man hinh Identify 4 byte
        for (int i = 0; i < 4; i++)
        {
            printf("%02X ", ID[i]);
        }
        printf("\n");
        return R_OK;
    }
    // In ra man hinh loi phan hoi tu ECU
    return R_ERROR;
}

int writeDataByIdentifierHandler(uint8_t *request)
{
    uint16_t DID = ((uint16_t)request[1] << 8) | request[2];
    if (DID == BootSoftwareIdentificationDataIdentifier)
    {
        // in ra man hinh DID IDEN OK
        printf("in ra man hinh DID IDEN OK\n");
        return R_OK;
    }
    else if (DID == applicationSoftwareIdentificationDataIdentifier)
    {
        // in ra man hinh DID IDEN OK
        printf("in ra man hinh DID IDEN OK\n");
        return R_OK;
    }
    else {
        printf("In khong thanh cong\n");
    }
    return R_ERROR;
}

int securityAccessHandler(uint8_t *request)
{
    ISO iso;
    uint16_t Key;
    uint8_t check_security = request[1];
    if (check_security == 0x01)
    {
        Key = maHoa(request[2], request[3]);
        uint8_t highByte = (uint8_t)(Key >> 8);
        uint8_t lowByte = (uint8_t)(Key & 0xFF);
        iso.as.service27.SID = 0x27;
        iso.as.service27.SUB = 0x02;
        iso.as.service27.highbyte_key = highByte;
        iso.as.service27.lowbyte_key = lowByte;
        memset(iso.as.service27.Empty, 0, sizeof(iso.as.service27.Empty));
        // SendData
        SendCANData(TX_MAILBOX, TX_MSG_ID, iso.as.data.ptr, 7);
        return R_OK;
    }
    else if (check_security = 0x02)
    {
        // Da pass security
        // In ra man hinh security pass
        printf("In ra man hinh security pass\n");
        return R_OK;
    }
    return R_ERROR;
}

int requestDownloadHandler(uint8_t *request)
{
    uint8_t maxNumberOfBlockLength = request[2];
    // In ra man hinh yeu cau download dc chap nhan
    printf("In ra man hinh yeu cau download dc chap nhan\n");
    return maxNumberOfBlockLength;
}

int transferDataHandler(uint8_t *request)
{
    uint8_t numberOfBlock = request[1];
    // In ra màn hình đã gửi thành công block;
    printf("In ra màn hình đã gửi thành công block\n");
    return numberOfBlock;
}

int requestTransferExitHandler(uint8_t *request)
{
    // In ra man hinh qua trinh gui du lieu hoan tat
    printf("In ra man hinh qua trinh gui du lieu hoan tat\n");
    return R_OK;
}

int negativeResponse(uint8_t *request)
{
    // CHECK NEGATIVE SUB DE IN RA MAN HINH LOI
    printf("NEGATIVE\n");
    return 0;
}

int send_s10_st1()
{
    ISO iso;
    iso.as.service11.SID = 0x10;
    iso.as.service11.SUB = 0x02;
    memset(iso.as.service11.Empty, 0, sizeof(iso.as.service11.Empty));
    // SendData
    SendCANData(TX_MAILBOX, TX_MSG_ID, iso.as.data.ptr, 7);
    return R_OK;
}

int send_s11_st1()
{
    ISO iso;
    iso.as.service11.SID = 0x11;
    iso.as.service11.SUB = 0x02;
    memset(iso.as.service11.Empty, 0, sizeof(iso.as.service11.Empty));
    // SendData
    SendCANData(TX_MAILBOX, TX_MSG_ID, iso.as.data.ptr, 7);
    return R_OK;
}

int send_s22_st1(uint16_t (*DataIdentifier)())
{
    ISO iso;
    uint16_t dataIdentifier = DataIdentifier();
    uint8_t MSB = (dataIdentifier >> 8) & 0xFF;
    uint8_t LSB = dataIdentifier & 0xFF;
    iso.as.service22.SID = 0x22;
    iso.as.service2E.MSB = MSB;
    iso.as.service2E.LSB = LSB;
    memset(iso.as.service22.Empty, 0, sizeof(iso.as.service22.Empty));
    // SendData
    SendCANData(TX_MAILBOX, TX_MSG_ID, iso.as.data.ptr, 7);
    return R_OK;
}

int send_s2E_st1(uint16_t (*DataIdentifier)(), uint32_t dataRecord)
{
    ISO iso;
    uint16_t dataIdentifier = DataIdentifier();
    uint8_t MSB = (dataIdentifier >> 8) & 0xFF;
    uint8_t LSB = dataIdentifier & 0xFF;
    uint8_t dataRecord1, dataRecord2, dataRecord3, dataRecord4;
    dataRecord1 = (dataRecord >> 24) & 0xFF;
    dataRecord2 = (dataRecord >> 16) & 0xFF;
    dataRecord3 = (dataRecord >> 8) & 0xFF;
    dataRecord4 = dataRecord & 0xFF;

    iso.as.service2E.SID = 0x2E;
    iso.as.service2E.MSB = MSB;
    iso.as.service2E.LSB = LSB;
    iso.as.service2E.dataRecord1 = dataRecord1;
    iso.as.service2E.dataRecord2 = dataRecord2;
    iso.as.service2E.dataRecord3 = dataRecord3;
    iso.as.service2E.dataRecord4 = dataRecord4;
    SendCANData(TX_MAILBOX, TX_MSG_ID, iso.as.data.ptr, 7);
    return R_OK;
}

int send_s27_st1()
{
    ISO iso;
    iso.as.service27.SID = 0x27;
    iso.as.service27.SUB = 0x01;
    iso.as.service27.highbyte_key = 0x00;
    iso.as.service27.lowbyte_key = 0x00;
    memset(iso.as.service27.Empty, 0, sizeof(iso.as.service27.Empty));
    // SendData
    SendCANData(TX_MAILBOX, TX_MSG_ID, iso.as.data.ptr, 7);
    return R_OK;
}

int send_s34_st1()
{
    ISO iso;
    iso.as.service34.SID = 0x34;
    iso.as.service34.memoryAddress1 = 0x03;
    iso.as.service34.memoryAddress2 = 0x20;
    iso.as.service34.memoryAddress3 = 0x00;
    iso.as.service34.memorySize1 = 0x01;
    iso.as.service34.memorySize2 = 0x90;
    iso.as.service34.memorySize3 = 0x00;
    // SendData
    SendCANData(TX_MAILBOX, TX_MSG_ID, iso.as.data.ptr, 7);
    return R_OK;
}

int send_s37_st1()
{
    ISO iso;
    iso.as.service37.SID = 0x37;
    memset(iso.as.service37.Empty, 0, sizeof(iso.as.service37.Empty));
    // SendData
    SendCANData(TX_MAILBOX, TX_MSG_ID, iso.as.data.ptr, 7);
    return R_OK;
}

uint16_t maHoa(uint8_t byte1, uint8_t byte2)
{
    uint16_t thanhGhi;
    thanhGhi = (uint16_t)byte1 << 8 | (uint16_t)byte2;
    thanhGhi = ~thanhGhi; // Đảo bit
    thanhGhi += 1;        // Cộng 1
    return thanhGhi;
}

uint16_t DIDBootSoftwareIdentifier()
{
    return BootSoftwareIdentificationDataIdentifier;
}

uint16_t DIDApplicationSoftwareIdentifier()
{
    return applicationSoftwareIdentificationDataIdentifier;
}