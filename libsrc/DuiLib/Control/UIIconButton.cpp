#include "stdafx.h"
#include "UIIconButton.h"

#define MOUSEENTER_TIMER (WM_USER+2)
#define MOUSELEAVE_TIMER (WM_USER+3)

namespace DuiLib
{
	CIconButtonUI::CIconButtonUI()
		: m_uButtonState(0)
		, m_dwHotTextColor(0)
		, m_dwPushedTextColor(0)
		, m_dwFocusedTextColor(0)
		, m_dwHotBkColor(0)
		, m_dwDisabledBkColor(0x00000000)
		, m_iBindTabIndex(-1)
		, m_bUseMidLayoutColor(false)
		, m_nColorUseValue(0)
		, m_dwMidLayoutColor(0xFF003300)
		, m_bMouseEnter(false)
		, m_bMouseLeave(false)
		, m_nLeavetimerlen(0)
		, m_nEntertimerlen(0)
		, m_bOpenleavetimer(false)
		, m_bDbClick(false)
		, m_bDisbleDbClick(true)
		, m_hIcon(NULL)
		, m_sBkIcomImage(_T(""))
	{	  
		m_uTextStyle = DT_SINGLELINE | DT_VCENTER | DT_CENTER;
		m_rcIconDestRc = CDuiRect(0, 0, 0, 0);
	}

	CIconButtonUI::~CIconButtonUI()
	{
		if (m_hIcon) ::DestroyIcon(m_hIcon);
	}

	LPCTSTR CIconButtonUI::GetClass() const
	{
		return _T("ButtonUI");
	}

	LPVOID CIconButtonUI::GetInterface(LPCTSTR pstrName)
	{
		if( _tcscmp(pstrName, DUI_CTR_ICONBUTTON) == 0 ) return static_cast<CIconButtonUI*>(this);
		return CLabelUI::GetInterface(pstrName);
	}

	UINT CIconButtonUI::GetControlFlags() const
	{
		return (IsKeyboardEnabled() ? UIFLAG_TABSTOP : 0) | (IsEnabled() ? UIFLAG_SETCURSOR : 0);
	}

	void CIconButtonUI::DoEvent(TEventUI& event)
	{
		if( !IsMouseEnabled() && event.Type > UIEVENT__MOUSEBEGIN && event.Type < UIEVENT__MOUSEEND ) {
			if( m_pParent != NULL ) m_pParent->DoEvent(event);
			else CLabelUI::DoEvent(event);
			return;
		}

		if( event.Type == UIEVENT_SETFOCUS ) 
		{
			Invalidate();
		}
		else if( event.Type == UIEVENT_KILLFOCUS ) 
		{
			Invalidate();
		}
		else if( event.Type == UIEVENT_KEYDOWN )
		{
			if (IsKeyboardEnabled()) {
				if( event.chKey == VK_SPACE || event.chKey == VK_RETURN ) {
					Activate();
					return;
				}
			}
		}		
		else if( event.Type == UIEVENT_BUTTONDOWN)
		{
			if( ::PtInRect(&m_rcItem, event.ptMouse) && IsEnabled() ) {
				m_uButtonState |= UISTATE_PUSHED | UISTATE_CAPTURED;
				Invalidate();
			}
			return;
		}	
		else if ( event.Type == UIEVENT_DBLCLICK)
		{
			if (::PtInRect(&m_rcItem, event.ptMouse) && IsEnabled()) {
				m_uButtonState |= UISTATE_PUSHED | UISTATE_CAPTURED;
				Invalidate();
				if (!m_bDisbleDbClick) m_bDbClick = true;
			}
			return;
		}
		else if( event.Type == UIEVENT_MOUSEMOVE )
		{
			if( (m_uButtonState & UISTATE_CAPTURED) != 0 ) {
				if( ::PtInRect(&m_rcItem, event.ptMouse) ) m_uButtonState |= UISTATE_PUSHED;
				else m_uButtonState &= ~UISTATE_PUSHED;
				Invalidate();
			}
			return;
		}
		else if( event.Type == UIEVENT_BUTTONUP )
		{
			if( (m_uButtonState & UISTATE_CAPTURED) != 0 ) {
				if (::PtInRect(&m_rcItem, event.ptMouse)) 
				{
					if(m_bDisbleDbClick) Activate();
					else if (m_bDbClick && m_pManager) 
					{
						m_pManager->SendNotify(this, DUI_MSGTYPE_DBCLICK);
						m_bDbClick = false;
					}
				}
				m_uButtonState &= ~(UISTATE_PUSHED | UISTATE_CAPTURED);
				Invalidate();			
			}
			return;
		}
		else if( event.Type == UIEVENT_CONTEXTMENU )
		{
			if( IsContextMenuUsed() ) {
				m_pManager->SendNotify(this, DUI_MSGTYPE_MENU, event.wParam, event.lParam);
			}
			return;
		}
		else if( event.Type == UIEVENT_MOUSEENTER )
		{
			if( IsEnabled() ) {
				m_uButtonState |= UISTATE_HOT;
				Invalidate();
				if (m_bMouseEnter && m_pManager) {
					if(m_nEntertimerlen<=100) m_pManager->SendNotify(this, DUI_MSGTYPE_MOUSEENTER, event.wParam, event.lParam);
					else {
						if (m_bOpenleavetimer) {
							m_pManager->KillTimer(this, MOUSELEAVE_TIMER);
							m_bOpenleavetimer = false;
						}						
						m_pManager->SetTimer(this, MOUSEENTER_TIMER, m_nEntertimerlen);
					}
				}
			}
			// return;
		}
		else if( event.Type == UIEVENT_MOUSELEAVE )
		{
			if( IsEnabled() ) {
				m_uButtonState &= ~UISTATE_HOT;
				Invalidate();
				if (m_bMouseLeave && m_pManager) {
					if (m_nEntertimerlen > 100) m_pManager->KillTimer(this, MOUSEENTER_TIMER);
					if (m_nLeavetimerlen <= 100) m_pManager->SendNotify(this, DUI_MSGTYPE_MOUSELEAVE, event.wParam, event.lParam);
					else {
						m_pManager->SetTimer(this, MOUSELEAVE_TIMER, m_nLeavetimerlen);
						m_bOpenleavetimer = true;
					}
				}
			}
			// return;
		}
		else if( event.Type == UIEVENT_SETCURSOR ) {
			::SetCursor(::LoadCursor(NULL, IDC_HAND));
			return;
		}
		else if (event.Type == UIEVENT_TIMER && m_pManager)
		{
			if (event.wParam == MOUSEENTER_TIMER && m_bMouseEnter)
			{
				m_pManager->KillTimer(this, MOUSEENTER_TIMER);
				m_pManager->SendNotify(this, DUI_MSGTYPE_MOUSEENTER, event.wParam, event.lParam);				
			}
			else if (event.wParam == MOUSELEAVE_TIMER && m_bMouseLeave)
			{
				m_pManager->KillTimer(this, MOUSELEAVE_TIMER);
				m_pManager->SendNotify(this, DUI_MSGTYPE_MOUSELEAVE, event.wParam, event.lParam);
				m_bOpenleavetimer = false;
			}
		}
		
		CLabelUI::DoEvent(event);
	}

	bool CIconButtonUI::Activate()
	{
		if( !CControlUI::Activate() ) return false;
		if( m_pManager != NULL )
		{
			m_pManager->SendNotify(this, DUI_MSGTYPE_CLICK);
			BindTriggerTabSel();
		}
		return true;
	}

	void CIconButtonUI::SetEnabled(bool bEnable)
	{
		CControlUI::SetEnabled(bEnable);
		if( !IsEnabled() ) {
			m_uButtonState = 0;
		}
	}

	void CIconButtonUI::SetHotBkColor( DWORD dwColor )
	{
		m_dwHotBkColor = dwColor;
	}

	DWORD CIconButtonUI::GetHotBkColor() const
	{
		return m_dwHotBkColor;
	}

	void CIconButtonUI::SetDisiableBkColor(DWORD dwColor)
	{
		m_dwDisabledBkColor = dwColor;
	}

	DWORD CIconButtonUI::GetDisiableBkColor() const 
	{
		return m_dwDisabledBkColor;
	}

	void CIconButtonUI::SetHotTextColor(DWORD dwColor)
	{
		m_dwHotTextColor = dwColor;
	}

	DWORD CIconButtonUI::GetHotTextColor() const
	{
		return m_dwHotTextColor;
	}

	void CIconButtonUI::SetPushedTextColor(DWORD dwColor)
	{
		m_dwPushedTextColor = dwColor;
	}

	DWORD CIconButtonUI::GetPushedTextColor() const
	{
		return m_dwPushedTextColor;
	}

	void CIconButtonUI::SetFocusedTextColor(DWORD dwColor)
	{
		m_dwFocusedTextColor = dwColor;
	}

	DWORD CIconButtonUI::GetFocusedTextColor() const
	{
		return m_dwFocusedTextColor;
	}

	LPCTSTR CIconButtonUI::GetNormalImage()
	{
		return m_sNormalImage;
	}

	void CIconButtonUI::SetNormalImage(LPCTSTR pStrImage)
	{
		m_sNormalImage = pStrImage;
		Invalidate();
	}

	LPCTSTR CIconButtonUI::GetHotImage()
	{
		return m_sHotImage;
	}

	void CIconButtonUI::SetHotImage(LPCTSTR pStrImage)
	{
		m_sHotImage = pStrImage;
		Invalidate();
	}

	LPCTSTR CIconButtonUI::GetPushedImage()
	{
		return m_sPushedImage;
	}

	void CIconButtonUI::SetPushedImage(LPCTSTR pStrImage)
	{
		m_sPushedImage = pStrImage;
		Invalidate();
	}

	LPCTSTR CIconButtonUI::GetFocusedImage()
	{
		return m_sFocusedImage;
	}

	void CIconButtonUI::SetFocusedImage(LPCTSTR pStrImage)
	{
		m_sFocusedImage = pStrImage;
		Invalidate();
	}

	LPCTSTR CIconButtonUI::GetDisabledImage()
	{
		return m_sDisabledImage;
	}

	void CIconButtonUI::SetDisabledImage(LPCTSTR pStrImage)
	{
		m_sDisabledImage = pStrImage;
		Invalidate();
	}

	void CIconButtonUI::SetDisDbClickEvent(bool bdisible)
	{
		m_bDisbleDbClick = bdisible;
	}

	LPCTSTR CIconButtonUI::GetForeImage()
	{
		return m_sForeImage;
	}

	void CIconButtonUI::SetForeImage( LPCTSTR pStrImage )
	{
		m_sForeImage = pStrImage;
		Invalidate();
	}

	LPCTSTR CIconButtonUI::GetHotForeImage()
	{
		return m_sHotForeImage;
	}

	void CIconButtonUI::SetHotForeImage( LPCTSTR pStrImage )
	{
		m_sHotForeImage = pStrImage;
		Invalidate();
	}

	void CIconButtonUI::BindTabIndex(int _BindTabIndex )
	{
		if( _BindTabIndex >= 0)
			m_iBindTabIndex	= _BindTabIndex;
	}

	void CIconButtonUI::BindTabLayoutName( LPCTSTR _TabLayoutName )
	{
		if(_TabLayoutName)
			m_sBindTabLayoutName = _TabLayoutName;
	}

	void CIconButtonUI::BindTriggerTabSel( int _SetSelectIndex /*= -1*/ )
	{
		if(GetBindTabLayoutName().IsEmpty() || (GetBindTabLayoutIndex() < 0 && _SetSelectIndex < 0))
			return;

		CTabLayoutUI* pTabLayout = static_cast<CTabLayoutUI*>(GetManager()->FindControl(GetBindTabLayoutName()));
		if(!pTabLayout)
			return;

		pTabLayout->SelectItem(_SetSelectIndex >=0?_SetSelectIndex:GetBindTabLayoutIndex());
	}

	void CIconButtonUI::RemoveBindTabIndex()
	{
		m_iBindTabIndex	= -1;
		m_sBindTabLayoutName.Empty();
	}

	int CIconButtonUI::GetBindTabLayoutIndex()
	{
		return m_iBindTabIndex;
	}

	CDuiString CIconButtonUI::GetBindTabLayoutName()
	{
		return m_sBindTabLayoutName;
	}

	SIZE CIconButtonUI::EstimateSize(SIZE szAvailable)
	{
		if( m_cxyFixed.cy == 0 ) return CDuiSize(m_cxyFixed.cx, m_pManager->GetFontInfo(GetFont())->tm.tmHeight + 8);
		return CControlUI::EstimateSize(szAvailable);
	}

	void CIconButtonUI::SetMouseEnterEvent(bool bEnter)
	{
		m_bMouseEnter = bEnter;
	}

	void CIconButtonUI::SetMouseLeaveEvent(bool bLeave)
	{
		m_bMouseLeave = bLeave;
	}

	void CIconButtonUI::SetMouserEnterLeaveTimerlen(int nentertimer, int nleavetimer)
	{
		m_nEntertimerlen = nentertimer;
		m_nLeavetimerlen = nleavetimer;
	}

	void CIconButtonUI::SetUseMidLayoutColor(bool buse)
	{
		m_bUseMidLayoutColor = buse;
		Invalidate();
	}

	void CIconButtonUI::SetColorUseValue(int nvalue)
	{
		m_nColorUseValue = nvalue;
		Invalidate();
	}

	void CIconButtonUI::SetMidlayoutColor(DWORD dwColor)
	{
		m_dwMidLayoutColor = dwColor;
		Invalidate();
	}

	HICON CIconButtonUI::LoadIconImage(LPCTSTR bitmap, int cx, int cy)
	{
		LPBYTE pData = NULL;
		DWORD dwSize = 0;
		CDuiString bitmapfile = ParsingIconImageString(bitmap);
		do
		{
			CDuiString sFile = CPaintManagerUI::GetResourcePath();
			if (CPaintManagerUI::GetResourceZip().IsEmpty())
			{
				sFile += bitmapfile;
				if (FileExist(sFile))
				{
					HICON icobitmap = (HICON)::LoadImage(NULL, sFile, IMAGE_ICON, cx, cy, LR_LOADFROMFILE);
					return icobitmap;
				}
				else
				{
					break;
				}

			}

		} while (0);

		if (FileExist(bitmapfile))
		{
			HICON icobitmap = (HICON)::LoadImage(NULL, bitmapfile, IMAGE_ICON, cx, cy, LR_LOADFROMFILE);
			return icobitmap;
		}
		return NULL;
	}

	CDuiString CIconButtonUI::ParsingIconImageString(LPCTSTR pStrImage)
	{
		CDuiString sImageName = pStrImage;
		CDuiString sItem;
		CDuiString sValue;
		LPTSTR pstr = NULL;
		while (*pStrImage != _T('\0'))
		{
			sItem.Empty();
			sValue.Empty();
			while (*pStrImage > _T('\0') && *pStrImage <= _T(' ')) pStrImage = ::CharNext(pStrImage);
			while (*pStrImage != _T('\0') && *pStrImage != _T('=') && *pStrImage > _T(' '))
			{
				LPTSTR pstrTemp = ::CharNext(pStrImage);
				while (pStrImage < pstrTemp)
				{
					sItem += *pStrImage++;
				}
			}
			while (*pStrImage > _T('\0') && *pStrImage <= _T(' ')) pStrImage = ::CharNext(pStrImage);
			if (*pStrImage++ != _T('=')) break;
			while (*pStrImage > _T('\0') && *pStrImage <= _T(' ')) pStrImage = ::CharNext(pStrImage);
			if (*pStrImage++ != _T('\'')) break;
			while (*pStrImage != _T('\0') && *pStrImage != _T('\''))
			{
				LPTSTR pstrTemp = ::CharNext(pStrImage);
				while (pStrImage < pstrTemp)
				{
					sValue += *pStrImage++;
				}
			}
			if (*pStrImage++ != _T('\'')) break;
			if (!sValue.IsEmpty())
			{
				if (sItem == _T("file") || sItem == _T("res"))
				{
					sImageName = sValue;
				}
				else if (sItem == _T("dest")) {
					m_rcIconDestRc.left = _tcstol(sValue.GetData(), &pstr, 10);  ASSERT(pstr);
					m_rcIconDestRc.top = _tcstol(pstr + 1, &pstr, 10);    ASSERT(pstr);
					m_rcIconDestRc.right = _tcstol(pstr + 1, &pstr, 10);  ASSERT(pstr);
					m_rcIconDestRc.bottom = _tcstol(pstr + 1, &pstr, 10); ASSERT(pstr);
				}
			}
			if (*pStrImage++ != _T(' ')) break;
		}

		return sImageName;
	}

	bool CIconButtonUI::FileExist(LPCTSTR strPath)
	{
		WIN32_FIND_DATA wfd;
		bool rValue = false;
		HANDLE hFind = FindFirstFile(strPath, &wfd);
		if ((hFind != INVALID_HANDLE_VALUE))
		{
			rValue = true;
		}
		FindClose(hFind);
		return rValue;
	}

	void CIconButtonUI::SetAttribute(LPCTSTR pstrName, LPCTSTR pstrValue)
	{
		if( _tcscmp(pstrName, _T("normalimage")) == 0 ) SetNormalImage(pstrValue);
		else if( _tcscmp(pstrName, _T("hotimage")) == 0 ) SetHotImage(pstrValue);
		else if( _tcscmp(pstrName, _T("pushedimage")) == 0 ) SetPushedImage(pstrValue);
		else if( _tcscmp(pstrName, _T("focusedimage")) == 0 ) SetFocusedImage(pstrValue);
		else if( _tcscmp(pstrName, _T("disabledimage")) == 0 ) SetDisabledImage(pstrValue);
		else if( _tcscmp(pstrName, _T("foreimage")) == 0 ) SetForeImage(pstrValue);
		else if( _tcscmp(pstrName, _T("hotforeimage")) == 0 ) SetHotForeImage(pstrValue);
		else if( _tcscmp(pstrName, _T("bindtabindex")) == 0 ) BindTabIndex(_ttoi(pstrValue));
		else if( _tcscmp(pstrName, _T("bindtablayoutname")) == 0 ) BindTabLayoutName(pstrValue);
		else if( _tcscmp(pstrName, _T("hotbkcolor")) == 0 )
		{
			if( *pstrValue == _T('#')) pstrValue = ::CharNext(pstrValue);
			LPTSTR pstr = NULL;
			DWORD clrColor = _tcstoul(pstrValue, &pstr, 16);
			SetHotBkColor(clrColor);
		}
		else if (_tcscmp(pstrName, _T("disiablebkcolor")) == 0)
		{
			if (*pstrValue == _T('#')) pstrValue = ::CharNext(pstrValue);
			LPTSTR pstr = NULL;
			DWORD clrColor = _tcstoul(pstrValue, &pstr, 16);
			SetDisiableBkColor(clrColor);
		}
		else if( _tcscmp(pstrName, _T("hottextcolor")) == 0 )
		{
			if( *pstrValue == _T('#')) pstrValue = ::CharNext(pstrValue);
			LPTSTR pstr = NULL;
			DWORD clrColor = _tcstoul(pstrValue, &pstr, 16);
			SetHotTextColor(clrColor);
		}
		else if( _tcscmp(pstrName, _T("pushedtextcolor")) == 0 )
		{
			if( *pstrValue == _T('#')) pstrValue = ::CharNext(pstrValue);
			LPTSTR pstr = NULL;
			DWORD clrColor = _tcstoul(pstrValue, &pstr, 16);
			SetPushedTextColor(clrColor);
		}
		else if( _tcscmp(pstrName, _T("focusedtextcolor")) == 0 )
		{
			if( *pstrValue == _T('#')) pstrValue = ::CharNext(pstrValue);
			LPTSTR pstr = NULL;
			DWORD clrColor = _tcstoul(pstrValue, &pstr, 16);
			SetFocusedTextColor(clrColor);
		}
		else if( _tcscmp(pstrName, _T("multiline")) == 0 )
		{
			if( _tcscmp(pstrValue, _T("true")) == 0)
			{
				m_uTextStyle = m_uTextStyle & ~DT_SINGLELINE;
				m_uTextStyle |= DT_WORDBREAK;
			}
			else
			{
				m_uTextStyle = m_uTextStyle & ~DT_WORDBREAK;
				m_uTextStyle |= DT_SINGLELINE;
			}
		}
		else if (_tcscmp(pstrName, _T("colorusevalue")) == 0)
		{
			LPTSTR pstr = NULL;
			int nvalue = _tcstoul(pstrValue, &pstr, 10);
			SetColorUseValue(nvalue);
		}
		else if (_tcscmp(pstrName, _T("midlayoutcolor")) == 0)
		{
			if (*pstrValue == _T('#')) pstrValue = ::CharNext(pstrValue);
			LPTSTR pstr = NULL;
			DWORD clrColor = _tcstoul(pstrValue, &pstr, 16);
			SetMidlayoutColor(clrColor);
		}
		else if (_tcscmp(pstrName, _T("usemidcolor")) == 0)
		{
			SetUseMidLayoutColor(_tcscmp(pstrValue, _T("true")) == 0);
		}
		else if (_tcscmp(pstrName, _T("bkiconimg")) == 0)
		{
			m_sBkIcomImage = pstrValue;
		}
		else if (_tcscmp(pstrName, _T("mouseenterevent")) == 0) SetMouseEnterEvent(_tcscmp(pstrValue, _T("true")) == 0);
		else if (_tcscmp(pstrName, _T("mouseleaveevent")) == 0) SetMouseLeaveEvent(_tcscmp(pstrValue, _T("true")) == 0);
		else CLabelUI::SetAttribute(pstrName, pstrValue);
	}

	void CIconButtonUI::SetBkIconImage(LPCTSTR sImage)
	{
		m_sBkIcomImage = sImage;
		Invalidate();
	}

	void CIconButtonUI::PaintBkImage(HDC hDC)
	{
		__super::PaintBkImage(hDC);

		if (!m_pManager) return;
		if (m_sBkIcomImage.IsEmpty()) return;
		if (!m_hIcon)
		{
			m_hIcon = LoadIconImage(m_sBkIcomImage, m_rcIconDestRc.right - m_rcIconDestRc.left, m_rcIconDestRc.bottom - m_rcIconDestRc.top);
		}
		if (m_hIcon)
		{
			DrawIconEx(hDC, m_rcItem.left + m_rcIconDestRc.left, m_rcItem.top + m_rcIconDestRc.top, m_hIcon, m_rcIconDestRc.right - m_rcIconDestRc.left,
				m_rcIconDestRc.bottom - m_rcIconDestRc.top, 0, NULL, DI_NORMAL);
		}
	}

	void CIconButtonUI::PaintText(HDC hDC)
	{
		if( IsFocused() ) m_uButtonState |= UISTATE_FOCUSED;
		else m_uButtonState &= ~ UISTATE_FOCUSED;
		if( !IsEnabled() ) m_uButtonState |= UISTATE_DISABLED;
		else m_uButtonState &= ~ UISTATE_DISABLED;

		if( m_dwTextColor == 0 ) m_dwTextColor = m_pManager->GetDefaultFontColor();
		if( m_dwDisabledTextColor == 0 ) m_dwDisabledTextColor = m_pManager->GetDefaultDisabledColor();

		if( m_sText.IsEmpty() ) return;
		int nLinks = 0;
		RECT rc = m_rcItem;
		rc.left += m_rcTextPadding.left;
		rc.right -= m_rcTextPadding.right;
		rc.top += m_rcTextPadding.top;
		rc.bottom -= m_rcTextPadding.bottom;

		DWORD clrColor = IsEnabled()?m_dwTextColor:m_dwDisabledTextColor;

		if( ((m_uButtonState & UISTATE_PUSHED) != 0) && (GetPushedTextColor() != 0) )
			clrColor = GetPushedTextColor();
		else if( ((m_uButtonState & UISTATE_HOT) != 0) && (GetHotTextColor() != 0) )
			clrColor = GetHotTextColor();
		else if( ((m_uButtonState & UISTATE_FOCUSED) != 0) && (GetFocusedTextColor() != 0) )
			clrColor = GetFocusedTextColor();

		if( m_bShowHtml )
			CRenderEngine::DrawHtmlText(hDC, m_pManager, rc, m_sText, clrColor, \
			NULL, NULL, nLinks, m_uTextStyle);
		else
			CRenderEngine::DrawText(hDC, m_pManager, rc, m_sText, clrColor, \
			m_iFont, m_uTextStyle);
	}

	void CIconButtonUI::PaintStatusImage(HDC hDC)
	{
		if( IsFocused() ) m_uButtonState |= UISTATE_FOCUSED;
		else m_uButtonState &= ~ UISTATE_FOCUSED;
		if( !IsEnabled() ) m_uButtonState |= UISTATE_DISABLED;
		else m_uButtonState &= ~ UISTATE_DISABLED;

		if (m_bUseMidLayoutColor)
		{ //绘制背景与状态层中间的颜色;
			RECT midcolorRc = m_rcPaint;
			int nHeight = m_rcPaint.bottom - m_rcPaint.top;
			midcolorRc.top = m_rcPaint.bottom - (long)(nHeight*(m_nColorUseValue/100.0));
			CRenderEngine::DrawColor(hDC, midcolorRc, GetAdjustColor(m_dwMidLayoutColor));
		}

		if( (m_uButtonState & UISTATE_DISABLED) != 0 ) {
			if( !m_sDisabledImage.IsEmpty() )
			{
				if( !DrawImage(hDC, (LPCTSTR)m_sDisabledImage) ) m_sDisabledImage.Empty();
				else goto Label_ForeImage;
			}
			else if (m_dwDisabledBkColor != 0) {
				CRenderEngine::DrawColor(hDC, m_rcPaint, GetAdjustColor(m_dwDisabledBkColor));
				return;
			}
		}
		else if( (m_uButtonState & UISTATE_PUSHED) != 0 ) {
			if( !m_sPushedImage.IsEmpty() ) {
				if( !DrawImage(hDC, (LPCTSTR)m_sPushedImage) ){
					m_sPushedImage.Empty();
				}
				if( !m_sPushedForeImage.IsEmpty() )
				{
					if( !DrawImage(hDC, (LPCTSTR)m_sPushedForeImage) )
						m_sPushedForeImage.Empty();
					return;
				}
				else goto Label_ForeImage;
			}
		}
		else if( (m_uButtonState & UISTATE_HOT) != 0 ) {
			if( !m_sHotImage.IsEmpty() ) {
				if( !DrawImage(hDC, (LPCTSTR)m_sHotImage) ){
					m_sHotImage.Empty();
				}
				if( !m_sHotForeImage.IsEmpty() ) {
					if( !DrawImage(hDC, (LPCTSTR)m_sHotForeImage) )
						m_sHotForeImage.Empty();
					return;
				}
				else goto Label_ForeImage;
			}
			else if(m_dwHotBkColor != 0) {
				CRenderEngine::DrawColor(hDC, m_rcPaint, GetAdjustColor(m_dwHotBkColor));
				return;
			}
		}
		else if( (m_uButtonState & UISTATE_FOCUSED) != 0 ) {
			if( !m_sFocusedImage.IsEmpty() ) {
				if( !DrawImage(hDC, (LPCTSTR)m_sFocusedImage) ) m_sFocusedImage.Empty();
				else goto Label_ForeImage;
			}
		}

		if( !m_sNormalImage.IsEmpty() ) {
			if( !DrawImage(hDC, (LPCTSTR)m_sNormalImage) ) m_sNormalImage.Empty();
			else goto Label_ForeImage;
		}

		if(!m_sForeImage.IsEmpty() )
			goto Label_ForeImage;

		return;

Label_ForeImage:
		if(!m_sForeImage.IsEmpty() ) {
			if( !DrawImage(hDC, (LPCTSTR)m_sForeImage) ) m_sForeImage.Empty();
		}
	}
}