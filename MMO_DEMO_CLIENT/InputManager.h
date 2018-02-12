#pragma once

#include <bitset>

#define KEY_MAX 256
#define INPUT_MANAGER InputManager::GetInstance()

class InputManager
{
public:
	InputManager();
	~InputManager();

	void Init();

	bool IsOnceKeyDown(int key);
	bool IsOnceKeyUp(int key);
	bool IsStayKeyDown(int key);
	//bool IsToggleKey(int key);
	static InputManager* GetInstance()
	{
		static InputManager instance;
		return &instance;
	}
private:
	std::bitset<KEY_MAX> m_keyUp;
	std::bitset<KEY_MAX> m_keyDown;

public:
	void SetKeyDown(int key, bool state) { m_keyDown.set(key, state); }
	void SetKeyUp(int key, bool state) { m_keyUp.set(key, state); }

	std::bitset<KEY_MAX> GetKeyUp() { return m_keyUp; }
	std::bitset<KEY_MAX> GetKeyDown() { return m_keyDown; }
};

