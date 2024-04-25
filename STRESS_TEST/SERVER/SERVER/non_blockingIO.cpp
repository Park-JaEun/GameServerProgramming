#include <iostream>
#include <array>
#include <WS2tcpip.h>
#include <vector>
#include <mutex>
#include "protocol.h"

#pragma comment(lib, "WS2_32.lib")
using namespace std;

void disconnect(int c_id);
void process_packet(int c_id, char* packet);

enum S_STATE { ST_FREE, ST_ALLOC, ST_INGAME };
class SESSION {
public:
	mutex _s_lock;
	S_STATE _state;
	int _id;
	SOCKET _socket;
	short	x, y;
	char	_name[NAME_SIZE];
	int		_prev_remain;
	int		_last_move_time;
	char _recv_buf[BUF_SIZE];
public:
	SESSION()
	{
		_id = -1;
		_socket = 0;
		x = y = 0;
		_name[0] = 0;
		_state = ST_FREE;
		_prev_remain = 0;
	}

	~SESSION() {}

	void do_recv()
	{
		DWORD recv_flag = 0;
		int ret = recv(_socket, _recv_buf, BUF_SIZE, recv_flag);
		if (ret == SOCKET_ERROR) {
			int err = WSAGetLastError();
			if (err != WSAEWOULDBLOCK) {
				if (err == WSAECONNRESET || err == WSAECONNABORTED) {
					cout << "Client disconnected: " << _id << endl;
				}
				else {
					cout << "Recv Error: " << err << endl;
				}
				disconnect(_id);
			}
			return;
		}

		int remain_data = ret + _prev_remain;
		char* p = _recv_buf;
		while (remain_data > 0) {
			int packet_size = p[0];
			if (packet_size <= remain_data) {
				process_packet(_id, p);
				p = p + packet_size;
				remain_data = remain_data - packet_size;
			}
			else break;
		}
		_prev_remain = remain_data;
		if (remain_data > 0) {
			memcpy(_recv_buf, p, remain_data);
		}
	}

	void do_send(void* packet)
	{
		char* p = reinterpret_cast<char*>(packet);
		int packet_size = p[0];
		int ret = send(_socket, p, packet_size, 0);
		if (ret == SOCKET_ERROR) {
			int err = WSAGetLastError();
			if (err == WSAECONNRESET || err == WSAECONNABORTED) {
				cout << "Client disconnected: " << _id << endl;
			}
			else {
				cout << "Send Error: " << err << endl;
			}
			disconnect(_id);
		}
	}

	void send_login_info_packet()
	{
		SC_LOGIN_INFO_PACKET p;
		p.id = _id;
		p.size = sizeof(SC_LOGIN_INFO_PACKET);
		p.type = SC_LOGIN_INFO;
		p.x = x;
		p.y = y;
		do_send(&p);
	}
	void send_move_packet(int c_id);
	void send_add_player_packet(int c_id);
	void send_remove_player_packet(int c_id)
	{
		SC_REMOVE_PLAYER_PACKET p;
		p.id = c_id;
		p.size = sizeof(p);
		p.type = SC_REMOVE_PLAYER;
		do_send(&p);
	}
};

array<SESSION, MAX_USER> clients;
SOCKET g_s_socket;

void SESSION::send_move_packet(int c_id)
{
	SC_MOVE_PLAYER_PACKET p;
	p.id = c_id;
	p.size = sizeof(SC_MOVE_PLAYER_PACKET);
	p.type = SC_MOVE_PLAYER;
	p.x = clients[c_id].x;
	p.y = clients[c_id].y;
	p.move_time = clients[c_id]._last_move_time;
	do_send(&p);
}

void SESSION::send_add_player_packet(int c_id)
{
	SC_ADD_PLAYER_PACKET add_packet;
	add_packet.id = c_id;
	strcpy_s(add_packet.name, clients[c_id]._name);
	add_packet.size = sizeof(add_packet);
	add_packet.type = SC_ADD_PLAYER;
	add_packet.x = clients[c_id].x;
	add_packet.y = clients[c_id].y;
	do_send(&add_packet);
}

int get_new_client_id()
{
	for (int i = 0; i < MAX_USER; ++i) {
		lock_guard <mutex> ll{ clients[i]._s_lock };
		if (clients[i]._state == ST_FREE)
			return i;
	}
	return -1;
}

void process_packet(int c_id, char* packet)
{
	switch (packet[1]) {
	case CS_LOGIN: {
		CS_LOGIN_PACKET* p = reinterpret_cast<CS_LOGIN_PACKET*>(packet);
		strcpy_s(clients[c_id]._name, p->name);
		clients[c_id].send_login_info_packet();
		{
			lock_guard<mutex> ll{ clients[c_id]._s_lock };
			clients[c_id]._state = ST_INGAME;
		}
		for (auto& pl : clients) {
			lock_guard<mutex> ll(pl._s_lock);
			if (ST_INGAME != pl._state) continue;
			if (pl._id == c_id) continue;
			pl.send_add_player_packet(c_id);
			clients[c_id].send_add_player_packet(pl._id);
		}
		break;
	}
	case CS_MOVE: {
		CS_MOVE_PACKET* p = reinterpret_cast<CS_MOVE_PACKET*>(packet);
		clients[c_id]._last_move_time = p->move_time;
		short x = clients[c_id].x;
		short y = clients[c_id].y;
		switch (p->direction) {
		case 0: if (y > 0) y--; break;
		case 1: if (y < W_HEIGHT - 1) y++; break;
		case 2: if (x > 0) x--; break;
		case 3: if (x < W_WIDTH - 1) x++; break;
		}
		clients[c_id].x = x;
		clients[c_id].y = y;

		for (auto& cl : clients) {
			lock_guard<mutex> ll(cl._s_lock);
			if (cl._state != ST_INGAME) continue;
			cl.send_move_packet(c_id);
		}
	}
	}
}

void disconnect(int c_id)
{
	{
		lock_guard<mutex> ll(clients[c_id]._s_lock);
		if (clients[c_id]._state == ST_FREE) return;
		closesocket(clients[c_id]._socket);
		clients[c_id]._state = ST_FREE;
	}

	for (auto& pl : clients) {
		lock_guard<mutex> ll(pl._s_lock);
		if (ST_INGAME != pl._state) continue;
		if (pl._id == c_id) continue;
		pl.send_remove_player_packet(c_id);
	}
}

int main()
{
	WSADATA WSAData;
	WSAStartup(MAKEWORD(2, 2), &WSAData);
	g_s_socket = WSASocket(AF_INET, SOCK_STREAM, 0, NULL, 0, WSA_FLAG_OVERLAPPED);
	SOCKADDR_IN server_addr;
	memset(&server_addr, 0, sizeof(server_addr));
	server_addr.sin_family = AF_INET;
	server_addr.sin_port = htons(PORT_NUM);
	server_addr.sin_addr.S_un.S_addr = INADDR_ANY;
	bind(g_s_socket, reinterpret_cast<sockaddr*>(&server_addr), sizeof(server_addr));
	listen(g_s_socket, SOMAXCONN);

	u_long byte = 1;
	for (auto& cl : clients)
		ioctlsocket(cl._socket, FIONBIO, &byte);

	while (true) {
		SOCKADDR_IN cl_addr;
		int addr_size = sizeof(cl_addr);
		SOCKET c_socket = accept(g_s_socket, reinterpret_cast<sockaddr*>(&cl_addr), &addr_size);
		if (c_socket != INVALID_SOCKET) {
			int client_id = get_new_client_id();
			if (client_id != -1) {
				lock_guard<mutex> ll(clients[client_id]._s_lock);
				if (clients[client_id]._state == ST_FREE) {
					clients[client_id]._state = ST_ALLOC;
					clients[client_id].x = 0;
					clients[client_id].y = 0;
					clients[client_id]._id = client_id;
					clients[client_id]._name[0] = 0;
					clients[client_id]._prev_remain = 0;
					clients[client_id]._socket = c_socket;
					clients[client_id].x = rand() % W_WIDTH;
					clients[client_id].y = rand() % W_HEIGHT;
					cout << "Client " << client_id << " Connected" << endl;
				}
				else {
					cout << "Max user exceeded." << endl;
					closesocket(c_socket);
				}
			}
			else {
				cout << "Max user exceeded." << endl;
				closesocket(c_socket);
			}
		}

		for (auto& cl : clients) {
			if (cl._state != ST_FREE) {
				DWORD recv_flag = 0;
				int ret = recv(cl._socket, cl._recv_buf, BUF_SIZE, recv_flag);
				if (ret == SOCKET_ERROR) {
					int err = WSAGetLastError();
					if (err == WSAEWOULDBLOCK)
						continue;
					if (err == WSAECONNRESET || err == WSAECONNABORTED) {
						cout << "Client disconnected: " << cl._id << endl;
					}
					else {
						cout << "Recv Error: " << err << endl;
					}
					disconnect(cl._id);
				}
				else if (ret == 0) {
					disconnect(cl._id);
				}
				else {
					int remain_data = ret + cl._prev_remain;
					char* p = cl._recv_buf;
					while (remain_data > 0) {
						int packet_size = p[0];
						if (packet_size <= remain_data) {
							process_packet(cl._id, p);
							p = p + packet_size;
							remain_data = remain_data - packet_size;
						}
						else break;
					}
					cl._prev_remain = remain_data;
					if (remain_data > 0) {
						memcpy(cl._recv_buf, p, remain_data);
					}
				}
			}
		}
	}

	closesocket(g_s_socket);
	WSACleanup();
}