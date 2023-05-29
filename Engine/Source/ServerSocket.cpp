#include "ServerSocket.hpp"

ServerSocketIPV4::ServerSocketIPV4(int port) : ServerSocket::ServerSocket(port)
{
	int iResult = 0;
	ZeroMemory(&socket_config, sizeof(socket_config));
	socket_config.ai_family = AF_INET;
	socket_config.ai_socktype = SOCK_STREAM;
	socket_config.ai_protocol = IPPROTO_TCP;
	socket_config.ai_flags = AI_PASSIVE;

	iResult = getaddrinfo(NULL, std::to_string(port).c_str(), &socket_config, &socket_settings);
	if (iResult != 0)
	{
		std::cout << "ERROR: Falha ao gerar endereço de rede! (" << iResult << ")" << std::endl;
		WSACleanup();
		return;
	}

	server_socket = socket(socket_settings->ai_family, socket_settings->ai_socktype, socket_settings->ai_protocol);
	if (server_socket == INVALID_SOCKET)
	{
		std::cout << "ERROR: Falha ao criar socket!" << std::endl << WSAGetLastError << std::endl;
		freeaddrinfo(socket_settings);
		WSACleanup();
		return;
	}

	iResult = bind(server_socket, socket_settings->ai_addr, (int)socket_settings->ai_addrlen);
	if (iResult == SOCKET_ERROR)
	{
		std::cout << "ERROR: Falha no bind do socket!" << std::endl << WSAGetLastError << std::endl;
		freeaddrinfo(socket_settings);
		closesocket(server_socket);
		WSACleanup();
		return;
	}
	freeaddrinfo(socket_settings); // depois do bind, libera result pois não é mais necessário

	if (listen(server_socket, SOMAXCONN) == SOCKET_ERROR)
	{
		std::cout << "ERROR: Falhar ao escutar o socket!" << std::endl << WSAGetLastError << std::endl;
		closesocket(server_socket);
		WSACleanup();
		return;
	}
	b_listening = true;
}

void ServerSocketIPV4::Connect()
{
	//std::thread connecting_thread(AssincConnect);
	client_socket = accept(server_socket, NULL, NULL);
	if (client_socket == INVALID_SOCKET) {
		std::cout << "ERROR: Falhar ao se connectar!" << std::endl << WSAGetLastError << std::endl;
		closesocket(server_socket);
		WSACleanup();
		return;
	}
	closesocket(server_socket);
	b_connected = true;
}

void ServerSocket::AssincConnect()
{
	client_socket = accept(server_socket, NULL, NULL);
	if (client_socket == INVALID_SOCKET) {
		std::cout << "ERROR: Falhar ao se connectar!" << std::endl << WSAGetLastError << std::endl;
		closesocket(server_socket);
		WSACleanup();
		return;
	}
	closesocket(server_socket);
	b_connected = true;
}

void ServerSocket::Send(char* buffer, unsigned int buffer_size)
{
	int iResult = send(client_socket, buffer, buffer_size, 0);
	b_sending = false;
	if (iResult == SOCKET_ERROR) {
		std::cout << "Erro de conexão: " << WSAGetLastError() << std::endl;
		closesocket(client_socket);
		WSACleanup();
		return;
	}
}

void ServerSocket::Recieve(char* buffer, unsigned int buffer_size)
{
	int iResult = recv(client_socket, buffer, buffer_size, MSG_WAITALL);
	b_recieving = false;
	if (iResult == 0)
	{
		b_connected = false;
	}
	else if (iResult < 0)
	{
		b_connected = false;
		std::cout << "Erro de conexão: " << WSAGetLastError() << std::endl;
	}
}
