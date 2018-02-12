#pragma once

#define MAX_CLIENT             120		// 최대클라이언트에 + 1을 해줘야 한다 -> 0은 더미 클라
#define ATTACKER_MONSTER_START 1
#define DEFENDER_MONSTER_START 5001
#define SURPPORT_MONSTER_START 10001
#define MAX_MONSTER            15001
#define MAX_ITEM               2001

#define MAX_BUF_SIZE    4096
#define MAX_PACKET_SIZE 256
#define MAX_CHAT_SIZE   32
#define MAX_SC_PACKET_SIZE sizeof(sc_packet_any)
#define MAX_CS_PACKET_SIZE sizeof(cs_packet_any)

#define SERVER_PORT               9054
#define SERVER_IP                 "127.0.0.1"
#define PACKET_SIZE(packet)       packet[0]
#define PACKET_TYPE(packet)       packet[1]
#define WM_SOCKET                 WM_USER + 1 
#define WSAGETSELECTERROR(lParam) HIWORD(lParam) 
#define WSAGETSELECTEVENT(lParam) LOWORD(lParam)

#define WORLD_WIDTH  10000
#define WORLD_HEIGHT 10000

#define SECTOR_WIDTH  500
#define SECTOR_HEIGHT 500

#define SECTOR_X_LENGTH (WORLD_WIDTH / SECTOR_WIDTH)
#define SECTOR_Y_LENGTH (WORLD_HEIGHT / SECTOR_HEIGHT)

#define CELL_WIDTH  20
#define CELL_HEIGHT 20

#define VIEW_RADIUS 375

#define PLAYER_INIT_X 0
#define PLAYER_INIT_Y 0
#define PLAYER_INIT_Z 0

#define PLAYER_SHIFT 50



//#define ACCEPT_EX
#define ONCE_SEND

enum CSPacketType : BYTE
{
	CS_LEFT,
	CS_RIGHT,
	CS_UP,
	CS_DOWN,
	CS_FORWARD,
	CS_BACKWARD,
	CS_CHAT,
	CS_LOGIN,
	CS_LOGOUT,
	CS_KEYBOARD_MOVE_START,
	CS_KEYBOARD_MOVE_STOP,
	CS_MOUSE_MOVE,
	CSPACKET_TYPE_END
};

enum SCPacketType : BYTE
{
	SC_PLAYER_POS,
	SC_CHAT,
	SC_PUT_PLAYER,
	SC_REMOVE_PLAYER,
	SC_LOGIN,
	SC_PUT_MONSTER,
	SC_PLAYER_HP,
	SC_REMOVE_MONSTER,
	SC_MONSTER_POS,
	//SC_KEYBOARD_MOVE_START,
	//SC_KEYBOARD_MOVE_STOP,
	SCPACKET_TYPE_END
};

enum Direction : BYTE
{
	LEFT,
	RIGHT,
	UP,
	DOWN,
	FORWARD,
	BACKWARD
};


#pragma pack(push, 1)

/////////////////////////////////////////////////////////////////////
// Server -> Client

struct sc_packet_chat
{
	BYTE           size;
	SCPacketType   type;
	UINT           player_id;
	char           string[MAX_CHAT_SIZE];
};

struct sc_packet_player_pos
{
	BYTE         size;
	SCPacketType type;
	UINT         player_id;
	float		 x;
	float		 y;
	float		 z;
	//float        dx;
	//float		 dy;
	//float		 dz;
	//float speed;
};


struct sc_packet_login
{
	BYTE         size;
	SCPacketType type;
	UINT         player_id;
	float		 x;
	float		 y;
	float		 z;
	int          hp;
};

struct sc_packet_put_player
{
	BYTE         size;
	SCPacketType type;
	UINT         player_id;
	float		 x;
	float		 y;
	float		 z;
	int          hp;
};

struct sc_packet_remove_player
{
	BYTE         size;
	SCPacketType type;
	UINT         player_id;
};

struct sc_packet_player_hp
{
	BYTE         size;
	SCPacketType type;
	UINT         player_id;
	int          hp;
};

struct sc_packet_put_monster
{
	BYTE         size;
	SCPacketType type;
	UINT	     monster_id;
	UINT         monster_type;
	float x;
	float y;
	float z;
	int   hp;
};

struct sc_packet_monster_pos
{
	BYTE         size;
	SCPacketType type;
	UINT         monster_id;
	float x;
	float y;
	float z;
};

struct sc_packet_remove_monster
{
	BYTE         size;
	SCPacketType type;
	UINT         monster_id;
};

struct sc_packet_any
{
	union
	{
		sc_packet_player_pos	 player_pos;
		sc_packet_chat			 chat;
		sc_packet_put_player	 put_player;
		sc_packet_login			 loginOK;
		sc_packet_remove_player  remove_player;
		sc_packet_player_hp      player_hp;
		sc_packet_put_monster    put_monster;
		sc_packet_monster_pos	 monster_pos;
		sc_packet_remove_monster remove_monster;
	};
};
////////////////////////////////////////////////////////////////////

////////////////////////////////////////////////////////////////////
// Client -> Server

struct cs_packet_chat
{
	BYTE           size;
	CSPacketType   type;
	UINT           player_id;
	char           string[MAX_CHAT_SIZE];
};

struct cs_packet_move_left
{
	BYTE         size;
	CSPacketType type;
};

struct cs_packet_move_right
{
	BYTE         size;
	CSPacketType type;
};

struct cs_packet_move_up
{
	BYTE         size;
	CSPacketType type;
};

struct cs_packet_move_down
{
	BYTE         size;
	CSPacketType type;
};

struct cs_packet_move_forward
{
	BYTE         size;
	CSPacketType type;
};

struct cs_packet_move_backward
{
	BYTE         size;
	CSPacketType type;
};

struct cs_packet_login
{
	BYTE         size;
	CSPacketType type;
};

struct cs_packet_player_keyboard_move_start
{
	BYTE         size;
	CSPacketType type;
	BYTE		 direction;
	//float		 x;
	//float		 y;
	//float		 z;
	//int			 speed;
	//DWORD        time_stamp;
};

struct cs_packet_player_keyboard_move_stop
{
	BYTE         size;
	CSPacketType type;
	BYTE		 direction;
	float		 x;
	float		 y;
	float		 z;
};


struct cs_packet_player_mouse_move
{
	BYTE size;
	CSPacketType type;
	float x;
	float y;
	float z;
	float dest_x;
	float dest_y;
	float dest_z;
	float dx;
	float dy;
	float dz;
	int   speed;
};

struct cs_packet_any
{
	union
	{
		cs_packet_move_left  move_left;
		cs_packet_move_right move_right;
		cs_packet_move_up    move_up;
		cs_packet_move_down  move_down;
		cs_packet_chat       chat;
		cs_packet_login      login;
		cs_packet_player_keyboard_move_start player_keyboard_move_start;
		cs_packet_player_keyboard_move_stop player_keyboard_move_stop;
		cs_packet_player_mouse_move player_mouse_move;
	};
};
////////////////////////////////////////////////////////////////////////////////
#pragma pack(pop)