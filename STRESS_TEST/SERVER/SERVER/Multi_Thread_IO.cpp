#include <iostream>
#include <array>
#include <WS2tcpip.h>
#include <thread>
#include <vector>
#include <mutex>
#include <unordered_set>
#include "protocol.h"

#pragma comment(lib, "WS2_32.lib")
using namespace std;

void disconnect(int c_id);
void process_packet(int c_id, char* packet);
void client_thread(int c_id);

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
			cout << "Recv Error" << endl;
			disconnect(_id);
			return;
		}
		else if (ret == 0) {
			disconnect(_id);
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
			cout << "Send Error" << endl;
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
	void send_add_object_packet(int c_id);
	void send_remove_object_packet(int c_id)
	{
		SC_REMOVE_PLAYER_PACKET p;
		p.id = c_id;
		p.size = sizeof(p);
		p.type = SC_REMOVE_PLAYER;
		do_send(&p);
	}
};

array<SESSION, MAX_USER> objects;
SOCKET g_s_socket;

void SESSION::send_move_packet(int c_id)
{
	SC_MOVE_PLAYER_PACKET p;
	p.id = c_id;
	p.size = sizeof(SC_MOVE_PLAYER_PACKET);
	p.type = SC_MOVE_PLAYER;
	p.x = objects[c_id].x;
	p.y = objects[c_id].y;
	p.move_time = objects[c_id]._last_move_time;
	do_send(&p);
}

void SESSION::send_add_object_packet(int c_id)
{
	SC_ADD_PLAYER_PACKET add_packet;
	add_packet.id = c_id;
	strcpy_s(add_packet.name, objects[c_id]._name);
	add_packet.size = sizeof(add_packet);
	add_packet.type = SC_ADD_PLAYER;
	add_packet.x = objects[c_id].x;
	add_packet.y = objects[c_id].y;
	do_send(&add_packet);
}

int get_new_client_id()
{
	for (int i = 0; i < MAX_USER; ++i) {
		lock_guard <mutex> ll{ objects[i]._s_lock };
		if (objects[i]._state == ST_FREE)
			return i;
	}
	return -1;
}

void process_packet(int c_id, char* packet)
{
	switch (packet[1]) {
	case CS_LOGIN: {
		CS_LOGIN_PACKET* p = reinterpret_cast<CS_LOGIN_PACKET*>(packet);
		strcpy_s(objects[c_id]._name, p->name);
		objects[c_id].send_login_info_packet();
		{
			lock_guard<mutex> ll{ objects[c_id]._s_lock };
			objects[c_id]._state = ST_INGAME;
		}
		for (auto& pl : objects) {
			lock_guard<mutex> ll(pl._s_lock);
			if (ST_INGAME != pl._state) continue;
			if (pl._id == c_id) continue;
			pl.send_add_object_packet(c_id);
			objects[c_id].send_add_object_packet(pl._id);
		}
		break;
	}
	case CS_MOVE: {
		CS_MOVE_PACKET* p = reinterpret_cast<CS_MOVE_PACKET*>(packet);
		objects[c_id]._last_move_time = p->move_time;
		short x = objects[c_id].x;
		short y = objects[c_id].y;
		switch (p->direction) {
		case 0: if (y > 0) y--; break;
		case 1: if (y < W_HEIGHT - 1) y++; break;
		case 2: if (x > 0) x--; break;
		case 3: if (x < W_WIDTH - 1) x++; break;
		}
		objects[c_id].x = x;
		objects[c_id].y = y;

		for (auto& cl : objects) {
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
		lock_guard<mutex> ll(objects[c_id]._s_lock);
		if (objects[c_id]._state == ST_FREE) return;
		closesocket(objects[c_id]._socket);
		objects[c_id]._state = ST_FREE;
	}

	for (auto& pl : objects) {
		lock_guard<mutex> ll(pl._s_lock);
		if (ST_INGAME != pl._state) continue;
		if (pl._id == c_id) continue;
		pl.send_remove_object_packet(c_id);
	}
}

void client_thread(int c_id)
{
	cout << "Client " << c_id << " Connected" << endl;

	while (true) {
		fd_set read_set;
		FD_ZERO(&read_set);

		{
			lock_guard<mutex> ll(objects[c_id]._s_lock);
			if (objects[c_id]._state == ST_FREE)
				break;
			FD_SET(objects[c_id]._socket, &read_set);
		}

		timeval timeout;
		timeout.tv_sec = 1;
		timeout.tv_usec = 0;

		int ret = select(0, &read_set, 0, 0, &timeout);
		if (ret == SOCKET_ERROR) {
			cout << "Select Error" << endl;
			break;
		}

		if (FD_ISSET(objects[c_id]._socket, &read_set)) {
			objects[c_id].do_recv();
		}
	}

	cout << "Client " << c_id << " Disconnected" << endl;
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

	vector <thread> client_threads;

	while (true) {
		SOCKADDR_IN cl_addr;
		int addr_size = sizeof(cl_addr);
		SOCKET c_socket = accept(g_s_socket, reinterpret_cast<sockaddr*>(&cl_addr), &addr_size);
		if (c_socket == INVALID_SOCKET) {
			cout << "Accept Error" << endl;
			break;
		}

		int client_id = get_new_client_id();
		if (client_id != -1) {
			{
				lock_guard<mutex> ll(objects[client_id]._s_lock);
				objects[client_id]._state = ST_ALLOC;
			}
			objects[client_id].x = 0;
			objects[client_id].y = 0;
			objects[client_id]._id = client_id;
			objects[client_id]._name[0] = 0;
			objects[client_id]._prev_remain = 0;
			objects[client_id]._socket = c_socket;

			client_threads.emplace_back(client_thread, client_id);
			client_threads.back().detach();
		}
		else {
			cout << "Max user exceeded." << endl;
			closesocket(c_socket);
		}
	}

	for (auto& th : client_threads)
		if (th.joinable())
			th.join();

	closesocket(g_s_socket);
	WSACleanup();
}