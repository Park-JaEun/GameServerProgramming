#include <iostream>
#include <conio.h> 

using namespace std;

const int SIZE = 8; // 체스판의 크기
int board[SIZE][SIZE]; // 체스판 데이터
int x = 0, y = 0; // 현재 Pawn 위치

// 체스판을 초기화하는 함수
void initializeBoard() 
{
    for(int i = 0; i < SIZE; ++i) {
        for(int j = 0; j < SIZE; ++j) {
            // 검은색과 흰색 칸 번갈아가며 초기화
            if((i + j) % 2 == 0) {
                board[i][j] = -1; // 흰색 칸
            } else {
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

    for(int i = 0; i < SIZE; ++i) {
        for(int j = 0; j < SIZE; ++j) {
            switch (board[i][j])
            {
            case 1:
                cout << "■ ";
                break;
            case -1:
                cout << "□ ";
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
    printBoard(); // 체스판 그림

    while(true) 
    {
        char key = _getch(); // 키 입력

        switch(key) {
            case 'w': // 위로 이동
                if(y > 0) {
                    // 이전 위치를 원래 바둑판 색으로 복원
                    board[y][x] = ((y + x) % 2 == 0) ? -1 : 1;
                    y--;
                    
                    board[y][x] = 0; // 새 위치에 Pawn 설정
                }
                break;
            case 's': // 아래로 이동
                if(y < SIZE - 1) {
                    board[y][x] = ((y + x) % 2 == 0) ? -1 : 1;
                    y++;

                    board[y][x] = 0;
                }
                break;
            case 'a': // 왼쪽으로 이동
                if(x > 0) {
                    board[y][x] = ((y + x) % 2 == 0) ? -1 : 1;
                    x--;

                    board[y][x] = 0;
                }
                break;
            case 'd': // 오른쪽으로 이동
                if(x < SIZE - 1) {
                    board[y][x] = ((y + x) % 2 == 0) ? -1 : 1;
                    x++;

                    board[y][x] = 0;
                }
                break;

        }
        printBoard(); // 매 이동 후 체스판을 다시 그림
    }

    return 0;
}
