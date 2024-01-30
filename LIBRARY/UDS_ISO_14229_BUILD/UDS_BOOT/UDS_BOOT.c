#include <UDS_BOOT.h>
/*Variable*/
Comm comm;
uint8_t request[7] = {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
int (*serviceHandlers[NUM_UDS_SERVICES])(Comm *comm, uint8_t *) = {
    [DIAGNOSTIC_SESSION_CONTROL] = diagnosticSessionControlHandler,
    [ECU_RESET] = ecuResetHandler,
    [SECURITY_ACCESS] = securityAccessHandler,
    [TRANSFER_DATA] = transferDataHandler,
    [REQUEST_TRANSFER_EXIT] = requestTransferExitHandler};

int processUDSService(Comm *comm, uint8_t request[7])
{
    ISO iso;
    UDS_Service service = (UDS_Service)request[0];
    int ret;
    ret = 0;
    // Reset request and return error if service SID is 0x00
    if (service == 0x00)
    {
        memset(request, 0, SIZE_REQUEST);
        return R_ERROR;
    }
    // Reset request and return Error if service SID not support
    if (service != 0x10 && service != 0x11 && service != 0x27 && service != 0x36 && service != 0x37)
    {
        iso.as.Negative.negative = 0x7F;
        iso.as.Negative.SID = 0x00;
        iso.as.Negative.responseCode = NRC_SID;
        memset(iso.as.Negative.Empty, 0, sizeof(iso.as.Negative.Empty));
        // SendData
        SendCANData(TX_MAILBOX, TX_MSG_ID, iso.as.data.ptr, 7);
        return R_ERROR;
    }
    // Execute service based on the service ID
    if (service == SECURITY_ACCESS)
    {
        comm->check_security = serviceHandlers[service](comm, request);
    }
    else if (service == DIAGNOSTIC_SESSION_CONTROL)
    {
        if (comm->check_security == UNLOCK)
        {
            comm->session_control = serviceHandlers[service](comm, request);
        }
        else
        {
            // Need to pass security first
            memset(request, 0, SIZE_REQUEST);
            return R_WRONG;
        }
    }
    else if (comm->session_control == DEFAULT_SESSION &&
             (service == ECU_RESET))
    {
        ret = serviceHandlers[service](comm, request);
    }
    else if (comm->session_control == CUSTOM_SESSION &&
             (service == ECU_RESET || service == TRANSFER_DATA || service == REQUEST_TRANSFER_EXIT))
    {
        ret = serviceHandlers[service](comm, request);
    }
    else
    {
        // Service not supported in the current session/control
        iso.as.Negative.negative = 0x7F;
        iso.as.Negative.SID = 0x10;
        iso.as.Negative.responseCode = NRC_SUBFUNCTION;
        memset(iso.as.Negative.Empty, 0, sizeof(iso.as.Negative.Empty));
        // SendData
        SendCANData(TX_MAILBOX, TX_MSG_ID, iso.as.data.ptr, 7);
        memset(request, 0, SIZE_REQUEST);
        return R_WRONG;
    }

    // Clear request and return success
    memset(request, 0, SIZE_REQUEST);
    return ret;
}

int diagnosticSessionControlHandler(Comm *comm, uint8_t *request) // OK
{
    ISO iso;
    uint8_t session_request = request[1];

    if (session_request == 0x01) // 10 01
    {
        return DEFAULT_SESSION;
    }
    else if (session_request == 0x02) // 10 02
    {
        return CUSTOM_SESSION;
    }
    else
    {
        iso.as.Negative.negative = 0x7F;
        iso.as.Negative.SID = 0x10;
        iso.as.Negative.responseCode = NRC_SUBFUNCTION;
        memset(iso.as.Negative.Empty, 0, sizeof(iso.as.Negative.Empty));
        // SendData
        SendCANData(TX_MAILBOX, TX_MSG_ID, iso.as.data.ptr, 7);
        return DEFAULT_SESSION;
    }
    return DEFAULT_SESSION;
}

int ecuResetHandler(Comm *comm, uint8_t *request) // OK
{
    ISO iso;
    uint8_t ecu_reset = request[1];
    if (ecu_reset == 0x02)
    {
        // S32_SCB->AIRCR = S32_SCB_AIRCR_VECTKEY(
        // 		0x5FA) | S32_SCB_AIRCR_SYSRESETREQ(1);
        iso.as.service11.SID = 0x51;
        iso.as.service11.SUB = 0x02; // type Reset = sortware Reset
        memset(iso.as.service11.Empty, 0, sizeof(iso.as.service11.Empty));
        // SendData
        SendCANData(TX_MAILBOX, TX_MSG_ID, iso.as.data.ptr, 7);
        return R_OK;
    }
    else
    {
        // NRC
        iso.as.Negative.negative = 0x7F;
        iso.as.Negative.SID = 0x11;
        iso.as.Negative.responseCode = NRC_SUBFUNCTION;
        memset(iso.as.Negative.Empty, 0, sizeof(iso.as.Negative.Empty));
        // SendData
        SendCANData(TX_MAILBOX, TX_MSG_ID, iso.as.data.ptr, 7);
        return R_WRONG;
    }
    return R_ERROR;
}

int securityAccessHandler(Comm *comm, uint8_t *request)
{
    ISO iso;
    uint8_t check_security = request[1];
    if (check_security == 0x01) // Client yeu cau seed
    {
        uint16_t seed = generateSeed();
        uint8_t highByte = (uint8_t)(seed >> 8);
        uint8_t lowByte = (uint8_t)(seed & 0xFF);
        iso.as.service27.SID = 0x67;
        iso.as.service27.SUB = 0x01;
        iso.as.service27.highbyte_seed = highByte;
        iso.as.service27.lowbyte_seed = lowByte;
        memset(iso.as.service27.Empty, 0, sizeof(iso.as.service27.Empty));
        comm->Key = maHoa(highByte, lowByte);
        // SendData
        SendCANData(TX_MAILBOX, TX_MSG_ID, iso.as.data.ptr, 7);
        return LOCK;
    }

    else if (check_security == 0x02) // Client gui den key
    {
        uint16_t combinedValue = ((uint16_t)request[2] << 8) | request[3];
        if (combinedValue == comm->Key) // Neu la correct key
        {
            iso.as.service27.SID = 0x67;
            iso.as.service27.SUB = 0x02; // Positive response
            iso.as.service27.highbyte_seed = 0x00;
            iso.as.service27.lowbyte_seed = 0x00;
            memset(iso.as.service27.Empty, 0, sizeof(iso.as.service27.Empty));
            // SendData
            SendCANData(TX_MAILBOX, TX_MSG_ID, iso.as.data.ptr, 7);
            return UNLOCK;
        }
        else // Neu la wrong key
        {
            // NRC
            iso.as.Negative.negative = 0x7F;
            iso.as.Negative.SID = 0x27;
            iso.as.Negative.responseCode = NRC_KEY;
            memset(iso.as.Negative.Empty, 0, sizeof(iso.as.Negative.Empty));
            // SendData
            SendCANData(TX_MAILBOX, TX_MSG_ID, iso.as.data.ptr, 7);
            return LOCK;
        }
    }
    return LOCK;
}

int transferDataHandler(Comm *comm, uint8_t *request)
{
    ISO iso;
    iso.as.service36.SID = 0x77;
    iso.as.service36.blockSequenceCounter = 0x01;
    memset(iso.as.service36.Empty, 0, sizeof(iso.as.service36.Empty));
    // SendData
    SendCANData(TX_MAILBOX, TX_MSG_ID, iso.as.data.ptr, 7);
    return R_OK;
}

int requestTransferExitHandler(Comm *comm, uint8_t *request)
{
    ISO iso;
    iso.as.service37.SID = 0x77;
    memset(iso.as.service37.Empty, 0, sizeof(iso.as.service37.Empty));
    //	check = 1;
    SendCANData(TX_MAILBOX, TX_MSG_ID, iso.as.data.ptr, 7);
    return R_EXIT;
}

uint16_t generateSeed()
{
    srand((int)time(0));
    uint16_t r;
    for (int i = 0; i < 6; ++i)
    {
        r = 0 + rand() % (65355 + 1 - 0);
    }
    return r;
}

uint16_t maHoa(uint8_t byte1, uint8_t byte2)
{
    uint16_t thanhGhi;
    thanhGhi = (uint16_t)byte1 << 8 | (uint16_t)byte2;
    thanhGhi = ~thanhGhi; // Đảo bit
    thanhGhi += 1;        // Cộng 1
    return thanhGhi;
}