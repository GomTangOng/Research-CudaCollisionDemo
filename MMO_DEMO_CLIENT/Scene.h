//-----------------------------------------------------------------------------
// File: Scene.h
//-----------------------------------------------------------------------------

#pragma once

#include "Camera.h"
#include "Shader.h"

#define MAX_LIGHTS			4 //Multiple of 4
#define POINT_LIGHT			1.0f
#define SPOT_LIGHT			2.0f
#define DIRECTIONAL_LIGHT	3.0f

struct LIGHT
{
	D3DXCOLOR				m_d3dxcAmbient;
	D3DXCOLOR				m_d3dxcDiffuse;
	D3DXCOLOR				m_d3dxcSpecular;
	D3DXVECTOR3				m_d3dxvPosition;
	float					m_fRange;
	D3DXVECTOR3				m_d3dxvDirection;
	float					m_nType;
	D3DXVECTOR3				m_d3dxvAttenuation;
	float 					m_fFalloff;
	float 					m_fTheta; //cos(m_fTheta)
	float					m_fPhi; //cos(m_fPhi)
	float					m_bEnable;
	float					padding;
};

struct LIGHTS
{
	LIGHT					m_pLights[MAX_LIGHTS];
	D3DXCOLOR				m_d3dxcGlobalAmbient;
	D3DXVECTOR4				m_d3dxvCameraPosition;
};

class CScene
{
public:
	CScene();
	virtual ~CScene();

	bool OnProcessingMouseMessage(HWND hWnd, UINT nMessageID, WPARAM wParam, LPARAM lParam);
	bool OnProcessingKeyboardMessage(HWND hWnd, UINT nMessageID, WPARAM wParam, LPARAM lParam);

	void BuildObjects(ID3D11Device *pd3dDevice);
	void ReleaseObjects();

	void CreateShaderVariables(ID3D11Device *pd3dDevice);
	void UpdateShaderVariable(ID3D11DeviceContext *pd3dDeviceContext, LIGHTS *pLights);
	void ReleaseShaderVariables();

	bool ProcessInput(UCHAR *pKeysBuffer);
	void AnimateObjects(float fTimeElapsed);
	void OnPreRender(ID3D11DeviceContext *pd3dDeviceContext);
	void Render(ID3D11DeviceContext	*pd3dDeviceContext, CCamera *pCamera);

	CGameObject *PickObjectPointedByCursor(int xClient, int yClient);

	void SetCamera(CCamera *pCamera) { m_pCamera = pCamera; }
	void SetPlayer(CPlayer *pPlayer) { m_pPlayer = pPlayer; }

	CHeightMapTerrain *GetTerrain();
	void OnChangeSkyBoxTextures(ID3D11Device *pd3dDevice, CMaterial *pMaterial, int nIndex = 0);

private:
	CGameObject						**m_ppObjects;
	int								m_nObjects;

	CObjectsShader					**m_ppObjectShaders;
	int								m_nObjectShaders;

	CInstancedObjectsShader			**m_ppInstancingShaders;
	int								m_nInstancingShaders;

	CBillboardInstancedObjectShader **m_ppBillboardInstancingShaders;
	int                              m_nBillboardInstancingShaders;

	CPlayer							*m_pPlayer;
	CCamera							*m_pCamera;
	CGameObject						*m_pSelectedObject;

	LIGHTS							*m_pLights;
	ID3D11Buffer					*m_pd3dcbLights;
};

