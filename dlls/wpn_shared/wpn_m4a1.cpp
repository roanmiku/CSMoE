/***
*
*	Copyright (c) 1996-2002, Valve LLC. All rights reserved.
*
*	This product contains software technology licensed from Id
*	Software, Inc. ("Id Technology").  Id Technology (c) 1996 Id Software, Inc.
*	All Rights Reserved.
*
*   Use, distribution, and modification of this source code and/or resulting
*   object code is restricted to non-commercial enhancements to products from
*   Valve LLC.  All other use, distribution, or modification is prohibited
*   without written permission from Valve LLC.
*
****/

#include "extdll.h"
#include "util.h"
#include "cbase.h"
#include "player.h"
#include "weapons.h"
#include "wpn_m4a1.h"

#ifdef CLIENT_DLL
namespace cl {
#else
namespace sv {
#endif

enum m4a1_e
{
	M4A1_IDLE,
	M4A1_SHOOT1,
	M4A1_SHOOT2,
	M4A1_SHOOT3,
	M4A1_RELOAD,
	M4A1_DRAW,
	M4A1_ATTACH_SILENCER,
	M4A1_UNSIL_IDLE,
	M4A1_UNSIL_SHOOT1,
	M4A1_UNSIL_SHOOT2,
	M4A1_UNSIL_SHOOT3,
	M4A1_UNSIL_RELOAD,
	M4A1_UNSIL_DRAW,
	M4A1_DETACH_SILENCER
};

LINK_ENTITY_TO_CLASS(weapon_m4a1, CM4A1)

void CM4A1::Spawn(void)
{
	pev->classname = MAKE_STRING("weapon_m4a1");

	Precache();
	m_iId = WEAPON_M4A1;
	SET_MODEL(ENT(pev), "models/w_m4a1.mdl");

	m_iDefaultAmmo = M4A1_DEFAULT_GIVE;
	m_flAccuracy = 0.2;
	m_iShotsFired = 0;
#ifndef CLIENT_DLL
	if ((int)CVAR_GET_FLOAT("mp_csgosilencedwpn"))
	{
		m_iWeaponState |= WPNSTATE_M4A1_SILENCED;
	}
#endif
	m_bDelayFire = true;

	FallInit();
}

void CM4A1::Precache(void)
{
	PRECACHE_MODEL("models/v_m4a1.mdl");
	PRECACHE_MODEL("models/w_m4a1.mdl");

	PRECACHE_SOUND("weapons/m4a1-1.wav");
	PRECACHE_SOUND("weapons/m4a1_unsil-1.wav");
	PRECACHE_SOUND("weapons/m4a1_unsil-2.wav");
	PRECACHE_SOUND("weapons/m4a1_clipin.wav");
	PRECACHE_SOUND("weapons/m4a1_clipout.wav");
	PRECACHE_SOUND("weapons/m4a1_boltpull.wav");
	PRECACHE_SOUND("weapons/m4a1_deploy.wav");

	m_iShell = PRECACHE_MODEL("models/rshell.mdl");
	m_usFireM4A1 = PRECACHE_EVENT(1, "events/m4a1.sc");
}

int CM4A1::GetItemInfo(ItemInfo *p)
{
	p->pszName = STRING(pev->classname);
	p->pszAmmo1 = "556Nato";
	p->iMaxAmmo1 = MAX_AMMO_556NATO;
	p->pszAmmo2 = NULL;
	p->iMaxAmmo2 = -1;
	p->iMaxClip = M4A1_MAX_CLIP;
	p->iSlot = 0;
	p->iPosition = 6;
	p->iId = m_iId = WEAPON_M4A1;
	p->iFlags = 0;
	p->iWeight = M4A1_WEIGHT;

	return 1;
}

BOOL CM4A1::Deploy(void)
{
#ifndef CLIENT_DLL
	if (m_pPlayer->IsAlive())
		CheckWeapon(m_pPlayer, this);
#endif
	m_bDelayFire = true;
	iShellOn = 1;
	m_flAccuracy = 0.2;
	m_iShotsFired = 0;
	m_NextInspect = gpGlobals->time + 0.75s;
	if (m_iWeaponState & WPNSTATE_M4A1_SILENCED)
		return DefaultDeploy("models/v_m4a1.mdl", "models/p_m4a1.mdl", M4A1_DRAW, "rifle", UseDecrement() != FALSE);
	else
		return DefaultDeploy("models/v_m4a1.mdl", "models/p_m4a1.mdl", M4A1_UNSIL_DRAW, "rifle", UseDecrement() != FALSE);
}

void CM4A1::SecondaryAttack(void)
{
	m_flLastFire = invalid_time_point;
	m_NextInspect = gpGlobals->time + 2s;
	if (m_iWeaponState & WPNSTATE_M4A1_SILENCED)
	{
		m_iWeaponState &= ~WPNSTATE_M4A1_SILENCED;
		SendWeaponAnim(M4A1_DETACH_SILENCER, UseDecrement() != FALSE);
		strcpy(m_pPlayer->m_szAnimExtention, "rifle");
	}
	else
	{
		m_iWeaponState |= WPNSTATE_M4A1_SILENCED;
		SendWeaponAnim(M4A1_ATTACH_SILENCER, UseDecrement() != FALSE);
		strcpy(m_pPlayer->m_szAnimExtention, "rifle");
	}
	m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + 2s;
	m_flNextSecondaryAttack = UTIL_WeaponTimeBase() + 2s;
	m_flNextPrimaryAttack = UTIL_WeaponTimeBase() + 2s;
}

void CM4A1::PrimaryAttack(void)
{
	if (m_iWeaponState & WPNSTATE_M4A1_SILENCED)
	{
		if (!FBitSet(m_pPlayer->pev->flags, FL_ONGROUND))
			M4A1Fire(0.035 + (0.4) * m_flAccuracy, 0.0875s, FALSE);
		else if (m_pPlayer->pev->velocity.Length2D() > 140)
			M4A1Fire(0.035 + (0.07) * m_flAccuracy, 0.0875s, FALSE);
		else
			M4A1Fire((0.025) * m_flAccuracy, 0.0875s, FALSE);
	}
	else
	{
		if (!FBitSet(m_pPlayer->pev->flags, FL_ONGROUND))
			M4A1Fire(0.035 + (0.4) * m_flAccuracy, 0.0875s, FALSE);
		else if (m_pPlayer->pev->velocity.Length2D() > 140)
			M4A1Fire(0.035 + (0.07) * m_flAccuracy, 0.0875s, FALSE);
		else
			M4A1Fire((0.02) * m_flAccuracy, 0.0875s, FALSE);
	}
}

void CM4A1::M4A1Fire(float flSpread, duration_t flCycleTime, BOOL fUseAutoAim)
{
	m_bDelayFire = true;
	m_iShotsFired++;
	m_flAccuracy = ((float)(m_iShotsFired * m_iShotsFired * m_iShotsFired) / 220) + 0.3;

	if (m_flAccuracy > 1)
		m_flAccuracy = 1;

	if (m_iClip <= 0)
	{
		if (m_fFireOnEmpty)
		{
			PlayEmptySound();
			m_flNextPrimaryAttack = UTIL_WeaponTimeBase() + 0.2s;
		}

		return;
	}

	m_iClip--;
#ifndef CLIENT_DLL
	m_pPlayer->SetAnimation(PLAYER_ATTACK1);
#endif

	UTIL_MakeVectors(m_pPlayer->pev->v_angle + m_pPlayer->pev->punchangle);

	m_pPlayer->m_iWeaponVolume = NORMAL_GUN_VOLUME;
	m_pPlayer->m_iWeaponFlash = BRIGHT_GUN_FLASH;

	Vector vecSrc = m_pPlayer->GetGunPosition();
	Vector vecDir;

	if (m_iWeaponState & WPNSTATE_M4A1_SILENCED)
	{
		vecDir = m_pPlayer->FireBullets3(vecSrc, gpGlobals->v_forward, flSpread, 8192, 2, BULLET_PLAYER_556MM, 33, 0.95, m_pPlayer->pev, FALSE, m_pPlayer->random_seed);
	}
	else
	{
		vecDir = m_pPlayer->FireBullets3(vecSrc, gpGlobals->v_forward, flSpread, 8192, 2, BULLET_PLAYER_556MM, 32, 0.97, m_pPlayer->pev, FALSE, m_pPlayer->random_seed);
		m_pPlayer->pev->effects |= EF_MUZZLEFLASH;
	}

	int flags;
#ifdef CLIENT_WEAPONS
	flags = FEV_NOTHOST;
#else
	flags = 0;
#endif

	PLAYBACK_EVENT_FULL(flags, m_pPlayer->edict(), m_usFireM4A1, 0, (float *)&g_vecZero, (float *)&g_vecZero, vecDir.x, vecDir.y, (int)(m_pPlayer->pev->punchangle.x * 100), (int)(m_pPlayer->pev->punchangle.y * 100), (m_iWeaponState & WPNSTATE_M4A1_SILENCED) ? true : false, FALSE);
	m_flNextPrimaryAttack = m_flNextSecondaryAttack = UTIL_WeaponTimeBase() + flCycleTime;
#ifndef CLIENT_DLL
	if (!m_iClip && m_pPlayer->m_rgAmmo[m_iPrimaryAmmoType] <= 0)
		m_pPlayer->SetSuitUpdate("!HEV_AMO0", FALSE, 0);
#endif
	m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + 1.5s;
	m_flLastFire = gpGlobals->time;
	if (m_pPlayer->pev->velocity.Length2D() > 0)
		KickBack(1.0, 0.45, 0.28, 0.045, 3.75, 3.0, 7);
	else if (!FBitSet(m_pPlayer->pev->flags, FL_ONGROUND))
		KickBack(1.2, 0.5, 0.23, 0.15, 5.5, 3.5, 6);
	else if (FBitSet(m_pPlayer->pev->flags, FL_DUCKING))
		KickBack(0.6, 0.3, 0.2, 0.0125, 3.25, 2.0, 7);
	else
		KickBack(0.65, 0.35, 0.25, 0.015, 3.5, 2.25, 7);
}

void CM4A1::Reload(void)
{
	if (m_pPlayer->ammo_556nato <= 0)
		return;

	int iAnim;
	m_NextInspect = gpGlobals->time + M4A1_RELOAD_TIME;
	if (m_iWeaponState & WPNSTATE_M4A1_SILENCED)
		iAnim = M4A1_RELOAD;
	else
		iAnim = M4A1_UNSIL_RELOAD;

	if (DefaultReload(M4A1_MAX_CLIP, iAnim, 3.05s))
	{
#ifndef CLIENT_DLL
		m_pPlayer->SetAnimation(PLAYER_RELOAD);
#endif
		m_flAccuracy = 0.2;
		m_iShotsFired = 0;
		m_bDelayFire = false;
	}
}

void CM4A1::WeaponIdle(void)
{
	ResetEmptySound();
	m_pPlayer->GetAutoaimVector(AUTOAIM_10DEGREES);

	if (m_flTimeWeaponIdle > UTIL_WeaponTimeBase())
		return;

	m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + 20s;

	if (m_iWeaponState & WPNSTATE_M4A1_SILENCED)
		SendWeaponAnim(M4A1_IDLE, UseDecrement() != FALSE);
	else
		SendWeaponAnim(M4A1_UNSIL_IDLE, UseDecrement() != FALSE);
}

void CM4A1::Inspect()
{
	if (!m_fInReload)
	{
		if (m_flLastFire != invalid_time_point || gpGlobals->time > m_NextInspect)
		{
#ifndef CLIENT_DLL
			if (m_iWeaponState & WPNSTATE_M4A1_SILENCED)
				SendWeaponAnim(14, 0);
			else
				SendWeaponAnim(15, 0);
#endif
			m_NextInspect = gpGlobals->time + GetInspectTime();
			m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + GetInspectTime();
			m_flLastFire = invalid_time_point;
		}
	}
}
}
