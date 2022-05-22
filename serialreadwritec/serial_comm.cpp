#include "serial_comm.h"

serial_comm::serial_comm(serial* serial_port)
{
	this->serial_port = serial_port;
	threads_enabled = true;
	next_packet_id = 0;

	packet_listener_worker = new std::thread(&serial_comm::packet_listener, this);
	packet_handler_worker = new std::thread(&serial_comm::packet_handler, this);
}

serial_comm::~serial_comm()
{
	stop_threads();
}

void serial_comm::stop_threads()
{
	threads_enabled = false;
}

uint8_t serial_comm::send_packet(packet_t packet_type, std::string data)
{
	uint8_t packet_id = next_packet_id++;
	uint8_t* packet_data = new uint8_t[data.length() + 1];
	memcpy(packet_data, data.c_str(), data.length() + 1);

	packet* to_send = new packet(packet_id, packet_type, packet_data, state_t::AWAITING_ACKNOWLEDGEMENT);
	if (packets.find(packet_id) != packets.end())
	{
		packet* old_packet = packets.at(packet_id);
		if (!old_packet->safe_to_override())
		{
			throw std::overflow_error("PacketBufferOverflowException");
		}
	}

	packets[packet_id] = to_send;

	serial_port->write(to_send->convert_to_bytes());
	serial_port->flush();

	return packet_id;
}

packet* serial_comm::get_packet_info(uint8_t packet_id)
{
	if (packets.find(packet_id) == packets.end())
	{
		throw std::out_of_range("Not found");
	}

	return packets.at(packet_id);
}

void serial_comm::packet_listener()
{
	while (threads_enabled)
	{
		uint8_t* serial_buffer = serial_port->read();
		if (serial_buffer == nullptr)
		{
			using namespace std::chrono_literals;
			std::this_thread::sleep_for(100ms);
		}
		else
		{
			decode_packets(serial_buffer);
			break; // TODO: for testing
		}
	}
}

void serial_comm::packet_handler()
{
	while (threads_enabled)
	{
		if (received_packets.empty())
		{
			for (std::pair<uint8_t, packet*> packet_id : packets)
			{
				packets.at(packet_id.first)->detect_timeout(); // And, do something with it (Like remove)
			}
			using namespace std::chrono_literals;
			std::this_thread::sleep_for(100ms);
		}
		else
		{
			received_packets_mtx.lock();
			while (!received_packets.empty())
			{
				packet* current_packet = received_packets.front();
				received_packets.pop();

				uint8_t packet_id = current_packet->get_packet_id();
				packet_t packet_type = current_packet->get_packet_type();
				if (packet_type == packet_t::RESPONSE_PACKET)
				{
					if (packets.find(packet_id) != packets.end())
					{
						packets.at(packet_id)->receive_response(current_packet);
					}
					else
					{
						std::cout << "Response packet does no longer have a matching sent packet\n";
					}
				}
				else if (packet_type == packet_t::ACKNOWLEDGEMENT)
				{
					if (packets.find(packet_id) != packets.end())
					{
						packets.at(packet_id)->receive_acknowledgement();
					}
					else
					{
						std::cout << "Acknowledgement packet does no longer have a matching sent packet\n";
					}
				}
				else
				{
					switch (packet_type)
					{
						default:
						{
							std::cout << "Unsupported packet: " << current_packet->to_str() << "\n";
						}
					}
				}
			}

			received_packets_mtx.unlock();
		}
	}
}

void serial_comm::decode_packets(uint8_t* packets)
{
	received_packets_mtx.lock();
	for (size_t i = 0; packets[i] != NULL;)
	{
		size_t packet_size = packets[i];

		uint8_t* buffer = new uint8_t[packet_size];
		memcpy(buffer, packets + i, packet_size);
		packet* new_packet = new packet(buffer);
		received_packets.push(new_packet);
		i += packet_size;
		std::cout << "Receiving a packet: " << new_packet->to_str() << "\n";
	}

	received_packets_mtx.unlock();
}

serial::serial()
{
	buffer = nullptr;
}

void serial::flush() { }
void serial::write(uint8_t* packet) { }

uint8_t* serial::read()
{
	return buffer;
}
