// GameObject.h: interface for the CGameObject class.
//
//////////////////////////////////////////////////////////////////////
#pragma once

#include "../xrEngine/xr_object.h"
#include "../xrParticles/ParticlesPlayer.h"
#include "../xrServerEntities/xrServer_Space.h"
#include "../xrServerEntities/alife_space.h"
#include "UsableScriptObject.h"
#include "script_binder.h"
#include "Hit.h"
#include "game_object_space.h"

class IPhysicsShellEx;
class CSE_Abstract;
class CPHSynchronize;
class CScriptGameObject;
class CInventoryItem;
class CEntity;
class CEntityAlive;
class CInventoryOwner;
class CActor;
class CPhysicsShellHolder;
class CParticlesPlayer;
class CCustomZone;
class IInputReceiver;
class CArtefact;
class CCustomMonster;
class CAI_Stalker;
class CScriptEntity;
class CAI_ObjectLocation;
class CWeapon;
class CExplosive;
class CHolderCustom;
class CAttachmentOwner;
class CBaseMonster;
class CSpaceRestrictor;
class CAttachableItem;
class animation_movement_controller;
class CBlend;
class ai_obstacle;

class IKinematics;

template <typename _return_type>
class CScriptCallbackEx;

class GAME_API CGameObject : 
	public CObject, 
	public CUsableScriptObject,
	public CScriptBinder
{
	using inherited = CObject;
	bool							m_spawned;
	Flags32							m_server_flags;
	CAI_ObjectLocation				*m_ai_location;
	ALife::_STORY_ID				m_story_id;
	animation_movement_controller	*m_anim_mov_ctrl;
protected:
	//время удаления объекта
	bool					m_bObjectRemoved;
public:
	CGameObject();
	virtual ~CGameObject();
public:
	//functions used for avoiding most of the smart_cast
	virtual CAttachmentOwner*			cast_attachment_owner		()						{return nullptr;}
	virtual CInventoryOwner*			cast_inventory_owner		()						{return nullptr;}
	virtual CInventoryItem*				cast_inventory_item			()						{return nullptr;}
	virtual CEntity*					cast_entity					()						{return nullptr;}
	virtual CEntityAlive*				cast_entity_alive			()						{return nullptr;}
	virtual CActor*						cast_actor					()						{return nullptr;}
	virtual CGameObject*				cast_game_object			()						{return this;}
	virtual CCustomZone*				cast_custom_zone			()						{return nullptr;}
	virtual CPhysicsShellHolder*		cast_physics_shell_holder	()						{return nullptr;}
	virtual IInputReceiver*				cast_input_receiver			()						{return nullptr;}
	virtual CParticlesPlayer*			cast_particles_player		()						{return nullptr;}
	virtual CArtefact*					cast_artefact				()						{return nullptr;}
	virtual CCustomMonster*				cast_custom_monster			()						{return nullptr;}
	virtual CAI_Stalker*				cast_stalker				()						{return nullptr;}
	virtual CScriptEntity*				cast_script_entity			()						{return nullptr;}
	virtual CWeapon*					cast_weapon					()						{return nullptr;}
	virtual CExplosive*					cast_explosive				()						{return nullptr;}
	virtual CSpaceRestrictor*			cast_restrictor				()						{return nullptr;}
	virtual CAttachableItem*			cast_attachable_item		()						{return nullptr;}
	virtual CHolderCustom*				cast_holder_custom			()						{return nullptr;}
	virtual CBaseMonster*				cast_base_monster			()						{return nullptr;}

public:
	virtual BOOL						feel_touch_on_contact	(CObject *)					{return TRUE;}
	virtual bool						use						(CGameObject* who_use)		{return CUsableScriptObject::use(who_use);};

public:
	CInifile				*m_ini_file;

	// Utilities
	static void				u_EventGen			(NET_Packet& P, u32 type, u32 dest	);
	static void				u_EventSend			(NET_Packet& P);
	
	// Methods
	virtual void			Load				(LPCSTR section);
	virtual BOOL			net_Spawn			(CSE_Abstract* DC);
	virtual void			net_Destroy			();
	virtual	void			net_Relcase			( CObject* O );	
	virtual void			UpdateCL			( );
	virtual void			OnChangeVisual		( );
	//object serialization
	virtual void			net_Save			(NET_Packet &net_packet);
	virtual void			net_Load			(IReader	&ireader);
	virtual BOOL			net_SaveRelevant	();
	virtual void			save				(NET_Packet &output_packet);
	virtual void			load				(IReader &input_packet);

	virtual BOOL			net_Relevant		()	{ return getLocal();	}	// send messages only if active and local
	virtual void			spatial_move		();
	virtual BOOL			Ready				()	{ return getReady();	}	// update only if active and fully initialized by/for network
//	virtual float			renderable_Ambient	();

	virtual void			shedule_Update		(u32 dt);	
	virtual bool			shedule_Needed		();

	virtual void			renderable_Render	();
	virtual void			OnEvent				(NET_Packet& P, u16 type);
	virtual	void			Hit					(SHit* pHDS) {};
	virtual void			SetHitInfo				(CObject* who, CObject* weapon, s16 element, Fvector Pos, Fvector Dir)	{};
	virtual	BOOL			BonePassBullet		(int boneID) { return FALSE; }


	//игровое имя объекта
	virtual LPCSTR			Name                () const;
	
	//virtual void			OnH_A_Independent	();
	virtual void			OnH_B_Chield		();
	virtual void			OnH_B_Independent	(bool just_before_destroy);

	virtual bool			IsVisibleForZones	() { return true; }
///////////////////////////////////////////////////////////////////////
	virtual bool			NeedToDestroyObject	() const;
	virtual void			DestroyObject		();
///////////////////////////////////////////////////////////////////////

	// Position stack
	virtual	SavedPosition	ps_Element			(u32 ID) const;

			void			setup_parent_ai_locations(bool assign_position = true);
			void			validate_ai_locations(bool decrement_reference = true);

	//animation_movement_controller
	virtual	void			create_anim_mov_ctrl			( CBlend *b, Fmatrix *start_pose, bool local_animation  );
	virtual	void			destroy_anim_mov_ctrl			( );
			void			update_animation_movement_controller();
			bool			animation_movement_controlled	( ) const	;
const animation_movement_controller* animation_movement		( ) const	{ return	m_anim_mov_ctrl; }
	  animation_movement_controller* animation_movement		( )			{ return	m_anim_mov_ctrl; }
	// Game-specific events

	virtual BOOL			UsedAI_Locations				();
			BOOL			TestServerFlag					(u32 Flag) const;
	virtual	bool			can_validate_position_on_spawn	(){return true;}
#ifdef DEBUG
	virtual void			OnRender			();
#endif

			void			init				();
	virtual	void			reinit				();
	virtual	void			reload				(LPCSTR section);
	///////////////////// network /////////////////////////////////////////
	bool					object_removed		() const { return m_bObjectRemoved; };

public:
#ifdef DEBUG
	virtual void			PH_Ch_CrPr			() {}; // 
	virtual	void			dbg_DrawSkeleton	();
#endif
	///////////////////////////////////////////////////////////////////////
	virtual const SRotation	Orientation			() const
	{
		SRotation			rotation;
		float				h,p,b;
		XFORM().getHPB		(h,p,b);
		rotation.yaw		= h;
		rotation.pitch		= p;
		return				(rotation);
	};

	virtual bool			use_parent_ai_locations	() const
	{
		return				(true);
	}

public:
	using visual_callback = void __stdcall(IKinematics *);
	using CALLBACK_VECTOR = svector<visual_callback*,6>;
	using CALLBACK_VECTOR_IT = CALLBACK_VECTOR::iterator;

	CALLBACK_VECTOR			m_visual_callback;

public:
			void			add_visual_callback		(visual_callback *callback);
			void			remove_visual_callback	(visual_callback *callback);
			void			SetKinematicsCallback	(bool set);

	IC		CALLBACK_VECTOR &visual_callbacks	()
	{
		return				(m_visual_callback);
	}


private:
	mutable CScriptGameObject	*m_lua_game_object;
	int						m_script_clsid;
public:
			CScriptGameObject	*lua_game_object() const;
			int				clsid			() const
	{
		THROW				(m_script_clsid >= 0);
		return				(m_script_clsid);
	}
public:
	IC		CInifile		*spawn_ini			()
	{
		return				(m_ini_file);
	}
protected:
	virtual	void			spawn_supplies		();

public:
	IC		CAI_ObjectLocation	&ai_location		() const
	{
		VERIFY				(m_ai_location);
		return				(*m_ai_location);
	}

private:
	u32						m_spawn_time;

public:
	IC		u32				spawn_time			() const
	{
		VERIFY				(m_spawned);
		return				(m_spawn_time);
	}

	IC		const ALife::_STORY_ID &story_id	() const
	{
		return				(m_story_id);
	}
	
public:
	virtual u32				ef_creature_type	() const;
	virtual u32				ef_equipment_type	() const;
	virtual u32				ef_main_weapon_type	() const;
	virtual u32				ef_anomaly_type		() const;
	virtual u32				ef_weapon_type		() const;
	virtual u32				ef_detector_type	() const;
	virtual bool			natural_weapon		() const {return true;}
	virtual bool			natural_detector	() const {return true;}
	virtual bool			use_center_to_aim	() const {return false;}
	// [12.11.07] Alexander Maniluk: added this method for moving object
	virtual void MoveTo(Fvector const & position) {};

public:
	
	using CScriptCallbackExVoid = CScriptCallbackEx<void>;

private:
    using CALLBACK_MAP = xr_map<GameObject::ECallbackType, CScriptCallbackExVoid>;
	CALLBACK_MAP			*m_callbacks;

public:
	CScriptCallbackExVoid	&callback			(GameObject::ECallbackType type) const;
	virtual	LPCSTR			visual_name			(CSE_Abstract *server_entity);

	virtual	void			On_B_NotCurrentEntity () {};

	// for moving objects
private:
			u32				new_level_vertex_id	() const;
			void			update_ai_locations	(bool decrement_reference);

private:
	ai_obstacle				*m_ai_obstacle;
	Fmatrix					m_previous_matrix;

public:
	virtual	bool			is_ai_obstacle		() const;

public:
	IC		ai_obstacle		&obstacle			() const
	{
		VERIFY				(m_ai_obstacle);
		return				(*m_ai_obstacle);
	}

	virtual void			on_matrix_change	(const Fmatrix &previous);
};
