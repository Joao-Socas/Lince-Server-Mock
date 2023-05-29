#include "TransactionManager.hpp"

TransactionManager::TransactionManager(ServerSocket* client_socket, BufferController* buffer_controller) : client_socket(client_socket), buffer_controller(buffer_controller)
{
	connecting_thread = std::make_unique<std::thread>(std::thread(&TransactionManager::ManageTransactions, this));
}

TransactionManager::~TransactionManager()
{
	connecting_thread->join();
}

void TransactionManager::ManageTransactions()
{
	while (client_socket->b_connected)
	{
		if (buffer_controller->new_on_wait)
		{
			client_socket->Send((char*)(void*)buffer_controller->GetBufferToSendSize(), 4);
			client_socket->Send(buffer_controller->GetBufferToSend(), buffer_controller->GetBufferToSendSize());
		}
	}
}
