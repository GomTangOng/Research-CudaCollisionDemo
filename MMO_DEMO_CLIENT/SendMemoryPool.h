#pragma once

#include "MemoryPool.h"
#include "protocol.h"

class CSendMemoryPool : public CMemoryPool<CSendMemoryPool>
{
public :
	CSendMemoryPool();
	~CSendMemoryPool();
private :
	UCHAR size[MAX_CS_PACKET_SIZE];		// Warning : 클라에서 쓸 경우에만 MAX_CS_PACKET_SIZE
										//			 서버일때는 MAX_SC_PACKET_SIZE
};

