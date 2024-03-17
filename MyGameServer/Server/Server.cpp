// server.cpp
#include <iostream>
#include <string>
#include <WS2tcpip.h>		// �� ���̺귯���� ǥ�� �Լ���� �浹�ϴϱ� useing namespace std; �� ������� �ʴ´�.
#pragma comment(lib, "WS2_32.LIB")	// �� ���̺귯���� ����ϰڴٰ� �����Ѵ�.

constexpr short PORT = 4000;
constexpr int BUFSIZE = 256;


const int BOARDSIZE = 8;
int board[BOARDSIZE][BOARDSIZE]; // ü���� ������
int x = 0, y = 0; // ó�� Pawn ��ġ

void initializeBoard() {
    for (int i = 0; i < BOARDSIZE; ++i) {
        for (int j = 0; j < BOARDSIZE; ++j) {
            if ((i + j) % 2 == 0) {
                board[i][j] = -1; // ��� ĭ
            }
            else {
                board[i][j] = 1; // ������ ĭ
            }
        }
    }
    board[y][x] = 0; // Pawn �ʱ� ��ġ
}

void updateBoard(char key) {
    // ���� ��ġ ����
    board[y][x] = ((y + x) % 2 == 0) ? -1 : 1;

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

    board[y][x] = 0; // �� ��ġ�� Pawn ����
    std::cout<< "Pawn ��ġ: " << x+1 << ", " << y+1 << std::endl;
}


void print_error(const char* msg, int err_no)
{
    WCHAR* msg_buf;
    FormatMessage(FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM,
        NULL, err_no,
        MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
        reinterpret_cast<LPWSTR>(&msg_buf), 0, NULL);
    std::cout << msg;
    std::wcout << L" : ���� : " << msg_buf;
    LocalFree(msg_buf);
    while (true);   // ����ϰ� ����� �����ϵ��� �ϴ� �ڵ�, ���� ���α׷������� ���� ��
    return;
}

int main() {
    std::wcout.imbue(std::locale("korean"));		// ���� �޼��� �ѱ۷�

    WSADATA WSAData;
    WSAStartup(MAKEWORD(2, 0), &WSAData);		// �����쿡���� �ʿ��� �ڵ�	

    SOCKET server_s = WSASocket(AF_INET, SOCK_STREAM, IPPROTO_TCP, nullptr, 0, 0);
    SOCKADDR_IN server_a;
    server_a.sin_family = AF_INET;
    server_a.sin_port = htons(PORT);						// �ٸ� ���α׷��� �浹���� �ʴ� ���ڷ� �����ؾ� ��
    server_a.sin_addr.S_un.S_addr = htonl(INADDR_ANY);	// ���������� �̷��� �����ؾ� �Ѵ�.

    bind(server_s, reinterpret_cast<SOCKADDR*>(&server_a), sizeof(server_a));
    listen(server_s, SOMAXCONN);	// SOMAXCONN�� �ִ� ���� ��

   
    int addr_size = sizeof(server_a);
    SOCKET client_s = WSAAccept(server_s, reinterpret_cast<SOCKADDR*>(&server_a), &addr_size, nullptr, 0);

    initializeBoard();      // ü���� �ʱ�ȭ

    while (true) {
        char buf[BUFSIZE];

        WSABUF wsabuf[1];
        wsabuf[0].buf = buf;
        wsabuf[0].len = BUFSIZE;	// �ϴ� ũ��
        DWORD recv_size;
        DWORD recv_flag = 0;
        int res = WSARecv(client_s, wsabuf, 1, &recv_size, &recv_flag, nullptr, nullptr);
        if (0 != res) {
            print_error("WSARecv", WSAGetLastError());
        }
        if (recv_size == 0)
            break;

        updateBoard(buf[0]); // �� ��ġ ������Ʈ

        // Pawn ��ġ ����
        std::string position = std::to_string(x) + " " + std::to_string(y);
        wsabuf[0].buf = const_cast<char*>(position.c_str());
        wsabuf[0].len = position.length() + 1;
        DWORD sent_size;
        //std::cout << wsabuf[0].buf << std::endl;

        WSASend(client_s, wsabuf, 1, &sent_size, 0, nullptr, nullptr);
    }
    closesocket(client_s);
    closesocket(server_s);

    WSACleanup();
}
