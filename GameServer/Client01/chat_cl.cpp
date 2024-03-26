#include <iostream>
#include <WS2tcpip.h>
#pragma comment (lib, "WS2_32.LIB")

constexpr short PORT = 4000;
constexpr char SERVER_ADDR[] = "127.0.0.1";
constexpr int BUFSIZE = 256;

bool bshutdown = false;
SOCKET server_s;
WSABUF wsabuf[1];
char buf[BUFSIZE];
WSAOVERLAPPED wsaover;
void CALLBACK send_callback(DWORD, DWORD, LPWSAOVERLAPPED, DWORD);

void read_n_send()
{
	std::cout << "Enter Message : ";
	std::cin.getline(buf, BUFSIZE);
	wsabuf[0].buf = buf;
	wsabuf[0].len = static_cast<int>(strlen(buf)) + 1;
	if (wsabuf[0].len == 1) {
		bshutdown = true;
		return;
	}
	ZeroMemory(&wsaover, sizeof(wsaover));
	WSASend(server_s, wsabuf, 1, nullptr, 0, &wsaover, send_callback);
}

void CALLBACK recv_callback(DWORD err, DWORD recv_size,
	LPWSAOVERLAPPED pwsaover, DWORD sendflag)
{
	int p_size = 0;
	while (recv_size > p_size) {
		int m_size = buf[0 + p_size];
		std::cout << "Player [" << static_cast<int>(buf[1 + p_size]) << "] : ";
		for (DWORD i = 0; i < m_size; ++i)
			std::cout << buf[i + p_size + 2];
		std::cout << std::endl;
		p_size = p_size + m_size;
	}
	read_n_send();
}

void CALLBACK send_callback(DWORD err, DWORD sent_size, 
	LPWSAOVERLAPPED pwsaover, DWORD sendflag)
{
	wsabuf[0].len = BUFSIZE;
	DWORD recv_flag = 0;
	ZeroMemory(pwsaover, sizeof(*pwsaover));
	WSARecv(server_s, wsabuf, 1, nullptr, &recv_flag, pwsaover, recv_callback);
}

int main()
{
	std::wcout.imbue(std::locale("korean"));
	WSADATA WSAData;
	WSAStartup(MAKEWORD(2, 0), &WSAData);
	server_s = WSASocket(AF_INET, SOCK_STREAM, 
		IPPROTO_TCP, nullptr, 0, WSA_FLAG_OVERLAPPED);
	SOCKADDR_IN server_a;
	server_a.sin_family = AF_INET;
	server_a.sin_port = htons(PORT);
	inet_pton(AF_INET, SERVER_ADDR, &server_a.sin_addr);
	connect(server_s, reinterpret_cast<sockaddr *>(&server_a), sizeof(server_a));
	read_n_send();
	while (false == bshutdown) {
		SleepEx(0, TRUE);
	}
	closesocket(server_s);
	WSACleanup();
}