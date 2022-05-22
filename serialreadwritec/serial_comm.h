#pragma once

#include <cstdint>
#include <unordered_map>
#include <queue>
#include <mutex>
#include <thread>
#include <iostream>

#include "packet.h"

class serial
{
	uint8_t* buffer;

public:
	serial();

	virtual void flush();
	virtual void write(uint8_t* packet);
	virtual uint8_t* read();
};

class serial_comm
{
	uint8_t next_packet_id;
	serial* serial_port;
	std::unordered_map<uint8_t, packet*> packets;
	std::queue<packet*> received_packets;
	std::mutex received_packets_mtx;

	volatile bool threads_enabled;
	std::thread* packet_listener_worker;
	std::thread* packet_handler_worker;

public:
	serial_comm(serial* serial_port);
	~serial_comm();

	void stop_threads();

	uint8_t send_packet(packet_t packet_type, std::string data);
	packet* get_packet_info(uint8_t packet_id);

private:
	void packet_listener();
	void packet_handler();

	void decode_packets(uint8_t* packets);
};