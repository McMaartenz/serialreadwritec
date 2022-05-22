#include <iostream>
#include <thread>
#include <chrono>

#include "mock_serial.h"
#include "serial_comm.h"

int main()
{
	std::cout << "Hello, World!\n";
	mock_serial fake_serial;
	serial_comm serial_communication((serial*) &fake_serial);

	uint8_t packet_id1 = serial_communication.send_packet(packet_t::REQUEST_POSITION, "hello world1");
	uint8_t packet_id2 = serial_communication.send_packet(packet_t::REQUEST_POSITION, "hello world2");
	uint8_t packet_id3 = serial_communication.send_packet(packet_t::REQUEST_POSITION, "hello world3");

	packet* packet1 = serial_communication.get_packet_info(packet_id1);
	packet* packet2 = serial_communication.get_packet_info(packet_id2);
	packet* packet3 = serial_communication.get_packet_info(packet_id3);

	packet* a = new packet((uint8_t)0, packet_t::ACKNOWLEDGEMENT, (uint8_t*)std::string("hello1").c_str(), state_t::AWAITING_ACKNOWLEDGEMENT);
	packet* b = new packet((uint8_t)0, packet_t::RESPONSE_PACKET, (uint8_t*)std::string("hello2").c_str(), state_t::AWAITING_ACKNOWLEDGEMENT);
	packet* c = new packet((uint8_t)1, packet_t::ACKNOWLEDGEMENT, (uint8_t*)std::string("hello3").c_str(), state_t::AWAITING_ACKNOWLEDGEMENT);
	packet* d = new packet((uint8_t)1, packet_t::RESPONSE_PACKET, (uint8_t*)std::string("hello4").c_str(), state_t::AWAITING_ACKNOWLEDGEMENT);

	uint8_t* bytes_a = a->convert_to_bytes();
	uint8_t* bytes_b = b->convert_to_bytes();
	uint8_t* bytes_c = c->convert_to_bytes();
	uint8_t* bytes_d = d->convert_to_bytes();

	uint8_t** bytes = new uint8_t*[4];
	bytes[0] = bytes_a;
	bytes[1] = bytes_b;
	bytes[2] = bytes_c;
	bytes[3] = bytes_d;

	fake_serial.mock_packet(bytes, 4);

	{
		using namespace std::chrono_literals;
		std::this_thread::sleep_for(4000ms);
	}

	std::cout << "Packet 1: " << packet1->to_str() << "\n";
	std::cout << "Packet 2: " << packet2->to_str() << "\n";
	std::cout << "Packet 3: " << packet3->to_str() << "\n";
}
