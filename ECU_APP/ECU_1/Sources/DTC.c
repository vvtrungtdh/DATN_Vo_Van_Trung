#include "DTC.h"
/*Variablr*/
flag lastOperationCycle;
flag currentOperationCycle;
uint32_t sample_time = 0;
DTCData DTCData_Recent[2];
DTCData DTCHistory[2];
DTCData DTCCurrent[2] = {{ .Sensor = { 37, 100, 0, NORESULT }, .DTC = { 0x01,
		0x01, 0x02 }, .Status = 0, .Condition = 10, .DTCHandler = DTCStatus,
		.Snapshot = takeSnapshot }, { .Sensor = { 37, 100, 0, NORESULT }, .DTC =
		{ 0x01, 0x01, 0x03 }, .Status = 0, .Condition = 10, .Snapshot = takeSnapshot }, };

void takeSnapshot(DTCData *data, DTCData *DTCHistory) {
	int bit3 = (data->Status >> 3) & 0x01;
	if (bit3 == 1) {
		*DTCHistory = *data;
		DTCHistory[0].Status = DTCCurrent[0].Status;
		DTCHistory[1].Status = DTCCurrent[1].Status;
	}
}
int checkSensor(DTCData *data) {
	if (data->Sensor.value >= data->Sensor.limit) {
		return TEST_FAILED;
	} else if (data->Sensor.value < data->Sensor.limit) {
		return TEST_OK;
	}
	return 0;
}

void updateSensorPretest(DTCData *data, int checkResult) {
	if (checkResult == TEST_OK) {
		data->Sensor.Pretest -= 1;
	} else {
		if (data->Sensor.Pretest >= 0) {
			data->Sensor.Pretest += 2;
		} else {
			data->Sensor.Pretest = 1;
		}
	}
}

void updateSensorResult(DTCData *data) {
	int COUNTER_PREFAILED = data->Condition;
	int COUNTER_PREPASSED = -(data->Condition);
	if (data->Sensor.Pretest >= COUNTER_PREFAILED) {
		data->Sensor.result = FAILED;
		data->Sensor.Pretest = 0;
	} else if (data->Sensor.Pretest <= COUNTER_PREPASSED) {
		data->Sensor.result = PASS;
		data->Sensor.Pretest = 0;
	}
}

int scanFaultDetection(DTCData *data) {
	int checkResult1 = checkSensor(&data[0]);
	int checkResult2 = checkSensor(&data[1]);

	updateSensorPretest(&data[0], checkResult1);
	updateSensorPretest(&data[1], checkResult2);

	updateSensorResult(&data[0]);
	updateSensorResult(&data[1]);
	return 0;
}

int getBit(uint8_t reg, int bitPosition) {
	return (reg >> bitPosition) & 1;
}

void setBit(uint8_t *reg, int bitPosition, int value) {
	if (value) {
		*reg |= (1u << bitPosition);
	} else {
		*reg &= ~(1u << bitPosition);
	}
}

void copyDataToRecent(DTCData *data, DTCData *DTCData_Recent, int size) {
	for (int i = 0; i < size; ++i) {
		DTCData_Recent[i] = data[i];
	}
}

int compareStatusBit2(DTCData *data, DTCData *DTCData_Recent) {
	int bit2_data = (data->Status >> 1) & 1;
	int bit2_DTCData_Recent = (DTCData_Recent->Status >> 1) & 1;
	if (bit2_data == 1 && bit2_DTCData_Recent == 1) {
		return 1;
	} else {
		return 0;
	}
}

int DTCStatus(DTCData *data, DTCData *DTCData_Recent) {
	int testFailed = getBit(data->Status, 0);
	int testFailedThisOperationCycle = getBit(data->Status, 1);
	int pendingDTC = getBit(data->Status, 2);
	int confirmedDTC = getBit(data->Status, 3);
	int testNotCompletedSinceLastClear = getBit(data->Status, 4);
	int testFailedSinceLastClear = getBit(data->Status, 5);
	int testNotCompletedThisOperationCycle = getBit(data->Status, 6);
	scanFaultDetection(data);
	// Update bits based on most recent test results
	if (data->Sensor.result == PASS || (data->Sensor.result == NORESULT)) {
		testFailed = 0;
	} else if ((data->Sensor.result == FAILED)) {
		testFailed = 1;
	}
	setBit(&(data->Status), 0, testFailed);

	// Update testFailedThisOperationCycle based on operation cycle and most recent test result
	if (lastOperationCycle == STOP && currentOperationCycle == START) {
		testFailedThisOperationCycle = 0;
	} else if (data->Sensor.result == FAILED) {
		testFailedThisOperationCycle = 1;
	}
	setBit(&(data->Status), 1, testFailedThisOperationCycle);

	// Update pendingDTC based on most recent test result and operation cycle
	if (data->Sensor.result == FAILED) {
		pendingDTC = 1;
	} else if (currentOperationCycle == STOP
			&& testFailedThisOperationCycle == 0) {
		pendingDTC = 0;
	}
	setBit(&(data->Status), 2, pendingDTC);

	// Determine and update confirmedDTC based on threshold and confirm stage
	int threshold = compareStatusBit2(data, DTCData_Recent);
	if (data->confirmStage == INITIAL_MONITOR) {
		// If threshold met, confirm the DTC, update aging status, and set confirm stage to AGING_MONITOR
		if (threshold == 1) {
			confirmedDTC = 1;
			data->agingStatus = 0;
			data->confirmStage = AGING_MONITOR;
		} else {
			confirmedDTC = 0;
		}
	} else if (data->confirmStage == AGING_MONITOR) {
		// Manage aging status and confirm DTC accordingly
		if (data->agingStatus == 6) {
			confirmedDTC = 0;
			data->confirmStage = INITIAL_MONITOR;
			data->agingStatus = 0;
		} else if (data->Sensor.result == FAILED) {
			data->agingStatus = 0;
		} else {
			if (!threshold && currentOperationCycle == STOP) {
				data->agingStatus++;
			}
		}
	}
	setBit(&(data->Status), 3, confirmedDTC);

	// Update testNotCompletedSinceLastClear based on most recent test result
	if (data->Sensor.result == PASS || data->Sensor.result == FAILED) {
		testNotCompletedSinceLastClear = 0;
	}
	setBit(&(data->Status), 4, testNotCompletedSinceLastClear);

	// Manage testFailedSinceLastClear based on aging status and most recent test result
	if (data->agingStatus == 2) {
		testFailedSinceLastClear = 0;
	} else if (data->Sensor.result == FAILED) {
		testFailedSinceLastClear = 1;
	}
	setBit(&(data->Status), 5, testFailedSinceLastClear);

	// Manage testNotCompletedThisOperationCycle flag
	if (lastOperationCycle == STOP && currentOperationCycle == START) {
		testNotCompletedThisOperationCycle = 1;
	} else if (data->Sensor.result == PASS || data->Sensor.result == FAILED) {
		testNotCompletedThisOperationCycle = 0;
	}
	setBit(&(data->Status), 6, testNotCompletedThisOperationCycle);

	// Save DTC
	if (lastOperationCycle == STOP && currentOperationCycle == STOP) {
		copyDataToRecent(data, DTCData_Recent, DTC_NUMBER);
	}
	return 0;
}
