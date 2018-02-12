#pragma once

#include "Timer.h"
#include "Player.h"
#include "Scene.h"

class CGameFramework
{
public:
	CGameFramework();
	~CGameFramework();

	bool OnCreate(HINSTANCE hInstance, HWND hMainWnd);
	void OnDestroy();

	bool CreateRenderTargetDepthStencilView();
	bool CreateDirect3DDisplay();

	void CreateShaderVariables();
	void ReleaseShaderVariables();

	void BuildObjects();
	void ReleaseObjects();

	void ProcessInput();
	void AnimateObjects();
	void FrameAdvance();

	void OnProcessingMouseMessage(HWND hWnd, UINT nMessageID, WPARAM wParam, LPARAM lParam);
	void OnProcessingKeyboardMessage(HWND hWnd, UINT nMessageID, WPARAM wParam, LPARAM lParam);
	LRESULT CALLBACK OnProcessingWindowMessage(HWND hWnd, UINT nMessageID, WPARAM wParam, LPARAM lParam);

private:
	HINSTANCE						m_hInstance;
	HWND							m_hWnd;
	HMENU							m_hMenu;

	int								m_nWndClientWidth;
	int								m_nWndClientHeight;

	ID3D11Device					*m_pd3dDevice;
	IDXGISwapChain					*m_pDXGISwapChain;
	ID3D11RenderTargetView			*m_pd3dRenderTargetView;
	ID3D11DeviceContext				*m_pd3dDeviceContext;

	UINT							m_n4xMSAAQualities;

	ID3D11Texture2D					*m_pd3dDepthStencilBuffer;
	ID3D11DepthStencilView			*m_pd3dDepthStencilView;

	CGameTimer						m_GameTimer;

	CScene							*m_pScene;

	CPlayer							*m_pPlayer;
	CCamera							*m_pCamera;

	POINT							m_ptOldCursorPos;
	_TCHAR							m_pszBuffer[50];
public :
	HWND GetHwnd() const { return m_hWnd; }
};
