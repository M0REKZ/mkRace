/* (c) Magnus Auvinen. See licence.txt in the root of the distribution for more information. */
/* If you are missing that file, acquire a complete release at teeworlds.com.                */

//+KZ modified this file

#include <generated/server_data.h>
#include <game/server/gamecontext.h>
#include <game/server/player.h>

#include <game/server/entities/character.h>
#include "golfball.h"

CGolfBall::CGolfBall(CGameWorld *pGameWorld, vec2 Pos, vec2 Direction, float StartEnergy, int Owner, int Type)
: CEntity(pGameWorld, CGameWorld::KZ_ENTTYPE_GOLFBALL, Pos)
{
	m_Owner = Owner;
	m_Energy = StartEnergy;
	m_Dir = Direction;
	m_Bounces = 0;
	m_EvalTick = 0;
	m_Type = Type;
	m_StartTick = Server()->Tick();
	GameWorld()->InsertEntity(this);
	DoBounce();
}

void CGolfBall::DoBounce()
{
	CCharacter *pOwnerChar = GameServer()->GetPlayerChar(m_Owner);
	if(!pOwnerChar)
	{
		GameWorld()->DestroyEntity(this);
		return;
	}

	m_EvalTick = Server()->Tick();

	if(m_Energy < 0)
	{
		TeleportOwnerTo(m_Pos);
		GameWorld()->DestroyEntity(this);
		return;
	}

	m_PrevPos = m_Pos;
	vec2 To = m_Pos + m_Dir * maximum(5.f, (100.f - 0.6f * (Server()->Tick() - m_StartTick)));

	vec2 teleintersect;
	int t = 0;
	if(GameServer()->Collision()->IntersectLineTeleWeapon(m_Pos, To, &teleintersect, nullptr, &t) && t != 0)
	{
		int Num = GameServer()->Collision()->m_TeleOuts[t-1].size();
		if(Num > 0)
		{
			TeleportOwnerTo(GameServer()->Collision()->m_TeleOuts[t-1][(!Num)?Num:rand() % Num]);
			GameWorld()->DestroyEntity(this);
			return;
		}
	}

	if(GameServer()->Collision()->IntersectLine(m_Pos, To, 0x0, &To))
	{
		// intersected
		m_From = m_Pos;
		m_Pos = To;

		vec2 TempPos = m_Pos;
		vec2 TempDir = m_Dir * 4.0f;

		GameServer()->Collision()->MovePoint(&TempPos, &TempDir, 1.0f, 0);
		m_Pos = TempPos;
		m_Dir = normalize(TempDir);

		m_Energy -= distance(m_From, m_Pos);
		m_Bounces++;

		GameServer()->CreateSound(m_Pos, SOUND_LASER_BOUNCE, CmaskRace(GameServer(), m_Owner));
	}
	else
	{
		m_From = m_Pos;
		m_Pos = To;
		m_Energy -= distance(m_From, m_Pos);
	}
}

void CGolfBall::TeleportOwnerTo(vec2 ToPos)
{
	CCharacter *pOwnerChar = GameServer()->GetPlayerChar(m_Owner);
	if(!pOwnerChar)
		return;

	vec2 NewPos = vec2((ToPos.x - (round_to_int(ToPos.x) % 32)) + 16.f, (ToPos.y - (round_to_int(ToPos.y) % 32)) + 16.f);

	if(!GameServer()->Collision()->TestBox(vec2(NewPos.x, ToPos.y), vec2(pOwnerChar->ms_PhysSize,pOwnerChar->ms_PhysSize)))
	{
		NewPos.y = ToPos.y;
	}

	if(!GameServer()->Collision()->TestBox(vec2(ToPos.x, NewPos.y), vec2(pOwnerChar->ms_PhysSize,pOwnerChar->ms_PhysSize)))
	{
		NewPos.x = ToPos.x;
	}
		
	pOwnerChar->SetPos(NewPos);
	pOwnerChar->m_Pos = NewPos;
}

void CGolfBall::Reset()
{
	GameWorld()->DestroyEntity(this);
}

void CGolfBall::Tick()
{
	if((Server()->Tick() - m_EvalTick) > 4)
		DoBounce();
}

void CGolfBall::TickPaused()
{
	++m_EvalTick;
}

void CGolfBall::Snap(int SnappingClient)
{
	if(!CheckShowOthers(SnappingClient, m_Owner))
		return;

	CNetObj_Laser *pObj = static_cast<CNetObj_Laser *>(Server()->SnapNewItem(NETOBJTYPE_LASER, GetID(), sizeof(CNetObj_Laser)));
	if(!pObj)
		return;

	pObj->m_X = round_to_int(m_Pos.x);
	pObj->m_Y = round_to_int(m_Pos.y);
	pObj->m_FromX = round_to_int(m_Pos.x);
	pObj->m_FromY = round_to_int(m_Pos.y);
	pObj->m_StartTick = m_EvalTick;
}
