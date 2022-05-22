#pragma once

#include "serial_comm.h"

class mock_serial: serial
{
	uint8_t* buffer;

public:
	mock_serial();

	void flush();
	void write(uint8_t packet[]);
	uint8_t* read();

	void mock_packet(uint8_t* packets_data[], size_t packet_count);
};