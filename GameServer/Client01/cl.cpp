#include <iostream>
#include <WS2tcpip.h>		// 이 라이브러리가 표준 함수들과 충돌하니까 useing namespace std; 를 사용하지 않는다.
#pragma comment(lib, "WS2_32.LIB")	// 이 라이브러리를 사용하겠다고 선언한다.


constexpr short PORT = 4000;
constexpr char SERVER_ADDR[] = "127.0.0.1";
constexpr int BUFSIZE = 256;

int main()
{
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
        char buf[BUFSIZE];
        std::cout << "Enter Message : ";
        std::cin.getline(buf, BUFSIZE);
        WSABUF wsabuf[1];
        wsabuf[0].buf = buf;
        wsabuf[0].len = static_cast<int>(strlen(buf)) + 1;      // 문자열 끝의 0까지 포함해서 전송한다.
        if (wsabuf[0].len == 1)
            break;

        DWORD sent_size;
        WSASend(server_s, wsabuf, 1, &sent_size, 0, nullptr, nullptr);

        wsabuf[0].len = BUFSIZE;	// 서버에서 얼만큼 보낼 지 모르니까 일단 크게
        DWORD recv_size;
        DWORD recv_flag = 0;
        WSARecv(server_s, wsabuf, 1, &recv_size, &recv_flag, nullptr, nullptr);

        for (DWORD i = 0; i < recv_size; ++i) {
            std::cout << buf[i];
        }
        std::cout << std::endl;
    }
    closesocket(server_s);

    WSACleanup();
}
