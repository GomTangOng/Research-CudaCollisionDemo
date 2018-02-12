#include "stdafx.h"
#include "PacketManager.h"
//#include "SessionManager.h"
//#include "WorldManager.h"
//#include "CollisionManager.h"
#include <mutex>
//#include "cuda_function.cu"
//extern "C" void Init_Cuda2(CUDA_OBJECT *obj_list, int obj_cnt, int my_id, bool *result);

#include "protocol.h"

CPacketManager::CPacketManager()
{
}


CPacketManager::~CPacketManager()
{
}

bool CPacketManager::Start()
{
	m_packetProcessFuncTable.reserve(SCPacketType::SCPACKET_TYPE_END);

	m_packetProcessFuncTable[SCPacketType::SC_CHAT] = std::bind(&CPacketManager::ProcessChat,
		this,
		std::placeholders::_1,
		std::placeholders::_2);
	m_packetProcessFuncTable[SCPacketType::SC_PLAYER_POS] = std::bind(&CPacketManager::ProcessPlayerPosition,
		this,
		std::placeholders::_1,
		std::placeholders::_2);
	m_packetProcessFuncTable[SCPacketType::SC_LOGIN] = std::bind(&CPacketManager::ProcessLogin,
		this,
		std::placeholders::_1,
		std::placeholders::_2);

	m_packetProcessFuncTable[SCPacketType::SC_PUT_PLAYER] = std::bind(&CPacketManager::ProcessPutPlayer,
		this,
		std::placeholders::_1,
		std::placeholders::_2);

	m_packetProcessFuncTable[SCPacketType::SC_REMOVE_PLAYER] = std::bind(&CPacketManager::ProcessRemovePlayer,
		this,
		std::placeholders::_1,
		std::placeholders::_2);

	return true;
}

void CPacketManager::ShutDown()
{
#if defined(SERVER)
	for (UINT i = 0; i < CSPacketType::CSPACKET_TYPE_END; ++i)
	{
		m_packetProcessFuncTable.erase(i);
	}
#else
	for (UINT i = 0; i < SCPacketType::SCPACKET_TYPE_END; ++i)
	{
		m_packetProcessFuncTable.erase(i);
	}
#endif
	m_packetProcessFuncTable.clear();
}


void CPacketManager::Assemble(UINT size_to_process, CRingBuffer* recv_ring_buffer, UINT id)
{
	UINT size_to_build_packet{ 0 };

	recv_ring_buffer->Commit_Enqueue(size_to_process);

	// Note : Recv 요청을 한번만 하기 때문에 락은 따로 필요없음
	while (size_to_process > 0)
	{
		char        *recv_buffer = recv_ring_buffer->GetDequeuePosttion();
		size_to_build_packet = recv_buffer[0];

		if (size_to_process >= size_to_build_packet)	// 패킷 처리 가능
		{
			Dispatch(recv_buffer, id);
			size_to_process -= size_to_build_packet;
			recv_ring_buffer->Commit_Dequeue(size_to_build_packet);
		}
		else
		{
			memcpy(recv_ring_buffer->GetBufStartPosition(), recv_buffer, size_to_process);	// 잘린패킷의 데이터를
																							// recv버퍼의 시작점으로 복사시킴
			recv_ring_buffer->Clear();
			recv_ring_buffer->Commit_Enqueue(size_to_process);
			break;
		}
	}
}

void CPacketManager::Dispatch(char * packet, UINT id)
{
	UINT type = packet[1];

#if defined(SERVER)
	assert(type < CSPacketType::CSPACKET_TYPE_END);
#else
	assert(type < SCPacketType::SCPACKET_TYPE_END);
#endif
	m_packetProcessFuncTable[type](packet, id);
}

void CPacketManager::ProcessChat(const char * packet, const UINT id)
{
	cs_packet_chat *chat_packet = (cs_packet_chat *)packet;
	
	cout << chat_packet->string << endl;

	// for Echo 
	sc_packet_any send_pkt;
	send_pkt.chat.size = chat_packet->size;
	send_pkt.chat.type = SCPacketType::SC_CHAT;
	send_pkt.chat.player_id = id;
	memcpy(send_pkt.chat.string, chat_packet->string, strlen(chat_packet->string));	// Warning : 클라이언트에서 받을때 맨 마지막 문자열 뒤에 '\0'삽입시켜야함

//#if defined(ONCE_SEND)
//	NETWORK_ENGINE->SendPacket((char *)&send_pkt);
//	//SESSION_MANAGER->FindConnectedClient(id)->OnceSend(packet);
//#else
//	SESSION_MANAGER->FindConnectedClient(id)->PreSend((char *)&send_pkt);
//#endif
}

void CPacketManager::ProcessPlayerPosition(const char * packet, const UINT id)
{
	sc_packet_player_pos *pkt = (sc_packet_player_pos *)packet;

	/*CPlayer *pPlayer = PLAYER_MANAGER->FindPlayer(pkt->player_id);

	pPlayer->SetPosition(D3DXVECTOR3(pkt->x, pkt->y, pkt->z));
	static_cast<Engine::CTransform *>(pPlayer->Get_Component(L"Com_Transform"))->m_fAngle[Engine::CTransform::ANGLE_Y] = pkt->radian;*/
}

void CPacketManager::ProcessPutPlayer(const char * packet, const UINT id)
{
	sc_packet_put_player *pkt = (sc_packet_put_player *)packet;
	/*CPlayer *pPlayer;

	if (pkt->player_id <= 0) return;

	pPlayer = PLAYER_MANAGER->AddPlayer(pkt->player_id);

	pPlayer->SetPosition(D3DXVECTOR3(pkt->x, pkt->y, pkt->z));
	pPlayer->SetHP(pkt->hp);
	pPlayer->SetConnected(true);
	Engine::CManagement::GetInstance()->Add_Object(L"GameLogic", L"Player", pPlayer);

	cout << "ProcessOutPlayer : " << pkt->player_id << " 추가" << endl;*/
}

// Warning : 여기서 인자로 넣어진 ID를 쓰지말 것! 아직 ID 등록 안된 상태
void CPacketManager::ProcessLogin(const char * packet, const UINT id)
{
	sc_packet_login *pkt = (sc_packet_login *)packet;

	if (pkt->player_id <= 0) return;

	//	CPlayer *pPlayer = PLAYER_MANAGER->AddPlayer(pkt->player_id);
	//
	//	NETWORK_ENGINE->SetMyPlayer(pPlayer);
	//	NETWORK_ENGINE->SetMyID(pkt->player_id);
	//	pPlayer->SetConnected(true);
	//
	//	pPlayer->SetPosition(D3DXVECTOR3(pkt->x, pkt->y, pkt->z));
	//	//pPlayer->Move(Direction::STOP);		// 임시로
	//	pPlayer->SetHP(pkt->hp);
	//	Engine::CManagement::GetInstance()->Add_Object(L"GameLogic", L"Player", pPlayer);
	//
	//	Engine::CGameObject *pMouse = CMouseCol::Create(Engine::Get_GraphicDev());
	//	Engine::CManagement::GetInstance()->Add_Object(L"GameLogic", L"MouseCol", pMouse);
	//	pPlayer->Set_MouseCol((CMouseCol*)pMouse);
	//
	//	Engine::CGameObject *pCamera = CStaticCamera::Create(Engine::Get_GraphicDev(), &_vec3(0.f, 100.f, -10.f), &_vec3(0.f, 0.f, 0.f));
	//	Engine::CManagement::GetInstance()->Add_Object(L"GameLogic", L"Camera", pCamera);
	//
	//	//wsprintf(name,L"Player%d", pkt->player_id);
	//	//pPlayer->SetName(name);
	//
	//	cout << "ProcessLogin : " << pkt->player_id << " 로그인" << endl;
	//}
}

void CPacketManager::ProcessRemovePlayer(const char * packet, const UINT id)
{
	sc_packet_remove_player *pkt = (sc_packet_remove_player *)packet;


	//PLAYER_MANAGER->DeletePlayer(pkt->player_id);
}