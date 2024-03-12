// server.cpp
#include <iostream>
#include <WS2tcpip.h>
#include <string>

#pragma comment (lib, "ws2_32.lib")

using namespace std;

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
    cout<< "Pawn ��ġ: " << x+1 << ", " << y+1 << endl;
}

int main() {
    WSADATA wsData;
    WSAStartup(MAKEWORD(2, 2), &wsData);

    SOCKET listening = socket(AF_INET, SOCK_STREAM, 0);
    if (listening == INVALID_SOCKET) {
        cerr << "Can't create a socket! Quitting" << endl;
        return 1;
    }

    sockaddr_in hint;
    hint.sin_family = AF_INET;
    hint.sin_port = htons(54000); // Port ��ȣ
    hint.sin_addr.S_un.S_addr = INADDR_ANY; // ��� �������̽����� ����

    bind(listening, (sockaddr*)&hint, sizeof(hint));
    listen(listening, SOMAXCONN);

    sockaddr_in client;
    int clientSize = sizeof(client);

    SOCKET clientSocket = accept(listening, (sockaddr*)&client, &clientSize);
    if (clientSocket == INVALID_SOCKET) {
        cerr << "Can't accept client! Quitting" << endl;
        return 1;
    }

    char host[NI_MAXHOST];
    char service[NI_MAXSERV];

    ZeroMemory(host, NI_MAXHOST); // ���� ������ memset(host, 0, NI_MAXHOST);
    ZeroMemory(service, NI_MAXSERV);

    if (getnameinfo((sockaddr*)&client, sizeof(client), host, NI_MAXHOST, service, NI_MAXSERV, 0) == 0) {
        cout << host << " connected on port " << service << endl;
    }
    else {
        inet_ntop(AF_INET, &client.sin_addr, host, NI_MAXHOST);
        cout << host << " connected on port " << ntohs(client.sin_port) << endl;
    }

    closesocket(listening);

    initializeBoard();

    char buf[4096];
    while (true) {
        ZeroMemory(buf, 4096);
        int bytesReceived = recv(clientSocket, buf, 4096, 0);
        if (bytesReceived == SOCKET_ERROR) {
            cerr << "Error in recv(). Quitting" << endl;
            break;
        }
        if (bytesReceived == 0) {
            cout << "Client disconnected " << endl;
            break;
        }

        updateBoard(buf[0]); // �� ��ġ ������Ʈ

        // ����� ���� ��ġ�� Ŭ���̾�Ʈ�� ����
        string position = to_string(x) + " " + to_string(y);
        send(clientSocket, position.c_str(), position.size() + 1, 0);
    }

    closesocket(clientSocket);
    WSACleanup();
}
