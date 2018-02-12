#pragma once
//#include "SRWLock.h"

#define MAX_QUEUE_SIZE 4096	

class CRingBuffer
{
public:
	CRingBuffer();
	~CRingBuffer();

	int Enqueue(const char *pBuf, const int nSize);
	int Dequeue(char *pOut, const int nSize);

	int Commit_Enqueue(const int nSize);	// Enqueue한 Size만큼 Tail의 위치를 이동시킨다.
	int Commit_Dequeue(const int nSize);	// Dequeue한 Size만큼 head의 위치를 이동시킨다.

	void ReSize();	// Note : 쓰지 않을 것을 권고
	void Clear();

	inline bool IsPacket(const int packet_size);
	inline bool IsFull();
	inline bool IsFull(int ioSize);
	inline bool IsEmpty();
private:
	char *m_pQueue;

	USHORT m_tail;
	USHORT m_head;

	int m_nCurrentSize;
public:
	// Setter, Getter
	int GetCurrentSize() const { return m_nCurrentSize; }
	int GetEmptySize()   const { return MAX_QUEUE_SIZE - m_tail - 1; }

	char* GetBufStartPosition() { return &m_pQueue[0]; }
	char* GetBufEndPosition()   { return &m_pQueue[MAX_QUEUE_SIZE - 1]; }
	char* GetEuqueuePosition()  { return &m_pQueue[m_tail]; }
	char* GetDequeuePosttion()  { return &m_pQueue[m_head]; }
};

