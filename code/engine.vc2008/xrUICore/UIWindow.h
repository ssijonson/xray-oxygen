#pragma once
#include "linker.h"
#include "../xrEngine/xr_level_controller.h"
#include "UIMessages.h"
#include "../xrScripts/export/script_export_space.h"
#include "uiabstract.h"
#include "xrCore/XMLCore/Expression.h"
#include "xrCore/xrLazyVector.h"

//template <typename T, typename allocator = xalloc<T>>
//using ui_list = xr_list<T, allocator>; // xr_vector<T, allocator>;
template <typename T>
using ui_list = xrLazyVector<T>; // xr_vector<T, allocator>;
class UI_API CUIWindow  : public CUISimpleWindow
{
public:
				CUIWindow						();
	virtual		~CUIWindow						();


	////////////////////////////////////
	//работа с дочерними и родительскими окнами
	virtual void			AttachChild			(CUIWindow* pChild);
	virtual void			DetachChild			(CUIWindow* pChild);
	virtual bool			IsChild				(CUIWindow* pChild) const;
	virtual void			DetachAll			();
	int						GetChildNum			()								{return u32(m_ChildWndList.size());} 

	void					SetParent			(CUIWindow* pNewParent);
	CUIWindow*				GetParent			()	const							{return m_pParentWnd;}
	
	//получить окно самого верхнего уровня
	CUIWindow*				GetTop				()								{if(m_pParentWnd == nullptr) return this; 
																				else return  m_pParentWnd->GetTop();}
	CUIWindow*				GetCurrentMouseHandler();
	CUIWindow*				GetChildMouseHandler();


	virtual bool			OnKeyboardAction			(u8 dik, EUIMessages keyboard_action);
	virtual bool			OnKeyboardHold		(u8 dik);

	virtual bool 			OnMouseAction				(float x, float y, EUIMessages mouse_action);
	virtual void 			OnMouseMove			();
	virtual void 			OnMouseScroll		(float iDirection);
	virtual bool 			OnDbClick			();
	virtual bool 			OnMouseDown			(int mouse_btn);
	virtual void 			OnMouseUp			(int mouse_btn);

	virtual void 			OnFocusReceive		();
	virtual void 			OnFocusLost			();
	
	//захватить/освободить мышь окном
	//сообщение посылается дочерним окном родительскому
	void					SetCapture			(CUIWindow* pChildWindow, bool capture_status);
	CUIWindow*				GetMouseCapturer	()													{return m_pMouseCapturer;}

	//окошко, которому пересылаются сообщения,
	//если NULL, то шлем на GetParent()
	void					SetMessageTarget	(CUIWindow* pWindow)								{m_pMessageTarget = pWindow;}
	CUIWindow*				GetMessageTarget	();

			void			SetKeyboardCapture	(CUIWindow* pChildWindow, bool capture_status);

	virtual void			SendMessageToWnd	(CUIWindow* pWnd, s16 msg, void* pData = NULL);

	virtual void			Enable				(bool status)									{m_bIsEnabled=status;}
			bool			IsEnabled			()												{return m_bIsEnabled;}

	//убрать/показать окно и его дочерние окна
	virtual void			Show				(bool status)									{SetVisible(status); Enable(status); }
	IC		bool			IsShown				()												{return GetVisible();}
			void			ShowChildren		(bool show);
	
	//абсолютные координаты
	IC void					GetAbsoluteRect		(Frect& r) ;
	IC void					GetAbsolutePos		(Fvector2& p) 	{Frect abs; GetAbsoluteRect(abs); p.set(abs.x1,abs.y1);}


			void			SetWndRect_script	(const Frect &rect)		noexcept				{CUISimpleWindow::SetWndRect(rect);}
			void			SetWndPos_script	(const Fvector2 &pos)	noexcept				{CUISimpleWindow::SetWndPos(pos);}
			void			SetWndSize_script	(const Fvector2 &size)	noexcept				{CUISimpleWindow::SetWndSize(size);}

	//прорисовка окна
	virtual void			Draw				();
	virtual void			Draw				(float x, float y);
	//обновление окна передпрорисовкой
	virtual void			Update				();


			void			SetPPMode			();
			void			ResetPPMode			();
	IC		bool			GetPPMode			()		{return m_bPP;};
	//для перевода окна и потомков в исходное состояние
	virtual void			Reset				();
			void			ResetAll			();

	using WINDOW_LIST		= ui_list<CUIWindow*>;
	using WINDOW_LIST_it	= WINDOW_LIST::iterator;
	WINDOW_LIST&			GetChildWndList		()							{return m_ChildWndList; }


	IC bool					IsAutoDelete		()							{return m_bAutoDelete;}
	IC void					SetAutoDelete		(bool auto_delete)			{m_bAutoDelete = auto_delete;}

	// Name of the window
	const shared_str		WindowName			() const					{ return m_windowName; }
	void					SetWindowName		(LPCSTR wn)					{ m_windowName = wn; }
	LPCSTR					WindowName_script	()							{return m_windowName.c_str();}
	CUIWindow*				FindChild			(const shared_str name);

	IC bool					CursorOverWindow	() const					{ return m_bCursorOverWindow; }
	IC u32					FocusReceiveTime	() const					{ return m_dwFocusReceiveTime; }
	
	IC bool					GetCustomDraw		() const					{return m_bCustomDraw;}
	IC void					SetCustomDraw		(bool b) 					{m_bCustomDraw = b;}

    // Expression that provide data for this widget
    CExpression             m_expression;

protected:
	IC void					SafeRemoveChild(CUIWindow* child)				{WINDOW_LIST_it it = std::find(m_ChildWndList.begin(),m_ChildWndList.end(),child); if(it!=m_ChildWndList.end())m_ChildWndList.erase(it);};

	shared_str				m_windowName;
	//список дочерних окон
	WINDOW_LIST				m_ChildWndList;
	
	//указатель на родительское окно
	CUIWindow*				m_pParentWnd;

	//дочернее окно которое, захватило ввод мыши
	CUIWindow*				m_pMouseCapturer;
	
	//дочернее окно которое, захватило ввод клавиатуры
	CUIWindow*				m_pKeyboardCapturer;

	//кому шлем сообщения
	CUIWindow*				m_pMessageTarget;

	// Последняя позиция мышки
	Fvector2 cursor_pos;

	//время прошлого клика мышки
	//для определения DoubleClick
	u32						m_dwLastClickTime;
	u32						m_dwFocusReceiveTime;

	//флаг автоматического удаления во время вызова деструктора
	bool					m_bAutoDelete;

	bool					m_bPP;
	bool					m_bIsEnabled;

	// Если курсор над окном
	bool					m_bCursorOverWindow;
	bool					m_bCustomDraw;

#ifdef DEBUG
	int m_dbg_id;
#endif

public:
	DECLARE_SCRIPT_REGISTER_FUNCTION
};

UI_API bool fit_in_rect(CUIWindow* w, Frect const& vis_rect, float border = 0.0f, float dx16pos = 0.0f );
