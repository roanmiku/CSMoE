
#include "extdll.h"
#include "util.h"
#include "cbase.h"
#include "monsters.h"
#include "weapons.h"
#include "player.h"
#include "gamerules.h"
#include "training_gamerules.h"
#include "hltv.h"
#include "game.h"
#include "trains.h"
#include "vehicle.h"
#include "globals.h"

#include "bot_include.h"

namespace sv {

void HostageRetreatState::OnEnter(CHostageImprov *improv)
{
	improv->Walk();
	improv->MoveTo(improv->GetEntity()->m_vStart);
}

void HostageRetreatState::OnUpdate(CHostageImprov *improv)
{
	if (improv->IsAtHome())
	{
		improv->Stop();
		improv->Idle();
		return;
	}

	CBasePlayer *player = improv->GetClosestVisiblePlayer(UNASSIGNED);

	if (player != NULL)
	{
		const float farRange = 400.0f;
		if ((player->pev->origin - improv->GetCentroid()).IsLengthGreaterThan(farRange))
		{
			if (player->m_iTeam == CT && !improv->IsScared())
			{
				improv->Stop();
				improv->Idle();
				return;
			}
		}
	}

	if (improv->IsScared() && improv->GetScareIntensity() == CHostageImprov::TERRIFIED)
		improv->Run();
	else
		improv->Walk();
}

void HostageRetreatState::OnExit(CHostageImprov *improv)
{
	;
}

}
