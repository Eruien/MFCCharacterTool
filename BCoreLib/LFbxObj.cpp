#include "LFbxObj.h"

void LFbxObj::SetInstancing(bool IsIntancing)
{
	m_bInstancing = IsIntancing;
}

void LFbxObj::SetAnimation(int startFrame, int endFrame, int frameSpeed, int tickForFrame)
{
	m_iStartFrame = startFrame;
	m_iEndFrame = endFrame;
	m_iFrameSpeed = frameSpeed;
	m_iTickForFrame = tickForFrame;
}

void LFbxObj::SetDefaultFilePath(std::wstring filePath)
{
	m_DefaultFilePath = filePath;
}

void LFbxObj::LoadTexture(std::wstring texFileName)
{
	LTexture* tex = LManager<LTexture>::GetInstance().Load(texFileName.c_str());
	m_TexArray.push_back(tex);
}

bool LFbxObj::CreateVertexData()
{
	CreateInstance(2);
	return true;
}

bool LFbxObj::CreateIndexData()
{
	return true;
}

bool LFbxObj::CreateVertexBuffer()
{
	LDXObject::CreateVertexBuffer();

	D3D11_BUFFER_DESC bufferDesc;
	ZeroMemory(&bufferDesc, sizeof(bufferDesc));
	bufferDesc.ByteWidth = sizeof(LVertexIW) * m_pVertexListIW.size();
	bufferDesc.Usage = D3D11_USAGE_DEFAULT;
	bufferDesc.BindFlags = D3D11_BIND_VERTEX_BUFFER;

	D3D11_SUBRESOURCE_DATA initialData;
	ZeroMemory(&initialData, sizeof(initialData));
	initialData.pSysMem = &m_pVertexListIW.at(0);

	HRESULT hr = m_pDevice->CreateBuffer(
		&bufferDesc,
		&initialData,
		m_pVBWeightList.GetAddressOf());

	if (FAILED(hr))
	{
		MessageBoxA(NULL, "Create VertexBuffer Error", "Error Box", MB_OK);
		return false;
	}

	return true;
}

bool LFbxObj::CreateLayout()
{
	D3D11_INPUT_ELEMENT_DESC layout[] =
	{
		{ "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0,  0, D3D11_INPUT_PER_VERTEX_DATA, 0 },
		{ "NORMAL",   0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 12, D3D11_INPUT_PER_VERTEX_DATA, 0 },
		{ "COLOR",    0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, 24, D3D11_INPUT_PER_VERTEX_DATA, 0 },
		{ "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT,    0, 40, D3D11_INPUT_PER_VERTEX_DATA, 0 },

		{ "INDEX", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 1,  0, D3D11_INPUT_PER_VERTEX_DATA, 0 },
		{ "WEIGHT", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 1, 16, D3D11_INPUT_PER_VERTEX_DATA, 0 },
		{ "TANGENT", 0, DXGI_FORMAT_R32G32B32_FLOAT, 1, 32, D3D11_INPUT_PER_VERTEX_DATA, 0 },

		{ "TRANSFORM", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 2, 0,  D3D11_INPUT_PER_INSTANCE_DATA, 1 },
		{ "TRANSFORM", 1, DXGI_FORMAT_R32G32B32A32_FLOAT, 2, 16, D3D11_INPUT_PER_INSTANCE_DATA, 1 },
		{ "TRANSFORM", 2, DXGI_FORMAT_R32G32B32A32_FLOAT, 2, 32, D3D11_INPUT_PER_INSTANCE_DATA, 1 },
		{ "TRANSFORM", 3, DXGI_FORMAT_R32G32B32A32_FLOAT, 2, 48, D3D11_INPUT_PER_INSTANCE_DATA, 1 },
	};

	HRESULT hr = m_pDevice->CreateInputLayout(
		layout, sizeof(layout) / sizeof(layout[0]),
		m_Shader->m_pVSBlob->GetBufferPointer(),
		m_Shader->m_pVSBlob->GetBufferSize(),
		m_pVertexLayout.GetAddressOf());

	if (FAILED(hr))
	{
		MessageBoxA(NULL, "Create Input Layout Error", "Error Box", MB_OK);
		return false;
	}

	return true;
}

bool LFbxObj::CreateLayoutInstancing()
{
	D3D11_INPUT_ELEMENT_DESC layout[] =
	{
		{ "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0,  0, D3D11_INPUT_PER_VERTEX_DATA, 0 },
		{ "NORMAL",   0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 12, D3D11_INPUT_PER_VERTEX_DATA, 0 },
		{ "COLOR",    0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, 24, D3D11_INPUT_PER_VERTEX_DATA, 0 },
		{ "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT,    0, 40, D3D11_INPUT_PER_VERTEX_DATA, 0 },

		{ "INDEX", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 1,  0, D3D11_INPUT_PER_VERTEX_DATA, 0 },
		{ "WEIGHT", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 1, 16, D3D11_INPUT_PER_VERTEX_DATA, 0 },
		{ "TANGENT", 0, DXGI_FORMAT_R32G32B32_FLOAT, 1, 32, D3D11_INPUT_PER_VERTEX_DATA, 0 },

		{ "TRANSFORM", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 2, 0,  D3D11_INPUT_PER_INSTANCE_DATA, 1 },
		{ "TRANSFORM", 1, DXGI_FORMAT_R32G32B32A32_FLOAT, 2, 16, D3D11_INPUT_PER_INSTANCE_DATA, 1 },
		{ "TRANSFORM", 2, DXGI_FORMAT_R32G32B32A32_FLOAT, 2, 32, D3D11_INPUT_PER_INSTANCE_DATA, 1 },
		{ "TRANSFORM", 3, DXGI_FORMAT_R32G32B32A32_FLOAT, 2, 48, D3D11_INPUT_PER_INSTANCE_DATA, 1 },
	};

	HRESULT hr = m_pDevice->CreateInputLayout(
		layout, sizeof(layout) / sizeof(layout[0]),
		m_Shader->m_pVSBlob->GetBufferPointer(),
		m_Shader->m_pVSBlob->GetBufferSize(),
		m_pVertexLayout.GetAddressOf());

	if (FAILED(hr))
	{
		MessageBoxA(NULL, "Create Input Layout Error", "Error Box", MB_OK);
		return false;
	}

	return true;
}

bool LFbxObj::CreateInstance(int numInstance)
{
	m_InstanceSize = numInstance;
	m_InstanceList.resize(m_InstanceSize);

	for (int i = 0; i < m_InstanceSize; i++)
	{
		D3DXMatrixIdentity(&m_InstanceList[i].matWorld);
		m_InstanceList[i].matWorld._41 = i * 50;
		D3DXMatrixTranspose(&m_InstanceList[i].matWorld, &m_InstanceList[i].matWorld);
	}

	CreateInstanceBuffer();

	return true;
}

bool LFbxObj::CreateInstanceBuffer()
{
	D3D11_BUFFER_DESC bufferDesc{};
	ZeroMemory(&bufferDesc, sizeof(bufferDesc));

	bufferDesc.ByteWidth = sizeof(InstanceMatrix) * m_InstanceList.size();
	bufferDesc.Usage = D3D11_USAGE_DEFAULT;
	bufferDesc.BindFlags = D3D11_BIND_VERTEX_BUFFER;

	D3D11_SUBRESOURCE_DATA initialData;
	ZeroMemory(&initialData, sizeof(initialData));
	initialData.pSysMem = &m_InstanceList.at(0);

	HRESULT hr = m_pDevice->CreateBuffer(
		&bufferDesc,
		&initialData,
		m_pCBInstance.GetAddressOf());

	if (FAILED(hr))
	{
		MessageBoxA(NULL, "Create ConstantBuffer Error", "Error Box", MB_OK);
		return false;
	}
}

bool LFbxObj::PostRender()
{
	for (int iSubMtrl = 0; iSubMtrl < m_SubMtrlCount; iSubMtrl++)
	{
		if (m_TexArray.size() && m_TexArray[iSubMtrl]->m_pTexSRV)
		{
			m_TexArray[iSubMtrl]->Apply();
		}

		if (m_pIndexBuffer == nullptr)
		{
			if (m_bInstancing)
			{
				m_pImmediateContext->DrawInstanced(m_VertexList.size(),
					m_InstanceSize, m_TriangleOffsetList[iSubMtrl], 0);
			}
			else
			{
				m_pImmediateContext->Draw(m_SubMtrlVertexList[iSubMtrl],
					m_TriangleOffsetList[iSubMtrl]);
			}
		}
 	}

	return true;
}
