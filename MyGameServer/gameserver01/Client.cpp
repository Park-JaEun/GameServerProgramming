#define _CRT_SECURE_NO_WARNINGS
#include <iostream>
#include <conio.h> 
#include <WS2tcpip.h>
#include <string>

#pragma comment (lib, "ws2_32.lib")

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

int main(int argc, char* argv[]) {

    string ipAddress;
    cout << "IP �ּ� �Է� : ";
    cin >> ipAddress; // IP �ּ� �Է�

    int port = 54000; // ���� ��Ʈ

    WSADATA wsData;
    WSAStartup(MAKEWORD(2, 2), &wsData);

    SOCKET sock = socket(AF_INET, SOCK_STREAM, 0);
    if (sock == INVALID_SOCKET) {
        cerr << "Can't create socket, Err #" << WSAGetLastError() << endl;
        WSACleanup();
        return 1;
    }

    sockaddr_in hint;
    hint.sin_family = AF_INET;
    hint.sin_port = htons(port);
    inet_pton(AF_INET, ipAddress.c_str(), &hint.sin_addr);

    int connResult = connect(sock, (sockaddr*)&hint, sizeof(hint));
    if (connResult == SOCKET_ERROR) {
        cerr << "Can't connect to server, Err #" << WSAGetLastError() << endl;
        closesocket(sock);
        WSACleanup();
        return 1;
    }

    char buf[4096];
    char key;

    initializeBoard();
    printBoard();

    do {
        // Ű �Է� �ޱ�
        cout << "> ";
        char key = _getch(); // Ű �Է�

        if (sizeof(key) > 0) {
            // ������ Ű �Է� ������
            int sendResult = send(sock, &key, sizeof(key), 0);  

            // �����κ��� ���� �ޱ�
            if (sendResult != SOCKET_ERROR) {
                ZeroMemory(buf, 4096);
                int bytesReceived = recv(sock, buf, 4096, 0);
                if (bytesReceived > 0) {
                    // �����κ��� ���� ���ο� ��ġ ������ �� �̵�
                    string newPosition(buf, 0, bytesReceived);
                    movePiece(x, y, newPosition);
                    printBoard();
                    cout << "Pawn Position: (" << x+1 << ", " << y+1 << ")" << endl;

                }
            }
        }

    } while (sizeof(key) > 0);

    closesocket(sock);
    WSACleanup();
}
