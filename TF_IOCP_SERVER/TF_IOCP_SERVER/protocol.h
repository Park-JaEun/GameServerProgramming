constexpr int PORT_NUM = 4000;
constexpr int BUF_SIZE = 200;
constexpr int NAME_SIZE = 20;		// 로그인 시 아이디 크기

constexpr int W_WIDTH = 8;			// 월드 크기
constexpr int W_HEIGHT = 8;

// Packet ID
constexpr char CS_LOGIN = 0;			// 클라이언트에서 서버로 보내는 패킷
constexpr char CS_MOVE = 1;

constexpr char SC_LOGIN_INFO = 2;		// 서버에서 클라이언트로 보내는 패킷
constexpr char SC_ADD_PLAYER = 3;
constexpr char SC_REMOVE_PLAYER = 4;
constexpr char SC_MOVE_PLAYER = 5;

#pragma pack (push, 1)
struct CS_LOGIN_PACKET {			// 모든 패킷은 첫번째로 크기, 두번째로 타입을 가진다.
	unsigned char size;
	char	type;
	char	name[NAME_SIZE];
};

struct CS_MOVE_PACKET {
	unsigned char size;
	char	type;
	char	direction;				// 0 : UP, 1 : DOWN, 2 : LEFT, 3 : RIGHT
};

struct SC_LOGIN_INFO_PACKET {
	unsigned char size;
	char	type;
	short	id;
	short	x, y;
};

struct SC_ADD_PLAYER_PACKET {
	unsigned char size;
	char	type;
	short	id;
	short	x, y;
	char	name[NAME_SIZE];
};

struct SC_REMOVE_PLAYER_PACKET {		// 플레이어 이름보다는 id로 구분하는 것이 좋다.
	unsigned char size;
	char	type;
	short	id;
};

struct SC_MOVE_PLAYER_PACKET {
	unsigned char size;
	char	type;
	short	id;
	short	x, y;
};

#pragma pack (pop)