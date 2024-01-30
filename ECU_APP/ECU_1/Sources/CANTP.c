#include "CANTP.h"
#include "CAN_standard.h"
Link link;
void wait(uint32_t milliseconds) {
	volatile uint32_t i, j;

	for (i = 0; i < milliseconds; i++) {
		for (j = 0; j < 7000; j++) {
			__asm("NOP");
		}
	}
}

int send_single_frame(Link *link, uint32_t messageId) {
	CanMessage message;
	int ret;

	if (*(link->send_size) < 1 || *(link->send_size) > 7) {
		return RET_LENGTH;
	}

	uint8_t SendBuffer[8];
	message.as.single_frame.SF_DL = *(link->send_size);
	message.as.single_frame.type = PCI_TYPE_SINGLE;
	memcpy(message.as.single_frame.data, link->send_data, *(link->send_size));
	memcpy(SendBuffer, &message, 8);

	SendCANData(TX_MAILBOX, messageId, SendBuffer, 8);
	ret = RET_OK;
	return ret;
}

int send_first_frame(Link *link, uint32_t messageId) {
	CanMessage message;
	int ret;
	if (*(link->send_size) <= 7) {
		return RET_LENGTH;
	}
	uint8_t SendBuffer[8];
	message.as.first_frame.FF_DL_high = (*(link->send_size) >> 8) & 0xF;
	message.as.first_frame.type = PCI_TYPE_FIRST_FRAME;
	message.as.first_frame.FF_DL_low = *(link->send_size) & 0xFF;
	memcpy(message.as.first_frame.data, link->send_data, 6);
	memcpy(SendBuffer, &message, 8);
	SendCANData(TX_MAILBOX, messageId, SendBuffer, 8);
	link->send_offset += sizeof(message.as.first_frame.data); // SĂ¡Â»â€˜ lĂ†Â°Ă¡Â»Â£ng byte Ă„â€˜Ä‚Â£ gĂ¡Â»Â­i
	link->send_sn = 1;								// SĂ¡Â»â€˜ frame Ă„â€˜Ä‚Â£ gĂ¡Â»Â­i Ă„â€˜i
	ret = RET_OK;
	return ret;
}

int send_flow_control(Link *link) {
	CanMessage message;
	int ret;

	if (*(link->send_size) <= 7) {
		return RET_LENGTH;
	}

	uint8_t SendBuffer[8];

	message.as.flow_control.FS = link->flow_status;
	message.as.flow_control.type = TYPE_FLOW_CONTROL_FRAME;
	message.as.flow_control.BS = link->block_size;
	message.as.flow_control.STmin = link->stmin;

	memset(message.as.flow_control.reserve, 0,
			sizeof(message.as.flow_control.reserve));
	memcpy(SendBuffer, &message, 8);

	SendCANData(TX_MAILBOX, TX_MSG_ID, SendBuffer, 8);

	ret = RET_OK;

	return ret;
}

int send_consecutive_frame(Link *link) {
	CanMessage message;
	int ret;
	uint16_t data_length;
	uint8_t SendBuffer[8];

	message.as.consecutive_frame.type = TYPE_CONSECUTIVE_FRAME;
	message.as.consecutive_frame.SN = link->send_sn;

	data_length = *(link->send_size) - link->send_offset;
	if (data_length > sizeof(message.as.consecutive_frame.data)) {
		data_length = sizeof(message.as.consecutive_frame.data);
	}
	memcpy(message.as.consecutive_frame.data,
			link->send_data + link->send_offset, data_length);
	memset(message.as.consecutive_frame.data + data_length, 0,
			sizeof(message.as.consecutive_frame.data) - data_length);
	memcpy(SendBuffer, &message, 8);

	SendCANData(TX_MAILBOX, TX_MSG_ID, SendBuffer, 8);

	ret = RET_OK;

	if (RET_OK == ret) {
		link->send_offset += data_length;
		if (++(link->send_sn) > 0x0F) {
			link->send_sn = 0;
		}
	}

	return ret;
}

int receive_single_frame(Link *link, CanMessage *message) {
	if (message == NULL) {
		return RET_ERROR;
	}

	if ((message->as.single_frame.SF_DL == 0)
			|| (message->as.single_frame.SF_DL > 7)) {
		return RET_LENGTH;
	}

	memcpy(link->receive_data, message->as.single_frame.data,
			message->as.single_frame.SF_DL);
	link->receive_size = message->as.single_frame.SF_DL;
	return RET_OK;
}

int receive_first_frame(Link *link, CanMessage *message) {
	if (message == NULL) {
		return RET_ERROR;
	}
	uint16_t payload_length;
	payload_length = message->as.first_frame.FF_DL_high;
	payload_length = (payload_length << 8) + message->as.first_frame.FF_DL_low;
	if (payload_length <= 7) {
		return RET_LENGTH;
	}
	if (payload_length > link->receive_buffer_size) {
		return RET_OVERFLOW;
	}

	if (payload_length <= 1785) {
		link->block_size = (payload_length / 7) + (payload_length % 7 != 0);
		link->number_of_block = 1;
		link->remaining_block = 0;
	} else if ((payload_length > 1785) && (payload_length <= 3570)) {
		link->block_size = 255;
		link->number_of_block = 2; // Can gui first frame 2 lan
		link->remaining_block = (payload_length - 1785) / 7
				+ ((payload_length - 1785) % 7 != 0);
	} else if ((payload_length > 3570) && (payload_length <= 4095)) {
		link->block_size = 255;
		link->number_of_block = 3;
		link->remaining_block = (payload_length - 3570) / 7
				+ ((payload_length - 3570) % 7 != 0);
	}

	memcpy(link->receive_data, message->as.first_frame.data,
			sizeof(message->as.first_frame.data));
	link->receive_size = payload_length;
	link->receive_offset = sizeof(message->as.first_frame.data);
	link->receive_sn = 1;
	return RET_OK;
}

int receive_flow_control(CanMessage *message) {
	if (message->as.flow_control.type != TYPE_FLOW_CONTROL_FRAME) {
		return RET_LENGTH;
	}
	return RET_OK;
}

int receive_consecutive_frame(Link *link, CanMessage *message) {
	uint16_t remaining_bytes;
	if (link->receive_sn != message->as.consecutive_frame.SN) {
		return RET_WRONG_SN;
	}

	remaining_bytes = link->receive_size - link->receive_offset;
	if (remaining_bytes > sizeof(message->as.consecutive_frame.data)) {
		remaining_bytes = sizeof(message->as.consecutive_frame.data);
	}

	memcpy(link->receive_data + link->receive_offset,
			message->as.consecutive_frame.data, remaining_bytes);
	link->receive_offset += remaining_bytes;
	if (++(link->receive_sn) > 0x0F) {
		link->receive_sn = 0;
	}

	return RET_OK;
}

int can_message(Link *link, uint8_t *data) {
	CanMessage message;
	int ret;
	memcpy(message.as.data_array.ptr, data, 8);
	switch (message.as.common.type) {

	case PCI_TYPE_SINGLE:
		if (link->receive_status == RECEIVE_STATUS_INPROGRESS) {
			break;
		}
		ret = receive_single_frame(link, &message);
//		if (RET_OK == ret) {
//			link->receive_status = RECEIVE_STATUS_FULL;
//		}
		break;

	case PCI_TYPE_FIRST_FRAME:
		if (link->receive_status == RECEIVE_STATUS_INPROGRESS) {
			break;
		}
		ret = receive_first_frame(link, &message);
		if (RET_OVERFLOW == ret) {
			link->receive_status = RECEIVE_STATUS_IDLE;
			break;
		}
		if (RET_OK == ret) {
			link->receive_status = RECEIVE_STATUS_INPROGRESS;
			send_flow_control(link);
		}
		break;

	case TYPE_FLOW_CONTROL_FRAME:
		ret = receive_flow_control(&message);

		if (RET_OK == ret) {
			if (PCI_FLOW_STATUS_OVERFLOW == message.as.flow_control.FS) {
				break;
			} else if (PCI_FLOW_STATUS_WAIT == message.as.flow_control.FS) {
				break;
			}

			else if (PCI_FLOW_STATUS_CONTINUE == message.as.flow_control.FS) {
				if (0 == message.as.flow_control.BS) {
					break;
				} else {
					link->send_bs_remain = message.as.flow_control.BS;
				}
			}
		}
		break;

	case TYPE_CONSECUTIVE_FRAME:
		if (RECEIVE_STATUS_INPROGRESS != link->receive_status) {
			break;
		}
		ret = receive_consecutive_frame(link, &message);
		if (RET_OK == ret) {
			if (link->receive_offset >= link->receive_size) {
				link->receive_status = RECEIVE_STATUS_FULL;
			}
			if ((link->receive_offset == 1791)
					&& (link->number_of_block == 2)) {
				link->block_size = link->remaining_block;
				send_flow_control(link);
			}
			if (link->receive_offset == 1791 && link->number_of_block == 3) {
				link->block_size = 255;
				send_flow_control(link);
			}
			if (link->receive_offset == 3576 && link->number_of_block == 3) {
				link->block_size = link->remaining_block;
				send_flow_control(link);
			}
		}
		break;
	}
	return 0;
}

int sendData(Link *link, uint16_t size) {
	link->send_size = &size;
	if (size == 0) {
		return RET_LENGTH;
	} else if ((size > 0) && (size < 8)) {
		send_single_frame(link, TX_MSG_ID);
		return RET_OK;
	}

	else if ((size >= 8) && (size <= 4096)) {
		send_first_frame(link, TX_MSG_ID);

		// Delay link->stmin
		wait(link->stmin);

		int maxIterations = 3; // Max = 3
		int iterations = 0;

		while (link->send_bs_remain > 0 && iterations < maxIterations) {
			for (int i = 0; i < link->send_bs_remain; i++) {
				wait(link->stmin);
				send_consecutive_frame(link);
			}
			link->send_bs_remain = 0;
			wait(link->stmin);
			iterations++;
		}
		link->send_offset = 0;
		link->send_sn = 0;
		link->send_bs_remain = 0;
		return 0;
	} else {
		return RET_LENGTH;
	}
}
