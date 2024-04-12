#include <iostream>
#include <WS2tcpip.h>
#include <conio.h>
#pragma comment (lib, "WS2_32.LIB")

constexpr short PORT = 4000;
constexpr char SERVER_ADDR[] = "127.0.0.1";
constexpr int BUFSIZE = 256;

const int BOARDSIZE = 8;

int board[BOARDSIZE][BOARDSIZE];
int x = 0;
int y = 0;
int Pawn[10][2]= { -1, };		// id, ��ǥ
bool isf = true;

bool bshutdown = false;
SOCKET server_s;
WSABUF wsabuf[1];
char buf[BUFSIZE];
WSAOVERLAPPED wsaover;
void CALLBACK send_callback(DWORD, DWORD, LPWSAOVERLAPPED, DWORD);

void initializeBoard();
void printBoard();
void updateKey(char key);

// ü���� �ʱ�ȭ�ϴ� �Լ�
void initializeBoard()
{
	for (int i = 0; i < BOARDSIZE; ++i) {
		for (int j = 0; j < BOARDSIZE; ++j) {
			// �������� ��� ĭ �����ư��� �ʱ�ȭ
			if ((i + j) % 2 == 0) {
				board[i][j] = -3; // ��� ĭ
			}
			else {
				board[i][j] = -1; // ������ ĭ
			}
		}
	}
	// Pawn �ʱ� ��ġ
	//board[y][x] = 0;
}



// ü���� ����ϴ� �Լ�
void printBoard()
{
	system("cls"); // �ܼ� Ŭ����

	for (int i = 0; i < BOARDSIZE; ++i) {
		for (int j = 0; j < BOARDSIZE; ++j) {

			if(board[i][j] == -1)
				std::cout << " �� ";
			else if(board[i][j] == -3)
				std::cout << " �� ";
			else 
				std::cout << " "<<board[i][j]<<" ";

		}
		std::cout << std::endl;
		std::cout << std::endl;
	}
}

void updateKey(char key) 
{

	// ���� ��ġ ����
	//board[y][x] = ((y + x) % 2 == 0) ? -1 : 1;

	switch (key) {
		// WASD�� �̵��ϴ� ���
	case 'w': if (y > 0) y--; break;
	case 's': if (y < BOARDSIZE - 1) y++; break;
	case 'a': if (x > 0) x--; break;
	case 'd': if (x < BOARDSIZE - 1) x++; break;

		// Ŀ��Ű�� �̵��ϴ� ���
	case 72: if (y > 0) y--; break; // UP
	case 80: if (y < BOARDSIZE - 1) y++; break; // DOWN
	case 75: if (x > 0) x--; break; // LEFT
	case 77: if (x < BOARDSIZE - 1) x++; break; // RIGHT
	}

	//board[y][x] = 0; // �� ��ġ�� Pawn ����
	//std::cout << "Pawn ��ġ: " << x << ", " << y << std::endl;

	sprintf_s(buf, "%d %d", x, y);
	std::cout << buf << std::endl;


	

}


void read_n_send()
{
	std::cout << "Enter Message : ";

	char key;
	key = _getch(); // Ű �Է� �ޱ�

	if (key == 'q') { // ���� ����
		strcpy_s(buf, "quit"); // ������ ������ ���� �޽���
		bshutdown = true; // ���� �÷��� ����
	}
	else {
		updateKey(key); // Ű�� ���� ��ġ ������Ʈ
	}

	wsabuf[0].buf = buf;
	wsabuf[0].len = static_cast<int>(strlen(buf)) + 1;
	if (wsabuf[0].len == 1) {
		bshutdown = true;
		return;
	}
	ZeroMemory(&wsaover, sizeof(wsaover));
	WSASend(server_s, wsabuf, 1, nullptr, 0, &wsaover, send_callback);			// ������Ʈ�� Pawn ��ġ ����

}

void CALLBACK recv_callback(DWORD err, DWORD recv_size,
	LPWSAOVERLAPPED pwsaover, DWORD sendflag)
{
	int p_size = 0;
	
	while (recv_size > p_size) {
		int m_size = buf[0 + p_size];
		int cl_id = static_cast<int>(buf[1 + p_size]);
		std::cout << "Player [" << cl_id  << "] : ";
		for (DWORD i = 0; i < m_size; ++i)
			std::cout << cl_id <<" " << buf[i + p_size + 2];
		std::cout << std::endl;

		
		// Ŭ�� ���� ��ġ ������Ʈ
		board[Pawn[cl_id][1]][Pawn[cl_id][0]] = ((Pawn[cl_id][0] + Pawn[cl_id][1]) % 2 == 0) ? -3 : -1;		// ���� Pawn ��ġ �ʱ�ȭ
		sscanf_s(buf + p_size + 2, "%d %d", &Pawn[cl_id][0], &Pawn[cl_id][1]);	// id�� ���� ��ǥ ����

		if (isf) {
			x = Pawn[cl_id][0];
			y = Pawn[cl_id][1];
			isf = false;
		}

		board[Pawn[cl_id][1]][Pawn[cl_id][0]] = cl_id;		// Pawn ��ġ ������Ʈ

		p_size = p_size + m_size;

	}
		printBoard();

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
	initializeBoard();

	std::wcout.imbue(std::locale("korean"));
	WSADATA WSAData;
	WSAStartup(MAKEWORD(2, 0), &WSAData);
	server_s = WSASocket(AF_INET, SOCK_STREAM,
		IPPROTO_TCP, nullptr, 0, WSA_FLAG_OVERLAPPED);
	SOCKADDR_IN server_a;
	server_a.sin_family = AF_INET;
	server_a.sin_port = htons(PORT);
	inet_pton(AF_INET, SERVER_ADDR, &server_a.sin_addr);
	connect(server_s, reinterpret_cast<sockaddr*>(&server_a), sizeof(server_a));
	read_n_send();
	while (false == bshutdown) {
		SleepEx(0, TRUE);
	}
	closesocket(server_s);
	WSACleanup();
}