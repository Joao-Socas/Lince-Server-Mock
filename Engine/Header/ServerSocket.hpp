#pragma once

// STD INCLUDES
#include<iostream>
#include<string>
#include<thread>

// WINDOWS INCLUDES
#include<WinSock2.h>
#include<WS2tcpip.h>
#include<sys/types.h>

class ServerSocket
{
public:
	void Send(char* buffer, unsigned int buffer_size);
	void Recieve(char* buffer, unsigned int buffer_size);

	bool b_connected = false;

protected:
	ServerSocket(int port) : port_number(port) {};
	
	void AssincConnect();

	addrinfo socket_config = {};
	addrinfo* socket_settings = nullptr;
	SOCKET server_socket = INVALID_SOCKET;
	SOCKET client_socket = INVALID_SOCKET;

	unsigned int port_number;
	bool b_listening = false;
	bool b_sending = false;
	bool b_recieving = false;
};

class ServerSocketIPV4 : public ServerSocket
{
public:
	ServerSocketIPV4(int port);
	void Connect();

};

//class LinceServerSocketIPV6 : public LinceServerSocket
//{
//public:
//	LinceServerSocketIPV6(int port);
//};