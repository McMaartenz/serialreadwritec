#pragma once

#include <cstdint>
#include <string>
#include <sstream>
#include <chrono>

enum state_t
{
	AWAITING_ACKNOWLEDGEMENT,
	AWAITING_RESPONSE_PACKET,
	RESPONSE_PACKET_RECEIVED,
	PACKET_REACHED_A_TIMEOUT
};

enum packet_t
{
	ACKNOWLEDGEMENT,
	RESPONSE_PACKET,
	REQUEST_POSITION
};

std::string packet_state_to_str(state_t packet_type);
std::string packet_type_to_str(packet_t packet_type);
uint8_t packet_type_to_byte(packet_t packet_type);

long long ms_since_epoch();

class packet
{
	uint8_t packet_id;
	packet_t packet_type;
	uint8_t* packet_data;
	state_t packet_state;
	
	long long time_sent;
	long long time_ack_received;
	long long time_response_received;

	packet* response_packet;

public:
	packet(uint8_t packet_id, packet_t packet_type, uint8_t packet_data[], state_t packet_state);
	packet(uint8_t packet_buffer[]);
	
	void receive_acknowledgement();
	void receive_response(packet* response_packet);

	void set_time_sent(long long unix_time);
	void set_time_ack_received(long long unix_time);
	void set_time_response_received(long long unix_time);

	bool is_acknowledged();
	bool received_response();

	long long get_time_sent();
	long long get_time_ack_received();
	long long get_time_response_received();

	uint8_t get_packet_id();
	packet_t get_packet_type();
	uint8_t* get_packet_data();
	std::string get_packet_data_as_string();

	uint8_t* convert_to_bytes();
	bool detect_timeout();
	bool safe_to_override();
	long long last_active_time();

	std::string to_str();
};