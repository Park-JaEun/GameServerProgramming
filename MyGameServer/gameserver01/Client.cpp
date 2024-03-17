#define _CRT_SECURE_NO_WARNINGS
#include <iostream>
#include <conio.h> 
#include <string>
#include <WS2tcpip.h>		// �� ���̺귯���� ǥ�� �Լ���� �浹�ϴϱ� useing namespace std; �� ������� �ʴ´�.
#pragma comment(lib, "WS2_32.LIB")	// �� ���̺귯���� ����ϰڴٰ� �����Ѵ�.

constexpr short PORT = 4000;
constexpr int BUFSIZE = 256;
constexpr char SERVER_ADDR[] = "127.0.0.1";


const int BOARDSIZE = 8;
int board[BOARDSIZE][BOARDSIZE]; // ü���� ������
int x = 0, y = 0; // ó�� Pawn ��ġ

using namespace std;

void movePiece(int& x, int& y, const string& newPosition) {

    board[y][x] = ((y + x) % 2 == 0) ? -1 : 1;    // ���� Pawn ��ġ �ʱ�ȭ

    int newX, newY;

    sscanf(newPosition.c_str(), "%d %d", &newX, &newY);	// �� ��ġ �޾ƿ���
    x = newX;
    y = newY;

    board[y][x] = 0;    // �� ��ġ�� Pawn ����
}

// ü���� �ʱ�ȭ�ϴ� �Լ�
void initializeBoard()
{
    for (int i = 0; i < BOARDSIZE; ++i) {
        for (int j = 0; j < BOARDSIZE; ++j) {
            // �������� ��� ĭ �����ư��� �ʱ�ȭ
            if ((i + j) % 2 == 0) {
                board[i][j] = -1; // ��� ĭ
            }
            else {
                board[i][j] = 1; // ������ ĭ
            }
        }
    }

    // Pawn �ʱ� ��ġ
    board[y][x] = 0;

}

// ü���� ����ϴ� �Լ�
void printBoard()
{
    system("cls"); // �ܼ� Ŭ����

    for (int i = 0; i < BOARDSIZE; ++i) {
        for (int j = 0; j < BOARDSIZE; ++j) {
            switch (board[i][j])
            {
            case 1:
                cout << " �� ";
                break;
            case -1:
                cout << " �� ";
                break;
            case 0:
                cout << " �� ";
                break;

            }

        }
        cout << endl;
        cout << endl;
    }
}

int main(int argc, char* argv[]) 
{

    char buf[BUFSIZE];
    char key;

    initializeBoard();
    printBoard();


    //cout << "IP �ּ� �Է� : ";

    
    std::wcout.imbue(std::locale("korean"));    // ���� �޼��� �ѱ۷�

    WSADATA WSAData;
    WSAStartup(MAKEWORD(2, 0), &WSAData);       // �����쿡���� �ʿ��� �ڵ�

    SOCKET server_s = WSASocket(AF_INET, SOCK_STREAM, IPPROTO_TCP, nullptr, 0, 0);
    SOCKADDR_IN server_a;
    server_a.sin_family = AF_INET;
    server_a.sin_port = htons(PORT);        // �ٸ� ���α׷��� �浹���� �ʴ� ���ڷ� �����ؾ� ��
    inet_pton(AF_INET, SERVER_ADDR, &server_a.sin_addr);
    connect(server_s, reinterpret_cast<SOCKADDR*>(&server_a), sizeof(server_a));

    while (true) {

        // Ű �Է� �ޱ�
        std::cout << "key �Է� > ";
        key = _getch(); // Ű �Է� �ޱ�

        // �Է¹��� Ű ���� buf�� ù ��° ��ҿ� ����
        buf[0] = key;

        // ���ڿ��� ���� ��Ÿ���� ���� �� ��° ��ҿ� �� ���� �߰�
        buf[1] = '\0';
        //cout << buf << endl;

        WSABUF wsabuf[1];
        wsabuf[0].buf = buf;
        wsabuf[0].len = static_cast<int>(strlen(buf)) + 1;      // ���ڿ� ���� 0���� �����ؼ� �����Ѵ�.
        if (wsabuf[0].len == 1)
            break;

        DWORD sent_size;
        WSASend(server_s, wsabuf, 1, &sent_size, 0, nullptr, nullptr);

        wsabuf[0].buf = buf;
        wsabuf[0].len = BUFSIZE;	// �������� ��ŭ ���� �� �𸣴ϱ� �ϴ� ũ��
        DWORD recv_size;
        DWORD recv_flag = 0;
        WSARecv(server_s, wsabuf, 1, &recv_size, &recv_flag, nullptr, nullptr);

        cout << buf  << endl;

        // �����κ��� ���� ���ο� ��ġ ������ �� �̵� �ڵ�
        movePiece(x, y, buf);
        printBoard();

    }
    closesocket(server_s);

    WSACleanup();


  
}
