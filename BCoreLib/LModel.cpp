#include "LModel.h"
#include "LGlobal.h"

void LModel::SetAnimationArrayTexture()
{
	D3D11_TEXTURE2D_DESC texDesc;
	ZeroMemory(&texDesc, sizeof(texDesc));
	texDesc.Width = m_matBoneArraySize * 4;
	texDesc.Height = m_InstanceSize;
	texDesc.MipLevels = 1;
	texDesc.ArraySize = 1;
	texDesc.Format = DXGI_FORMAT_R32G32B32A32_FLOAT;
	texDesc.SampleDesc.Count = 1;
	texDesc.Usage = D3D11_USAGE_DEFAULT;
	texDesc.BindFlags = D3D11_BIND_SHADER_RESOURCE;
	texDesc.CPUAccessFlags = 0;
	texDesc.MiscFlags = 0;

	HRESULT hr = LGlobal::g_pDevice->CreateTexture2D(&texDesc, nullptr, pAnimationArrayTex.GetAddressOf());

	if (FAILED(hr))
	{
		MessageBoxA(NULL, "Create AnimationArrayTex 2DTexture Error", "Error Box", MB_OK);
		return;
	}

	return;
}

void LModel::SetAnimationArraySRV()
{
	D3D11_SHADER_RESOURCE_VIEW_DESC srvDesc;
	ZeroMemory(&srvDesc, sizeof(srvDesc));
	srvDesc.Format = DXGI_FORMAT_R32G32B32A32_FLOAT;
	srvDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
	srvDesc.Texture2D.MostDetailedMip = 0;
	srvDesc.Texture2D.MipLevels = 1;

	HRESULT hr = LGlobal::g_pDevice->CreateShaderResourceView(pAnimationArrayTex.Get(), &srvDesc, pAnimationArraySRV.GetAddressOf());

	if (FAILED(hr))
	{
		MessageBoxA(NULL, "Create AnimationArraySRV 2DTexture Error", "Error Box", MB_OK);
		return;
	}

	LGlobal::g_pImmediateContext->PSSetShaderResources(2, 1, pAnimationArraySRV.GetAddressOf());

	return;
}

void LModel::SetActionList(LFbxObj* fbxObj)
{
	m_ActionList.push_back(fbxObj);
}

void LModel::SetInstancing(bool IsIntancing, int instancingCount)
{
	auto fbxMeshList = m_pModel->m_DrawList;

	for (int iSub = 0; iSub < fbxMeshList.size(); iSub++)
	{
		fbxMeshList[iSub]->SetInstancing(IsIntancing);
	}

	m_InstanceSize = instancingCount;
	m_ActionList.resize(instancingCount);
	m_CurrentAnimTimeList.resize(instancingCount);
	m_matBoneArrayList.resize(instancingCount);
	m_matMeshBoneArrayList.resize(instancingCount);
}

void LModel::SetInstancing(bool IsIntancing)
{
	auto fbxMeshList = m_pModel->m_DrawList;

	for (int iSub = 0; iSub < fbxMeshList.size(); iSub++)
	{
		fbxMeshList[iSub]->SetInstancing(IsIntancing);
	}
}

void LModel::SetLFbxObj(LFbxObj* fbxObj)
{
	m_pModel = fbxObj;
}

LFbxObj* LModel::GetLFbxObj()
{
	return m_pModel;
}

bool LModel::CreateBoneBuffer()
{
	return true;
}

bool LModel::Frame()
{
	if (m_pModel == nullptr) return false;

	m_fCurrentAnimTime += m_pModel->m_iFrameSpeed * LGlobal::g_fSPF * 1.0f;

	if (m_fCurrentAnimTime >= m_pModel->m_iEndFrame)
	{
		m_fCurrentAnimTime = m_pModel->m_iStartFrame;
	}

	return true;
}

bool LModel::FrameInstancing()
{
	if (m_pModel == nullptr) return false;

	auto fbxMeshList = m_pModel->m_DrawList;
	for (int iSub = 0; iSub < fbxMeshList.size(); iSub++)
	{
		LFbxObj* obj = fbxMeshList[iSub].get();
		//obj->m_InstanceList[iSub].matWorld = m_matControl;
		D3DXMatrixTranspose(&obj->m_InstanceList[iSub].matWorld, &obj->m_InstanceList[iSub].matWorld);
		LGlobal::g_pImmediateContext->UpdateSubresource(obj->m_pCBInstance.Get(), 0, NULL, &obj->m_InstanceList.at(0), 0, 0);
		//LGlobal::g_pImmediateContext->VSSetConstantBuffers(3, 1, obj->m_pCBInstance.GetAddressOf());
	}

	return true;
}

bool LModel::Render()
{
	if (m_pModel == nullptr) return false;

	auto fbxMeshList = m_pModel->m_DrawList;

	for (int iSub = 0; iSub < fbxMeshList.size(); iSub++)
	{
		LFbxObj* obj = fbxMeshList[iSub].get();
		//TMatrix matWorld = m_pModel->m_MatrixArray[m_fCurrentAnimTime][obj->m_iBoneIndex] * m_matControl;
		obj->SetMatrix(&m_matControl, &LGlobal::g_pMainCamera->m_matView, &LGlobal::g_pMainCamera->m_matProj);
		obj->PreRender();
		UINT Strides[2] = { sizeof(SimpleVertex), sizeof(InstanceMatrix) };
		UINT Offsets[2] = { 0, };

		ComPtr<ID3D11Buffer> buffer[2] = { obj->m_pVertexBuffer.Get(), obj->m_pCBInstance.Get() };
		LGlobal::g_pImmediateContext->IASetVertexBuffers(0, 2, buffer->GetAddressOf(), Strides, Offsets);
		obj->PostRender();
	}

	return true;
}

bool LModel::ShadowRender(TMatrix* matShadowWorld, TMatrix* matShadowView, TMatrix* matShadowProj)
{
	if (m_pModel == nullptr) return false;

	auto fbxMeshList = m_pModel->m_DrawList;

	for (int iSub = 0; iSub < fbxMeshList.size(); iSub++)
	{
		LFbxObj* obj = fbxMeshList[iSub].get();
		//TMatrix matWorld = m_pModel->m_MatrixArray[m_fCurrentAnimTime][obj->m_iBoneIndex] * m_matControl;
		obj->SetMatrix(matShadowWorld, matShadowView, matShadowProj);
		obj->PreRender();
		obj->PostRender();
	}

	return true;
}

bool LModel::Release()
{
	return true;
}

bool LSkinningModel::CreateBoneBuffer()
{
	D3D11_BUFFER_DESC bufferDesc{};
	ZeroMemory(&bufferDesc, sizeof(bufferDesc));

	bufferDesc.ByteWidth = sizeof(LBoneWorld);
	bufferDesc.Usage = D3D11_USAGE_DEFAULT;
	bufferDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
	HRESULT hr = LGlobal::g_pDevice->CreateBuffer(
		&bufferDesc,
		nullptr,
		m_pBoneArrayCB.GetAddressOf());

	if (FAILED(hr))
	{
		MessageBoxA(NULL, "Create ConstantBuffer Error", "Error Box", MB_OK);
		return false;
	}

	return true;
}

bool LSkinningModel::AllNodeRender()
{
	for (int iNode = 0; iNode < m_pModel->m_TreeList.size(); iNode++)
	{
		LFbxObj* pFbxNode = m_pModel->m_TreeList[iNode].get();
		if (pFbxNode->m_Tex == nullptr) continue;

		m_matBoneArray.matBoneWorld[iNode] = m_pModel->m_MatrixArray[m_fCurrentAnimTime][iNode];
		D3DXMatrixTranspose(&m_matBoneArray.matBoneWorld[iNode],
			&m_matBoneArray.matBoneWorld[iNode]);
		pFbxNode->SetMatrix(&m_matBoneArray.matBoneWorld[iNode],
			&LGlobal::g_pMainCamera->m_matView,
			&LGlobal::g_pMainCamera->m_matProj);
		pFbxNode->PreRender();
		pFbxNode->PostRender();
	}
	return true;
}

bool LSkinningModel::Frame()
{
	if (m_pActionModel == nullptr) return false;

	m_fCurrentAnimTime += m_pModel->m_iFrameSpeed * LGlobal::g_fSPF * 1.0f;

	if (m_fCurrentAnimTime >= m_pActionModel->m_iEndFrame)
	{
		m_fCurrentAnimTime = m_pActionModel->m_iStartFrame;
	}

	for (int iNode = 0; iNode < m_pActionModel->m_AnimationTreeSize; iNode++)
	{
		std::wstring name = m_pModel->m_pFbxNodeNameList[iNode];
		m_matBoneArray.matBoneWorld[iNode] = m_pActionModel->m_NameMatrixMap[int(m_fCurrentAnimTime)][name];
	}

	return true;
}

bool LSkinningModel::FrameInstancing()
{
	for (int i = 0; i < m_InstanceSize; i++)
	{
		m_CurrentAnimTimeList[i] += m_pModel->m_iFrameSpeed * LGlobal::g_fSPF * 1.0f;

		if (m_CurrentAnimTimeList[i] >= m_ActionList[i]->m_iEndFrame)
		{
			m_CurrentAnimTimeList[i] = m_ActionList[i]->m_iStartFrame;
		}

		for (int iNode = 0; iNode < m_ActionList[i]->m_AnimationTreeSize; iNode++)
		{
			std::wstring name = m_pModel->m_pFbxNodeNameList[iNode];
			m_matBoneArrayList[i].matBoneWorld[iNode] = m_ActionList[i]->m_NameMatrixMap[int(m_CurrentAnimTimeList[i])][name];
		}
	}
	return true;
}

bool LSkinningModel::Render()
{
	if (m_pModel == nullptr) return false;

	auto fbxMeshList = m_pModel->m_DrawList;

	for (int iSub = 0; iSub < fbxMeshList.size(); iSub++)
	{
		LFbxObj* obj = fbxMeshList[iSub].get();

		for (auto data : m_pModel->m_pFbxNodeNameMap)
		{
			auto model = obj->m_dxMatrixBindPoseMap.find(data.first);

			if (model == obj->m_dxMatrixBindPoseMap.end())
			{
				continue;
			}

			TMatrix matBindPose = model->second;
			int iIndex = data.second;
			m_matMeshBoneArray.matBoneWorld[iIndex] = matBindPose * m_matBoneArray.matBoneWorld[iIndex];

			// hlsl���� dx�������� ������ ��ġ���Ѽ� �Ѱ���� �Ѵ�
			D3DXMatrixTranspose(&m_matMeshBoneArray.matBoneWorld[iIndex],
				&m_matMeshBoneArray.matBoneWorld[iIndex]);
		}

		// 71���߿� �ش��ϴ� ���� ���õ� �ִϸ��̼� ��ĸ� CB�� �Ѱ���	
		LGlobal::g_pImmediateContext->UpdateSubresource(m_pBoneArrayCB.Get(), 0, NULL, &m_matMeshBoneArray, 0, 0);
		LGlobal::g_pImmediateContext->VSSetConstantBuffers(1, 1, m_pBoneArrayCB.GetAddressOf());

		// ���� IA�����̶� �ؽ�ó ������ ���⼭ �Ѵ�.
		obj->PreRender();

		// ���ؽ� ���۸� �ΰ� ����Ѵٰ� ������ �� �Ѱ���
		UINT Strides[3] = { sizeof(SimpleVertex), sizeof(LVertexIW), sizeof(InstanceMatrix) };
		UINT Offsets[3] = { 0, };

		ComPtr<ID3D11Buffer> buffer[3] = { obj->m_pVertexBuffer.Get(), obj->m_pVBWeightList.Get(), obj->m_pCBInstance.Get() };
		LGlobal::g_pImmediateContext->IASetVertexBuffers(0, 3, buffer->GetAddressOf(), Strides, Offsets);

		obj->PostRender();
		// ������ �� �� ī�޶� �����̸� ī�޶��� ������� ������
		obj->SetMatrix(&m_matControl,
			&LGlobal::g_pMainCamera->m_matView,
			&LGlobal::g_pMainCamera->m_matProj);
	}

	return true;
}

bool LSkinningModel::RenderInstancing()
{
	if (m_pModel == nullptr) return false;

	auto fbxMeshList = m_pModel->m_DrawList;

	for (int iIns = 0; iIns < m_InstanceSize; iIns++)
	{
		for (int iSub = 0; iSub < fbxMeshList.size(); iSub++)
		{
			LFbxObj* obj = fbxMeshList[iSub].get();

			for (auto data : m_pModel->m_pFbxNodeNameMap)
			{
				auto model = obj->m_dxMatrixBindPoseMap.find(data.first);

				if (model == obj->m_dxMatrixBindPoseMap.end())
				{
					continue;
				}

				TMatrix matBindPose = model->second;
				int iIndex = data.second;

				m_matMeshBoneArrayList[iIns].matBoneWorld[iIndex] = matBindPose * m_matBoneArrayList[iIns].matBoneWorld[iIndex];

				// hlsl���� dx�������� ������ ��ġ���Ѽ� �Ѱ���� �Ѵ�
				D3DXMatrixTranspose(&m_matMeshBoneArrayList[iIns].matBoneWorld[iIndex],
					&m_matMeshBoneArrayList[iIns].matBoneWorld[iIndex]);
			}

			// 71���߿� �ش��ϴ� ���� ���õ� �ִϸ��̼� ��ĸ� CB�� �Ѱ���
			/*LGlobal::g_pImmediateContext->UpdateSubresource(m_pBoneArrayCB.Get(), 0, NULL, &m_matMeshBoneArray.matBoneWorld, 0, 0);
			LGlobal::g_pImmediateContext->VSSetConstantBuffers(1, 1, m_pBoneArrayCB.GetAddressOf());*/

			//LGlobal::g_pImmediateContext->UpdateSubresource(pAnimationArrayTex.Get(), 0, NULL, &m_matMeshBoneArrayList.at(0), 0, 0);

			// ���� IA�����̶� �ؽ�ó ������ ���⼭ �Ѵ�.
			obj->PreRender();

			// ���ؽ� ���۸� �ΰ� ����Ѵٰ� ������ �� �Ѱ���
			UINT Strides[3] = { sizeof(SimpleVertex), sizeof(LVertexIW), sizeof(InstanceMatrix) };
			UINT Offsets[3] = { 0, };

			ComPtr<ID3D11Buffer> buffer[3] = { obj->m_pVertexBuffer.Get(), obj->m_pVBWeightList.Get(), obj->m_pCBInstance.Get() };
			LGlobal::g_pImmediateContext->IASetVertexBuffers(0, 3, buffer->GetAddressOf(), Strides, Offsets);

			obj->PostRender();
			// ������ �� �� ī�޶� �����̸� ī�޶��� ������� ������
			obj->SetMatrix(&m_matControl,
				&LGlobal::g_pMainCamera->m_matView,
				&LGlobal::g_pMainCamera->m_matProj);
		}
	}
	
	return true;
}

bool LSkinningModel::Release()
{
	return true;
}

LModel::LModel()
{

}

LModel::~LModel()
{

}