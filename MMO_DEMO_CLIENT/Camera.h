#pragma once

#include "Mesh.h"

#define ASPECT_RATIO				(float(FRAME_BUFFER_WIDTH) / float(FRAME_BUFFER_HEIGHT))

#define FIRST_PERSON_CAMERA			0x01
#define SPACESHIP_CAMERA			0x02
#define THIRD_PERSON_CAMERA			0x03

struct VS_CB_CAMERA
{
	D3DXMATRIX						m_d3dxmtxView;
	D3DXMATRIX						m_d3dxmtxProjection;
};

class CPlayer;

class CCamera
{
protected:
	D3DXVECTOR3						m_d3dxvPosition;
	D3DXVECTOR3						m_d3dxvRight;
	D3DXVECTOR3						m_d3dxvUp;
	D3DXVECTOR3						m_d3dxvLook;

	float           				m_fPitch;
	float           				m_fRoll;
	float           				m_fYaw;

	DWORD							m_nMode;

	D3DXVECTOR3						m_d3dxvOffset;
	float           				m_fTimeLag;

	D3DXMATRIX						m_d3dxmtxView;
	D3DXMATRIX						m_d3dxmtxProjection;

	D3D11_VIEWPORT					m_d3dViewport;

	static ID3D11Buffer				*m_pd3dcbCamera;

	CPlayer							*m_pPlayer;

	D3DXPLANE						m_pd3dxFrustumPlanes[6]; //World Coordinates          

public:
	CCamera(CCamera *pCamera);
	virtual ~CCamera();

	void SetMode(DWORD nMode) { m_nMode = nMode; }
	DWORD GetMode() { return(m_nMode); }

	void GenerateViewMatrix();
	void RegenerateViewMatrix();

	void GenerateProjectionMatrix(float fNearPlaneDistance, float fFarPlaneDistance, float fAspectRatio, float fFOVAngle);

	static void CreateShaderVariables(ID3D11Device *pd3dDevice);
	static void ReleaseShaderVariables();
	static void UpdateShaderVariable(ID3D11DeviceContext *pd3dDeviceContext, D3DXMATRIX *pd3dxmtxView, D3DXMATRIX *pd3dxmtxProjection);
	void UpdateShaderVariables(ID3D11DeviceContext *pd3dDeviceContext);

	void SetPlayer(CPlayer *pPlayer) { m_pPlayer = pPlayer; }
	CPlayer *GetPlayer() { return(m_pPlayer); }

	void SetViewport(ID3D11DeviceContext *pd3dDeviceContext, DWORD xStart, DWORD yStart, DWORD nWidth, DWORD nHeight, float fMinZ = 0.0f, float fMaxZ = 1.0f);
	void SetViewport(ID3D11DeviceContext *pd3dDeviceContext);
	D3D11_VIEWPORT GetViewport() { return(m_d3dViewport); }

	D3DXMATRIX GetViewMatrix() { return(m_d3dxmtxView); }
	D3DXMATRIX GetProjectionMatrix() { return(m_d3dxmtxProjection); }
	ID3D11Buffer *GetCameraConstantBuffer() { return(m_pd3dcbCamera); }

	void SetPosition(D3DXVECTOR3 d3dxvPosition) { m_d3dxvPosition = d3dxvPosition; }
	D3DXVECTOR3& GetPosition() { return(m_d3dxvPosition); }

	D3DXVECTOR3& GetRightVector() { return(m_d3dxvRight); }
	D3DXVECTOR3& GetUpVector() { return(m_d3dxvUp); }
	D3DXVECTOR3& GetLookVector() { return(m_d3dxvLook); }

	float& GetPitch() { return(m_fPitch); }
	float& GetRoll() { return(m_fRoll); }
	float& GetYaw() { return(m_fYaw); }

	void SetOffset(D3DXVECTOR3 d3dxvOffset) { m_d3dxvOffset = d3dxvOffset; m_d3dxvPosition += d3dxvOffset; }
	D3DXVECTOR3& GetOffset() { return(m_d3dxvOffset); }

	void SetTimeLag(float fTimeLag) { m_fTimeLag = fTimeLag; }
	float GetTimeLag() { return(m_fTimeLag); }

	virtual void Move(const D3DXVECTOR3& d3dxvShift) { m_d3dxvPosition += d3dxvShift; }
	virtual void Rotate(float fPitch = 0.0f, float fYaw = 0.0f, float fRoll = 0.0f) { }
	virtual void Update(D3DXVECTOR3& d3dxvLookAt, float fTimeElapsed) { }
	virtual void SetLookAt(D3DXVECTOR3& vLookAt) { }
	virtual void SetLookAt(D3DXVECTOR3& d3dxvPosition, D3DXVECTOR3& d3dxvLookAt, D3DXVECTOR3& vd3dxvUp);

	void CalculateFrustumPlanes();
	bool IsInFrustum(D3DXVECTOR3& d3dxvMinimum, D3DXVECTOR3& d3dxvMaximum);
	bool IsInFrustum(AABB *pAABB);
};

class CSpaceShipCamera : public CCamera
{
public:
	CSpaceShipCamera(CCamera *pCamera);
	virtual ~CSpaceShipCamera() { }

	virtual void Rotate(float fPitch = 0.0f, float fYaw = 0.0f, float fRoll = 0.0f);
};

class CFirstPersonCamera : public CCamera
{
public:
	CFirstPersonCamera(CCamera *pCamera);
	virtual ~CFirstPersonCamera() { }

	virtual void Rotate(float fPitch = 0.0f, float fYaw = 0.0f, float fRoll = 0.0f);
};

class CThirdPersonCamera : public CCamera
{
public:
	CThirdPersonCamera(CCamera *pCamera);
	virtual ~CThirdPersonCamera() { }

	virtual void Update(D3DXVECTOR3& d3dxvLookAt, float fTimeElapsed);
	virtual void SetLookAt(D3DXVECTOR3& vLookAt);
};

