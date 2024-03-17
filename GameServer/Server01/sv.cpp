#include <iostream>
#include <WS2tcpip.h>		// �� ���̺귯���� ǥ�� �Լ���� �浹�ϴϱ� useing namespace std; �� ������� �ʴ´�.
#pragma comment(lib, "WS2_32.LIB")	// �� ���̺귯���� ����ϰڴٰ� �����Ѵ�.


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
    std::wcout << L" : ���� : " << msg_buf;
    LocalFree(msg_buf);
    while (true);   // ����ϰ� ����� �����ϵ��� �ϴ� �ڵ�, ���� ���α׷������� ���� ��
    return;
}

int main()
{
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
    while (true) {
        char buf[BUFSIZE];

        WSABUF wsabuf[1];
        wsabuf[0].buf = buf;
        wsabuf[0].len = BUFSIZE;	// �������� ��ŭ ���� �� �𸣴ϱ� �ϴ� ũ��
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

