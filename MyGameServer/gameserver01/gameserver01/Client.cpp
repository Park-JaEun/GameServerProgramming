#include <iostream>
#include <conio.h> 

using namespace std;

const int SIZE = 8; // ü������ ũ��
int board[SIZE][SIZE]; // ü���� ������
int x = 0, y = 0; // ���� Pawn ��ġ

// ü������ �ʱ�ȭ�ϴ� �Լ�
void initializeBoard() 
{
    for(int i = 0; i < SIZE; ++i) {
        for(int j = 0; j < SIZE; ++j) {
            // �������� ��� ĭ �����ư��� �ʱ�ȭ
            if((i + j) % 2 == 0) {
                board[i][j] = -1; // ��� ĭ
            } else {
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

    for(int i = 0; i < SIZE; ++i) {
        for(int j = 0; j < SIZE; ++j) {
            switch (board[i][j])
            {
            case 1:
                cout << "�� ";
                break;
            case -1:
                cout << "�� ";
                break;
            case 0:
                cout << "P ";
                break;

            }
            
        }
        cout << endl;
    }
}

int main() {
    
    initializeBoard();
    printBoard(); // ü���� �׸�

    while(true) 
    {
        char key = _getch(); // Ű �Է�

        switch(key) {
            case 'w': // ���� �̵�
                if(y > 0) {
                    // ���� ��ġ�� ���� �ٵ��� ������ ����
                    board[y][x] = ((y + x) % 2 == 0) ? -1 : 1;
                    y--;
                    
                    board[y][x] = 0; // �� ��ġ�� Pawn ����
                }
                break;
            case 's': // �Ʒ��� �̵�
                if(y < SIZE - 1) {
                    board[y][x] = ((y + x) % 2 == 0) ? -1 : 1;
                    y++;

                    board[y][x] = 0;
                }
                break;
            case 'a': // �������� �̵�
                if(x > 0) {
                    board[y][x] = ((y + x) % 2 == 0) ? -1 : 1;
                    x--;

                    board[y][x] = 0;
                }
                break;
            case 'd': // ���������� �̵�
                if(x < SIZE - 1) {
                    board[y][x] = ((y + x) % 2 == 0) ? -1 : 1;
                    x++;

                    board[y][x] = 0;
                }
                break;

        }
        printBoard(); // �� �̵� �� ü������ �ٽ� �׸�
    }

    return 0;
}
