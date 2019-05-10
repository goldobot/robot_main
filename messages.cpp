#include <zmq.h>
#include "messages.hpp"


void* zmq_context;
void* zmq_comm_pub_socket;
void* zmq_comm_sub_socket;

void* zmq_debug_pub_socket;

void zmq_init()
{
	zmq_context = zmq_init(1);
	
    zmq_comm_sub_socket = zmq_socket(zmq_context, ZMQ_SUB);
    zmq_connect(zmq_comm_sub_socket, "tcp://localhost:3001");
    zmq_setsockopt(zmq_comm_sub_socket,ZMQ_SUBSCRIBE, "", 0); 
	
	zmq_comm_pub_socket = zmq_socket(zmq_context, ZMQ_PUB);
    zmq_connect(zmq_comm_pub_socket, "tcp://localhost:3002");
	
	zmq_debug_pub_socket = zmq_socket(zmq_context, ZMQ_PUB);
    zmq_connect(zmq_debug_pub_socket, "tcp://localhost:3004");
}

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

void zmq_check_messages()
{
	zmq_receive_message(zmq_comm_sub_socket, &zmq_process_message);
	
};

