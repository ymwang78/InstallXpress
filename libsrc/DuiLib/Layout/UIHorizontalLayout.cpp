#include "stdafx.h"
#include "UIHorizontalLayout.h"

#define MOUSEENTER_TIMER (WM_USER+7)
#define MOUSELEAVE_TIMER (WM_USER+8)

namespace DuiLib
{
	CHorizontalLayoutUI::CHorizontalLayoutUI() : m_iSepWidth(0), m_uButtonState(0), m_bImmMode(false)
	{
		ptLastMouse.x = ptLastMouse.y = 0;
		::ZeroMemory(&m_rcNewPos, sizeof(m_rcNewPos));

		m_dwTextColor = 0xFF000000;
		m_bBkCenterDraw = false;
		m_bMouseEnter = false;
		m_bMouseLeave = false;
		m_bxposAsktype = true;
		m_nEntertimerlen = 0;
		m_nLeavetimerlen = 0;
		m_nBkImageWidth = 0;
		m_nBkImageHeight = 0;
		m_nEventRectValue = 120;
		m_bDrawBk = true;
		m_bOpenleavetimer = false;
		m_iFont = 0;
		::ZeroMemory(&m_rcTextPadding, sizeof(m_rcTextPadding));
	}

	LPCTSTR CHorizontalLayoutUI::GetClass() const
	{
		return _T("HorizontalLayoutUI");
	}

	LPVOID CHorizontalLayoutUI::GetInterface(LPCTSTR pstrName)
	{
		if( _tcscmp(pstrName, DUI_CTR_HORIZONTALLAYOUT) == 0 ) return static_cast<CHorizontalLayoutUI*>(this);
		return CContainerUI::GetInterface(pstrName);
	}

	UINT CHorizontalLayoutUI::GetControlFlags() const
	{
		if( IsEnabled() && m_iSepWidth != 0 ) return UIFLAG_SETCURSOR;
		else return 0;
	}

	void CHorizontalLayoutUI::SetPos(RECT rc)
	{
		CControlUI::SetPos(rc);
		rc = m_rcItem;

		// Adjust for inset
		rc.left += m_rcInset.left;
		rc.top += m_rcInset.top;
		rc.right -= m_rcInset.right;
		rc.bottom -= m_rcInset.bottom;

		if( m_items.GetSize() == 0) {
			ProcessScrollBar(rc, 0, 0);
			return;
		}

		if( m_pVerticalScrollBar && m_pVerticalScrollBar->IsVisible() ) rc.right -= m_pVerticalScrollBar->GetFixedWidth();
		if( m_pHorizontalScrollBar && m_pHorizontalScrollBar->IsVisible() ) rc.bottom -= m_pHorizontalScrollBar->GetFixedHeight();

		// Determine the width of elements that are sizeable
		SIZE szAvailable = { rc.right - rc.left, rc.bottom - rc.top };
		if( m_pHorizontalScrollBar && m_pHorizontalScrollBar->IsVisible() ) 
			szAvailable.cx += m_pHorizontalScrollBar->GetScrollRange();

		int nAdjustables = 0;
		int cxFixed = 0;
		int nEstimateNum = 0;
		for( int it1 = 0; it1 < m_items.GetSize(); it1++ ) {
			CControlUI* pControl = static_cast<CControlUI*>(m_items[it1]);
			if( !pControl->IsVisible() ) continue;
			if( pControl->IsFloat() ) continue;
			SIZE sz = pControl->EstimateSize(szAvailable);
			if( sz.cx == 0 ) {
				nAdjustables++;
			}
			else {
				if( sz.cx < pControl->GetMinWidth() ) sz.cx = pControl->GetMinWidth();
				if( sz.cx > pControl->GetMaxWidth() ) sz.cx = pControl->GetMaxWidth();
			}
			cxFixed += sz.cx +  pControl->GetPadding().left + pControl->GetPadding().right;
			nEstimateNum++;
		}
		cxFixed += (nEstimateNum - 1) * m_iChildPadding;

		int cxExpand = 0;
        int cxNeeded = 0;
		if( nAdjustables > 0 ) cxExpand = MAX(0, (szAvailable.cx - cxFixed) / nAdjustables);
		// Position the elements
		SIZE szRemaining = szAvailable;
		int iPosX = rc.left;
		if( m_pHorizontalScrollBar && m_pHorizontalScrollBar->IsVisible() ) {
			iPosX -= m_pHorizontalScrollBar->GetScrollPos();
		}
		int iAdjustable = 0;
		int cxFixedRemaining = cxFixed;
		for( int it2 = 0; it2 < m_items.GetSize(); it2++ ) {
			CControlUI* pControl = static_cast<CControlUI*>(m_items[it2]);
			if( !pControl->IsVisible() ) continue;
			if( pControl->IsFloat() ) {
				SetFloatPos(it2);
				continue;
			}
			RECT rcPadding = pControl->GetPadding();
			szRemaining.cx -= rcPadding.left;
			SIZE sz = pControl->EstimateSize(szRemaining);
			if( sz.cx == 0 ) {
				iAdjustable++;
				sz.cx = cxExpand;
				// Distribute remaining to last element (usually round-off left-overs)
// 				if( iAdjustable == nAdjustables ) {
// 					sz.cx = MAX(0, szRemaining.cx - rcPadding.right - cxFixedRemaining);
// 				}
				if( sz.cx < pControl->GetMinWidth() ) sz.cx = pControl->GetMinWidth();
				if( sz.cx > pControl->GetMaxWidth() ) sz.cx = pControl->GetMaxWidth();
			}
			else {
				if( sz.cx < pControl->GetMinWidth() ) sz.cx = pControl->GetMinWidth();
				if( sz.cx > pControl->GetMaxWidth() ) sz.cx = pControl->GetMaxWidth();

//				cxFixedRemaining -= sz.cx + rcPadding.left + rcPadding.right ;
			}

//			cxFixedRemaining -= m_iChildPadding;

			sz.cy = pControl->GetFixedHeight();
			if( sz.cy == 0 ) sz.cy = rc.bottom - rc.top - rcPadding.top - rcPadding.bottom;
			if( sz.cy < 0 ) sz.cy = 0;
			if( sz.cy < pControl->GetMinHeight() ) sz.cy = pControl->GetMinHeight();
			if( sz.cy > pControl->GetMaxHeight() ) sz.cy = pControl->GetMaxHeight();

			RECT rcCtrl = { iPosX + rcPadding.left, rc.top + rcPadding.top, iPosX + sz.cx + rcPadding.left , rc.top + rcPadding.top + sz.cy};
			pControl->SetPos(rcCtrl);
			iPosX += sz.cx + m_iChildPadding + rcPadding.left + rcPadding.right;
            cxNeeded += sz.cx + rcPadding.left + rcPadding.right;
			szRemaining.cx -= sz.cx + m_iChildPadding + rcPadding.right;
		}
        cxNeeded += (nEstimateNum - 1) * m_iChildPadding;
//reddrain
		if( m_pHorizontalScrollBar != NULL ) {
			if( cxNeeded > rc.right - rc.left ) {
				if( m_pHorizontalScrollBar->IsVisible() ) {
					m_pHorizontalScrollBar->SetScrollRange(cxNeeded - (rc.right - rc.left));
				}
				else {
					m_pHorizontalScrollBar->SetVisible(true);
					m_pHorizontalScrollBar->SetScrollRange(cxNeeded - (rc.right - rc.left));
					m_pHorizontalScrollBar->SetScrollPos(0);
					rc.bottom -= m_pHorizontalScrollBar->GetFixedHeight();
				}
			}
			else {
				if( m_pHorizontalScrollBar->IsVisible() ) {
					m_pHorizontalScrollBar->SetVisible(false);
					m_pHorizontalScrollBar->SetScrollRange(0);
					m_pHorizontalScrollBar->SetScrollPos(0);
					rc.bottom += m_pHorizontalScrollBar->GetFixedHeight();
				}
			}
		}
// 
		// Process the scrollbar
		ProcessScrollBar(rc, cxNeeded, 0);
	}

	void CHorizontalLayoutUI::DoPostPaint(HDC hDC, const RECT& rcPaint)
	{
		if( (m_uButtonState & UISTATE_CAPTURED) != 0 && !m_bImmMode ) {
			RECT rcSeparator = GetThumbRect(true);
			CRenderEngine::DrawColor(hDC, rcSeparator, 0xAA000000);
		}
	}

	void CHorizontalLayoutUI::SetSepWidth(int iWidth)
	{
		m_iSepWidth = iWidth;
	}

	int CHorizontalLayoutUI::GetSepWidth() const
	{
		return m_iSepWidth;
	}

	void CHorizontalLayoutUI::SetSepImmMode(bool bImmediately)
	{
		if( m_bImmMode == bImmediately ) return;
		if( (m_uButtonState & UISTATE_CAPTURED) != 0 && !m_bImmMode && m_pManager != NULL ) {
			m_pManager->RemovePostPaint(this);
		}

		m_bImmMode = bImmediately;
	}

	bool CHorizontalLayoutUI::IsSepImmMode() const
	{
		return m_bImmMode;
	}

	void CHorizontalLayoutUI::SetBkCenterDraw(bool bcenter)
	{
		m_bBkCenterDraw = bcenter;
	}
	
	void CHorizontalLayoutUI::SetBkImageWidth(int nwidth)
	{
		m_nBkImageWidth = nwidth;
	}
	
	void CHorizontalLayoutUI::SetBkImageHeight(int nheight)
	{
		m_nBkImageHeight = nheight;
	}

	void CHorizontalLayoutUI::SetMouseEnterEvent(bool bEnter)
	{
		m_bMouseEnter = bEnter;
	}

	void CHorizontalLayoutUI::SetMouseLeaveEvent(bool bLeave)
	{
		m_bMouseLeave = bLeave;
	}

	void CHorizontalLayoutUI::SetEventRectValue(int nSize)
	{
		m_nEventRectValue = nSize;
	}

	void CHorizontalLayoutUI::SetEventRectType(bool bAsktype) 
	{
		m_bxposAsktype = bAsktype;
	}

	void CHorizontalLayoutUI::SetAttribute(LPCTSTR pstrName, LPCTSTR pstrValue)
	{
		if( _tcscmp(pstrName, _T("sepwidth")) == 0 ) SetSepWidth(_ttoi(pstrValue));
		else if( _tcscmp(pstrName, _T("sepimm")) == 0 ) SetSepImmMode(_tcscmp(pstrValue, _T("true")) == 0);
		else if (_tcscmp(pstrName, _T("bkcenterdraw")) == 0) SetBkCenterDraw(_tcscmp(pstrValue, _T("true")) == 0);
		else if (_tcscmp(pstrName, _T("bkimagew")) == 0) SetBkImageWidth(_ttoi(pstrValue));
		else if (_tcscmp(pstrName, _T("bkimageh")) == 0) SetBkImageHeight(_ttoi(pstrValue));
		else if (_tcscmp(pstrName, _T("mouseenterevent")) == 0) SetMouseEnterEvent(_tcscmp(pstrValue, _T("true")) == 0);
		else if (_tcscmp(pstrName, _T("mouseleaveevent")) == 0) SetMouseLeaveEvent(_tcscmp(pstrValue, _T("true")) == 0);
		else if (_tcscmp(pstrName, _T("eventrectvalue")) == 0) SetEventRectValue(_ttoi(pstrValue));
		else if (_tcscmp(pstrName, _T("asktype")) == 0) SetEventRectType(_tcscmp(pstrValue, _T("true")) == 0);
		else if (_tcscmp(pstrName, _T("bkcoverimage")) == 0) SetBkCoverImage(pstrValue);
		else if (_tcscmp(pstrName, _T("drawbk")) == 0) SetDrawBk(_tcscmp(pstrValue, _T("true")) == 0);
		else if (_tcscmp(pstrName, _T("extext")) == 0)  m_extext = pstrValue;
		else if (_tcscmp(pstrName, _T("textpadding")) == 0) {
			RECT rcTextPadding = { 0 };
			LPTSTR pstr = NULL;
			rcTextPadding.left = _tcstol(pstrValue, &pstr, 10);  ASSERT(pstr);
			rcTextPadding.top = _tcstol(pstr + 1, &pstr, 10);    ASSERT(pstr);
			rcTextPadding.right = _tcstol(pstr + 1, &pstr, 10);  ASSERT(pstr);
			rcTextPadding.bottom = _tcstol(pstr + 1, &pstr, 10); ASSERT(pstr);
			SetTextPadding(rcTextPadding);
		}
		else if (_tcscmp(pstrName, _T("textfont")) == 0) SetFont(_ttoi(pstrValue));
		else if (_tcscmp(pstrName, _T("textcolor")) == 0) {
			if (*pstrValue == _T('#')) pstrValue = ::CharNext(pstrValue);
			LPTSTR pstr = NULL;
			DWORD clrColor = _tcstoul(pstrValue, &pstr, 16);
			SetTextColor(clrColor);
		}
		else CContainerUI::SetAttribute(pstrName, pstrValue);
	}

	void CHorizontalLayoutUI::SetExText(LPCTSTR pstrValue)
	{
		m_extext = pstrValue;
		Invalidate();
	}

	void CHorizontalLayoutUI::SetTextColor(DWORD dtextcolor)
	{
		m_dwTextColor = dtextcolor;
	}

	void CHorizontalLayoutUI::SetFont(int nfont)
	{
		m_iFont = nfont;
	}

	void CHorizontalLayoutUI::SetDrawBk(bool bDrawBk)
	{
		m_bDrawBk = bDrawBk;
		Invalidate();
	}

	void CHorizontalLayoutUI::SetTextPadding(RECT rc)
	{
		m_rcTextPadding = rc;
		Invalidate();
	}

	void CHorizontalLayoutUI::DoEvent(TEventUI& event)
	{
		if( m_iSepWidth != 0 ) 
		{
			if( event.Type == UIEVENT_BUTTONDOWN && IsEnabled() )
			{
				RECT rcSeparator = GetThumbRect(false);
				if( ::PtInRect(&rcSeparator, event.ptMouse) ) {
					m_uButtonState |= UISTATE_CAPTURED;
					ptLastMouse = event.ptMouse;
					m_rcNewPos = m_rcItem;
					if( !m_bImmMode && m_pManager ) m_pManager->AddPostPaint(this);
					return;
				}
			}
			if( event.Type == UIEVENT_BUTTONUP )
			{
				if( (m_uButtonState & UISTATE_CAPTURED) != 0 ) {
					m_uButtonState &= ~UISTATE_CAPTURED;
					m_rcItem = m_rcNewPos;
					if( !m_bImmMode && m_pManager ) m_pManager->RemovePostPaint(this);
					NeedParentUpdate();
					return;
				}
			}
			if( event.Type == UIEVENT_MOUSEMOVE )
			{
				if( (m_uButtonState & UISTATE_CAPTURED) != 0 ) {
					LONG cx = event.ptMouse.x - ptLastMouse.x;
					ptLastMouse = event.ptMouse;
					RECT rc = m_rcNewPos;
					if( m_iSepWidth >= 0 ) {
						if( cx > 0 && event.ptMouse.x < m_rcNewPos.right - m_iSepWidth ) return;
						if( cx < 0 && event.ptMouse.x > m_rcNewPos.right ) return;
						rc.right += cx;
						if( rc.right - rc.left <= GetMinWidth() ) {
							if( m_rcNewPos.right - m_rcNewPos.left <= GetMinWidth() ) return;
							rc.right = rc.left + GetMinWidth();
						}
						if( rc.right - rc.left >= GetMaxWidth() ) {
							if( m_rcNewPos.right - m_rcNewPos.left >= GetMaxWidth() ) return;
							rc.right = rc.left + GetMaxWidth();
						}
					}
					else {
						if( cx > 0 && event.ptMouse.x < m_rcNewPos.left ) return;
						if( cx < 0 && event.ptMouse.x > m_rcNewPos.left - m_iSepWidth ) return;
						rc.left += cx;
						if( rc.right - rc.left <= GetMinWidth() ) {
							if( m_rcNewPos.right - m_rcNewPos.left <= GetMinWidth() ) return;
							rc.left = rc.right - GetMinWidth();
						}
						if( rc.right - rc.left >= GetMaxWidth() ) {
							if( m_rcNewPos.right - m_rcNewPos.left >= GetMaxWidth() ) return;
							rc.left = rc.right - GetMaxWidth();
						}
					}

					CDuiRect rcInvalidate = GetThumbRect(true);
					m_rcNewPos = rc;
					m_cxyFixed.cx = m_rcNewPos.right - m_rcNewPos.left;

					if( m_bImmMode ) {
						m_rcItem = m_rcNewPos;
						NeedParentUpdate();
					}
					else {
						rcInvalidate.Join(GetThumbRect(true));
						rcInvalidate.Join(GetThumbRect(false));
						if( m_pManager ) m_pManager->Invalidate(rcInvalidate);
					}
					return;
				}
			}
			if( event.Type == UIEVENT_SETCURSOR )
			{
				RECT rcSeparator = GetThumbRect(false);
				if( IsEnabled() && ::PtInRect(&rcSeparator, event.ptMouse) ) {
					::SetCursor(::LoadCursor(NULL, IDC_SIZEWE));
					return;
				}
			}
		}
		else 
		{
			RECT rect = m_rcItem;
			if (m_nEventRectValue > 0)
			{
				if (m_bxposAsktype)
					rect.left = rect.right - m_nEventRectValue;
				else
					rect.right = rect.left + m_nEventRectValue;
			}			
			if (event.Type == UIEVENT_MOUSEENTER && m_bMouseEnter && m_pManager && PtInRect(&rect,event.ptMouse))
			{				
				if (m_nEntertimerlen <= 100) {
					m_pManager->SendNotify(this, DUI_MSGTYPE_MOUSEENTER, event.wParam, event.lParam);
				}
				else {
					if (m_bOpenleavetimer) {
						m_pManager->KillTimer(this, MOUSELEAVE_TIMER);
						m_bOpenleavetimer = false;
					}
					m_pManager->SetTimer(this, MOUSEENTER_TIMER, m_nEntertimerlen);
				}
				//m_pManager->SendNotify(this, DUI_MSGTYPE_MOUSEENTER, event.wParam, event.lParam);
			}
			else if (event.Type == UIEVENT_MOUSELEAVE && m_bMouseLeave && m_pManager)
			{		
				if (m_nEntertimerlen > 100) {
					m_pManager->KillTimer(this, MOUSEENTER_TIMER);
				}
				if (m_nLeavetimerlen <= 100) {
					m_pManager->SendNotify(this, DUI_MSGTYPE_MOUSELEAVE, event.wParam, event.lParam);
				}
				else {
					m_pManager->SetTimer(this, MOUSELEAVE_TIMER, m_nLeavetimerlen);
					m_bOpenleavetimer = true;
				}
				//m_pManager->SendNotify(this, DUI_MSGTYPE_MOUSELEAVE, event.wParam, event.lParam);					
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
		}
		CContainerUI::DoEvent(event);
	}

	void CHorizontalLayoutUI::SetMouserEnterLeaveTimerlen(int nentertimer, int nleavetimer)
	{
		m_nEntertimerlen = nentertimer;
		m_nLeavetimerlen = nleavetimer;
	}

	RECT CHorizontalLayoutUI::GetThumbRect(bool bUseNew) const
	{
		if( (m_uButtonState & UISTATE_CAPTURED) != 0 && bUseNew) {
			if( m_iSepWidth >= 0 ) return CDuiRect(m_rcNewPos.right - m_iSepWidth, m_rcNewPos.top, m_rcNewPos.right, m_rcNewPos.bottom);
			else return CDuiRect(m_rcNewPos.left, m_rcNewPos.top, m_rcNewPos.left - m_iSepWidth, m_rcNewPos.bottom);
		}
		else {
			if( m_iSepWidth >= 0 ) return CDuiRect(m_rcItem.right - m_iSepWidth, m_rcItem.top, m_rcItem.right, m_rcItem.bottom);
			else return CDuiRect(m_rcItem.left, m_rcItem.top, m_rcItem.left - m_iSepWidth, m_rcItem.bottom);
		}
	}

	void CHorizontalLayoutUI::PaintBkImage(HDC hDC)
	{
		if (!m_bDrawBk)
			return;

		if ( !m_bBkCenterDraw || m_sBkImage.IsEmpty() || m_nBkImageWidth==0 || m_nBkImageHeight==0)
		{
			__super::PaintBkImage(hDC);
			return;
		}

		if (m_bBkCenterDraw)
		{
			RECT rcSource;
			rcSource.bottom = rcSource.top = 0;
			rcSource.left = rcSource.right = 0;

			int nwidth = m_rcItem.right - m_rcItem.left;
			int nheight = m_rcItem.bottom - m_rcItem.top;

			rcSource.left = (m_nBkImageWidth - nwidth) / 2;
			rcSource.right = rcSource.left + nwidth;

			rcSource.top = (m_nBkImageHeight - nheight) / 2;
			rcSource.bottom = rcSource.top + nheight;

			CRenderEngine::DrawImageString(hDC, m_pManager, m_rcItem, m_rcPaint, (LPCTSTR)m_sBkImage, 0, 0, rcSource, NULL);
		}
	}

	void CHorizontalLayoutUI::PaintBkConverImage(HDC hDC)
	{
		if (!(m_sBkCoverImage.IsEmpty()) && !DrawImage(hDC, (LPCTSTR)m_sBkCoverImage)) m_sBkCoverImage.Empty();

		if (m_extext.IsEmpty())
			return;

		RECT rc = m_rcItem;
		rc.left += m_rcTextPadding.left;
		rc.right -= m_rcTextPadding.right;
		rc.top += m_rcTextPadding.top;
		rc.bottom -= m_rcTextPadding.bottom;		
		CRenderEngine::DrawText(hDC, m_pManager, rc, m_extext, m_dwTextColor, m_iFont, DT_SINGLELINE);
	}
}
