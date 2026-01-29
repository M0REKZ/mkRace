/* (c) Magnus Auvinen. See licence.txt in the root of the distribution for more information. */
/* If you are missing that file, acquire a complete release at teeworlds.com.                */
#ifndef GAME_SERVER_ENTITIES_KZ_GOLFBALL_H
#define GAME_SERVER_ENTITIES_KZ_GOLFBALL_H

#include <game/server/entity.h>

class CGolfBall : public CEntity
{
public:
	CGolfBall(CGameWorld *pGameWorld, vec2 Pos, vec2 Direction, float StartEnergy, int Owner, int Type);

	virtual void Reset();
	virtual void Tick();
	virtual void TickPaused();
	virtual void Snap(int SnappingClient);
	virtual int GetOwner() { return m_Owner; }

protected:
	void DoBounce();
	void TeleportOwnerTo(vec2 ToPos);

private:
	vec2 m_From;
	vec2 m_Dir;
	float m_Energy;
	int m_Bounces;
	int m_EvalTick;
	int m_Owner;

	int m_Type;
	vec2 m_PrevPos;
	int m_StartTick;
};

#endif
