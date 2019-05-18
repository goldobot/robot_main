#pragma once
#include <cstdint>
#include <cstddef>

bool zmq_init();
void zmq_cleanup();

void zmq_receive_message(void* socket, void (*callback)(uint16_t, void*, size_t));
void zmq_send_message(void* socket, uint16_t type, char* data, size_t data_size);

void zmq_process_message(uint16_t type, void* data, size_t data_size);
void zmq_process_debug_message(uint16_t type, void* data, size_t data_size);

void zmq_check_messages();