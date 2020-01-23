# include <iostream>
#include <WS2tcpip.h>
#include <string>
#include <ctime>
#pragma comment(lib, "ws2_32.lib")

using namespace std;


string cleanup(string input)
{
	input.erase(input.size() - 4, 6);
	input += "\r\n";
	return input;
}


SOCKET GetSocket(string ipAddress, int port)
{
	WSAData data;
	WORD ver = MAKEWORD(2, 2);
	int wsResult = WSAStartup(ver, &data);

	if (wsResult != 0)
	{
		cerr << "Can't start Winsock, Err #" << wsResult << endl;
		return NULL;
	}

	SOCKET sock = socket(AF_INET, SOCK_STREAM, 0);
	if (sock == INVALID_SOCKET)
	{
		cerr << "Can't create socket, Err #" << WSAGetLastError() << endl;
		WSACleanup();
		return NULL;
	}

	sockaddr_in hint;
	hint.sin_family = AF_INET;
	hint.sin_port = htons(port);
	inet_pton(AF_INET, ipAddress.c_str(), &hint.sin_addr);

	int connResult = connect(sock, (sockaddr*)& hint, sizeof(hint));
	if (connResult == SOCKET_ERROR)
	{
		cerr << "Can't connect to server, Err #" << WSAGetLastError() << endl;
		closesocket(sock);
		WSACleanup();
		return NULL;
	}

	return sock;
}

void main()
{
	string ipAddress = "127.0.0.1";
	int port = 21000;

	SOCKET sock = GetSocket(ipAddress, port);

	char buf[4096];
	string input;
	string output;
	string connectedIP;


	cout << "Sending message GET Brown,David\\r\\n" << endl;

	string message = "GET Brown,David\r\n";

	int sendResult = send(sock, message.c_str(), message.size() + 1, 0);

	if (sendResult != SOCKET_ERROR)
	{
		ZeroMemory(buf, 4096);
		int bytesRecieved = recv(sock, buf, 4096, 0);
		if (bytesRecieved > 0)
		{
			output = string(buf, 0, bytesRecieved);
			cout << "SERVER: " << output << endl;
			connectedIP = output.substr(output.size() - 11, 9);

		}
	}

	closesocket(sock);
	WSACleanup();

	ipAddress = connectedIP;
	port = 21001;

	sock = GetSocket(ipAddress, port);

	for (int i = 0; i < 3; i++)
	{
		srand(time(0) + i);
		int random = rand() % 20;
		cout << "Sending message GET " << random << "\\r\\n" << endl;

		message = "GET " + to_string(random) + "\r\n";


		sendResult = send(sock, message.c_str(), message.size() + 1, 0);

		if (sendResult != SOCKET_ERROR)
		{
			ZeroMemory(buf, 4096);
			int bytesRecieved = recv(sock, buf, 4096, 0);
			if (bytesRecieved > 0)
			{
				output = string(buf, 0, bytesRecieved);
				cout << "SERVER: " << output << endl;

			}
		}
	}

	closesocket(sock);
	WSACleanup();

	system("pause");
}