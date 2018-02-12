#pragma once
#include "stdafx.h"
#include "Memory.h"

template <class T, int ALLOC_BLOCK_COUNT = 1024>
class CMemoryPool
{
public :
	CMemoryPool()
	{
		
	}
	~CMemoryPool()
	{

	}

	static void* operator new(size_t allocLength)
	{
		if (m_pFreeList==nullptr) AllocBlock();
		/*
		T* data;

		m_freeList.try_pop(data);

		return data;*/
		UCHAR *pReturn = m_pFreeList;

		m_pFreeList = *reinterpret_cast<UCHAR **>(pReturn);

		return pReturn;
	}

	static void operator delete(void *pDel)
	{
		*reinterpret_cast<UCHAR **>(pDel) = m_pFreeList;
		m_pFreeList = static_cast<UCHAR *>(pDel);
		//m_freeList.push(reinterpret_cast<T *>(pDel));
	}

	static void AllocBlock()
	{
		/*for (int i = 0; i < ALLOC_BLOCK_COUNT; ++i)
		{
			T* data = (T *)malloc(sizeof(T));
			m_freeList.push(data);
		}*/

		//cout << "메모리할당\n";
		m_pFreeList = new UCHAR[sizeof(T) * ALLOC_BLOCK_COUNT];

		UCHAR **ppCurrent = reinterpret_cast<UCHAR **>(m_pFreeList);
		UCHAR *pNext = m_pFreeList;

		for (int i = 0; i < ALLOC_BLOCK_COUNT - 1; ++i)
		{
			pNext += sizeof(T);
			*ppCurrent = pNext;
			ppCurrent = reinterpret_cast<UCHAR **>(pNext);
		}

		*ppCurrent = nullptr;
	}
private :
	//static tbb::concurrent_queue<T *> m_freeList;
	static UCHAR* m_pFreeList;
};

template <class T, int ALLOC_BLOCK_COUNT>
//tbb::concurrent_queue<T *> CMemoryPool<T, ALLOC_BLOCK_COUNT>::m_freeList;
UCHAR* CMemoryPool<T, ALLOC_BLOCK_COUNT>::m_pFreeList;
