// server.cpp
#include <iostream>
#include <string>
#include <WS2tcpip.h>		// 이 라이브러리가 표준 함수들과 충돌하니까 useing namespace std; 를 사용하지 않는다.
#pragma comment(lib, "WS2_32.LIB")	// 이 라이브러리를 사용하겠다고 선언한다.

constexpr short PORT = 4000;
constexpr int BUFSIZE = 256;


const int BOARDSIZE = 8;
int board[BOARDSIZE][BOARDSIZE]; // 체스판 데이터
int x = 0, y = 0; // 처음 Pawn 위치

void initializeBoard() {
    for (int i = 0; i < BOARDSIZE; ++i) {
        for (int j = 0; j < BOARDSIZE; ++j) {
            if ((i + j) % 2 == 0) {
                board[i][j] = -1; // 흰색 칸
            }
            else {
                board[i][j] = 1; // 검은색 칸
            }
        }
    }
    board[y][x] = 0; // Pawn 초기 위치
}

void updateBoard(char key) {
    // 이전 위치 복원
    board[y][x] = ((y + x) % 2 == 0) ? -1 : 1;

    switch (key) {
        // WASD로 이동하는 경우
    case 'w': if (y > 0) y--; break;
    case 's': if (y < BOARDSIZE - 1) y++; break;
    case 'a': if (x > 0) x--; break;
    case 'd': if (x < BOARDSIZE - 1) x++; break;

        // 커서키로 이동하는 경우
    case 72: if (y > 0) y--; break; // UP
    case 80: if (y < BOARDSIZE - 1) y++; break; // DOWN
    case 75: if (x > 0) x--; break; // LEFT
    case 77: if (x < BOARDSIZE - 1) x++; break; // RIGHT
    }

    board[y][x] = 0; // 새 위치에 Pawn 설정
    std::cout<< "Pawn 위치: " << x+1 << ", " << y+1 << std::endl;
}


void print_error(const char* msg, int err_no)
{
    WCHAR* msg_buf;
    FormatMessage(FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM,
        NULL, err_no,
        MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
        reinterpret_cast<LPWSTR>(&msg_buf), 0, NULL);
    std::cout << msg;
    std::wcout << L" : 에러 : " << msg_buf;
    LocalFree(msg_buf);
    while (true);   // 편안하게 디버깅 가능하도록 하는 코드, 실제 프로그램에서는 빼야 함
    return;
}

int main() {
    std::wcout.imbue(std::locale("korean"));		// 에러 메세지 한글로

    WSADATA WSAData;
    WSAStartup(MAKEWORD(2, 0), &WSAData);		// 윈도우에서만 필요한 코드	

    SOCKET server_s = WSASocket(AF_INET, SOCK_STREAM, IPPROTO_TCP, nullptr, 0, 0);
    SOCKADDR_IN server_a;
    server_a.sin_family = AF_INET;
    server_a.sin_port = htons(PORT);						// 다른 프로그램과 충돌하지 않는 숫자로 설정해야 함
    server_a.sin_addr.S_un.S_addr = htonl(INADDR_ANY);	// 서버에서는 이렇게 설정해야 한다.

    bind(server_s, reinterpret_cast<SOCKADDR*>(&server_a), sizeof(server_a));
    listen(server_s, SOMAXCONN);	// SOMAXCONN은 최대 연결 수

   
    int addr_size = sizeof(server_a);
    SOCKET client_s = WSAAccept(server_s, reinterpret_cast<SOCKADDR*>(&server_a), &addr_size, nullptr, 0);

    initializeBoard();      // 체스판 초기화

    while (true) {
        char buf[BUFSIZE];

        WSABUF wsabuf[1];
        wsabuf[0].buf = buf;
        wsabuf[0].len = BUFSIZE;	// 일단 크게
        DWORD recv_size;
        DWORD recv_flag = 0;
        int res = WSARecv(client_s, wsabuf, 1, &recv_size, &recv_flag, nullptr, nullptr);
        if (0 != res) {
            print_error("WSARecv", WSAGetLastError());
        }
        if (recv_size == 0)
            break;

        updateBoard(buf[0]); // 말 위치 업데이트

        // Pawn 위치 전송
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
