#pragma once

// STD INCLUDES
#include<thread>

// ENGINE INLCUDES
#include "ServerSocket.hpp"
#include "BufferController.hpp"

class TransactionManager
{
public:
	TransactionManager(ServerSocket* client_socket, BufferController* buffer_controller);
	~TransactionManager();

	void ManageTransactions();
private:
	ServerSocket* client_socket;
	BufferController* buffer_controller;
	std::unique_ptr<std::thread> connecting_thread;
};