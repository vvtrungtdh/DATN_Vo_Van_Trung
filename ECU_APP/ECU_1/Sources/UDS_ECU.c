#include <UDS_ECU.h>

/*Variable*/
Comm comm;
flash_ssd_config_t flashSsdConfig;
uint8_t request[7] = { 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 };
int (*serviceHandlers[NUM_UDS_SERVICES])(Comm *comm, uint8_t *) = {
	[DIAGNOSTIC_SESSION_CONTROL] = diagnosticSessionControlHandler,
	[ECU_RESET] = ecuResetHandler,
	[CLEAR_DIAGNOSTIC_INFO] = clearDiagnosticInfoHandler,
	[READ_DTC_INFO] = readDtcInfoHandler,
	[READ_DATA_BY_IDENTIFIER] = readDataByIdentifierHandler,
	[WRITE_DATA_BY_IDENTIFIER] = writeDataByIdentifierHandler,
	[SECURITY_ACCESS] = securityAccessHandler,
	[REQUEST_DOWNLOAD] = requestDownloadHandler };

int processUDSService(Comm *comm, uint8_t request[7]) {
	ISO iso;
	UDS_Service service = (UDS_Service) request[0];

	// Reset request and return error if service ID is 0x00
	if (service == 0x00) {
		memset(request, 0, SIZE_REQUEST);
		return R_ERROR;
	}

	// Execute service based on the service ID
	if (service == SECURITY_ACCESS) {
		comm->check_security = serviceHandlers[service](comm, request);
	} else if (service == DIAGNOSTIC_SESSION_CONTROL) {
		if (comm->check_security == UNLOCK) {
			comm->session_control = serviceHandlers[service](comm, request);
		} else {
			// Need to pass security first
			iso.as.Negative.negative = 0x7F;
			iso.as.Negative.SID = 0x10;
			iso.as.Negative.responseCode = NRC_CONDITION;
			memset(iso.as.Negative.Empty, NULL_DATA,
					sizeof(iso.as.Negative.Empty));
			// SendData
			SendCANData(TX_MAILBOX, TX_MSG_ID, iso.as.data.ptr, 7);
			memset(request, 0, SIZE_REQUEST);
			return R_WRONG;
		}
	} else if (comm->session_control == DEFAULT_SESSION
			&& (service == ECU_RESET || service == CLEAR_DIAGNOSTIC_INFO
					|| service == READ_DTC_INFO)) {
		serviceHandlers[service](comm, request);
	} else if (comm->session_control == CUSTOM_SESSION
			&& (service == ECU_RESET || service == CLEAR_DIAGNOSTIC_INFO
					|| service == READ_DTC_INFO
					|| service == READ_DATA_BY_IDENTIFIER
					|| service == WRITE_DATA_BY_IDENTIFIER
					|| service == REQUEST_DOWNLOAD)) {
		serviceHandlers[service](comm, request);
	} else {
		// Service not supported in the current session/control
		iso.as.Negative.negative = 0x7F;
		iso.as.Negative.SID = 0x10;
		iso.as.Negative.responseCode = NRC_SUBFUNCTION;
		memset(iso.as.Negative.Empty, NULL_DATA, sizeof(iso.as.Negative.Empty));
		// SendData
		SendCANData(TX_MAILBOX, TX_MSG_ID, iso.as.data.ptr, 7);
		memset(request, 0, SIZE_REQUEST);
		return R_WRONG;
	}

	// Clear request and return success
	memset(request, 0, SIZE_REQUEST);
	return R_OK;
}

int diagnosticSessionControlHandler(Comm *comm, uint8_t *request) // OK
{
	ISO iso;
	uint8_t session_request = request[1];

	if (session_request == 0x01) // 10 01
			{
		iso.as.service10.SID = 0x50;
		iso.as.service10.SUB = 0x01;
		iso.as.service10.time_min_highbyte = TIME_MIN_HB;
		iso.as.service10.time_min_lowbyte = TIME_MIN_LB;
		iso.as.service10.time_max_highbyte = TIME_MAX_HB;
		iso.as.service10.time_max_lowbyte = TIME_MAX_LB;
		iso.as.service10.Empty = NULL_DATA;
		SendCANData(TX_MAILBOX, TX_MSG_ID, iso.as.data.ptr, 7);
		return DEFAULT_SESSION;
	} else if (session_request == 0x02) // 10 02
			{
		iso.as.service10.SID = 0x50;
		iso.as.service10.SUB = 0x02;
		iso.as.service10.time_min_highbyte = TIME_MIN_HB;
		iso.as.service10.time_min_lowbyte = TIME_MIN_LB;
		iso.as.service10.time_max_highbyte = TIME_MAX_HB;
		iso.as.service10.time_max_lowbyte = TIME_MAX_LB;
		iso.as.service10.Empty = NULL_DATA;
		SendCANData(TX_MAILBOX, TX_MSG_ID, iso.as.data.ptr, 7);
		return CUSTOM_SESSION;
	} else {
		iso.as.Negative.negative = 0x7F;
		iso.as.Negative.SID = 0x10;
		iso.as.Negative.responseCode = NRC_SUBFUNCTION;
		memset(iso.as.Negative.Empty, NULL_DATA, sizeof(iso.as.Negative.Empty));
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
	if (ecu_reset == 0x02) {
		iso.as.service11.SID = 0x51;
		iso.as.service11.SUB = 0x02; // type Reset = sortware Reset
		memset(iso.as.service11.Empty, NULL_DATA,
				sizeof(iso.as.service11.Empty));
		// SendData
		SendCANData(TX_MAILBOX, TX_MSG_ID, iso.as.data.ptr, 7);
		wait(10);
		S32_SCB->AIRCR = S32_SCB_AIRCR_VECTKEY(
				0x5FA) | S32_SCB_AIRCR_SYSRESETREQ(1);
		return R_OK;
	} else {
		// NRC
		iso.as.Negative.negative = 0x7F;
		iso.as.Negative.SID = 0x11;
		iso.as.Negative.responseCode = NRC_SUBFUNCTION;
		memset(iso.as.Negative.Empty, NULL_DATA, sizeof(iso.as.Negative.Empty));
		// SendData
		SendCANData(TX_MAILBOX, TX_MSG_ID, iso.as.data.ptr, 7);
		return R_WRONG;
	}
	return R_ERROR;
}

int clearDiagnosticInfoHandler(Comm *comm, uint8_t *request) {
	DTCHistory[0].DTC.high = 0;
	DTCHistory[0].DTC.mid = 0;
	DTCHistory[0].DTC.low = 0;
	DTCHistory[0].Sensor.value = 0;
	DTCHistory[0].Sensor.result = NORESULT;
	DTCHistory[0].Sensor.Pretest = 0;
	DTCHistory[0].Status = 0;
	DTCHistory[0].confirmStage = INITIAL_MONITOR;
	DTCHistory[1].DTC.high = 0;
	DTCHistory[1].DTC.mid = 0;
	DTCHistory[1].DTC.low = 0;
	DTCHistory[1].Sensor.value = 0;
	DTCHistory[1].Sensor.result = NORESULT;
	DTCHistory[1].Sensor.Pretest = 0;
	DTCHistory[1].Status = 0;
	DTCHistory[1].confirmStage = INITIAL_MONITOR;

	DTCCurrent[0].DTC.high = 0;
	DTCCurrent[0].DTC.mid = 0;
	DTCCurrent[0].DTC.low = 0;
	DTCCurrent[0].Sensor.value = 0;
	DTCCurrent[0].Sensor.result = NORESULT;
	DTCCurrent[0].Sensor.Pretest = 0;
	DTCCurrent[0].Status = 0;
	DTCCurrent[0].confirmStage = INITIAL_MONITOR;
	DTCCurrent[1].DTC.high = 0;
	DTCCurrent[1].DTC.mid = 0;
	DTCCurrent[1].DTC.low = 0;
	DTCCurrent[1].Sensor.value = 0;
	DTCCurrent[1].Sensor.result = NORESULT;
	DTCCurrent[1].Sensor.Pretest = 0;
	DTCCurrent[1].Status = 0;
	DTCCurrent[1].confirmStage = INITIAL_MONITOR;
	return 0;
}

int readDtcInfoHandler(Comm *comm, uint8_t *request) {
	uint8_t bitone = (DTCHistory[0].Status >> 3) & 0x01;
	uint8_t bittwo = (DTCHistory[1].Status >> 3) & 0x01;
	uint8_t bitbase[10];
	uint8_t sub = request[1];
	if ((bitone == 1) && (bittwo == 1)) {
		if (sub == 0x01) {
			bitbase[0] = 0x59;
			bitbase[1] = 0x01;
			bitbase[2] = 2;
			SendCANData(TX_MAILBOX, TX_MSG_ID, bitbase, 3);
			return R_OK;
		}
		if (sub == 0x02) {
			bitbase[0] = 0x59;
			bitbase[1] = 0x02;
			bitbase[2] = 3;
			bitbase[3] = DTCHistory[0].Status;
			bitbase[4] = DTCHistory[1].Status;
			SendCANData(TX_MAILBOX, TX_MSG_ID, bitbase, 5);
			return R_OK;
		}
		if (sub == 0x03) {
			bitbase[0] = 0x59;
			bitbase[1] = 0x03;
			bitbase[2] = 3;
			bitbase[3] = DTCHistory[0].Sensor.value;
			bitbase[4] = DTCHistory[1].Sensor.value;
			SendCANData(TX_MAILBOX, TX_MSG_ID, bitbase, 5);
			return R_OK;
		}
	} else if (bitone == 1) {
		if (sub == 0x01) {
			bitbase[0] = 0x59;
			bitbase[1] = 0x01;
			bitbase[2] = 1;
			SendCANData(TX_MAILBOX, TX_MSG_ID, bitbase, 3);
			return R_OK;
		}
		if (sub == 0x02) {
			bitbase[0] = 0x59;
			bitbase[1] = 0x02;
			bitbase[2] = 1;
			bitbase[3] = DTCHistory[0].Status;
			SendCANData(TX_MAILBOX, TX_MSG_ID, bitbase, 4);
			return R_OK;
		}
		if (sub == 0x03) {
			bitbase[0] = 0x59;
			bitbase[1] = 0x03;
			bitbase[2] = 1;
			bitbase[3] = DTCHistory[0].Sensor.value;
			SendCANData(TX_MAILBOX, TX_MSG_ID, bitbase, 4);
			return R_OK;
		}
	} else if (bittwo == 1) {
		if (sub == 0x01) {
			bitbase[0] = 0x59;
			bitbase[1] = 0x01;
			bitbase[2] = 1;
			SendCANData(TX_MAILBOX, TX_MSG_ID, bitbase, 3);
			return R_OK;
		}
		if (sub == 0x02) {
			bitbase[0] = 0x59;
			bitbase[1] = 0x02;
			bitbase[2] = 2;
			bitbase[3] = DTCHistory[1].Status;
			SendCANData(TX_MAILBOX, TX_MSG_ID, bitbase, 4);
			return R_OK;
		}
		if (sub == 0x03) {
			bitbase[0] = 0x59;
			bitbase[1] = 0x03;
			bitbase[2] = 2;
			bitbase[3] = DTCHistory[1].Sensor.value;
			SendCANData(TX_MAILBOX, TX_MSG_ID, bitbase, 4);
			return R_OK;
		}
	} else {
		bitbase[0] = 0x59;
		bitbase[1] = 0x04;
		SendCANData(TX_MAILBOX, TX_MSG_ID, bitbase, 2);
		return R_OK;
	}
	return 0;
}

int readDataByIdentifierHandler(Comm *comm, uint8_t *request) {
	ISO iso;
	uint8_t ID[4] = { 0xAB, 0xCD, 0xEF, 0x01 };
	uint16_t DID = ((uint16_t) request[1] << 8) | request[2];
	if (DID == BootSoftwareIdentificationDataIdentifier) {
		// read flash to check DID Boot Function
		// function read ID
		iso.as.service2E.SID = 0x62;
		iso.as.service2E.MSB = request[1];
		iso.as.service2E.LSB = request[2];
		iso.as.service22.dataRecord1 = comm->ID[0];
		iso.as.service22.dataRecord2 = comm->ID[1];
		iso.as.service22.dataRecord3 = comm->ID[2];
		iso.as.service22.dataRecord4 = comm->ID[3];
		SendCANData(TX_MAILBOX, TX_MSG_ID, iso.as.data.ptr, 7);
		return R_OK;
	} else if (DID == applicationSoftwareIdentificationDataIdentifier) {
		// read flash to check DID App Function
		// function read ID
		iso.as.service2E.SID = 0x62;
		iso.as.service2E.MSB = request[1];
		iso.as.service2E.LSB = request[2];
		iso.as.service22.dataRecord1 = comm->ID[0];
		iso.as.service22.dataRecord2 = comm->ID[1];
		iso.as.service22.dataRecord3 = comm->ID[2];
		iso.as.service22.dataRecord4 = comm->ID[3];
		SendCANData(TX_MAILBOX, TX_MSG_ID, iso.as.data.ptr, 7);
		return R_OK;
	} else {
		// NRC
		iso.as.Negative.negative = 0x7F;
		iso.as.Negative.SID = 0x22;
		iso.as.Negative.responseCode = NRC_INVALID;
		memset(iso.as.Negative.Empty, NULL_DATA, sizeof(iso.as.Negative.Empty));
		// SendData
		SendCANData(TX_MAILBOX, TX_MSG_ID, iso.as.data.ptr, 7);
		return R_WRONG;
	}
	return R_ERROR;
}

int writeDataByIdentifierHandler(Comm *comm, uint8_t *request) {
	ISO iso;
	uint16_t DID = ((uint16_t) request[1] << 8) | request[2];
	if (DID == BootSoftwareIdentificationDataIdentifier) {
		comm->ID[0] = request[3];
		comm->ID[1] = request[4];
		comm->ID[2] = request[5];
		comm->ID[3] = request[6];

		iso.as.service2E.SID = 0x6E;
		iso.as.service2E.MSB = request[1];
		iso.as.service2E.LSB = request[2];
		memset(iso.as.service2E.Empty, NULL_DATA,
				sizeof(iso.as.service2E.Empty));
		SendCANData(TX_MAILBOX, TX_MSG_ID, iso.as.data.ptr, 7);
		return R_OK;
	} else if (DID == applicationSoftwareIdentificationDataIdentifier) {
		comm->ID[0] = request[3];
		comm->ID[1] = request[4];
		comm->ID[2] = request[5];
		comm->ID[3] = request[6];

		iso.as.service2E.SID = 0x6E;
		iso.as.service2E.MSB = request[1];
		iso.as.service2E.LSB = request[2];
		memset(iso.as.service2E.Empty, NULL_DATA,
				sizeof(iso.as.service2E.Empty));
		SendCANData(TX_MAILBOX, TX_MSG_ID, iso.as.data.ptr, 7);
		return R_OK;
	} else {
		// NRC
		iso.as.Negative.negative = 0x7F;
		iso.as.Negative.SID = 0x2E;
		iso.as.Negative.responseCode = NRC_INVALID;
		memset(iso.as.Negative.Empty, NULL_DATA, sizeof(iso.as.Negative.Empty));
		// SendData
		SendCANData(TX_MAILBOX, TX_MSG_ID, iso.as.data.ptr, 7);
		return R_WRONG;
	}
	return R_ERROR;
}

int securityAccessHandler(Comm *comm, uint8_t *request) {
	ISO iso;
	uint8_t check_security = request[1];
	if (check_security == 0x01) // Client yeu cau seed
			{
		uint16_t seed = generateSeed();
		uint8_t highByte = (uint8_t) (seed >> 8);
		uint8_t lowByte = (uint8_t) (seed & 0xFF);
		iso.as.service27.SID = 0x67;
		iso.as.service27.SUB = 0x01;
		iso.as.service27.highbyte_seed = highByte;
		iso.as.service27.lowbyte_seed = lowByte;
		memset(iso.as.service27.Empty, NULL_DATA,
				sizeof(iso.as.service27.Empty));
		comm->Key = maHoa(highByte, lowByte);
		// SendData
		SendCANData(TX_MAILBOX, TX_MSG_ID, iso.as.data.ptr, 7);
		return LOCK;
	}

	else if (check_security == 0x02) // Client gui den key
			{
		uint16_t combinedValue = ((uint16_t) request[2] << 8) | request[3];
		if (combinedValue == comm->Key) // Neu la correct key
				{
			iso.as.service27.SID = 0x67;
			iso.as.service27.SUB = 0x02; // Positive response
			iso.as.service27.highbyte_seed = 0x00;
			iso.as.service27.lowbyte_seed = 0x00;
			memset(iso.as.service27.Empty, NULL_DATA,
					sizeof(iso.as.service27.Empty));
			// SendData
			SendCANData(TX_MAILBOX, TX_MSG_ID, iso.as.data.ptr, 7);
			return UNLOCK;
		} else // Neu la wrong key
		{
			// NRC
			iso.as.Negative.negative = 0x7F;
			iso.as.Negative.SID = 0x27;
			iso.as.Negative.responseCode = NRC_KEY;
			memset(iso.as.Negative.Empty, NULL_DATA,
					sizeof(iso.as.Negative.Empty));
			// SendData
			SendCANData(TX_MAILBOX, TX_MSG_ID, iso.as.data.ptr, 7);
			return LOCK;
		}
	} else {
		iso.as.Negative.negative = 0x7F;
		iso.as.Negative.SID = 0x27;
		iso.as.Negative.responseCode = NRC_SUBFUNCTION;
		memset(iso.as.Negative.Empty, NULL_DATA, sizeof(iso.as.Negative.Empty));
		// SendData
		SendCANData(TX_MAILBOX, TX_MSG_ID, iso.as.data.ptr, 7);
		return LOCK;
	}
}

int requestDownloadHandler(Comm *comm, uint8_t *request) {
	ISO iso;
	uint8_t Address[4];
	uint8_t Size[4];
	uint8_t SuccessFlash[8];
	status_t ret;
	memset(SuccessFlash, 0x00, 8);
	Address[0] = 0x00;
	Address[1] = request[1];
	Address[2] = request[2];
	Address[3] = request[3];
	Size[0] = 0x00;
	Size[1] = request[4];
	Size[2] = request[5];
	Size[3] = request[6];
	comm->memoryAddress |= (uint32_t) Address[0] << 24;
	comm->memoryAddress |= (uint32_t) Address[1] << 16;
	comm->memoryAddress |= (uint32_t) Address[2] << 8;
	comm->memoryAddress |= (uint32_t) Address[3];
	comm->memorySize |= (uint32_t) Size[0] << 24;
	comm->memorySize |= (uint32_t) Size[1] << 16;
	comm->memorySize |= (uint32_t) Size[2] << 8;
	comm->memorySize |= (uint32_t) Size[3];
	comm->maxNumberOfBlockLength = MAX_NUMBER_OF_BLOCK; // toi da 100kb = 25 x 4096 byte

	// Send Data
	iso.as.service34.SID = 0x74;
	iso.as.service34.lengthFormatIdentifier = 0x01;
	iso.as.service34.maxNumberOfBlockLength1 = MAX_NUMBER_OF_BLOCK;
	memset(iso.as.service34.Empty, NULL_DATA, sizeof(iso.as.service34.Empty));
	SendCANData(TX_MAILBOX, TX_MSG_ID, iso.as.data.ptr, 7);

	// Reset Flag and reset
	S32_SCB->AIRCR = S32_SCB_AIRCR_VECTKEY(
			0x5FA) | S32_SCB_AIRCR_SYSRESETREQ(1);
	return 0;
}

uint16_t generateSeed() {
	srand((int) time(0));
	uint16_t r;
	for (int i = 0; i < 6; ++i) {
		r = 0 + rand() % (65355 + 1 - 0);
	}
	return r;
}

uint16_t maHoa(uint8_t byte1, uint8_t byte2) {
	uint16_t thanhGhi;
	thanhGhi = (uint16_t) byte1 << 8 | (uint16_t) byte2;
	thanhGhi = ~thanhGhi; // Ä�áº£o bit
	thanhGhi += 1;		  // Cá»™ng 1
	return thanhGhi;
}

void writeDataToDFlash(uint32_t address, uint8_t dataLength, uint8_t data[]) {
	status_t ret;
	uint32_t failAddr;
	// Program data from the source buffer to DFlash
	ret = FLASH_DRV_Program(&flashSsdConfig, address, dataLength, data);
	// DEV_ASSERT(STATUS_SUCCESS == ret);

	// Verify the program operation at margin level value of 1, user margin
	ret = FLASH_DRV_ProgramCheck(&flashSsdConfig, address, dataLength, data,
			&failAddr, 1u);
	// DEV_ASSERT(STATUS_SUCCESS == ret);
}
