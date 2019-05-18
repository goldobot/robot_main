#include <zmq.h>
#include "messages.hpp"

#include <string.h>

#include <iostream>


void* zmq_context = nullptr;
void* zmq_comm_pub_socket = nullptr;
void* zmq_comm_sub_socket = nullptr;

void* zmq_debug_pub_socket = nullptr;
void* zmq_debug_sub_socket = nullptr;

zmq_pollitem_t zmq_poll_items[2];

bool zmq_init()
{
	bool ok = true;
	
	zmq_context = zmq_init(1);
	
    zmq_comm_sub_socket = zmq_socket(zmq_context, ZMQ_SUB);
    ok = ok && (zmq_connect(zmq_comm_sub_socket, "tcp://localhost:3001") == 0);
    zmq_setsockopt(zmq_comm_sub_socket,ZMQ_SUBSCRIBE, "", 0); 
	
	zmq_comm_pub_socket = zmq_socket(zmq_context, ZMQ_PUB);
    ok = ok && (zmq_connect(zmq_comm_pub_socket, "tcp://localhost:3002") == 0);
	
	zmq_debug_sub_socket = zmq_socket(zmq_context, ZMQ_SUB);
    ok = ok && (zmq_connect(zmq_debug_sub_socket, "tcp://localhost:3003") == 0);
    zmq_setsockopt(zmq_debug_sub_socket,ZMQ_SUBSCRIBE, "", 0);
	
	zmq_debug_pub_socket = zmq_socket(zmq_context, ZMQ_PUB);
    ok = ok && (zmq_connect(zmq_debug_pub_socket, "tcp://localhost:3004") == 0);
	
	std::cout << ok << std::endl;
	
	zmq_poll_items[0].socket = zmq_comm_sub_socket;
    zmq_poll_items[0].fd = 0;
    zmq_poll_items[0].events = ZMQ_POLLIN;

    zmq_poll_items[1].socket = zmq_debug_sub_socket;
    zmq_poll_items[1].fd = 0;
	zmq_poll_items[1].events = ZMQ_POLLIN;
	
	return ok;
}

void zmq_cleanup()
{
	if(zmq_comm_pub_socket != nullptr)
	{
		zmq_close (zmq_comm_pub_socket);
		zmq_comm_pub_socket = nullptr;
	}
	
	zmq_close (zmq_comm_sub_socket);
	zmq_close (zmq_debug_pub_socket);
	zmq_close (zmq_debug_sub_socket);
    zmq_ctx_destroy (zmq_context);
};

void zmq_receive_message(void* socket, void (*callback)(uint16_t, void*, size_t))
{
	int64_t more;
    size_t more_size = sizeof(more);
	// Init message
    zmq_msg_t msg;
    auto rc = zmq_msg_init (&msg);
	
	// Receive first part of message and decode message id
    rc = zmq_recvmsg(socket, &msg, 0);
    uint16_t message_id = *(uint16_t*)zmq_msg_data (&msg);
    rc = zmq_getsockopt (socket, ZMQ_RCVMORE, &more, &more_size);
	zmq_msg_close (&msg);	
	
	if(more)
	{
		zmq_msg_init (&msg);
		zmq_recvmsg(socket, &msg, 0);
		callback(message_id, zmq_msg_data(&msg), zmq_msg_size(&msg));
		zmq_msg_close (&msg);
	} else 
	{
		callback(message_id, nullptr, 0);
	};
}

void zmq_send_message(void* socket, uint16_t type, char* data, size_t data_size)
{	
	// Init message
    zmq_msg_t msg;
    auto rc = zmq_msg_init_size(&msg, 2);
	memcpy(zmq_msg_data(&msg),&type, 2);
	zmq_msg_send(&msg, socket, data != nullptr ? ZMQ_SNDMORE : 0);	
}

void zmq_check_messages()
{
	zmq_poll (zmq_poll_items, 2, -1);

	if(zmq_poll_items[0].revents && ZMQ_POLLIN)
	{
		zmq_receive_message(zmq_comm_sub_socket, &zmq_process_message);
	};
	if(zmq_poll_items[1].revents && ZMQ_POLLIN)
	{
		zmq_receive_message(zmq_debug_sub_socket, &zmq_process_debug_message);
	};	
};

void zmq_process_debug_message(uint16_t type, void* data, size_t data_size)
{
	
};
