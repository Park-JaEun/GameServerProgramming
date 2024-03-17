#include <iostream>
#include <WS2tcpip.h>		// �� ���̺귯���� ǥ�� �Լ���� �浹�ϴϱ� useing namespace std; �� ������� �ʴ´�.
#pragma comment(lib, "WS2_32.LIB")	// �� ���̺귯���� ����ϰڴٰ� �����Ѵ�.


constexpr short PORT = 4000;
constexpr char SERVER_ADDR[] = "127.0.0.1";
constexpr int BUFSIZE = 256;

int main()
{
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
        char buf[BUFSIZE];
        std::cout << "Enter Message : ";
        std::cin.getline(buf, BUFSIZE);
        WSABUF wsabuf[1];
        wsabuf[0].buf = buf;
        wsabuf[0].len = static_cast<int>(strlen(buf)) + 1;      // ���ڿ� ���� 0���� �����ؼ� �����Ѵ�.
        if (wsabuf[0].len == 1)
            break;

        DWORD sent_size;
        WSASend(server_s, wsabuf, 1, &sent_size, 0, nullptr, nullptr);

        wsabuf[0].len = BUFSIZE;	// �������� ��ŭ ���� �� �𸣴ϱ� �ϴ� ũ��
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
