#define _CRT_SECURE_NO_WARNINGS
#include <iostream>
#include <conio.h> 
#include <string>
#include <WS2tcpip.h>		// 이 라이브러리가 표준 함수들과 충돌하니까 useing namespace std; 를 사용하지 않는다.
#pragma comment(lib, "WS2_32.LIB")	// 이 라이브러리를 사용하겠다고 선언한다.

constexpr short PORT = 4000;
constexpr int BUFSIZE = 256;
constexpr char SERVER_ADDR[] = "127.0.0.1";


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

int main(int argc, char* argv[]) 
{

    char buf[BUFSIZE];
    char key;

    initializeBoard();
    printBoard();


    //cout << "IP 주소 입력 : ";

    
    std::wcout.imbue(std::locale("korean"));    // 에러 메세지 한글로

    WSADATA WSAData;
    WSAStartup(MAKEWORD(2, 0), &WSAData);       // 윈도우에서만 필요한 코드

    SOCKET server_s = WSASocket(AF_INET, SOCK_STREAM, IPPROTO_TCP, nullptr, 0, 0);
    SOCKADDR_IN server_a;
    server_a.sin_family = AF_INET;
    server_a.sin_port = htons(PORT);        // 다른 프로그램과 충돌하지 않는 숫자로 설정해야 함
    inet_pton(AF_INET, SERVER_ADDR, &server_a.sin_addr);
    connect(server_s, reinterpret_cast<SOCKADDR*>(&server_a), sizeof(server_a));

    while (true) {

        // 키 입력 받기
        std::cout << "key 입력 > ";
        key = _getch(); // 키 입력 받기

        // 입력받은 키 값을 buf의 첫 번째 요소에 저장
        buf[0] = key;

        // 문자열의 끝을 나타내기 위해 두 번째 요소에 널 문자 추가
        buf[1] = '\0';
        //cout << buf << endl;

        WSABUF wsabuf[1];
        wsabuf[0].buf = buf;
        wsabuf[0].len = static_cast<int>(strlen(buf)) + 1;      // 문자열 끝의 0까지 포함해서 전송한다.
        if (wsabuf[0].len == 1)
            break;

        DWORD sent_size;
        WSASend(server_s, wsabuf, 1, &sent_size, 0, nullptr, nullptr);

        wsabuf[0].buf = buf;
        wsabuf[0].len = BUFSIZE;	// 서버에서 얼만큼 보낼 지 모르니까 일단 크게
        DWORD recv_size;
        DWORD recv_flag = 0;
        WSARecv(server_s, wsabuf, 1, &recv_size, &recv_flag, nullptr, nullptr);

        cout << buf  << endl;

        // 서버로부터 받은 새로운 위치 정보로 말 이동 코드
        movePiece(x, y, buf);
        printBoard();

    }
    closesocket(server_s);

    WSACleanup();


  
}
