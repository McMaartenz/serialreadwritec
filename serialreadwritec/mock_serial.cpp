#include "mock_serial.h"

mock_serial::mock_serial()
{
	buffer = nullptr;
}

void mock_serial::flush() { }
void mock_serial::write(uint8_t packet[]) { }

uint8_t* mock_serial::read()
{
	return buffer;
}

void mock_serial::mock_packet(uint8_t* packets_data[], size_t packet_count)
{
	size_t full_size = 0;
	for (size_t i = 0; i < packet_count; i++)
	{
		uint8_t* packet = packets_data[i];
		full_size += *packet;
	}

	uint8_t* buffer = new uint8_t[full_size + 1];
	for (size_t i = 0, cpy_offset = 0; i < packet_count; i++, cpy_offset += packets_data[i - 1][0])
	{
		memcpy(buffer + cpy_offset, packets_data[i], *(packets_data[i]));
	}
	buffer[full_size] = NULL;

	this->buffer = buffer;
}
