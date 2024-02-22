#include "stdafx.h"
#include "UITabOption.h"

#define UITABOPTION_TIMER (WM_USER+3017)

namespace DuiLib
{
	CTabOptionUI::CTabOptionUI() 
		: m_bSelected(false)
		, m_dwSelectedTextColor(0)
		, m_uCloseButtonState(0)
	{
		m_iSelfont = -1;
		m_sCloseBtnImage = _T("");
		m_sCloseBtnHotImage = _T("");
		m_sCloseBtnPushendImage = _T("");
		m_rcCloseRect = CDuiRect(0,0,0,0);
		m_rcCloseEvent.left = 0;  m_rcCloseRect.top = 0;
		m_rcCloseEvent.right = 0;  m_rcCloseEvent.bottom = 0;
		m_bForAnimo = false;
		m_bShowForeImage = true;
		m_nforimageAnim = 0;
		m_nclosebtnrightpadding = 0;
		m_nCurAnimcount = 0;
		m_mAnimEsp = 300;
		m_rcForeDestRect.Empty();
		m_rcSelTextPadding.Empty();
		m_sDrawText = _T("");
	}

	CTabOptionUI::~CTabOptionUI()
	{
		if( !m_sGroupName.IsEmpty() && m_pManager ) m_pManager->RemoveOptionGroup(m_sGroupName, this);
	}

	LPCTSTR CTabOptionUI::GetClass() const
	{
		return _T("TabOptionUI");
	}

	LPVOID CTabOptionUI::GetInterface(LPCTSTR pstrName)
	{
		if( _tcscmp(pstrName, DUI_CTR_TABOPTION) == 0 ) return static_cast<CTabOptionUI*>(this);
		return CButtonUI::GetInterface(pstrName);
	}

	void CTabOptionUI::SetManager(CPaintManagerUI* pManager, CControlUI* pParent, bool bInit)
	{
		CControlUI::SetManager(pManager, pParent, bInit);
		if( bInit && !m_sGroupName.IsEmpty() ) {
			if (m_pManager) m_pManager->AddOptionGroup(m_sGroupName, this);
		}
	}

	LPCTSTR CTabOptionUI::GetGroup() const
	{
		return m_sGroupName;
	}

	void CTabOptionUI::SetGroup(LPCTSTR pStrGroupName)
	{
		if( pStrGroupName == NULL ) {
			if( m_sGroupName.IsEmpty() ) return;
			m_sGroupName.Empty();
		}
		else {
			if( m_sGroupName == pStrGroupName ) return;
			if (!m_sGroupName.IsEmpty() && m_pManager) m_pManager->RemoveOptionGroup(m_sGroupName, this);
			m_sGroupName = pStrGroupName;
		}

		if( !m_sGroupName.IsEmpty() ) {
			if (m_pManager) m_pManager->AddOptionGroup(m_sGroupName, this);
		}
		else {
			if (m_pManager) m_pManager->RemoveOptionGroup(m_sGroupName, this);
		}

		Selected(m_bSelected);
	}

	bool CTabOptionUI::IsSelected() const
	{
		return m_bSelected;
	}

	void CTabOptionUI::Selected(bool bSelected)
	{
//		if( m_bSelected == bSelected ) return;
		m_bSelected = bSelected;
		if( m_bSelected ) m_uButtonState |= UISTATE_SELECTED;
		else m_uButtonState &= ~UISTATE_SELECTED;

		if( m_pManager != NULL ) {
			if( !m_sGroupName.IsEmpty() ) {
				if( m_bSelected ) {
					CStdPtrArray* aOptionGroup = m_pManager->GetOptionGroup(m_sGroupName);
					for( int i = 0; i < aOptionGroup->GetSize(); i++ ) {
						CTabOptionUI* pControl = static_cast<CTabOptionUI*>(aOptionGroup->GetAt(i));
						if( pControl != this ) {
							pControl->Selected(false);
						}
					}
					m_pManager->SendNotify(this, DUI_MSGTYPE_SELECTCHANGED);
				}
			}
			else {
				m_pManager->SendNotify(this, DUI_MSGTYPE_SELECTCHANGED);
			}
		}

		Invalidate();
	}

	void CTabOptionUI::Selected(bool bSelected, bool bSendEenvet)
	{
		m_bSelected = bSelected;
		if (m_bSelected) m_uButtonState |= UISTATE_SELECTED;
		else m_uButtonState &= ~UISTATE_SELECTED;

		if (m_pManager != NULL) {
			if (!m_sGroupName.IsEmpty()) {
				if (m_bSelected) {
					CStdPtrArray* aOptionGroup = m_pManager->GetOptionGroup(m_sGroupName);
					for (int i = 0; i < aOptionGroup->GetSize(); i++) {
						CTabOptionUI* pControl = static_cast<CTabOptionUI*>(aOptionGroup->GetAt(i));
						if (pControl != this) {
							pControl->Selected(false,bSendEenvet);
						}
					}
					if(bSendEenvet) m_pManager->SendNotify(this, DUI_MSGTYPE_SELECTCHANGED);
				}
			}
			else {
				if (bSendEenvet) m_pManager->SendNotify(this, DUI_MSGTYPE_SELECTCHANGED);
			}
		}
		Invalidate();
	}

	bool CTabOptionUI::Activate()
	{
		if( !CButtonUI::Activate() ) return false;
		if( !m_sGroupName.IsEmpty() ) Selected(true);
		else Selected(!m_bSelected);

		return true;
	}

	void CTabOptionUI::SetEnabled(bool bEnable)
	{
		CControlUI::SetEnabled(bEnable);
		if( !IsEnabled() ) {
			if( m_bSelected ) m_uButtonState = UISTATE_SELECTED;
			else m_uButtonState = 0;
		}
	}

	LPCTSTR CTabOptionUI::GetSelectedImage()
	{
		return m_sSelectedImage;
	}

	void CTabOptionUI::SetSelectedImage(LPCTSTR pStrImage)
	{
		m_sSelectedImage = pStrImage;
		Invalidate();
	}

	LPCTSTR CTabOptionUI::GetSelectedHotImage()
	{
		return m_sSelectedHotImage;
	}

	void CTabOptionUI::SetSelectedHotImage( LPCTSTR pStrImage )
	{
		m_sSelectedHotImage = pStrImage;
		Invalidate();
	}

	LPCTSTR CTabOptionUI::GetSelectedPushedImage()
	{
		return m_sSelectedPushedImage;
	}

	void CTabOptionUI::SetSelectedPushedImage(LPCTSTR pStrImage)
	{
		m_sSelectedPushedImage = pStrImage;
		Invalidate();
	}

	void CTabOptionUI::SetSelectedTextColor(DWORD dwTextColor)
	{
		m_dwSelectedTextColor = dwTextColor;
	}

	DWORD CTabOptionUI::GetSelectedTextColor()
	{
		if (m_dwSelectedTextColor == 0) m_dwSelectedTextColor = m_pManager->GetDefaultFontColor();
		return m_dwSelectedTextColor;
	}

	void CTabOptionUI::SetSelectedBkColor( DWORD dwBkColor )
	{
		m_dwSelectedBkColor = dwBkColor;
	}

	DWORD CTabOptionUI::GetSelectBkColor()
	{
		return m_dwSelectedBkColor;
	}

	LPCTSTR CTabOptionUI::GetForeImage()
	{
		return m_sForeImage;
	}

	void CTabOptionUI::SetForeImage(LPCTSTR pStrImage)
	{
		m_sForeImage = pStrImage;
		Invalidate();
	}

	LPCTSTR CTabOptionUI::GetSelectedForedImage()
	{
		return m_sSelectedForeImage;
	}

	void CTabOptionUI::SetSelectedForedImage(LPCTSTR pStrImage)
	{
		m_sSelectedForeImage = pStrImage;
		Invalidate();
	}

	SIZE CTabOptionUI::EstimateSize(SIZE szAvailable)
	{
		if( m_cxyFixed.cy == 0 ) return CDuiSize(m_cxyFixed.cx, m_pManager->GetFontInfo(GetFont())->tm.tmHeight + 8);
		return CControlUI::EstimateSize(szAvailable);
	}

	void CTabOptionUI::SetAttribute(LPCTSTR pstrName, LPCTSTR pstrValue)
	{
		if (_tcscmp(pstrName, _T("group")) == 0) SetGroup(pstrValue);
		else if (_tcscmp(pstrName, _T("selected")) == 0) Selected(_tcscmp(pstrValue, _T("true")) == 0);
		else if (_tcscmp(pstrName, _T("selectedimage")) == 0) SetSelectedImage(pstrValue);
		else if (_tcscmp(pstrName, _T("selectedhotimage")) == 0) SetSelectedHotImage(pstrValue);
		else if (_tcscmp(pstrName, _T("selectedpushedimage")) == 0) SetSelectedPushedImage(pstrValue);
		else if (_tcscmp(pstrName, _T("foreimage")) == 0) SetForeImage(pstrValue);
		else if (_tcscmp(pstrName, _T("selectedforeimage")) == 0) SetSelectedForedImage(pstrValue);
		else if (_tcscmp(pstrName, _T("selectedbkcolor")) == 0) {
			if (*pstrValue == _T('#')) pstrValue = ::CharNext(pstrValue);
			LPTSTR pstr = NULL;
			DWORD clrColor = _tcstoul(pstrValue, &pstr, 16);
			SetSelectedBkColor(clrColor);
		}
		else if (_tcscmp(pstrName, _T("selectedtextcolor")) == 0) {
			if (*pstrValue == _T('#')) pstrValue = ::CharNext(pstrValue);
			LPTSTR pstr = NULL;
			DWORD clrColor = _tcstoul(pstrValue, &pstr, 16);
			SetSelectedTextColor(clrColor);
		}
		else if (_tcscmp(pstrName, _T("selectfont")) == 0) {
			m_iSelfont = _ttoi(pstrValue);
		}
		else if (_tcscmp(pstrName, _T("closepadding")) == 0) {
			m_nclosebtnrightpadding = _ttoi(pstrValue);
		}
		else if (_tcscmp(pstrName, _T("closebtnimage")) == 0) SetCloseBtnImage(pstrValue);			//关闭按钮常态;
		else if (_tcscmp(pstrName, _T("closebtnhotimage")) == 0)  SetCloseBtnHotImage(pstrValue);	//关闭按钮悬浮态;
		else if (_tcscmp(pstrName, _T("closebtnpushedimage")) == 0) SetCloseBtnPushedImage(pstrValue);	//关闭按钮按下态;
		else if (_tcscmp(pstrName, _T("closebtnpos")) == 0)
		{/*关闭按钮位置与大小; left,top 表示位置X,Y默认右对齐, right,bottom代表大小;*/
			LPTSTR pstr = NULL;
			m_rcCloseRect.left = _tcstol(pstrValue, &pstr, 10);  ASSERT(pstr);
			m_rcCloseRect.top = _tcstol(pstr + 1, &pstr, 10);    ASSERT(pstr);
			m_rcCloseRect.right = _tcstol(pstr + 1, &pstr, 10);  ASSERT(pstr);
			m_rcCloseRect.bottom = _tcstol(pstr + 1, &pstr, 10); ASSERT(pstr);
		}
		else if (_tcscmp(pstrName, _T("foreamtionnum")) == 0) {		//前进动画最大值;
			SetForeImageAnimCount(_wtoi(pstrValue));
		}
		else if (_tcscmp(pstrName, _T("foreamtion")) == 0) {	//前景是否需要动画;
			SetForeImageAnim(_tcscmp(pstrValue, _T("true")) == 0);
		}
		else if (_tcscmp(pstrName, _T("foredest")) == 0)
		{	//前景位置;
			LPTSTR pstr = NULL;
			m_rcForeDestRect.left = _tcstol(pstrValue, &pstr, 10);  ASSERT(pstr);
			m_rcForeDestRect.top = _tcstol(pstr + 1, &pstr, 10);    ASSERT(pstr);
			m_rcForeDestRect.right = _tcstol(pstr + 1, &pstr, 10);  ASSERT(pstr);
			m_rcForeDestRect.bottom = _tcstol(pstr + 1, &pstr, 10); ASSERT(pstr);
		}		
		else if (_tcscmp(pstrName, _T("showforeimage")) == 0)
		{ //是否显示前景;
			SetShowForeImage(_tcscmp(pstrValue, _T("true")) == 0);
		}
		else if (_tcscmp(pstrName, _T("seltextpadding")) == 0) { //选中时候文字位置;
			LPTSTR pstr = NULL;
			m_rcSelTextPadding.left = _tcstol(pstrValue, &pstr, 10);  ASSERT(pstr);
			m_rcSelTextPadding.top = _tcstol(pstr + 1, &pstr, 10);    ASSERT(pstr);
			m_rcSelTextPadding.right = _tcstol(pstr + 1, &pstr, 10);  ASSERT(pstr);
			m_rcSelTextPadding.bottom = _tcstol(pstr + 1, &pstr, 10); ASSERT(pstr);
		}
		else CButtonUI::SetAttribute(pstrName, pstrValue);
	}

	LPCTSTR CTabOptionUI::GetCloseBtnImage()
	{
		return m_sCloseBtnImage;
	}

	void CTabOptionUI::SetCloseBtnImage(LPCTSTR pStrImage)
	{
		m_sCloseBtnImage = pStrImage;
	}

	LPCTSTR CTabOptionUI::GetCloseBtnHotImage()
	{
		return m_sCloseBtnHotImage;
	}

	void CTabOptionUI::SetCloseBtnHotImage(LPCTSTR pStrImage)
	{
		m_sCloseBtnHotImage = pStrImage;
	}

	LPCTSTR CTabOptionUI::GetCloseBtnPushedImage()
	{
		return m_sCloseBtnPushendImage;
	}

	void CTabOptionUI::SetCloseBtnPushedImage(LPCTSTR pStrImage)
	{
		m_sCloseBtnPushendImage = pStrImage;
	}

	CDuiRect CTabOptionUI::GetCloseBtnPositon()
	{
		return m_rcCloseRect;
	}

	void CTabOptionUI::SetForeImageAnim(bool bAnim)
	{
		m_bForAnimo = bAnim;
		if (m_bForAnimo) m_pManager->SetTimer(this, UITABOPTION_TIMER, m_mAnimEsp);
	}

	void CTabOptionUI::SetForeImageAnimCount(int nAnim)
	{
		m_nforimageAnim = nAnim;
	}

	void CTabOptionUI::SetShowForeImage(bool bShow)
	{
		m_bShowForeImage = bShow;
		Invalidate();
	}

	void CTabOptionUI::PaintStatusImage(HDC hDC)
	{
		if( (m_uButtonState & UISTATE_PUSHED) != 0 && IsSelected() && !m_sSelectedPushedImage.IsEmpty()) {
			if( !DrawImage(hDC, (LPCTSTR)m_sSelectedPushedImage) )
				m_sSelectedPushedImage.Empty();
			else goto Label_ForeImage;
		}
		else if ((m_uButtonState & UISTATE_HOT) != 0 && IsSelected() && !m_sSelectedHotImage.IsEmpty()) {		
				if (!DrawImage(hDC, (LPCTSTR)m_sSelectedHotImage))
					m_sSelectedHotImage.Empty();
				else goto Label_ForeImage;		
		}			
		else if( (m_uButtonState & UISTATE_SELECTED) != 0 ) {
			if( !m_sSelectedImage.IsEmpty() ) {
				if( !DrawImage(hDC, (LPCTSTR)m_sSelectedImage) ) m_sSelectedImage.Empty();
				else goto Label_ForeImage;
			}
			else if(m_dwSelectedBkColor != 0) {
				CRenderEngine::DrawColor(hDC, m_rcPaint, GetAdjustColor(m_dwSelectedBkColor));
				goto Label_ForeImage;
			}	
		}

		CButtonUI::PaintStatusImage(hDC);

	Label_ForeImage:
		if (m_bShowForeImage)
		{
			if (m_bForAnimo)
			{
				if (m_sForeImage.IsEmpty() == false)
				{
					CDuiString strImage = _T("");
					strImage.Format(_T("%s source='%d,%d,%d,%d' dest='%d,%d,%d,%d'"), m_sForeImage.GetData()
						,(m_rcForeDestRect.GetWidth())*(m_nCurAnimcount - 1),0, (m_rcForeDestRect.GetWidth())*m_nCurAnimcount
						, m_rcForeDestRect.GetHeight(), m_rcForeDestRect.left, m_rcForeDestRect.top, m_rcForeDestRect.right, m_rcForeDestRect.bottom);
					if (!DrawImage(hDC, (LPCTSTR)strImage) && m_nCurAnimcount > 0) m_sForeImage.Empty();
				}
				else
				{
					m_bForAnimo = false;
					m_nCurAnimcount = 0;
				}
			}
			else
			{
				if (IsSelected() && !m_sSelectedForeImage.IsEmpty())
				{
					if (!DrawImage(hDC, (LPCTSTR)m_sSelectedForeImage)) m_sSelectedForeImage.Empty();
				}
				else if (!m_sForeImage.IsEmpty())
				{
					if (!DrawImage(hDC, (LPCTSTR)m_sForeImage)) m_sForeImage.Empty();
				}
			}			
		}
		
		if ((m_uButtonState & UISTATE_SELECTED) != 0)
		{
			RECT rc = m_rcItem;
			CDuiString strClosePt = _T("");		
			int x = m_rcCloseRect.left > 0 ? m_rcCloseRect.left : (rc.right - rc.left) - m_rcCloseRect.right - m_nclosebtnrightpadding;
			int y = m_rcCloseRect.top;
			
			//if (m_rcCloseEvent.left == m_rcCloseEvent.right && m_rcCloseEvent.right == 0)
			{//设置事件触发区域;
				m_rcCloseEvent.left = m_rcCloseRect.left > 0 ? (m_rcItem.left + m_rcCloseRect.right) : (m_rcItem.right - m_rcCloseRect.right - m_nclosebtnrightpadding);
				m_rcCloseEvent.right = m_rcCloseEvent.left + m_rcCloseRect.right;
				m_rcCloseEvent.top = m_rcItem.top + y;
				m_rcCloseEvent.bottom = m_rcCloseEvent.top + m_rcCloseRect.bottom;
			}

			strClosePt.Format(_T("dest='%d,%d,%d,%d'"), x, y, x + m_rcCloseRect.right, y + m_rcCloseRect.bottom);
			if (m_sCloseBtnImage.IsEmpty() == false && m_uCloseButtonState == 0) {
				if (!DrawImage(hDC, (LPCTSTR)m_sCloseBtnImage, strClosePt)) m_sCloseBtnImage.Empty();
			}
			else if (m_sCloseBtnHotImage.IsEmpty() == false && (m_uCloseButtonState & UISTATE_HOT) != 0){
				if (!DrawImage(hDC, (LPCTSTR)m_sCloseBtnHotImage, strClosePt)) m_sCloseBtnHotImage.Empty();
			}
			else if (m_sCloseBtnPushendImage.IsEmpty() == false && (m_uCloseButtonState & UISTATE_PUSHED) != 0) {
				if (!DrawImage(hDC, (LPCTSTR)m_sCloseBtnPushendImage, strClosePt)) m_sCloseBtnPushendImage.Empty();
			}
		}
	}

	void CTabOptionUI::PaintText(HDC hDC)
	{
		if( (m_uButtonState & UISTATE_SELECTED) != 0 )
		{
			DWORD oldTextColor = m_dwTextColor;
			if( m_dwSelectedTextColor != 0 ) m_dwTextColor = m_dwSelectedTextColor;

			if( m_dwTextColor == 0 ) m_dwTextColor = m_pManager->GetDefaultFontColor();
			if( m_dwDisabledTextColor == 0 ) m_dwDisabledTextColor = m_pManager->GetDefaultDisabledColor();

			if(m_sDrawText.IsEmpty() )
				return;

			int nLinks = 0;
			RECT rc = m_rcItem;
			rc.left += m_rcSelTextPadding.left;
			rc.right -= m_rcSelTextPadding.right;
			rc.top += m_rcSelTextPadding.top;
			rc.bottom -= m_rcSelTextPadding.bottom;

			if( m_bShowHtml )
				CRenderEngine::DrawHtmlText(hDC, m_pManager, rc, m_sDrawText, IsEnabled()?m_dwTextColor:m_dwDisabledTextColor, \
				NULL, NULL, nLinks, ((m_uTextStyle & ~DT_CENTER)|DT_LEFT));
			else
				CRenderEngine::DrawText(hDC, m_pManager, rc, m_sDrawText, IsEnabled() ? m_dwTextColor : m_dwDisabledTextColor,  \
					m_iSelfont >= 0 ? m_iSelfont : m_iFont, m_uTextStyle);

			m_dwTextColor = oldTextColor;
		}
		else 
		{
			if (IsFocused()) m_uButtonState |= UISTATE_FOCUSED;
			else m_uButtonState &= ~UISTATE_FOCUSED;
			if (!IsEnabled()) m_uButtonState |= UISTATE_DISABLED;
			else m_uButtonState &= ~UISTATE_DISABLED;

			if (m_dwTextColor == 0) m_dwTextColor = m_pManager->GetDefaultFontColor();
			if (m_dwDisabledTextColor == 0) m_dwDisabledTextColor = m_pManager->GetDefaultDisabledColor();

			if (m_sDrawText.IsEmpty())
				return;

			int nLinks = 0;
			RECT rc = m_rcItem;
			rc.left += m_rcTextPadding.left;
			rc.right -= m_rcTextPadding.right;
			rc.top += m_rcTextPadding.top;
			rc.bottom -= m_rcTextPadding.bottom;

			DWORD clrColor = ((IsEnabled() || m_bIgnoreEnable)) ? m_dwTextColor : m_dwDisabledTextColor;

			if (((m_uButtonState & UISTATE_PUSHED) != 0) && (GetPushedTextColor() != 0))
				clrColor = GetPushedTextColor();
			else if (((m_uButtonState & UISTATE_HOT) != 0) && (GetHotTextColor() != 0))
				clrColor = GetHotTextColor();
			else if (((m_uButtonState & UISTATE_FOCUSED) != 0) && (GetFocusedTextColor() != 0))
				clrColor = GetFocusedTextColor();

			if (m_bShowHtml)
				CRenderEngine::DrawHtmlText(hDC, m_pManager, rc, m_sDrawText, clrColor, \
					NULL, NULL, nLinks, m_uTextStyle);
			else
				CRenderEngine::DrawText(hDC, m_pManager, rc, m_sDrawText, clrColor, \
					m_iFont, m_uTextStyle);
		}
	}

	void CTabOptionUI::SetPos(RECT rc)
	{
		CButtonUI::SetPos(rc);
		m_sDrawText = CRenderEngine::GetLimitText(m_pManager->GetPaintDC(), m_pManager, m_sText, m_iFont, (rc.right - rc.left) - m_nclosebtnrightpadding * 3);
		if (m_sDrawText == _T("..."))
			m_sDrawText = m_sText.GetAt(0);
	}

	void CTabOptionUI::DoEvent(TEventUI& event)
	{
		if (!IsMouseEnabled() && event.Type > UIEVENT__MOUSEBEGIN && event.Type < UIEVENT__MOUSEEND) {
			if (m_pParent != NULL) m_pParent->DoEvent(event);
			else CButtonUI::DoEvent(event);
			return;
		}

		if (event.Type == UIEVENT_SETFOCUS)
		{
			Invalidate();
		}
		else if (event.Type == UIEVENT_KILLFOCUS)
		{
			Invalidate();
		}
		else if (event.Type == UIEVENT_KEYDOWN)
		{
			if (IsKeyboardEnabled()) {
				if (event.chKey == VK_SPACE || event.chKey == VK_RETURN) {
					Activate();
					return;
				}
			}
		}
		else if (event.Type == UIEVENT_BUTTONDOWN)
		{
			if (::PtInRect(&m_rcItem, event.ptMouse) && (IsEnabled() || m_bIgnoreEnable)) {
				m_uButtonState |= UISTATE_PUSHED | UISTATE_CAPTURED;
				if (::PtInRect(&m_rcCloseEvent, event.ptMouse)) {
					m_uCloseButtonState |= UISTATE_PUSHED | UISTATE_CAPTURED;
				}	
				Invalidate();
			}
			return;
		}
		else if (event.Type == UIEVENT_DBLCLICK)
		{
			if (::PtInRect(&m_rcItem, event.ptMouse) && (IsEnabled() || m_bIgnoreEnable)) {
				m_uButtonState |= UISTATE_PUSHED | UISTATE_CAPTURED;
				if (::PtInRect(&m_rcCloseEvent, event.ptMouse)) {
					m_uCloseButtonState |= UISTATE_PUSHED | UISTATE_CAPTURED;
				}
				Invalidate();
				if (!m_bDisbleDbClick) m_bDbClick = true;
			}
			return;
		}
		else if (event.Type == UIEVENT_MOUSEMOVE)
		{
			if ((m_uButtonState & UISTATE_CAPTURED) != 0) 
			{
				if (::PtInRect(&m_rcItem, event.ptMouse)) m_uButtonState |= UISTATE_PUSHED;
				else m_uButtonState &= ~UISTATE_PUSHED;

				if (::PtInRect(&m_rcCloseEvent, event.ptMouse))
					m_uCloseButtonState |= UISTATE_CAPTURED;				
				else
					m_uCloseButtonState &= ~UISTATE_PUSHED;
				Invalidate();
			}
			else if ((m_uButtonState & UISTATE_CAPTURED) == 0 
				&& (IsEnabled() || m_bIgnoreEnable))
			{
				bool binvalidate = false;
				if ((m_uCloseButtonState & UISTATE_HOT) == 0 &&
					::PtInRect(&m_rcCloseEvent, event.ptMouse))
				{
					m_uCloseButtonState |= UISTATE_HOT;
					binvalidate = true;
				}
				else if ((m_uCloseButtonState & UISTATE_HOT) != 0 &&
					::PtInRect(&m_rcCloseEvent, event.ptMouse) == false)
				{
					m_uCloseButtonState &= ~UISTATE_HOT;
					binvalidate = true;
				}
				if (binvalidate) Invalidate();
			}
			return;
		}
		else if (event.Type == UIEVENT_BUTTONUP)
		{
			if ((m_uButtonState & UISTATE_CAPTURED) != 0) {
				if (::PtInRect(&m_rcItem, event.ptMouse))
				{
					if (m_bDisbleDbClick) Activate();
					else if (m_bDbClick && m_pManager)
					{
						m_pManager->SendNotify(this, DUI_MSGTYPE_DBCLICK);						
						m_bDbClick = false;
					}		

					if (::PtInRect(&m_rcCloseEvent, event.ptMouse))
					{
						m_pManager->SendNotify(this, DUI_MSGTYPE_CLOSEOPTION);
					}
				}		
				m_uCloseButtonState &= ~(UISTATE_PUSHED | UISTATE_CAPTURED);
				m_uButtonState &= ~(UISTATE_PUSHED | UISTATE_CAPTURED);
				Invalidate();
			}
			return;
		}
		else if (event.Type == UIEVENT_CONTEXTMENU)
		{
			if (IsContextMenuUsed()) {
				m_pManager->SendNotify(this, DUI_MSGTYPE_MENU, event.wParam, event.lParam);
			}
			return;
		}
		else if (event.Type == UIEVENT_MOUSEENTER)
		{
			if ((IsEnabled() || m_bIgnoreEnable)) 
			{
				m_uButtonState |= UISTATE_HOT;	
				Invalidate();				
			}		
		}
		else if (event.Type == UIEVENT_MOUSELEAVE)
		{
			if ((IsEnabled() || m_bIgnoreEnable)) 
			{
				m_uButtonState &= ~UISTATE_HOT;
				if (!(::PtInRect(&m_rcCloseEvent, event.ptMouse)))
					m_uCloseButtonState &= ~UISTATE_HOT;
				Invalidate();				
			}			
		}
		else if (event.Type == UIEVENT_SETCURSOR) {
			::SetCursor(::LoadCursor(NULL, IDC_HAND));
			return;
		}
		else if (event.Type == UIEVENT_TIMER && m_pManager)
		{			
			if (event.wParam == UITABOPTION_TIMER)
			{
				if (m_bForAnimo && m_bShowForeImage)
				{
					m_nCurAnimcount++;
					if (m_nCurAnimcount > m_nforimageAnim)
						m_nCurAnimcount = 0;
				}
				else
				{
					m_nCurAnimcount = 0;
					m_pManager->KillTimer(this, UITABOPTION_TIMER);
				}
				Invalidate();
			}			
		}
		CButtonUI::DoEvent(event);
	}
}