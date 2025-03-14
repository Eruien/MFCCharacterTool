#include "EnemyPatrol.h"
#include "LGlobal.h"
#include "LFbxMgr.h"

bool EnemyPatrol::Init()
{
    return true;
}

void EnemyPatrol::Process()
{
    if (m_pOwner->IsDead)
    {
        m_pOwner->SetTransition(Event::FATALDAMAGE);
        return;
    }

    if (m_pOwner->IsFindPlayer)
    {
        m_pOwner->SetTransition(Event::FINDPLAYER);
        return;
    }

    m_pOwner->Move(m_pOwner->m_RandomPos);
    m_pOwner->m_pActionModel = LFbxMgr::GetInstance().GetPtr(L"Zombie_Walk_Lock.fbx");
}

void EnemyPatrol::Release()
{

}

EnemyPatrol::EnemyPatrol(LNPC* parent) : NPCState(parent)
{

}
EnemyPatrol::~EnemyPatrol() {}