#pragma once

namespace Memory
{
	template <typename T>
	inline void SAFE_DELETE(T *p)
	{
		if (p)
		{
			delete p;
			p = nullptr;
		}
	}

	template <typename T>
	inline void SAFE_DELETE_ARRAY(T *p)
	{
		if (p)
		{
			delete[] p;
			p = nullptr;
		}
	}

	template <typename T>
	inline void SAFE_RELEASE(T* p)
	{
		if (p)
		{
			p->Release();
		}
	}
}