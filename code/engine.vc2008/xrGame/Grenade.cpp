#include "stdafx.h"
#include "Grenade.h"
#include "../xrPhysics/PhysicsShell.h"
#include "Entity.h"
#include "../xrParticles/psystem.h"
#include "../xrParticles/ParticlesObject.h"
#include "Actor.h"
#include "inventory.h"
#include "Level.h"
#include "xrMessages.h"
#include "..\xrEngine\xr_level_controller.h"
#include "xrServer_Objects_Alife.h"

const float default_grenade_detonation_threshold_hit=100;
CGrenade::CGrenade() 
{	
	m_destroy_callback.clear();
	m_eSoundCheckout = ESoundTypes(SOUND_TYPE_WEAPON_RECHARGING);
}

CGrenade::~CGrenade() 
{
}

void CGrenade::Load(LPCSTR section) 
{
	inherited::Load(section);
	CExplosive::Load(section);

	m_sounds.LoadSound(section,"snd_checkout", "sndCheckout", false, m_eSoundCheckout);

	//////////////////////////////////////
	//время убирания оружия с уровня
	if(pSettings->line_exist(section,"grenade_remove_time"))
		m_dwGrenadeRemoveTime = pSettings->r_u32(section,"grenade_remove_time");
	else
		m_dwGrenadeRemoveTime = 30000ui64;
	m_grenade_detonation_threshold_hit=READ_IF_EXISTS(pSettings,r_float,section,"detonation_threshold_hit",default_grenade_detonation_threshold_hit);
}

void CGrenade::Hit					(SHit* pHDS)
{
	if( ALife::eHitTypeExplosion==pHDS->hit_type && m_grenade_detonation_threshold_hit<pHDS->damage()&&CExplosive::Initiator()==u16(-1)) 
	{
		CExplosive::SetCurrentParentID(pHDS->who->ID());
		Destroy();
	}
	inherited::Hit(pHDS);
}

BOOL CGrenade::net_Spawn(CSE_Abstract* DC) 
{
	m_dwGrenadeIndependencyTime			= 0;
	BOOL ret= inherited::net_Spawn		(DC);
	Fvector box;BoundingBox().getsize	(box);
	float max_size						= std::max(std::max(box.x,box.y),box.z);
	box.set								(max_size,max_size,max_size);
	box.mul								(3.f);
	CExplosive::SetExplosionSize		(box);
	m_thrown							= false;
	return								ret;
}

void CGrenade::net_Destroy() 
{
	if(m_destroy_callback)
	{
		m_destroy_callback				(this);
		m_destroy_callback				= destroy_callback(nullptr);
	}

	inherited::net_Destroy				();
	CExplosive::net_Destroy				();
}

void CGrenade::OnH_B_Independent(bool just_before_destroy) 
{
	inherited::OnH_B_Independent(just_before_destroy);
}

void CGrenade::OnH_A_Independent() 
{
	m_dwGrenadeIndependencyTime			= Level().timeServer();
	inherited::OnH_A_Independent		();	
}

void CGrenade::OnH_A_Chield()
{
	m_dwGrenadeIndependencyTime			= 0;
	m_dwDestroyTime						= 0xffffffff;
	inherited::OnH_A_Chield				();
}

void CGrenade::State(u32 state) 
{
	switch (state)
	{
	case eThrowStart:
		{
			Fvector						C;
			Center						(C);
			PlaySound					("sndCheckout", C);
		}break;
	case eThrowEnd:
		{
			if(m_thrown)
			{
				if (m_pPhysicsShell)
					m_pPhysicsShell->Deactivate();
				xr_delete	( m_pPhysicsShell );
				m_dwDestroyTime			= 0xffffffff;
				PutNextToSlot			();
				if (Local())
				{
#ifndef MASTER_GOLD
					Msg( "Destroying local grenade[%d][%d]", ID(), Device.dwFrame );
#endif // #ifndef MASTER_GOLD
					DestroyObject();
				}
				
			};
		}break;
	};
	inherited::State( state );
}

bool CGrenade::DropGrenade()
{
	EMissileStates grenade_state = static_cast<EMissileStates>(GetState());
	if (((grenade_state == eThrowStart) ||
		(grenade_state == eReady) ||
		(grenade_state == eThrow)) &&
		(!m_thrown)
		)
	{
		Throw();
		return true;
	}
	return false;
}

void CGrenade::DiscardState()
{
	if(GetState()==eReady || GetState()==eThrow)
		OnStateSwitch(eIdle);
}

void CGrenade::SendHiddenItem						()
{
	if (GetState()==eThrow)
	{
//		Msg("MotionMarks !!![%d][%d]", ID(), Device.dwFrame);
		Throw				();
	}
	CActor* pActor = smart_cast<CActor*>( m_pInventory->GetOwner());
	if (pActor && (GetState()==eReady || GetState()==eThrow))
	{
		return;
	}

	inherited::SendHiddenItem();
}

void CGrenade::Throw() 
{
	if (m_thrown)
		return;

	if (!m_fake_missile)
		return;

	CGrenade					*pGrenade = smart_cast<CGrenade*>( m_fake_missile );
	VERIFY						(pGrenade);
	
	if (pGrenade) 
	{
		pGrenade->set_destroy_time(m_dwDestroyTimeMax);
		//установить ID того кто кинул гранату
		pGrenade->SetInitiator( H_Parent()->ID() );
	}
	inherited::Throw			();
	m_fake_missile->processing_activate();//@sliph
	m_thrown = true;
}



void CGrenade::Destroy() 
{
	//Generate Expode event
	Fvector						normal;

	if(m_destroy_callback)
	{
		m_destroy_callback		(this);
		m_destroy_callback	=	destroy_callback(nullptr);
	}

	FindNormal					(normal);
	CExplosive::GenExplodeEvent	(Position(), normal);
}



bool CGrenade::Useful() const
{

	bool res = (/* !m_throw && */ m_dwDestroyTime == 0xffffffff && CExplosive::Useful() && TestServerFlag(CSE_ALifeObject::flCanSave));

	return res;
}

void CGrenade::OnEvent(NET_Packet& P, u16 type) 
{
	inherited::OnEvent			(P,type);
	CExplosive::OnEvent			(P,type);
}

void CGrenade::PutNextToSlot()
{
	VERIFY									(!getDestroy());
	//выкинуть гранату из инвентаря
	NET_Packet						P;
	if (m_pInventory)
	{
		m_pInventory->Ruck					(this);

		this->u_EventGen				(P, GEG_PLAYER_ITEM2RUCK, this->H_Parent()->ID());
		P.w_u16							(this->ID());
		this->u_EventSend				(P);
	}
	else
		Msg ("! PutNextToSlot : m_pInventory = NULL [%d][%d]", ID(), Device.dwFrame);	

	if (smart_cast<CInventoryOwner*>(H_Parent()) && m_pInventory)
	{
		CGrenade *pNext						= smart_cast<CGrenade*>(	m_pInventory->Same(this,true)		);
		if(!pNext) pNext					= smart_cast<CGrenade*>(	m_pInventory->SameSlot(GRENADE_SLOT, this, true)	);

		VERIFY								(pNext != this);

		if(pNext && m_pInventory->Slot(pNext->BaseSlot(),pNext) )
		{
			pNext->u_EventGen				(P, GEG_PLAYER_ITEM2SLOT, pNext->H_Parent()->ID());
			P.w_u16							(pNext->ID());
			P.w_u16							(pNext->BaseSlot());
			pNext->u_EventSend				(P);
			m_pInventory->SetActiveSlot		(pNext->BaseSlot());
		}else
		{
			CActor* pActor = smart_cast<CActor*>( m_pInventory->GetOwner());
			
			if(pActor)
				pActor->OnPrevWeaponSlot();
		}

		m_thrown				= false;
	}
}

void CGrenade::OnAnimationEnd(u32 state) 
{
	switch(state)
	{
	case eThrowEnd: SwitchState(eHidden);	break;
	default : inherited::OnAnimationEnd(state);
	}
}


void CGrenade::UpdateCL() 
{
	inherited::UpdateCL			();
	CExplosive::UpdateCL		();
}


bool CGrenade::Action(u16 cmd, u32 flags) 
{
	if(inherited::Action(cmd, flags)) return true;

	switch(cmd) 
	{
	//переключение типа гранаты
	case kWPN_NEXT:
		{
            if(flags&CMD_START) 
			{
				if(m_pInventory)
				{
					TIItemContainer::iterator it = m_pInventory->m_ruck.begin();
					TIItemContainer::iterator it_e = m_pInventory->m_ruck.end();
					for(;it!=it_e;++it) 
					{
						CGrenade *pGrenade = smart_cast<CGrenade*>(*it);
						if(pGrenade && xr_strcmp(pGrenade->cNameSect(), cNameSect())) 
						{
							m_pInventory->Ruck			(this);
							m_pInventory->SetActiveSlot	(NO_ACTIVE_SLOT);
							m_pInventory->Slot			(pGrenade->BaseSlot(),pGrenade);
							return						true;
						}
					}
					return true;
				}
			}
			return true;
		};
	}
	return false;
}

bool CGrenade::NeedToDestroyObject()	const
{
	return false;
}

ALife::_TIME_ID	 CGrenade::TimePassedAfterIndependant()	const
{
	if(!H_Parent() && m_dwGrenadeIndependencyTime != 0)
		return Level().timeServer() - m_dwGrenadeIndependencyTime;
	else
		return 0;
}

BOOL CGrenade::UsedAI_Locations()
{
	return inherited::UsedAI_Locations();
}

void CGrenade::net_Relcase(CObject* O )
{
	CExplosive::net_Relcase(O);
	inherited::net_Relcase(O);
}

void CGrenade::DeactivateItem()
{
	//Drop grenade if primed
	StopCurrentAnimWithoutCallback();
	if ( !GetTmpPreDestroy() && Local() && ( GetState()==eThrowStart || GetState()==eReady || GetState()==eThrow ) )
	{
		if (m_fake_missile)
		{
			CGrenade*		pGrenade	= smart_cast<CGrenade*>( m_fake_missile );
			if ( pGrenade )
			{
				if ( m_pInventory->GetOwner() )
				{
					CActor* pActor = smart_cast<CActor*>( m_pInventory->GetOwner() );
					if (pActor)
					{
						if ( !pActor->g_Alive() )
						{
							m_constpower			= false;
							m_fThrowForce			= 0;
						}
					}
				}				
				Throw	();
			};
		};
	};

	inherited::DeactivateItem();
}


#include "luabind/luabind.hpp"

bool CGrenade::GetBriefInfo( II_BriefInfo& info )
{
	VERIFY( m_pInventory );
	info.clear();

	info.name._set( m_nameShort );
	info.icon._set( cNameSect() );

	u32 ThisGrenadeCount	= m_pInventory->dwfGetSameItemCount( cNameSect().c_str(), true );
	
	string16 stmp;
	xr_sprintf( stmp, "%d", ThisGrenadeCount );
	info.cur_ammo._set( stmp );
	return true;
}

using namespace luabind;

#pragma optimize("s",on)
void CGrenade::script_register(lua_State *L)
{
	module(L)
		[
			class_<CGrenade, CGameObject>("CGrenade")
			.def(constructor<>())
		];
}