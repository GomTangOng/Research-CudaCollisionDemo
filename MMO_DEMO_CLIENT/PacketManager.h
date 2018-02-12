#pragma once

/* CPacketManger.h
 *
 * 기능 : Packet 조립과 처리
 */
#include "stdafx.h"
#include "protocol.h"
#include "RingBuffer.h"
#include <unordered_map>


#define PACKET_MANAGER CPacketManager::GetInstance()

//#define SERVER
#define CLIENT

class CClientSession;
class CPacketManager      
{
public:
	typedef function<void(const char *, const UINT)> Processfunc;
		
	CPacketManager();
	~CPacketManager();
	
	static CPacketManager* GetInstance()
	{
		static CPacketManager instance;
		return &instance;
	}

	bool Start();
	void ShutDown();

	//void Assemble(UINT size_to_process, CClientSession *pClient);				// Server Version
	void Assemble(UINT size_to_process, CRingBuffer* recv_ring_buffer, UINT id);	// Client Version

	void Dispatch(char *packet, UINT id);

	// Process Functions
	void ProcessChat(const char *packet, const UINT id);
	void ProcessPlayerPosition(const char *packet, const UINT id);
	void ProcessLogin(const char *packet, const UINT id);
	void ProcessPutPlayer(const char *packet, const UINT id);
	void ProcessRemovePlayer(const char *packet, const UINT id);
private :
	unordered_map<UINT /*packet_type*/, Processfunc> m_packetProcessFuncTable;
public :
	const unordered_map<UINT, Processfunc>& GetPacketProcessFuncTable() const { return m_packetProcessFuncTable; }
};

