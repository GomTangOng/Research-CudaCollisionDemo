#include "stdafx.h"
#include "InputManager.h"


InputManager::InputManager()
{
}


InputManager::~InputManager()
{
}

void InputManager::Init()
{
	for (int i = 0; i < KEY_MAX; ++i)
	{
		GetKeyUp().set(i, false);
		GetKeyDown().set(i, false);
	}
}

bool InputManager::IsOnceKeyDown(int key)
{
	// GetAsyncKeyState(key) & 0x8000 : 이전에 누른적이 없고 호출 된 시점에서 눌린 상태면 TRUE
	if (GetAsyncKeyState(key) & 0x8000)
	{
		if (!GetKeyDown()[key])
		{
			SetKeyDown(key, true);
			return true;
		}
	}
	else
	{
		SetKeyDown(key, false);
	}
	return false;
}

bool InputManager::IsOnceKeyUp(int key)
{
	if (GetAsyncKeyState(key) & 0x8000)
	{
		SetKeyUp(key, true);
	}
	else
	{
		if (GetKeyUp()[key])
		{
			SetKeyUp(key, false);
			return true;
		}
	}
	return false;
}

bool InputManager::IsStayKeyDown(int key)
{
	if (GetAsyncKeyState(key) & 0x8000)
		return true;
	return false;
}
