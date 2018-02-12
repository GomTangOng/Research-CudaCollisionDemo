//-----------------------------------------------------------------------------
// File: Shader.h
//-----------------------------------------------------------------------------

#pragma once

#include "Object.h"
#include "Camera.h"
#include "Player.h"

#define _WITH_INSTANCING_CULLING
#ifdef _WITH_INSTANCING_CULLING
#define _WITH_INSTANCING_FROM_OBJECTS_DYNAMICALLY
#endif

struct VS_CB_WORLD_MATRIX
{
	D3DXMATRIX						m_d3dxmtxWorld;
};

struct VS_VB_INSTANCE
{
	D3DXMATRIX						m_d3dxTransform;
};

class CShader
{
public:
	CShader();
	virtual ~CShader();

private:
	int								m_nReferences;

public:
	void AddRef() { m_nReferences++; }
	void Release() { if (--m_nReferences <= 0) delete this; }

protected:
	UINT							m_nType;
	int								m_nInputElements;
	D3D11_INPUT_ELEMENT_DESC		*m_pd3dInputElementDescs;

	ID3D11VertexShader				*m_pd3dVertexShader;
	ID3D11InputLayout				*m_pd3dVertexLayout;

	ID3D11PixelShader				*m_pd3dPixelShader;

	ID3D11GeometryShader			*m_pd3dGeometryShader;

protected:
	void CreateVertexShaderFromFile(ID3D11Device *pd3dDevice, WCHAR *pszFileName, LPCSTR pszShaderName, LPCSTR pszShaderModel, ID3D11VertexShader **ppd3dVertexShader, D3D11_INPUT_ELEMENT_DESC *pd3dInputElements, UINT nElements, ID3D11InputLayout **ppd3dInputLayout);
	void CreatePixelShaderFromFile(ID3D11Device *pd3dDevice, WCHAR *pszFileName, LPCSTR pszShaderName, LPCSTR pszShaderModel, ID3D11PixelShader **ppd3dPixelShader);
	void CreateGeometryShaderFromFile(ID3D11Device *pd3dDevice, WCHAR *pszFileName, LPCSTR pszShaderName, LPCSTR pszShaderModel, ID3D11GeometryShader **ppd3dGeometryShader);

public:
	virtual void GetInputElementDesc(UINT nVertexElementType);
	virtual void GetShaderName(UINT nVertexElementType, LPCSTR *ppszVSShaderName, LPCSTR *ppszVSShaderModel, LPCSTR *ppszPSShaderName, LPCSTR *ppszPSShaderModel);

	virtual void CreateShader(ID3D11Device *pd3dDevice) { }
	virtual void CreateShader(ID3D11Device *pd3dDevice, UINT nType);
	virtual void CreateShader(ID3D11Device *pd3dDevice, D3D11_INPUT_ELEMENT_DESC *pd3dInputElementDesc, int nInputElements, WCHAR *pszFileName, LPCSTR pszVSShaderName, LPCSTR pszVSShaderModel, LPCSTR pszPSShaderName, LPCSTR pszPSShaderModel);

	virtual void CreateShaderVariables(ID3D11Device *pd3dDevice);
	virtual void ReleaseShaderVariables();

	virtual void OnPrepareRender(ID3D11DeviceContext *pd3dDeviceContext);
	virtual void Render(ID3D11DeviceContext *pd3dDeviceContext, CCamera *pCamera);

public:
	static ID3D11Buffer *CreateBuffer(ID3D11Device *pd3dDevice, UINT nStride, int nElements, void *pBufferData, UINT nBindFlags, D3D11_USAGE d3dUsage, UINT nCPUAccessFlags);
};

////////////////////////////////////////////////////////////////////////////////
//
class CTerrainShader : public CShader
{
public:
	CTerrainShader();
	virtual ~CTerrainShader();

	virtual void CreateShader(ID3D11Device *pd3dDevice);
};

class CSkyBoxShader : public CShader
{
public:
	CSkyBoxShader();
	virtual ~CSkyBoxShader();

	virtual void CreateShader(ID3D11Device *pd3dDevice);
};

class CWaterShader : public CShader
{
public:
	CWaterShader();
	virtual ~CWaterShader();

	virtual void CreateShader(ID3D11Device *pd3dDevice);
};

////////////////////////////////////////////////////////////////////////////////
//
class CObjectsShader : public CShader
{
public:
	CObjectsShader(int nObjects = 1);
	virtual ~CObjectsShader();

protected:
	CGameObject						**m_ppObjects;
	int								m_nObjects;

	CMaterial						*m_pMaterial;

	void							*m_pContext;

private:
	int								m_nIndexToAdd;

public:
	virtual void BuildObjects(ID3D11Device *pd3dDevice, void *pContext = NULL);
	virtual void ReleaseObjects();
	virtual void AnimateObjects(float fTimeElapsed);
	virtual void OnPrepareRender(ID3D11DeviceContext *pd3dDeviceContext);
	virtual void Render(ID3D11DeviceContext *pd3dDeviceContext, CCamera *pCamera);

	void SetObject(int nIndex, CGameObject *pGameObject);
	void AddObject(CGameObject *pGameObject);
	void SetMaterial(CMaterial *pMaterial);
	CGameObject *PickObjectByRayIntersection(D3DXVECTOR3 *pd3dxvPickPosition, D3DXMATRIX *pd3dxmtxView, MESHINTERSECTINFO *pd3dxIntersectInfo);
};

class CInstancedObjectsShader : public CObjectsShader
{
public:
	CInstancedObjectsShader(int nObjects = 1);
	virtual ~CInstancedObjectsShader();

	void SetMesh(CMesh *pMesh);

	virtual void CreateShader(ID3D11Device *pd3dDevice);
	virtual void CreateShader(ID3D11Device *pd3dDevice, UINT nType);

	virtual void BuildObjects(ID3D11Device *pd3dDevice, void *pContext = NULL);
	virtual void ReleaseObjects();
	virtual void Render(ID3D11DeviceContext *pd3dDeviceContext, CCamera *pCamera);

protected:
	CMesh							*m_pMesh;

	UINT							m_nInstanceBufferStride;
	UINT							m_nInstanceBufferOffset;

	ID3D11Buffer					*m_pd3dInstanceBuffer;
};

class CBillboardInstancedObjectShader : public CInstancedObjectsShader
{
public :
	CBillboardInstancedObjectShader(int nObjects = 1);
	virtual ~CBillboardInstancedObjectShader();

	void AnimateObjects(float fTimeElapsed, CCamera *pCamera);
};