#include "LNPC.h"
#include "string"
#include "LWrite.h"
#include "LGlobal.h"
#include "EnemyPatrol.h"
#include "EnemyTrace.h"
#include "EnemyDeath.h"


void LNPC::FSM(FSMType fsmType)
{
	auto iter = LFSMMgr::GetInstance().m_map.find(fsmType);

	if (LFSMMgr::GetInstance().m_map.end() == iter)
	{
		MessageBoxA(NULL, "FSMType Error", "Error Box", MB_OK);
		return;
	}

	m_pFsm = iter->second.get();

	m_pActionList.insert(std::make_pair(State::ENEMYPATROL, std::make_unique<EnemyPatrol>(this)));
	m_pActionList.insert(std::make_pair(State::ENEMYTRACE, std::make_unique<EnemyTrace>(this)));
	m_pActionList.insert(std::make_pair(State::ENEMYDEATH, std::make_unique<EnemyDeath>(this)));

	m_pAction = m_pActionList.find(State::ENEMYPATROL)->second.get();
	m_CurrentState = State::ENEMYPATROL;
}

void LNPC::SetTransition(Event inputEvent)
{
	m_CurrentState = m_pFsm->StateTransition(m_CurrentState, inputEvent);
	m_pAction = m_pActionList.find(m_CurrentState)->second.get();
}

State LNPC::GetState()
{
	return m_CurrentState;
}

void LNPC::Process()
{
	m_pAction->Process();
}

void LNPC::Move(TVector3 target)
{
	m_Dir = target - TVector3(m_matControl._41, m_matControl._42, m_matControl._43);
	m_Dir.Normalize();
	TVector3 forward = m_matControl.Forward();

	float dirX = m_Dir.x;
	float dirZ = m_Dir.z;

	/*float forwardX = forward.x;
	float forwardZ = forward.z;*/

	DirectX::XMVECTOR gRotation;
	DirectX::XMMATRIX matRotation;

	float yawRadians = atan2(dirZ, dirX);
	gRotation = DirectX::XMQuaternionRotationRollPitchYaw(0, -yawRadians - 1.5708, 0);
	//DirectX::XMMatrixRotationQuaternion(,)
	DirectX::XMVECTOR xmPos = DirectX::XMVectorSet(m_matControl._41, m_matControl._42, m_matControl._43, 1.0f);
	matRotation = DirectX::XMMatrixAffineTransformation(DirectX::g_XMOne, DirectX::g_XMZero, gRotation, xmPos);
	
	//D3DXMatrixRotationY(&m_matControl, yawRadians);
	m_matControl = matRotation;
	m_matControl._41 += m_Speed * LGlobal::g_fSPF * m_Dir.x;
	m_matControl._43 += m_Speed * LGlobal::g_fSPF * m_Dir.z;
}

int LNPC::GetRandomNumber()
{
	return m_Distribution(m_Generator);
}

bool LNPC::Frame()
{
	LSkinningModel::Frame();

	m_PlayerPos = { m_Player->m_matControl._41, m_Player->m_matControl._42, m_Player->m_matControl._43 };
	m_NPCPos = { m_matControl._41, m_matControl._42, m_matControl._43 };

	if (((m_NPCPos.x - m_PatrolRange) < m_PlayerPos.x) && (m_PlayerPos.x < (m_NPCPos.x + m_PatrolRange))
		&& ((m_NPCPos.z - m_PatrolRange) < m_PlayerPos.z) && (m_PlayerPos.z < (m_NPCPos.z + m_PatrolRange)))
	{
		IsFindPlayer = true;
	}

	if (((m_RandomPos.x - 50) < m_NPCPos.x) && (m_NPCPos.x < (m_RandomPos.x + 50)) &&
		((m_RandomPos.z - 50) < m_NPCPos.z) && (m_NPCPos.z < (m_RandomPos.z + 50)))
	{
		IsEndPatrol = true;
	}

	if (IsEndPatrol)
	{
		m_RandomPos = { float(GetRandomNumber()), 0.0f, float(GetRandomNumber()) };
		IsEndPatrol = false;
	}

	return true;
}

LNPC::LNPC(LPlayer* player) : m_Distribution(-500, 0)
{
	m_Player = player;

	unsigned seed = std::chrono::system_clock::now().time_since_epoch().count();
	m_Generator.seed(seed);

	m_RandomPos = { float(GetRandomNumber()), 0.0f, float(GetRandomNumber()) };
}