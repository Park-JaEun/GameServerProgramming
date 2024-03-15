#define _CRT_SECURE_NO_WARNINGS
#include <iostream>
#include <conio.h> 
#include <WS2tcpip.h>
#include <string>

#pragma comment (lib, "ws2_32.lib")

const int BOARDSIZE = 8;
int board[BOARDSIZE][BOARDSIZE]; // 체스판 데이터
int x = 0, y = 0; // 처음 Pawn 위치

using namespace std;

void movePiece(int& x, int& y, const string& newPosition) {

    board[y][x] = ((y + x) % 2 == 0) ? -1 : 1;    // 기존 Pawn 위치 초기화

    int newX, newY;

    sscanf(newPosition.c_str(), "%d %d", &newX, &newY);	// 새 위치 받아오기
    x = newX;
    y = newY;

    board[y][x] = 0;    // 새 위치에 Pawn 설정
}

// 체스판 초기화하는 함수
void initializeBoard()
{
    for (int i = 0; i < BOARDSIZE; ++i) {
        for (int j = 0; j < BOARDSIZE; ++j) {
            // 검은색과 흰색 칸 번갈아가며 초기화
            if ((i + j) % 2 == 0) {
                board[i][j] = -1; // 흰색 칸
            }
            else {
                board[i][j] = 1; // 검은색 칸
            }
        }
    }

    // Pawn 초기 위치
    board[y][x] = 0;

}

// 체스판 출력하는 함수
void printBoard()
{
    system("cls"); // 콘솔 클리어

    for (int i = 0; i < BOARDSIZE; ++i) {
        for (int j = 0; j < BOARDSIZE; ++j) {
            switch (board[i][j])
            {
            case 1:
                cout << " ■ ";
                break;
            case -1:
                cout << " □ ";
                break;
            case 0:
                cout << " ♣ ";
                break;

            }

        }
        cout << endl;
        cout << endl;
    }
}

int main(int argc, char* argv[]) {

    string ipAddress;
    cout << "IP 주소 입력 : ";
    cin >> ipAddress; // IP 주소 입력

    int port = 54000; // 수신 포트

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
        // 키 입력 받기
        cout << "> ";
        char key = _getch(); // 키 입력

        if (sizeof(key) > 0) {
            // 서버에 키 입력 보내기
            int sendResult = send(sock, &key, sizeof(key), 0);  

            // 서버로부터 응답 받기
            if (sendResult != SOCKET_ERROR) {
                ZeroMemory(buf, 4096);
                int bytesReceived = recv(sock, buf, 4096, 0);
                if (bytesReceived > 0) {
                    // 서버로부터 받은 새로운 위치 정보로 말 이동
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
