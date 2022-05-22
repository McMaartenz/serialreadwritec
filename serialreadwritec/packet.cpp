#include "packet.h"

packet::packet(uint8_t packet_id, packet_t packet_type, uint8_t packet_data[], state_t packet_state)
{
	this->packet_id = packet_id;
	this->packet_type = packet_type;
	this->packet_data = packet_data;
	this->packet_state = packet_state;

	response_packet = nullptr;
}

packet::packet(uint8_t packet_buffer[])
{
	size_t packet_length = packet_buffer[0];
	this->packet_id = packet_buffer[1];
	this->packet_type = (packet_t)packet_buffer[2];
	
	if (packet_length < 4)
	{
		packet_data = nullptr;
	}
	else
	{
		memcpy(packet_data, packet_buffer + 3, packet_length - 3);
	}

	response_packet = nullptr;
}

long long ms_since_epoch()
{
	namespace sc = std::chrono;
	return sc::duration_cast<sc::milliseconds>(sc::system_clock::now().time_since_epoch()).count();

}

void packet::receive_acknowledgement()
{
	packet_state = state_t::AWAITING_RESPONSE_PACKET;
	set_time_ack_received(ms_since_epoch());
}

void packet::receive_response(packet* response_packet)
{
	this->response_packet = response_packet;
	packet_state = state_t::RESPONSE_PACKET_RECEIVED;
	set_time_response_received(ms_since_epoch());
}

void packet::set_time_sent(long long unix_time)
{
	if (time_sent == 0)
	{
		time_sent = unix_time;
	}
	else
	{
		throw std::exception("Bad access exception");
	}
}

void packet::set_time_ack_received(long long unix_time)
{
	if (time_ack_received == 0)
	{
		time_ack_received = unix_time;
	}
	else
	{
		throw std::exception("Bad access exception");
	}
}

void packet::set_time_response_received(long long unix_time)
{
	if (time_response_received == 0)
	{
		time_response_received = unix_time;
	}
	else
	{
		throw std::exception("Bad access exception");
	}
}

bool packet::is_acknowledged()
{
	return time_ack_received > 0;
}

bool packet::received_response()
{
	return time_response_received > 0;
}

long long packet::get_time_sent()
{
	return time_sent;
}

long long packet::get_time_ack_received()
{
	return time_ack_received;
}

long long packet::get_time_response_received()
{
	return time_response_received;
}

uint8_t packet::get_packet_id()
{
	return packet_id;
}

packet_t packet::get_packet_type()
{
	return packet_type;
}

uint8_t* packet::get_packet_data()
{
	return packet_data;
}

std::string packet::get_packet_data_as_string()
{
	return std::string((char*)packet_data);
}

uint8_t* packet::convert_to_bytes()
{
	size_t i = 0;
	for (; i < 1024; i++)
	{
		if (packet_data[i] == NULL)
		{
			break;
		}
	}

	if (i == 1024)
	{
		throw std::length_error("Expected to be less than 1024");
	}

	uint8_t* out_buffer = new uint8_t[i + 3];
	out_buffer[0] = i + 3;
	out_buffer[1] = packet_id;
	out_buffer[2] = (uint8_t)packet_type;
	memcpy(out_buffer + 3, packet_data, i + 1);

	return out_buffer;
}

bool packet::detect_timeout()
{
	if (time_response_received != 0)
	{
		return false;
	}

	long long current_time = ms_since_epoch();
	bool timeout = (current_time - last_active_time()) > 5000;
	if (timeout)
	{
		packet_state = state_t::PACKET_REACHED_A_TIMEOUT;
	}

	return timeout;
}

bool packet::safe_to_override()
{
	if (detect_timeout())
	{
		return true;
	}

	long long time = ms_since_epoch();
	return ((time - time_response_received) > 2000);
}

long long packet::last_active_time()
{
	if (time_response_received != 0)
	{
		return time_response_received;
	}
	if (time_ack_received != 0)
	{
		return time_ack_received;
	}

	return time_sent;
}

std::string packet::to_str()
{
	std::stringstream ss;
	ss << "{\"packet_id\":\"" << (unsigned int)packet_id << "\",\"packet_type\":\"" << packet_type_to_str(packet_type) << "\",\"packet_state\":\""
		<< packet_state_to_str(packet_state) << "\",\"packet_data\":\""
		<< (packet_data == nullptr ? "" : (char*)packet_data) << "\",\"response_packet\":"
		<< (response_packet == nullptr ? "\"\"" : response_packet->to_str()) << "}";

	return ss.str();
}

std::string packet_state_to_str(state_t packet_type)
{
	switch (packet_type)
	{
		case state_t::AWAITING_ACKNOWLEDGEMENT:
		{
			return "AWAITING_ACKNOWLEDGEMENT";
		}

		case state_t::AWAITING_RESPONSE_PACKET:
		{
			return "AWAITING_RESPONSE_PACKET";
		}

		case state_t::PACKET_REACHED_A_TIMEOUT:
		{
			return "PACKET_REACHED_A_TIMEOUT";
		}

		case state_t::RESPONSE_PACKET_RECEIVED:
		{
			return "RESPONSE_PACKET_RECEIVED";
		}

		default:
		{
			return "<Unknown packet state>";
		}
	}
}

std::string packet_type_to_str(packet_t packet_type)
{
	switch (packet_type)
	{
		case packet_t::ACKNOWLEDGEMENT:
		{
			return "ACKNOWLEDGEMENT";
		}

		case packet_t::RESPONSE_PACKET:
		{
			return "RESPONSE_PACKET";
		}

		case packet_t::REQUEST_POSITION:
		{
			return "REQUEST_POSITION";
		}

		default:
		{
			return "<Unknown packet type>";
		}
	}
}

uint8_t packet_type_to_byte(packet_t packet_type)
{
	return (uint8_t)packet_type;
}
