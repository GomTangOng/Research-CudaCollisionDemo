//-----------------------------------------------------------------------------
// File: CGameObject.cpp
//-----------------------------------------------------------------------------

#include "stdafx.h"
#include "Object.h"
#include "Shader.h"

///////////////////////////////////////////////////////////////////////////
//
CMaterialColors::CMaterialColors()
{
	m_nReferences = 0;
	m_d3dxcAmbient = D3DXCOLOR(1.0f, 1.0f, 1.0f, 1.0f);
	m_d3dxcDiffuse = D3DXCOLOR(1.0f, 1.0f, 1.0f, 1.0f);
	m_d3dxcSpecular = D3DXCOLOR(1.0f, 1.0f, 1.0f, 1.0f);
	m_d3dxcEmissive = D3DXCOLOR(1.0f, 1.0f, 1.0f, 1.0f);
}

CMaterialColors::~CMaterialColors()
{
}

CMaterial::CMaterial(CMaterialColors *pColors)
{
	m_nReferences = 0;

	m_pColors = pColors;
	if (pColors) pColors->AddRef();

	m_pTexture = NULL;
}

CMaterial::~CMaterial()
{
	if (m_pColors) m_pColors->Release();
	if (m_pTexture) m_pTexture->Release();
}

void CMaterial::SetTexture(CTexture *pTexture)
{
	if (m_pTexture) m_pTexture->Release();
	m_pTexture = pTexture;
	if (m_pTexture) m_pTexture->AddRef();
}

void CMaterial::UpdateShaderVariable(ID3D11DeviceContext *pd3dDeviceContext)
{
	if (m_pColors) CGameObject::UpdateShaderVariable(pd3dDeviceContext, m_pColors);
	if (m_pTexture) m_pTexture->UpdateShaderVariable(pd3dDeviceContext);
}

///////////////////////////////////////////////////////////////////////////
//
ID3D11Buffer *CTexture::m_pd3dcbTextureMatrix = NULL;

CTexture::CTexture(int nTextures, int nSamplers, int nTextureStartSlot, int nSamplerStartSlot)
{
	m_nReferences = 0;

	m_nTextures = nTextures;
	m_ppd3dsrvTextures = new ID3D11ShaderResourceView*[m_nTextures];
	for (int i = 0; i < m_nTextures; i++) m_ppd3dsrvTextures[i] = NULL;
	m_nTextureStartSlot = nTextureStartSlot;

	m_nSamplers = nSamplers;
	m_ppd3dSamplerStates = new ID3D11SamplerState*[m_nSamplers];
	for (int i = 0; i < m_nSamplers; i++) m_ppd3dSamplerStates[i] = NULL;
	m_nSamplerStartSlot = nSamplerStartSlot;
}

CTexture::~CTexture()
{
	for (int i = 0; i < m_nTextures; i++) if (m_ppd3dsrvTextures[i]) m_ppd3dsrvTextures[i]->Release();
	for (int i = 0; i < m_nSamplers; i++) if (m_ppd3dSamplerStates[i]) m_ppd3dSamplerStates[i]->Release();
	if (m_ppd3dsrvTextures) delete[] m_ppd3dsrvTextures;
	if (m_ppd3dSamplerStates) delete[] m_ppd3dSamplerStates;
}

void CTexture::SetTexture(int nIndex, ID3D11ShaderResourceView *pd3dsrvTexture)
{
	if (m_ppd3dsrvTextures[nIndex]) m_ppd3dsrvTextures[nIndex]->Release();
	m_ppd3dsrvTextures[nIndex] = pd3dsrvTexture;
	if (pd3dsrvTexture) pd3dsrvTexture->AddRef();
}

void CTexture::SetSampler(int nIndex, ID3D11SamplerState *pd3dSamplerState)
{
	if (m_ppd3dSamplerStates[nIndex]) m_ppd3dSamplerStates[nIndex]->Release();
	m_ppd3dSamplerStates[nIndex] = pd3dSamplerState;
	if (pd3dSamplerState) pd3dSamplerState->AddRef();
}

void CTexture::UpdateShaderVariable(ID3D11DeviceContext *pd3dDeviceContext)
{
	pd3dDeviceContext->PSSetShaderResources(m_nTextureStartSlot, m_nTextures, m_ppd3dsrvTextures);
	pd3dDeviceContext->PSSetSamplers(m_nSamplerStartSlot, m_nSamplers, m_ppd3dSamplerStates);
}

void CTexture::UpdateTextureShaderVariable(ID3D11DeviceContext *pd3dDeviceContext, int nIndex, int nSlot)
{
	pd3dDeviceContext->PSSetShaderResources(nSlot, 1, &m_ppd3dsrvTextures[nIndex]);
}

void CTexture::UpdateSamplerShaderVariable(ID3D11DeviceContext *pd3dDeviceContext, int nIndex, int nSlot)
{
	pd3dDeviceContext->PSSetSamplers(nSlot, 1, &m_ppd3dSamplerStates[nIndex]);
}

void CTexture::CreateShaderVariables(ID3D11Device *pd3dDevice)
{
	D3D11_BUFFER_DESC d3dBufferDesc;
	ZeroMemory(&d3dBufferDesc, sizeof(D3D11_BUFFER_DESC));
	d3dBufferDesc.Usage = D3D11_USAGE_DYNAMIC;
	d3dBufferDesc.ByteWidth = sizeof(D3DXMATRIX);
	d3dBufferDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
	d3dBufferDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
	pd3dDevice->CreateBuffer(&d3dBufferDesc, NULL, &m_pd3dcbTextureMatrix);
}

void CTexture::ReleaseShaderVariables()
{
	if (m_pd3dcbTextureMatrix) m_pd3dcbTextureMatrix->Release();
}

void CTexture::UpdateShaderVariable(ID3D11DeviceContext *pd3dDeviceContext, D3DXMATRIX *pd3dxmtxTexture)
{
	D3D11_MAPPED_SUBRESOURCE d3dMappedResource;
	pd3dDeviceContext->Map(m_pd3dcbTextureMatrix, 0, D3D11_MAP_WRITE_DISCARD, 0, &d3dMappedResource);
	D3DXMATRIX *pcbWorldMatrix = (D3DXMATRIX *)d3dMappedResource.pData;
	D3DXMatrixTranspose(pcbWorldMatrix, pd3dxmtxTexture);
	pd3dDeviceContext->Unmap(m_pd3dcbTextureMatrix, 0);
	pd3dDeviceContext->VSSetConstantBuffers(VS_CB_SLOT_TEXTURE_MATRIX, 1, &m_pd3dcbTextureMatrix);
}

ID3D11ShaderResourceView *CTexture::CreateTexture2DArraySRV(ID3D11Device *pd3dDevice, _TCHAR(*ppstrFilePaths)[128], UINT nTextures)
{
	D3DX11_IMAGE_LOAD_INFO d3dxImageLoadInfo;
	d3dxImageLoadInfo.Width = D3DX11_FROM_FILE;
	d3dxImageLoadInfo.Height = D3DX11_FROM_FILE;
	d3dxImageLoadInfo.Depth = D3DX11_FROM_FILE;
	d3dxImageLoadInfo.FirstMipLevel = 0;
	d3dxImageLoadInfo.MipLevels = D3DX11_FROM_FILE;
	d3dxImageLoadInfo.Usage = D3D11_USAGE_STAGING;
	d3dxImageLoadInfo.BindFlags = 0;
	d3dxImageLoadInfo.CpuAccessFlags = D3D11_CPU_ACCESS_WRITE | D3D11_CPU_ACCESS_READ;
	d3dxImageLoadInfo.MiscFlags = 0;
	d3dxImageLoadInfo.Format = DXGI_FORMAT_FROM_FILE; //DXGI_FORMAT_R8G8B8A8_UNORM
	d3dxImageLoadInfo.Filter = D3DX11_FILTER_NONE;
	d3dxImageLoadInfo.MipFilter = D3DX11_FILTER_LINEAR;
	d3dxImageLoadInfo.pSrcInfo = 0;

	ID3D11Texture2D **ppd3dTextures = new ID3D11Texture2D*[nTextures];
	for (UINT i = 0; i < nTextures; i++) D3DX11CreateTextureFromFile(pd3dDevice, ppstrFilePaths[i], &d3dxImageLoadInfo, 0, (ID3D11Resource **)&ppd3dTextures[i], 0);

	D3D11_TEXTURE2D_DESC d3dTexure2DDesc;
	ppd3dTextures[0]->GetDesc(&d3dTexure2DDesc);

	D3D11_TEXTURE2D_DESC d3dTexture2DArrayDesc;
	d3dTexture2DArrayDesc.Width = d3dTexure2DDesc.Width;
	d3dTexture2DArrayDesc.Height = d3dTexure2DDesc.Height;
	d3dTexture2DArrayDesc.MipLevels = d3dTexure2DDesc.MipLevels;
	d3dTexture2DArrayDesc.ArraySize = nTextures;
	d3dTexture2DArrayDesc.Format = d3dTexure2DDesc.Format;
	d3dTexture2DArrayDesc.SampleDesc.Count = 1;
	d3dTexture2DArrayDesc.SampleDesc.Quality = 0;
	d3dTexture2DArrayDesc.Usage = D3D11_USAGE_DEFAULT;
	d3dTexture2DArrayDesc.BindFlags = D3D11_BIND_SHADER_RESOURCE;
	d3dTexture2DArrayDesc.CPUAccessFlags = 0;
	d3dTexture2DArrayDesc.MiscFlags = 0;

	ID3D11Texture2D *pd3dTexture2DArray;
	pd3dDevice->CreateTexture2D(&d3dTexture2DArrayDesc, 0, &pd3dTexture2DArray);

	ID3D11DeviceContext *pd3dDeviceContext;
	pd3dDevice->GetImmediateContext(&pd3dDeviceContext);

	D3D11_MAPPED_SUBRESOURCE d3dMappedTexture2D;
	for (UINT t = 0; t < nTextures; t++)
	{
		for (UINT m = 0; m < d3dTexure2DDesc.MipLevels; m++)
		{
			pd3dDeviceContext->Map(ppd3dTextures[t], m, D3D11_MAP_READ, 0, &d3dMappedTexture2D);
			pd3dDeviceContext->UpdateSubresource(pd3dTexture2DArray, D3D11CalcSubresource(m, t, d3dTexure2DDesc.MipLevels), 0, d3dMappedTexture2D.pData, d3dMappedTexture2D.RowPitch, d3dMappedTexture2D.DepthPitch);
			pd3dDeviceContext->Unmap(ppd3dTextures[t], m);
		}
	}

	D3D11_SHADER_RESOURCE_VIEW_DESC d3dTextureSRVDesc;
	ZeroMemory(&d3dTextureSRVDesc, sizeof(D3D11_SHADER_RESOURCE_VIEW_DESC));
	d3dTextureSRVDesc.Format = d3dTexture2DArrayDesc.Format;
	d3dTextureSRVDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2DARRAY;
	d3dTextureSRVDesc.Texture2DArray.MostDetailedMip = 0;
	d3dTextureSRVDesc.Texture2DArray.MipLevels = d3dTexture2DArrayDesc.MipLevels;
	d3dTextureSRVDesc.Texture2DArray.FirstArraySlice = 0;
	d3dTextureSRVDesc.Texture2DArray.ArraySize = nTextures;

	ID3D11ShaderResourceView *pd3dsrvTextureArray;
	pd3dDevice->CreateShaderResourceView(pd3dTexture2DArray, &d3dTextureSRVDesc, &pd3dsrvTextureArray);

	if (pd3dTexture2DArray) pd3dTexture2DArray->Release();

	for (UINT i = 0; i < nTextures; i++) if (ppd3dTextures[i]) ppd3dTextures[i]->Release();
	delete[] ppd3dTextures;

	if (pd3dDeviceContext) pd3dDeviceContext->Release();

	return(pd3dsrvTextureArray);
}

///////////////////////////////////////////////////////////////////////////
//
ID3D11Buffer *CGameObject::m_pd3dcbWorldMatrix = NULL;
ID3D11Buffer *CGameObject::m_pd3dcbMaterialColors = NULL;
D3DXMATRIX CGameObject::m_d3dxmtxIdentity;

CGameObject::CGameObject(int nMeshes)
{
	D3DXMatrixIdentity(&m_d3dxmtxLocal);
	D3DXMatrixIdentity(&m_d3dxmtxWorld);

	D3DXMatrixIdentity(&m_d3dxmtxIdentity);

	m_pShader = NULL;
	m_pMaterial = NULL;

	m_nMeshes = nMeshes;
	m_ppMeshes = NULL;
	if (m_nMeshes > 0) m_ppMeshes = new CMesh*[m_nMeshes];
	for (int i = 0; i < m_nMeshes; i++)	m_ppMeshes[i] = NULL;

	m_bcMeshBoundingCube = AABB();
	m_bcBoundingCube = AABB();

	m_bActive = true;
	m_bIsVisible = true;

	m_nReferences = 0;

	m_pChild = m_pSibling = m_pParent = NULL;

	m_pd3dDepthStencilState = NULL;
	m_pd3dRasterizerState = NULL;
	m_pd3dBlendState = NULL;
}

CGameObject::~CGameObject()
{
	if (m_pd3dRasterizerState) m_pd3dRasterizerState->Release();
	if (m_pd3dDepthStencilState) m_pd3dDepthStencilState->Release();
	if (m_pd3dBlendState) m_pd3dBlendState->Release();

	if (m_pShader) m_pShader->Release();

	if (m_pMaterial) m_pMaterial->Release();

	if (m_ppMeshes)
	{
		for (int i = 0; i < m_nMeshes; i++)
		{
			if (m_ppMeshes[i]) m_ppMeshes[i]->Release();
			m_ppMeshes[i] = NULL;
		}
		delete[] m_ppMeshes;
	}

	if (m_pSibling) delete m_pSibling;
	if (m_pChild) delete m_pChild;
}

void CGameObject::SetMesh(CMesh *pMesh, int nIndex)
{
	if (nIndex >= m_nMeshes)
	{
		CMesh **ppMeshes = new CMesh*[nIndex + 1];
		if (m_ppMeshes)
		{
			for (int i = 0; i < m_nMeshes; i++) ppMeshes[i] = m_ppMeshes[i];
			delete[] m_ppMeshes;
		}
		ppMeshes[nIndex] = pMesh;
		if (pMesh) pMesh->AddRef();
		m_nMeshes = nIndex + 1;
		m_ppMeshes = ppMeshes;
	}
	else
	{
		if (m_ppMeshes)
		{
			if (m_ppMeshes[nIndex]) m_ppMeshes[nIndex]->Release();
			m_ppMeshes[nIndex] = pMesh;
			if (pMesh) pMesh->AddRef();
		}
	}

	if (pMesh)
	{
		AABB bcBoundingCube = pMesh->GetBoundingCube();
		m_bcMeshBoundingCube.Merge(&bcBoundingCube);
	}
}

void CGameObject::SetChild(CGameObject *pChild)
{
	if (m_pChild)
	{
		if (pChild) pChild->m_pSibling = m_pChild->m_pSibling;
		m_pChild->m_pSibling = pChild;
	}
	else
	{
		m_pChild = pChild;
	}
	if (pChild) pChild->m_pParent = this;
}

void CGameObject::SetShader(CShader *pShader)
{
	if (m_pShader) m_pShader->Release();
	m_pShader = pShader;
	if (m_pShader) m_pShader->AddRef();
}

void CGameObject::SetMaterial(CMaterial *pMaterial)
{
	if (m_pMaterial) m_pMaterial->Release();
	m_pMaterial = pMaterial;
	if (m_pMaterial) m_pMaterial->AddRef();
}

void CGameObject::SetShadowMatrix(D3DXVECTOR4 d3dxvLight, D3DXPLANE d3dxPlane)
{
	D3DXMatrixShadow(&m_d3dxmtxShadow, &d3dxvLight, &d3dxPlane);
}

void CGameObject::GenerateRayForPicking(D3DXVECTOR3 *pd3dxvPickPosition, D3DXMATRIX *pd3dxmtxWorld, D3DXMATRIX *pd3dxmtxView, D3DXVECTOR3 *pd3dxvPickRayPosition, D3DXVECTOR3 *pd3dxvPickRayDirection)
{
	D3DXMATRIX d3dxmtxInverse;
	D3DXMATRIX d3dxmtxWorldView = *pd3dxmtxView;
	if (pd3dxmtxWorld) D3DXMatrixMultiply(&d3dxmtxWorldView, pd3dxmtxWorld, pd3dxmtxView);
	D3DXMatrixInverse(&d3dxmtxInverse, NULL, &d3dxmtxWorldView);
	/*
	pd3dxvPickRayDirection->x = pd3dxvPickPosition->x * d3dxmtxInverse._11 + pd3dxvPickPosition->y * d3dxmtxInverse._21 + pd3dxvPickPosition->z * d3dxmtxInverse._31;
	pd3dxvPickRayDirection->y = pd3dxvPickPosition->x * d3dxmtxInverse._12 + pd3dxvPickPosition->y * d3dxmtxInverse._22 + pd3dxvPickPosition->z * d3dxmtxInverse._32;
	pd3dxvPickRayDirection->z = pd3dxvPickPosition->x * d3dxmtxInverse._13 + pd3dxvPickPosition->y * d3dxmtxInverse._23 + pd3dxvPickPosition->z * d3dxmtxInverse._33;
	pd3dxvPickRayPosition->x = d3dxmtxInverse._41;
	pd3dxvPickRayPosition->y = d3dxmtxInverse._42;
	pd3dxvPickRayPosition->z = d3dxmtxInverse._43;
	*/
	D3DXVECTOR3 d3dxvCameraOrigin(0.0f, 0.0f, 0.0f);
	D3DXVec3TransformCoord(pd3dxvPickRayPosition, &d3dxvCameraOrigin, &d3dxmtxInverse);
	D3DXVec3TransformCoord(pd3dxvPickRayDirection, pd3dxvPickPosition, &d3dxmtxInverse);
	*pd3dxvPickRayDirection = *pd3dxvPickRayDirection - *pd3dxvPickRayPosition;
}

int CGameObject::PickObjectByRayIntersection(D3DXVECTOR3 *pd3dxvPickPosition, D3DXMATRIX *pd3dxmtxView, MESHINTERSECTINFO *pd3dxIntersectInfo)
{
	D3DXVECTOR3 d3dxvPickRayPosition, d3dxvPickRayDirection;
	int nIntersected = 0;
	if (m_bActive && m_bIsVisible && m_ppMeshes)
	{
		GenerateRayForPicking(pd3dxvPickPosition, &m_d3dxmtxWorld, pd3dxmtxView, &d3dxvPickRayPosition, &d3dxvPickRayDirection);
		for (int i = 0; i < m_nMeshes; i++)
		{
			nIntersected = m_ppMeshes[i]->CheckRayIntersection(&d3dxvPickRayPosition, &d3dxvPickRayDirection, pd3dxIntersectInfo);
			if (nIntersected > 0) break;
		}
	}
	return(nIntersected);
}

void CGameObject::SetPosition(float x, float y, float z)
{
	m_d3dxmtxLocal._41 = x;
	m_d3dxmtxLocal._42 = y;
	m_d3dxmtxLocal._43 = z;
	
	if (!m_pParent) { m_d3dxmtxWorld._41 = x; m_d3dxmtxWorld._42 = y; m_d3dxmtxWorld._43 = z; }
}

void CGameObject::SetPosition(D3DXVECTOR3 d3dxvPosition)
{
	SetPosition(d3dxvPosition.x, d3dxvPosition.y, d3dxvPosition.z);
}

void CGameObject::MoveStrafe(float fDistance)
{
	D3DXVECTOR3 d3dxvPosition = GetPosition();
	D3DXVECTOR3 d3dxvRight = GetRight();
	d3dxvPosition += fDistance * d3dxvRight;
	CGameObject::SetPosition(d3dxvPosition);
}

void CGameObject::MoveUp(float fDistance)
{
	D3DXVECTOR3 d3dxvPosition = GetPosition();
	D3DXVECTOR3 d3dxvUp = GetUp();
	d3dxvPosition += fDistance * d3dxvUp;
	CGameObject::SetPosition(d3dxvPosition);
}

void CGameObject::MoveForward(float fDistance)
{
	D3DXVECTOR3 d3dxvPosition = GetPosition();
	D3DXVECTOR3 d3dxvLookAt = GetLook();
	d3dxvPosition += fDistance * d3dxvLookAt;
	CGameObject::SetPosition(d3dxvPosition);
}

void CGameObject::Rotate(float fPitch, float fYaw, float fRoll)
{
	D3DXMATRIX mtxRotate;
	D3DXMatrixRotationYawPitchRoll(&mtxRotate, (float)D3DXToRadian(fYaw), (float)D3DXToRadian(fPitch), (float)D3DXToRadian(fRoll));
	m_d3dxmtxLocal = mtxRotate * m_d3dxmtxLocal;
	
	if (!m_pParent) m_d3dxmtxWorld = m_d3dxmtxLocal;
}

void CGameObject::Rotate(D3DXVECTOR3 *pd3dxvAxis, float fAngle)
{
	D3DXMATRIX mtxRotate;
	D3DXMatrixRotationAxis(&mtxRotate, pd3dxvAxis, (float)D3DXToRadian(fAngle));
	m_d3dxmtxLocal = mtxRotate * m_d3dxmtxLocal;
	
	if (!m_pParent) m_d3dxmtxWorld = m_d3dxmtxLocal;
}

D3DXVECTOR3 CGameObject::GetPosition(bool bIsLocal)
{
	return((bIsLocal) ? D3DXVECTOR3(m_d3dxmtxLocal._41, m_d3dxmtxLocal._42, m_d3dxmtxLocal._43) : D3DXVECTOR3(m_d3dxmtxWorld._41, m_d3dxmtxWorld._42, m_d3dxmtxWorld._43));
}

D3DXVECTOR3 CGameObject::GetRight(bool bIsLocal)
{
	D3DXVECTOR3 d3dxvRight = (bIsLocal) ? D3DXVECTOR3(m_d3dxmtxLocal._11, m_d3dxmtxLocal._12, m_d3dxmtxLocal._13) : D3DXVECTOR3(m_d3dxmtxWorld._11, m_d3dxmtxWorld._12, m_d3dxmtxWorld._13);
	D3DXVec3Normalize(&d3dxvRight, &d3dxvRight);
	return(d3dxvRight);
}

D3DXVECTOR3 CGameObject::GetUp(bool bIsLocal)
{
	D3DXVECTOR3 d3dxvUp = (bIsLocal) ? D3DXVECTOR3(m_d3dxmtxLocal._21, m_d3dxmtxLocal._22, m_d3dxmtxLocal._23) : D3DXVECTOR3(m_d3dxmtxWorld._21, m_d3dxmtxWorld._22, m_d3dxmtxWorld._23);
	D3DXVec3Normalize(&d3dxvUp, &d3dxvUp);
	return(d3dxvUp);
}

D3DXVECTOR3 CGameObject::GetLook(bool bIsLocal)
{
	D3DXVECTOR3 d3dxvLookAt = (bIsLocal) ? D3DXVECTOR3(m_d3dxmtxLocal._31, m_d3dxmtxLocal._32, m_d3dxmtxLocal._33) : D3DXVECTOR3(m_d3dxmtxWorld._31, m_d3dxmtxWorld._32, m_d3dxmtxWorld._33);
	D3DXVec3Normalize(&d3dxvLookAt, &d3dxvLookAt);
	return(d3dxvLookAt);
}

void CGameObject::CreateShaderVariables(ID3D11Device *pd3dDevice)
{
	D3D11_BUFFER_DESC d3dBufferDesc;
	ZeroMemory(&d3dBufferDesc, sizeof(D3D11_BUFFER_DESC));
	d3dBufferDesc.Usage = D3D11_USAGE_DYNAMIC;
	d3dBufferDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
	d3dBufferDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
	d3dBufferDesc.ByteWidth = sizeof(VS_CB_WORLD_MATRIX);
	pd3dDevice->CreateBuffer(&d3dBufferDesc, NULL, &m_pd3dcbWorldMatrix);

	d3dBufferDesc.ByteWidth = sizeof(D3DXCOLOR) * 4;
	pd3dDevice->CreateBuffer(&d3dBufferDesc, NULL, &m_pd3dcbMaterialColors);
}

void CGameObject::ReleaseShaderVariables()
{
	if (m_pd3dcbWorldMatrix) m_pd3dcbWorldMatrix->Release();
	if (m_pd3dcbMaterialColors) m_pd3dcbMaterialColors->Release();
}

void CGameObject::UpdateShaderVariable(ID3D11DeviceContext *pd3dDeviceContext, D3DXMATRIX *pd3dxmtxWorld)
{
	D3D11_MAPPED_SUBRESOURCE d3dMappedResource;
	pd3dDeviceContext->Map(m_pd3dcbWorldMatrix, 0, D3D11_MAP_WRITE_DISCARD, 0, &d3dMappedResource);
	VS_CB_WORLD_MATRIX *pcbWorldMatrix = (VS_CB_WORLD_MATRIX *)d3dMappedResource.pData;
	D3DXMatrixTranspose(&pcbWorldMatrix->m_d3dxmtxWorld, pd3dxmtxWorld);
	pd3dDeviceContext->Unmap(m_pd3dcbWorldMatrix, 0);
	pd3dDeviceContext->VSSetConstantBuffers(VS_CB_SLOT_WORLD_MATRIX, 1, &m_pd3dcbWorldMatrix);
}

void CGameObject::UpdateShaderVariable(ID3D11DeviceContext *pd3dDeviceContext, CMaterialColors *pMaterial)
{
	D3D11_MAPPED_SUBRESOURCE d3dMappedResource;
	pd3dDeviceContext->Map(m_pd3dcbMaterialColors, 0, D3D11_MAP_WRITE_DISCARD, 0, &d3dMappedResource);
	D3DXCOLOR *pcbMaterial = (D3DXCOLOR *)d3dMappedResource.pData;
	memcpy(pcbMaterial, &pMaterial->m_d3dxcAmbient, sizeof(D3DXCOLOR));
	memcpy(pcbMaterial + 1, &pMaterial->m_d3dxcDiffuse, sizeof(D3DXCOLOR));
	memcpy(pcbMaterial + 2, &pMaterial->m_d3dxcSpecular, sizeof(D3DXCOLOR));
	memcpy(pcbMaterial + 3, &pMaterial->m_d3dxcEmissive, sizeof(D3DXCOLOR));
	pd3dDeviceContext->Unmap(m_pd3dcbMaterialColors, 0);
	pd3dDeviceContext->PSSetConstantBuffers(PS_CB_SLOT_MATERIAL, 1, &m_pd3dcbMaterialColors);
}

void CGameObject::CreateRasterizerState(ID3D11Device *pd3dDevice)
{
	D3D11_RASTERIZER_DESC d3dRasterizerDesc;
	ZeroMemory(&d3dRasterizerDesc, sizeof(D3D11_RASTERIZER_DESC));
	d3dRasterizerDesc.CullMode = D3D11_CULL_BACK;
	d3dRasterizerDesc.FillMode = D3D11_FILL_SOLID;
	//	d3dRasterizerDesc.FillMode = D3D11_FILL_WIREFRAME;
	d3dRasterizerDesc.DepthClipEnable = true;
	pd3dDevice->CreateRasterizerState(&d3dRasterizerDesc, &m_pd3dRasterizerState);
}

bool CGameObject::IsVisible(CCamera *pCamera)
{
	OnPrepareRender();
	Update(NULL);

	m_bIsVisible = (m_bActive) ? true : false;
#ifdef _WITH_FRUSTUM_CULLING_BY_OBJECT
	if (m_bActive)
	{
		AABB bcBoundingCube = m_bcMeshBoundingCube;
		bcBoundingCube.Update(&m_d3dxmtxWorld);
		if (pCamera) m_bIsVisible = pCamera->IsInFrustum(&bcBoundingCube);

		if (m_pSibling) m_bIsVisible |= m_pSibling->IsVisible(pCamera);
		if (m_pChild) m_bIsVisible |= m_pChild->IsVisible(pCamera);
	}
#endif
	return(m_bIsVisible);
}

void CGameObject::Animate(float fTimeElapsed)
{
	if (m_pSibling) m_pSibling->Animate(fTimeElapsed);
	if (m_pChild) m_pChild->Animate(fTimeElapsed);
}

void CGameObject::Update(D3DXMATRIX *pd3dxmtxParent)
{
	m_d3dxmtxWorld = m_d3dxmtxLocal;
	if (pd3dxmtxParent) D3DXMatrixMultiply(&m_d3dxmtxWorld, &m_d3dxmtxLocal, pd3dxmtxParent);

	if (m_pSibling) m_pSibling->Update(pd3dxmtxParent);
	if (m_pChild) m_pChild->Update(&m_d3dxmtxWorld);
}

void CGameObject::Animate(float fTimeElapsed, D3DXMATRIX *pd3dxmtxParent)
{
	Animate(fTimeElapsed);

	Update(pd3dxmtxParent);
}

void CGameObject::OnPrepareRender()
{
}

void CGameObject::RenderMesh(ID3D11DeviceContext *pd3dDeviceContext, CCamera *pCamera)
{
	if (m_ppMeshes)
	{
		for (int i = 0; i < m_nMeshes; i++)
		{
			if (m_ppMeshes[i])
			{
				bool bIsVisible = true;
#ifdef _WITH_FRUSTUM_CULLING_BY_SUBMESH
				if (pCamera)
				{
					AABB bcBoundingCube = m_ppMeshes[i]->GetBoundingCube();
					bcBoundingCube.Update(&m_d3dxmtxWorld);
					bIsVisible = pCamera->IsInFrustum(&bcBoundingCube);
				}
#endif
				if (bIsVisible) m_ppMeshes[i]->Render(pd3dDeviceContext);
			}
		}
	}
}

void CGameObject::Render(ID3D11DeviceContext *pd3dDeviceContext, CCamera *pCamera)
{
	OnPrepareRender();
	Update(NULL);

	if (m_pShader) m_pShader->Render(pd3dDeviceContext, pCamera);

	CGameObject::UpdateShaderVariable(pd3dDeviceContext, &m_d3dxmtxWorld);
	if (m_pMaterial) m_pMaterial->UpdateShaderVariable(pd3dDeviceContext);

	pd3dDeviceContext->RSSetState(m_pd3dRasterizerState);
	pd3dDeviceContext->OMSetDepthStencilState(m_pd3dDepthStencilState, 0);
	if (m_pd3dBlendState) pd3dDeviceContext->OMSetBlendState(m_pd3dBlendState, NULL, 0xffffffff);

	RenderMesh(pd3dDeviceContext, pCamera);

	if (m_pSibling) m_pSibling->Render(pd3dDeviceContext, pCamera);
	if (m_pChild) m_pChild->Render(pd3dDeviceContext, pCamera);

	if (m_pd3dDepthStencilState) pd3dDeviceContext->OMSetDepthStencilState(NULL, 0);
	if (m_pd3dBlendState) pd3dDeviceContext->OMSetBlendState(NULL, NULL, 0xffffffff);
}

////////////////////////////////////////////////////////////////////////////////////////////////////
//
CRotatingObject::CRotatingObject(int nMeshes) : CGameObject(nMeshes)
{
	m_d3dxvRotationAxis = D3DXVECTOR3(0.0f, 1.0f, 0.0f);
	m_fRotationSpeed = 15.0f;
}

CRotatingObject::~CRotatingObject()
{
}

void CRotatingObject::Animate(float fTimeElapsed)
{
	CGameObject::Rotate(&m_d3dxvRotationAxis, m_fRotationSpeed * fTimeElapsed);
}

////////////////////////////////////////////////////////////////////////////////////////////////////
//
CRevolvingObject::CRevolvingObject(int nMeshes) : CGameObject(nMeshes)
{
	m_d3dxvRevolutionAxis = D3DXVECTOR3(1.0f, 0.0f, 0.0f);
	m_fRevolutionSpeed = 0.0f;
}

CRevolvingObject::~CRevolvingObject()
{
}

void CRevolvingObject::Animate(float fTimeElapsed)
{
	D3DXMATRIX mtxRotate;
	D3DXMatrixRotationAxis(&mtxRotate, &m_d3dxvRevolutionAxis, (float)D3DXToRadian(m_fRevolutionSpeed * fTimeElapsed));
	m_d3dxmtxLocal = m_d3dxmtxLocal * mtxRotate;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// CHeightMap
//
CHeightMap::CHeightMap(LPCTSTR pFileName, int nWidth, int nLength, D3DXVECTOR3 d3dxvScale)
{
	m_nWidth = nWidth;
	m_nLength = nLength;
	m_d3dxvScale = d3dxvScale;

	BYTE *pHeightMapImage = new BYTE[m_nWidth * m_nLength];

	HANDLE hFile = ::CreateFile(pFileName, GENERIC_READ, 0, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL | FILE_ATTRIBUTE_READONLY, NULL);
	DWORD dwBytesRead;
	::ReadFile(hFile, pHeightMapImage, (m_nWidth * m_nLength), &dwBytesRead, NULL);
	::CloseHandle(hFile);

	m_pHeightMapImage = new BYTE[m_nWidth * m_nLength];
	for (int y = 0; y < m_nLength; y++)
	{
		for (int x = 0; x < m_nWidth; x++)
		{
			m_pHeightMapImage[x + ((m_nLength - 1 - y)*m_nWidth)] = pHeightMapImage[x + (y*m_nWidth)];
		}
	}

	if (pHeightMapImage) delete[] pHeightMapImage;
}

CHeightMap::~CHeightMap()
{
	if (m_pHeightMapImage) delete[] m_pHeightMapImage;
	m_pHeightMapImage = NULL;
}

D3DXVECTOR3 CHeightMap::GetHeightMapNormal(int x, int z)
{
	if ((x < 0.0f) || (z < 0.0f) || (x >= m_nWidth) || (z >= m_nLength)) return(D3DXVECTOR3(0.0f, 1.0f, 0.0f));

	int nHeightMapIndex = x + (z * m_nWidth);
	int xHeightMapAdd, zHeightMapAdd;
	xHeightMapAdd = (x < (m_nWidth - 1)) ? 1 : -1;
	zHeightMapAdd = (z < (m_nLength - 1)) ? m_nWidth : -m_nWidth;
	float y1 = (float)m_pHeightMapImage[nHeightMapIndex] * m_d3dxvScale.y;
	float y2 = (float)m_pHeightMapImage[nHeightMapIndex + xHeightMapAdd] * m_d3dxvScale.y;
	float y3 = (float)m_pHeightMapImage[nHeightMapIndex + zHeightMapAdd] * m_d3dxvScale.y;
	D3DXVECTOR3 vEdge1 = D3DXVECTOR3(0.0f, y3 - y1, m_d3dxvScale.z);
	D3DXVECTOR3 vEdge2 = D3DXVECTOR3(m_d3dxvScale.x, y2 - y1, 0.0f);
	D3DXVECTOR3 vNormal;
	D3DXVec3Cross(&vNormal, &vEdge1, &vEdge2);
	D3DXVec3Normalize(&vNormal, &vNormal);
	return(vNormal);
}

#define _WITH_APPROXIMATE_OPPOSITE_CORNER

float CHeightMap::GetHeight(float fx, float fz, bool bReverseQuad)
{
	fx = fx / m_d3dxvScale.x;
	fz = fz / m_d3dxvScale.z;
	if ((fx < 0.0f) || (fz < 0.0f) || (fx >= m_nWidth) || (fz >= m_nLength)) return(0.0f);

	int x = (int)fx;
	int z = (int)fz;
	float fxPercent = fx - x;
	float fzPercent = fz - z;

	float fBottomLeft = (float)m_pHeightMapImage[x + (z*m_nWidth)];
	float fBottomRight = (float)m_pHeightMapImage[(x + 1) + (z*m_nWidth)];
	float fTopLeft = (float)m_pHeightMapImage[x + ((z + 1)*m_nWidth)];
	float fTopRight = (float)m_pHeightMapImage[(x + 1) + ((z + 1)*m_nWidth)];
#ifdef _WITH_APPROXIMATE_OPPOSITE_CORNER
	if (bReverseQuad)
	{
		if (fzPercent >= fxPercent)
			fBottomRight = fBottomLeft + (fTopRight - fTopLeft);
		else
			fTopLeft = fTopRight + (fBottomLeft - fBottomRight);
	}
	else
	{
		if (fzPercent < (1.0f - fxPercent))
			fTopRight = fTopLeft + (fBottomRight - fBottomLeft);
		else
			fBottomLeft = fTopLeft + (fBottomRight - fTopRight);
	}
#endif
	float fTopHeight = fTopLeft * (1 - fxPercent) + fTopRight * fxPercent;
	float fBottomHeight = fBottomLeft * (1 - fxPercent) + fBottomRight * fxPercent;
	float fHeight = fBottomHeight * (1 - fzPercent) + fTopHeight * fzPercent;

	return(fHeight);
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// CHeightMapTerrain

CHeightMapTerrain::CHeightMapTerrain(ID3D11Device *pd3dDevice, LPCTSTR pFileName, int nWidth, int nLength, int nBlockWidth, int nBlockLength, D3DXVECTOR3 d3dxvScale) : CGameObject(0)
{
	m_nWidth = nWidth;
	m_nLength = nLength;

	int cxQuadsPerBlock = nBlockWidth - 1;
	int czQuadsPerBlock = nBlockLength - 1;

	m_d3dxvScale = d3dxvScale;

	m_pHeightMap = new CHeightMap(pFileName, nWidth, nLength, d3dxvScale);

	long cxBlocks = (m_nWidth - 1) / cxQuadsPerBlock;
	long czBlocks = (m_nLength - 1) / czQuadsPerBlock;
	m_nMeshes = cxBlocks * czBlocks;
	m_ppMeshes = new CMesh*[m_nMeshes];
	for (int i = 0; i < m_nMeshes; i++)	m_ppMeshes[i] = NULL;

	CHeightMapGridMesh *pHeightMapGridMesh = NULL;
	for (int z = 0, zStart = 0; z < czBlocks; z++)
	{
		for (int x = 0, xStart = 0; x < cxBlocks; x++)
		{
			xStart = x * (nBlockWidth - 1);
			zStart = z * (nBlockLength - 1);
			pHeightMapGridMesh = new CHeightMapGridMesh(pd3dDevice, xStart, zStart, nBlockWidth, nBlockLength, d3dxvScale, m_pHeightMap);
			SetMesh(pHeightMapGridMesh, x + (z*cxBlocks));
		}
	}

#ifdef _WITH_TERRAIN_TEXTURE_ARRAY
	D3D11_BUFFER_DESC d3dBufferDesc;
	ZeroMemory(&d3dBufferDesc, sizeof(d3dBufferDesc));
	d3dBufferDesc.Usage = D3D11_USAGE_DYNAMIC;
	d3dBufferDesc.ByteWidth = sizeof(int) * 4;
	d3dBufferDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
	d3dBufferDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
	D3D11_SUBRESOURCE_DATA d3dBufferData;
	ZeroMemory(&d3dBufferData, sizeof(D3D11_SUBRESOURCE_DATA));
	int pnBuffers[4] = { 0, 0, 0, 0 };
	d3dBufferData.pSysMem = pnBuffers;
	pd3dDevice->CreateBuffer(&d3dBufferDesc, &d3dBufferData, &m_pd3dcbTextureIndex);

	ID3D11DeviceContext *pd3dDeviceContext;
	pd3dDevice->GetImmediateContext(&pd3dDeviceContext);
	pd3dDeviceContext->PSSetConstantBuffers(PS_CB_SLOT_TERRAIN, 1, &m_pd3dcbTextureIndex);
	if (pd3dDeviceContext) pd3dDeviceContext->Release();
#endif
}

CHeightMapTerrain::~CHeightMapTerrain(void)
{
#ifdef _WITH_TERRAIN_TEXTURE_ARRAY
	if (m_pd3dcbTextureIndex) m_pd3dcbTextureIndex->Release();
#endif
	if (m_pHeightMap) delete m_pHeightMap;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// 
CTerrainWater::CTerrainWater(ID3D11Device *pd3dDevice, int nWidth, int nLength, int nBlockWidth, int nBlockLength, D3DXVECTOR3 d3dxvScale)
{
	m_nWidth = nWidth;
	m_nLength = nLength;

	int cxQuadsPerBlock = nBlockWidth - 1;
	int czQuadsPerBlock = nBlockLength - 1;

	m_d3dxvScale = d3dxvScale;

	long cxBlocks = (m_nWidth - 1) / cxQuadsPerBlock;
	long czBlocks = (m_nLength - 1) / czQuadsPerBlock;
	m_nMeshes = cxBlocks * czBlocks;
	m_ppMeshes = new CMesh*[m_nMeshes];
	for (int i = 0; i < m_nMeshes; i++)	m_ppMeshes[i] = NULL;

	CWaterGridMesh *pWaterGridMesh = NULL;
	for (int z = 0, zStart = 0; z < czBlocks; z++)
	{
		for (int x = 0, xStart = 0; x < cxBlocks; x++)
		{
			xStart = x * (nBlockWidth - 1);
			zStart = z * (nBlockLength - 1);
			pWaterGridMesh = new CWaterGridMesh(pd3dDevice, xStart, zStart, nBlockWidth, nBlockLength, d3dxvScale, NULL, D3D11_USAGE_DYNAMIC);
			SetMesh(pWaterGridMesh, x + (z*cxBlocks));
		}
	}

	D3DXMatrixIdentity(&m_d3dxmtxTexture);
}

CTerrainWater::~CTerrainWater()
{
}

void CTerrainWater::Animate(float fTimeElapsed)
{
	static D3DXVECTOR3 d3dxOffset(0.0f, 0.0f, 0.0f);
	d3dxOffset.y += 0.005f * 0.001f;
	d3dxOffset.x = 0.0025f * sinf(4.0f * d3dxOffset.y);
	m_d3dxmtxTexture._41 += d3dxOffset.x;
	m_d3dxmtxTexture._42 += d3dxOffset.y;
}

void CTerrainWater::Render(ID3D11DeviceContext *pd3dDeviceContext, CCamera *pCamera)
{
	CTexture::UpdateShaderVariable(pd3dDeviceContext, &m_d3dxmtxTexture);

	CGameObject::Render(pd3dDeviceContext, pCamera);
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// 
CSkyBox::CSkyBox(ID3D11Device *pd3dDevice) : CGameObject(1)
{
	D3D11_DEPTH_STENCIL_DESC d3dDepthStencilDesc;
	ZeroMemory(&d3dDepthStencilDesc, sizeof(D3D11_DEPTH_STENCIL_DESC));
	d3dDepthStencilDesc.DepthEnable = false;
	d3dDepthStencilDesc.DepthWriteMask = D3D11_DEPTH_WRITE_MASK_ZERO;
	d3dDepthStencilDesc.DepthFunc = D3D11_COMPARISON_NEVER;
	d3dDepthStencilDesc.StencilEnable = false;
	d3dDepthStencilDesc.StencilReadMask = 0xFF;
	d3dDepthStencilDesc.StencilWriteMask = 0xFF;
	d3dDepthStencilDesc.FrontFace.StencilFailOp = D3D11_STENCIL_OP_KEEP;
	d3dDepthStencilDesc.FrontFace.StencilDepthFailOp = D3D11_STENCIL_OP_INCR;
	d3dDepthStencilDesc.FrontFace.StencilPassOp = D3D11_STENCIL_OP_KEEP;
	d3dDepthStencilDesc.FrontFace.StencilFunc = D3D11_COMPARISON_ALWAYS;
	d3dDepthStencilDesc.BackFace.StencilFailOp = D3D11_STENCIL_OP_KEEP;
	d3dDepthStencilDesc.BackFace.StencilDepthFailOp = D3D11_STENCIL_OP_DECR;
	d3dDepthStencilDesc.BackFace.StencilPassOp = D3D11_STENCIL_OP_KEEP;
	d3dDepthStencilDesc.BackFace.StencilFunc = D3D11_COMPARISON_ALWAYS;
	pd3dDevice->CreateDepthStencilState(&d3dDepthStencilDesc, &m_pd3dDepthStencilState);

#ifdef _WITH_SKYBOX_TEXTURE_ARRAY
	D3D11_BUFFER_DESC d3dBufferDesc;
	ZeroMemory(&d3dBufferDesc, sizeof(d3dBufferDesc));
	d3dBufferDesc.Usage = D3D11_USAGE_DYNAMIC;
	d3dBufferDesc.ByteWidth = sizeof(int) * 4;
	d3dBufferDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
	d3dBufferDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
	pd3dDevice->CreateBuffer(&d3dBufferDesc, NULL, &m_pd3dcbTextureIndex);

	ID3D11DeviceContext *pd3dDeviceContext;
	pd3dDevice->GetImmediateContext(&pd3dDeviceContext);
	pd3dDeviceContext->PSSetConstantBuffers(PS_CB_SLOT_SKYBOX, 1, &m_pd3dcbTextureIndex);
	if (pd3dDeviceContext) pd3dDeviceContext->Release();
#endif
}

CSkyBox::~CSkyBox()
{
#ifdef _WITH_SKYBOX_TEXTURE_ARRAY
	if (m_pd3dcbTextureIndex) m_pd3dcbTextureIndex->Release();
#endif
}

void CSkyBox::Render(ID3D11DeviceContext *pd3dDeviceContext, CCamera *pCamera)
{
	D3DXVECTOR3 d3dxvCameraPos = pCamera->GetPosition();
	SetPosition(d3dxvCameraPos.x, d3dxvCameraPos.y, d3dxvCameraPos.z);
	Update(NULL);

	CGameObject::UpdateShaderVariable(pd3dDeviceContext, &m_d3dxmtxWorld);

	if (m_pShader) m_pShader->Render(pd3dDeviceContext, pCamera);

	pd3dDeviceContext->RSSetState(m_pd3dRasterizerState);

	CSkyBoxMesh *pSkyBoxMesh = (CSkyBoxMesh *)m_ppMeshes[0];
	pSkyBoxMesh->OnPrepareRender(pd3dDeviceContext);

#ifdef _WITH_SKYBOX_TEXTURE_ARRAY
	m_pMaterial->m_pTexture->UpdateShaderVariable(pd3dDeviceContext);
#else
#ifdef _WITH_SKYBOX_TEXTURE_CUBE
	m_pMaterial->m_pTexture->UpdateShaderVariable(pd3dDeviceContext);
#else
	m_pMaterial->m_pTexture->UpdateSamplerShaderVariable(pd3dDeviceContext, 0, PS_SLOT_SAMPLER_SKYBOX);
#endif
#endif
	pd3dDeviceContext->OMSetDepthStencilState(m_pd3dDepthStencilState, 1);

	for (int i = 0; i < 6; i++)
	{
#ifdef _WITH_SKYBOX_TEXTURE_ARRAY
		D3D11_MAPPED_SUBRESOURCE d3dMappedResource;
		pd3dDeviceContext->Map(m_pd3dcbTextureIndex, 0, D3D11_MAP_WRITE_DISCARD, 0, &d3dMappedResource);
		int *pfIndex = (int *)d3dMappedResource.pData;
		pfIndex[0] = pfIndex[1] = pfIndex[2] = pfIndex[3] = i;
		pd3dDeviceContext->Unmap(m_pd3dcbTextureIndex, 0);
#else
#ifndef _WITH_SKYBOX_TEXTURE_CUBE
		m_pMaterial->m_pTexture->UpdateTextureShaderVariable(pd3dDeviceContext, i, PS_SLOT_TEXTURE_SKYBOX);
#endif
#endif
		pd3dDeviceContext->DrawIndexed(4, 0, i * 4);
	}

	pd3dDeviceContext->OMSetDepthStencilState(NULL, 1);
}



CBillboardObject::CBillboardObject(ID3D11Device *pd3dDevice) : CGameObject(0)
{
	//CreateRasterizerState(pd3dDevice);
	//CreateBlendState(pd3dDevice);
}

CBillboardObject::~CBillboardObject()
{
}

void CBillboardObject::Animate(float fTimeElapsed, CCamera * pCamera)
{
	D3DXVECTOR3 d3dxvCameraPosition = pCamera->GetPosition();
	SetLookAt(d3dxvCameraPosition);
}

void CBillboardObject::SetLookAt(D3DXVECTOR3 & d3dxvTarget)
{
	D3DXVECTOR3 d3dxvRight, d3dxvUp(0.0f, 1.0f, 0.0f);
	D3DXVECTOR3 d3dxvPosition(m_d3dxmtxWorld._41, m_d3dxmtxWorld._42, m_d3dxmtxWorld._43);
	D3DXVECTOR3 d3dxvLook = d3dxvTarget - d3dxvPosition;
	D3DXVec3Normalize(&d3dxvLook, &d3dxvLook);
	D3DXVec3Cross(&d3dxvRight, &d3dxvUp, &d3dxvLook);
	D3DXVec3Normalize(&d3dxvRight, &d3dxvRight);

	D3DXVECTOR3 d3dxvOldLook(m_d3dxmtxWorld._31, m_d3dxmtxWorld._32, m_d3dxmtxWorld._33);

	float angle = D3DXVec3Dot(&d3dxvLook, &d3dxvOldLook);

	Rotate(&d3dxvUp, angle);
}

void CBillboardObject::CreateRasterizerState(ID3D11Device *pd3dDevice)
{
	D3D11_RASTERIZER_DESC d3dRasterizerDesc;
	ZeroMemory(&d3dRasterizerDesc, sizeof(D3D11_RASTERIZER_DESC));
	d3dRasterizerDesc.CullMode = D3D11_CULL_NONE;
	d3dRasterizerDesc.FillMode = D3D11_FILL_SOLID;
	//	d3dRasterizerDesc.FillMode = D3D11_FILL_WIREFRAME;
	d3dRasterizerDesc.DepthClipEnable = true;
	pd3dDevice->CreateRasterizerState(&d3dRasterizerDesc, &m_pd3dRasterizerState);
}

void CBillboardObject::CreateBlendState(ID3D11Device *pd3dDevice)
{
	D3D11_BLEND_DESC d3dBlendDesc;
	ZeroMemory(&d3dBlendDesc, sizeof(d3dBlendDesc));

	d3dBlendDesc.AlphaToCoverageEnable = false;
	d3dBlendDesc.IndependentBlendEnable = false;
	d3dBlendDesc.RenderTarget[0].BlendEnable = false;
	d3dBlendDesc.RenderTarget[0].RenderTargetWriteMask = D3D11_COLOR_WRITE_ENABLE_ALL;

	pd3dDevice->CreateBlendState(&d3dBlendDesc, &m_pd3dBlendState);
}