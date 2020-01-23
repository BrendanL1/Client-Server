# include <iostream>
#include <WS2tcpip.h>
#include <string>
#include <ctime>
#include <regex>
#pragma comment(lib, "ws2_32.lib")

using namespace std;

class Quote
{
public:
	int number;
	string quote;

	Quote(int num, string qt)
	{
		number = num;
		quote = qt;
	}
	Quote()
	{
		number = 0;
		quote = "";
	}
};

string GetMsg(char* buffer)
{
	string message(buffer);
	int endPosition = message.find("\r\n");
	message = message.substr(0, endPosition);

	return message;
}

int main()
{

	Quote quotes[20];

	while (true)
	{
		int port = 21001;

		WSADATA wsData;
		WORD ver = MAKEWORD(2, 2);
		int wsOK = WSAStartup(ver, &wsData);
		if (wsOK != 0)
		{
			cerr << "Can't initialize winsock!" << endl;
			return 0;
		}

		SOCKET listening = socket(AF_INET, SOCK_STREAM, 0);
		if (listening == INVALID_SOCKET)
		{
			cerr << "Can't create a socket!" << endl;
			return 0;
		}

		sockaddr_in hint;
		hint.sin_family = AF_INET;
		hint.sin_port = htons(port);
		hint.sin_addr.S_un.S_addr = INADDR_ANY;

		bind(listening, (sockaddr*)& hint, sizeof(hint));

		listen(listening, SOMAXCONN);

		sockaddr_in client;
		int clientSize = sizeof(client);

		SOCKET clientSocket = accept(listening, (sockaddr*)& client, &clientSize);

		char host[NI_MAXHOST];
		char service[NI_MAXHOST];

		ZeroMemory(host, NI_MAXHOST);
		ZeroMemory(service, NI_MAXHOST);

		if (getnameinfo((sockaddr*)& client, clientSize, host, NI_MAXHOST, service, NI_MAXSERV, 0) == 0)
		{
			cout << host << " connected on port " << service << endl;
		}
		else
		{
			inet_ntop(AF_INET, &client.sin_addr, host, NI_MAXHOST);
			cout << host << " connected on port " << ntohs(client.sin_port) << endl;
		}

		closesocket(listening);

		char buf[4096];
		char buf2[4096] = "";
		string leftoverBytes = "";

		while (true)
		{
			ZeroMemory(buf, 4096);
			ZeroMemory(buf2, 4096);

			int bytesRecieved = 0;
			int totalBytes = 0;

			do
			{
				bytesRecieved = recv(clientSocket, buf, 4096, 0);
				totalBytes += bytesRecieved;
				strncat_s(buf2, buf, bytesRecieved);

				//If a request can be sent, dont get the rest of the data yet
				if (strstr(buf, "\r\n") != NULL || bytesRecieved == 0)
					break;

			} while ((buf[bytesRecieved - 1] != '\n' && buf[bytesRecieved - 2] != '\r'));

			cout << buf2 << endl;

			if (bytesRecieved == SOCKET_ERROR)
			{
				cerr << "Error in  recv()" << endl;
				break;
			}

			if (bytesRecieved == 0)
			{
				cout << "Client disconnected" << endl;
				break;
			}

			bool repeat = false;

			string message = GetMsg(buf2);

			if (leftoverBytes != "")
			{
				message = leftoverBytes + message;
			}

			leftoverBytes = "";
			int length = 0;
			do
			{
				repeat = false;
				int firstSpace = message.find(" ");
				string identifier = message.substr(0, firstSpace);

				int secondSpace = message.find(" ", firstSpace + 1);

				string numStr = "";
				string quote = "";

				if (identifier == "SET")
				{
					numStr = message.substr(firstSpace + 1, secondSpace - firstSpace - 1);
					quote = message.substr(secondSpace + 1, message.length() - secondSpace - 1);
				}
				else
				{
					numStr = message.substr(firstSpace + 1);
				}


				int num = atoi(numStr.c_str());
				length += message.length();

				if (num < 1 || num > 20)
				{
					string response = "ERR BAD REQUEST\r\n";
					int bytesSent = send(clientSocket, response.c_str(), response.length(), 0);
					cout << "Bytes sent: " << bytesSent << endl;
					continue;
				}


				if (identifier == "SET")
				{
					quotes[num - 1].number = num;
					quotes[num - 1].quote = quote;
					string response = "OK " + numStr + "\r\n";
					int bytesSent = send(clientSocket, response.c_str(), response.length(), 0);
					cout << "Bytes sent: " << bytesSent << endl;
				}
				else if (identifier == "GET")
				{
					string response = "OK " + to_string(num) + " " + quotes[num - 1].quote + "\r\n";
					int bytesSent = send(clientSocket, response.c_str(), response.length(), 0);
					cout << "Bytes sent: " << bytesSent << endl;
				}
				else
				{
					string response = "ERR BAD REQUEST\r\n";
					int bytesSent = send(clientSocket, response.c_str(), response.length(), 0);
					cout << "Bytes sent: " << bytesSent << endl;
				}

				if (totalBytes > length + 2)
				{
					string nextMsg(buf2);
					nextMsg = nextMsg.substr(length + 2, totalBytes - length + 1);

					//If there is an end to the next message, repeat to send correct response
					if (nextMsg.find("\r\n") != -1)
					{
						message = nextMsg;
						repeat = true;
					}
					else
					{
						//If there is no end to the message, keep the data and run recv again.
						leftoverBytes = nextMsg;
					}
				}
			} while (repeat);

		}

		closesocket(clientSocket);

		WSACleanup();
	}

	return 0;
}