#include <iostream>
#include <WS2tcpip.h>		// 이 라이브러리가 표준 함수들과 충돌하니까 useing namespace std; 를 사용하지 않는다.
#pragma comment(lib, "WS2_32.LIB")	// 이 라이브러리를 사용하겠다고 선언한다.


constexpr short PORT = 4000;
constexpr int BUFSIZE = 256;

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

int main()
{
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
    while (true) {
        char buf[BUFSIZE];

        WSABUF wsabuf[1];
        wsabuf[0].buf = buf;
        wsabuf[0].len = BUFSIZE;	// 서버에서 얼만큼 보낼 지 모르니까 일단 크게
        DWORD recv_size;
        DWORD recv_flag = 0;
        int res = WSARecv(client_s, wsabuf, 1, &recv_size, &recv_flag, nullptr, nullptr);
        if (0 != res) {
            print_error("WSARecv", WSAGetLastError());
        }
        if (recv_size == 0)
            break;

        for (DWORD i = 0; i < recv_size; ++i) {
            std::cout << buf[i];
        }
        std::cout << std::endl;

        wsabuf[0].buf = buf;
        wsabuf[0].len = recv_size;
        DWORD sent_size;
        WSASend(client_s, wsabuf, 1, &sent_size, 0, nullptr, nullptr);
    }
    closesocket(client_s);
    closesocket(server_s);

    WSACleanup();
}


/////////////////////////////////////////////////////////////////////////////////////

