#include "StdAfx.h"
#include "UISlider.h"

namespace DuiLib
{
	CSliderUI::CSliderUI() : m_uButtonState(0), m_nStep(1),m_bSendMove(false)
	{
		m_uTextStyle = DT_SINGLELINE | DT_CENTER;
		m_szThumb.cx = m_szThumb.cy = 10;
		m_sThumbDisibleImage = L"";
		m_sTForeDisibleImage = L"";
		m_bDisible = false;
		m_szForeImageIndent.Empty();
	}

	LPCTSTR CSliderUI::GetClass() const
	{
		return _T("SliderUI");
	}

	UINT CSliderUI::GetControlFlags() const
	{
		if( IsEnabled() ) return UIFLAG_SETCURSOR;
		else return 0;
	}

	LPVOID CSliderUI::GetInterface(LPCTSTR pstrName)
	{
		if( _tcscmp(pstrName, DUI_CTR_SLIDER) == 0 ) return static_cast<CSliderUI*>(this);
		return CProgressUI::GetInterface(pstrName);
	}

	void CSliderUI::SetEnabled(bool bEnable)
	{
		CControlUI::SetEnabled(bEnable);
		if( !IsEnabled() ) {
			m_uButtonState = 0;
		}
	}

	void CSliderUI::SetDisible(bool bDisible)
	{
		m_bDisible = bDisible;
		Invalidate();
	}

	int CSliderUI::GetChangeStep()
	{
		return m_nStep;
	}

	void CSliderUI::SetChangeStep(int step)
	{
		m_nStep = step;
	}

	void CSliderUI::SetThumbSize(SIZE szXY)
	{
		m_szThumb = szXY;
	}

	RECT CSliderUI::GetThumbRect() const
	{
		if( m_bHorizontal ) {
			int left = m_rcItem.left + (m_rcItem.right - m_rcItem.left - m_szThumb.cx) * (m_nValue - m_nMin) / (m_nMax - m_nMin);
			int top = (m_rcItem.bottom + m_rcItem.top - m_szThumb.cy) / 2;
			return CDuiRect(left, top, left + m_szThumb.cx, top + m_szThumb.cy); 
		}
		else {
			int left = (m_rcItem.right + m_rcItem.left - m_szThumb.cx) / 2;
			int top = m_rcItem.bottom - m_szThumb.cy - (m_rcItem.bottom - m_rcItem.top - m_szThumb.cy) * (m_nValue - m_nMin) / (m_nMax - m_nMin);
			return CDuiRect(left, top, left + m_szThumb.cx, top + m_szThumb.cy); 
		}
	}

	LPCTSTR CSliderUI::GetThumbImage() const
	{
		return m_sThumbImage;
	}

	void CSliderUI::SetThumbImage(LPCTSTR pStrImage)
	{
		m_sThumbImage = pStrImage;
		Invalidate();
	}

	LPCTSTR CSliderUI::GetThumbHotImage() const
	{
		return m_sThumbHotImage;
	}

	void CSliderUI::SetThumbHotImage(LPCTSTR pStrImage)
	{
		m_sThumbHotImage = pStrImage;
		Invalidate();
	}

	LPCTSTR CSliderUI::GetThumbPushedImage() const
	{
		return m_sThumbPushedImage;
	}

	void CSliderUI::SetThumbPushedImage(LPCTSTR pStrImage)
	{
		m_sThumbPushedImage = pStrImage;
		Invalidate();
	}

	LPCTSTR CSliderUI::GetThumbDisibleImage() const
	{
		return m_sThumbDisibleImage;
	}

	void CSliderUI::SetThumbDisibleImage(LPCTSTR pStrImage)
	{
		m_sThumbDisibleImage = pStrImage;
		Invalidate();
	}

	LPCTSTR CSliderUI::GetForeDisibleImage() const
	{
		return m_sTForeDisibleImage;
	}

	void CSliderUI::SetForeDisibleImage(LPCTSTR pStrImage)
	{
		m_sTForeDisibleImage = pStrImage;
		Invalidate();
	}


	void CSliderUI::SetValue(int nValue) //2014.7.28    ��������ڻ�������ʱ�����յ�SetValue��Ӱ�죬���绬���ı����ֵĽ��ȣ�������Ϊ�ⲿһֱ����SetValue���������޷���������
	{
		if( (m_uButtonState & UISTATE_CAPTURED) != 0 ) 
			return;
		CProgressUI::SetValue(nValue);
	}
	void CSliderUI::DoEvent(TEventUI& event)
	{
		if( !IsMouseEnabled() && event.Type > UIEVENT__MOUSEBEGIN && event.Type < UIEVENT__MOUSEEND ) {
			if( m_pParent != NULL ) m_pParent->DoEvent(event);
			else CProgressUI::DoEvent(event);
			return;
		}
		if (m_bDisible) return;
		if( event.Type == UIEVENT_BUTTONDOWN || event.Type == UIEVENT_DBLCLICK )
		{
			if( IsEnabled() ) {//2014.7.28   ע�͵�ԭ���Ĵ��룬������Щ����������Slider��������굯��ʱ�Ÿı们���λ��
				m_uButtonState |= UISTATE_CAPTURED;

				int nValue;

				if( m_bHorizontal ) {
					if( event.ptMouse.x >= m_rcItem.right - m_szThumb.cx / 2 ) nValue = m_nMax;
					else if( event.ptMouse.x <= m_rcItem.left + m_szThumb.cx / 2 ) nValue = m_nMin;
					else nValue = m_nMin + (m_nMax - m_nMin) * (event.ptMouse.x - m_rcItem.left - m_szThumb.cx / 2 ) / (m_rcItem.right - m_rcItem.left - m_szThumb.cx);
				}
				else {
					if( event.ptMouse.y >= m_rcItem.bottom - m_szThumb.cy / 2 ) nValue = m_nMin;
					else if( event.ptMouse.y <= m_rcItem.top + m_szThumb.cy / 2  ) nValue = m_nMax;
					else nValue = m_nMin + (m_nMax - m_nMin) * (m_rcItem.bottom - event.ptMouse.y - m_szThumb.cy / 2 ) / (m_rcItem.bottom - m_rcItem.top - m_szThumb.cy);
				}
				if(m_nValue !=nValue && nValue>=m_nMin && nValue<=m_nMax)
				{
					m_nValue =nValue;
					Invalidate();
				}
			}
				return;
		}

// 		if( event.Type == UIEVENT_BUTTONDOWN || event.Type == UIEVENT_DBLCLICK )
// 		{
// 			if( IsEnabled() ) {
// 				RECT rcThumb = GetThumbRect();
// 				if( ::PtInRect(&rcThumb, event.ptMouse) ) {
// 					m_uButtonState |= UISTATE_CAPTURED;
// 				}
// 			}
// 			return;
// 		}
		if( event.Type == UIEVENT_BUTTONUP )
		{
			int nValue;
			if( (m_uButtonState & UISTATE_CAPTURED) != 0 ) {
				m_uButtonState &= ~UISTATE_CAPTURED;
			}
			if( m_bHorizontal ) {
				if( event.ptMouse.x >= m_rcItem.right - m_szThumb.cx / 2 ) nValue = m_nMax;
				else if( event.ptMouse.x <= m_rcItem.left + m_szThumb.cx / 2 ) nValue = m_nMin;
				else nValue = m_nMin + (m_nMax - m_nMin) * (event.ptMouse.x - m_rcItem.left - m_szThumb.cx / 2 ) / (m_rcItem.right - m_rcItem.left - m_szThumb.cx);
			}
			else {
				if( event.ptMouse.y >= m_rcItem.bottom - m_szThumb.cy / 2 ) nValue = m_nMin;
				else if( event.ptMouse.y <= m_rcItem.top + m_szThumb.cy / 2  ) nValue = m_nMax;
				else nValue = m_nMin + (m_nMax - m_nMin) * (m_rcItem.bottom - event.ptMouse.y - m_szThumb.cy / 2 ) / (m_rcItem.bottom - m_rcItem.top - m_szThumb.cy);
			}
			if(/*m_nValue !=nValue && 2014.7.28   ���ע�ͺܹؼ�����������������϶������޷�����DUI_MSGTYPE_VALUECHANGED��Ϣ*/nValue>=m_nMin && nValue<=m_nMax)
			{
				m_nValue =nValue;
				m_pManager->SendNotify(this, DUI_MSGTYPE_VALUECHANGED);
				Invalidate();
			}
			return;
		}
		if( event.Type == UIEVENT_CONTEXTMENU )
		{
			return;
		}
		if( event.Type == UIEVENT_SCROLLWHEEL ) 
		{
			switch( LOWORD(event.wParam) ) {
		case SB_LINEUP:
			SetValue(GetValue() + GetChangeStep());
			m_pManager->SendNotify(this, DUI_MSGTYPE_VALUECHANGED);
			return;
		case SB_LINEDOWN:
			SetValue(GetValue() - GetChangeStep());
			m_pManager->SendNotify(this, DUI_MSGTYPE_VALUECHANGED);
			return;
			}
		}
		if( event.Type == UIEVENT_MOUSEMOVE )
		{
			if( (m_uButtonState & UISTATE_CAPTURED) != 0 ) {//2014.7.28   ��д�����Ϣ�ж���Slider����DUI_MSGTYPE_VALUECHANGED_MOVE��Ϣ�������ڻ�������Ҳ������Ϣ���������ڸı�����ʱ��һ�߻����Ϳ���һ�߸ı�����
				if( m_bHorizontal ) {
					if( event.ptMouse.x >= m_rcItem.right - m_szThumb.cx / 2 ) m_nValue = m_nMax;
					else if( event.ptMouse.x <= m_rcItem.left + m_szThumb.cx / 2 ) m_nValue = m_nMin;
					else m_nValue = m_nMin + (m_nMax - m_nMin) * (event.ptMouse.x - m_rcItem.left - m_szThumb.cx / 2 ) / (m_rcItem.right - m_rcItem.left - m_szThumb.cx);
				}
				else {
					if( event.ptMouse.y >= m_rcItem.bottom - m_szThumb.cy / 2 ) m_nValue = m_nMin;
					else if( event.ptMouse.y <= m_rcItem.top + m_szThumb.cy / 2  ) m_nValue = m_nMax;
					else m_nValue = m_nMin + (m_nMax - m_nMin) * (m_rcItem.bottom - event.ptMouse.y - m_szThumb.cy / 2 ) / (m_rcItem.bottom - m_rcItem.top - m_szThumb.cy);
				}
				if (m_bSendMove)
					m_pManager->SendNotify(this, DUI_MSGTYPE_VALUECHANGED_MOVE);
				Invalidate();
			}

			// Generate the appropriate mouse messages
			POINT pt = event.ptMouse;
			RECT rcThumb = GetThumbRect();
			if( IsEnabled() && ::PtInRect(&rcThumb, event.ptMouse) ) {

				m_uButtonState |= UISTATE_HOT;
				Invalidate();
			}else
			{
				m_uButtonState &= ~UISTATE_HOT;
				Invalidate();
			}
			return;
		}
// 		if( event.Type == UIEVENT_MOUSEMOVE )
// 		{
// 			if( (m_uButtonState & UISTATE_CAPTURED) != 0 ) {
// 				if( m_bHorizontal ) {
// 					if( event.ptMouse.x >= m_rcItem.right - m_szThumb.cx / 2 ) m_nValue = m_nMax;
// 					else if( event.ptMouse.x <= m_rcItem.left + m_szThumb.cx / 2 ) m_nValue = m_nMin;
// 					else m_nValue = m_nMin + (m_nMax - m_nMin) * (event.ptMouse.x - m_rcItem.left - m_szThumb.cx / 2 ) / (m_rcItem.right - m_rcItem.left - m_szThumb.cx);
// 				}
// 				else {
// 					if( event.ptMouse.y >= m_rcItem.bottom - m_szThumb.cy / 2 ) m_nValue = m_nMin;
// 					else if( event.ptMouse.y <= m_rcItem.top + m_szThumb.cy / 2  ) m_nValue = m_nMax;
// 					else m_nValue = m_nMin + (m_nMax - m_nMin) * (m_rcItem.bottom - event.ptMouse.y - m_szThumb.cy / 2 ) / (m_rcItem.bottom - m_rcItem.top - m_szThumb.cy);
// 				}
// 				Invalidate();
// 			}
// 			return;
// 		}
		if( event.Type == UIEVENT_SETCURSOR )
		{
			RECT rcThumb = GetThumbRect();
			if( IsEnabled()) {
				::SetCursor(::LoadCursor(NULL, IDC_HAND));
				return;
			}
		}
// 		if( event.Type == UIEVENT_SETCURSOR )
// 		{
// 			RECT rcThumb = GetThumbRect();
// 			if( IsEnabled() && ::PtInRect(&rcThumb, event.ptMouse) ) {
// 				::SetCursor(::LoadCursor(NULL, MAKEINTRESOURCE(IDC_HAND)));
// 				return;
// 			}
// 		}
		if( event.Type == UIEVENT_MOUSEENTER )
		{//2014.7.28   ֻ������ڻ���ķ�Χ�ڲű�ΪUISTATE_HOT
// 			if( IsEnabled() ) {
// 	  			m_uButtonState |= UISTATE_HOT;
// 				Invalidate();
// 			}
// 			return;
		}
		if( event.Type == UIEVENT_MOUSELEAVE )
		{
			if( IsEnabled() ) {
				m_uButtonState &= ~UISTATE_HOT;
				Invalidate();
			}
			return;
		}
		CControlUI::DoEvent(event);
	}

	void CSliderUI::SetCanSendMove(bool bCanSend) //2014.7.28  
	{
		m_bSendMove = bCanSend;
	}
	bool CSliderUI::GetCanSendMove() const //2014.7.28  
	{
		return m_bSendMove;
	}

	void CSliderUI::SetAttribute(LPCTSTR pstrName, LPCTSTR pstrValue)
	{
		if( _tcscmp(pstrName, _T("thumbimage")) == 0 ) SetThumbImage(pstrValue);
		else if( _tcscmp(pstrName, _T("thumbhotimage")) == 0 ) SetThumbHotImage(pstrValue);
		else if( _tcscmp(pstrName, _T("thumbpushedimage")) == 0 ) SetThumbPushedImage(pstrValue);
		else if( _tcscmp(pstrName, _T("thumbsize")) == 0 ) {
			SIZE szXY = {0};
			LPTSTR pstr = NULL;
			szXY.cx = _tcstol(pstrValue, &pstr, 10);  ASSERT(pstr);    
			szXY.cy = _tcstol(pstr + 1, &pstr, 10);    ASSERT(pstr); 
			SetThumbSize(szXY);
		}
		else if (_tcscmp(pstrName, _T("foreimageindent")) == 0)
		{
			LPTSTR pstr = NULL;
			m_szForeImageIndent.left = _tcstol(pstrValue, &pstr, 10);  ASSERT(pstr);
			m_szForeImageIndent.top = _tcstol(pstr + 1, &pstr, 10);    ASSERT(pstr);
			m_szForeImageIndent.right = _tcstol(pstr + 1, &pstr, 10);  ASSERT(pstr);
			m_szForeImageIndent.bottom = _tcstol(pstr + 1, &pstr, 10); ASSERT(pstr);
		}
		else if( _tcscmp(pstrName, _T("step")) == 0 ) {
			SetChangeStep(_ttoi(pstrValue));
		}
		else if( _tcscmp(pstrName, _T("sendmove")) == 0 ) {
			SetCanSendMove(_tcscmp(pstrValue, _T("true")) == 0);
		}
		else if (_tcscmp(pstrName, _T("thumbdisibleimage")) == 0) SetThumbDisibleImage(pstrValue);
		else if (_tcscmp(pstrName, _T("foredisibleimage")) == 0) SetForeDisibleImage(pstrValue);
		else CProgressUI::SetAttribute(pstrName, pstrValue);
	}

	void CSliderUI::PaintStatusImage(HDC hDC)
	{
		if (m_nMax <= m_nMin) m_nMax = m_nMin + 1;
		if (m_nValue > m_nMax) m_nValue = m_nMax;
		if (m_nValue < m_nMin) m_nValue = m_nMin;

		RECT rc = { 0 };
		if (m_bHorizontal) {
			rc.right = (m_nValue - m_nMin) * (m_rcItem.right - m_rcItem.left) / (m_nMax - m_nMin);
			rc.bottom = m_rcItem.bottom - m_rcItem.top;
		}
		else {
			rc.top = (m_rcItem.bottom - m_rcItem.top) * (m_nMax - m_nValue) / (m_nMax - m_nMin);
			rc.right = m_rcItem.right - m_rcItem.left;
			rc.bottom = m_rcItem.bottom - m_rcItem.top;
		}

		if (!m_sForeImage.IsEmpty()) {
			m_sForeImageModify.Empty();
			if (m_bStretchForeImage)
				m_sForeImageModify.SmallFormat(_T("dest='%d,%d,%d,%d'"), rc.left + m_szForeImageIndent.left, m_bHorizontal ?
					rc.top + m_szForeImageIndent.top : rc.top + 2 + m_szForeImageIndent.top, rc.right + m_szForeImageIndent.right, rc.bottom + m_szForeImageIndent.bottom);
			else
				m_sForeImageModify.SmallFormat(_T("dest='%d,%d,%d,%d' source='%d,%d,%d,%d'")
					, rc.left + m_szForeImageIndent.left, rc.top + m_szForeImageIndent.top, rc.right + m_szForeImageIndent.right, rc.bottom + m_szForeImageIndent.bottom
					, rc.left, rc.top, rc.right, rc.bottom);

			if (m_bDisible == false) {
				if (!DrawImage(hDC, (LPCTSTR)m_sForeImage, (LPCTSTR)m_sForeImageModify)) m_sForeImage.Empty();
			}
			else {
				if (!DrawImage(hDC, (LPCTSTR)m_sTForeDisibleImage, (LPCTSTR)m_sForeImageModify)) m_sTForeDisibleImage.Empty();
			}
		}

		RECT rcThumb = GetThumbRect();
		rcThumb.left -= m_rcItem.left;
		rcThumb.top -= m_rcItem.top;
		rcThumb.right -= m_rcItem.left;
		rcThumb.bottom -= m_rcItem.top;
		if( (m_uButtonState & UISTATE_CAPTURED) != 0 ) {
			if( !m_sThumbPushedImage.IsEmpty() ) {
				m_sImageModify.Empty();
				m_sImageModify.SmallFormat(_T("dest='%d,%d,%d,%d'"), rcThumb.left, rcThumb.top, rcThumb.right, rcThumb.bottom);
				if (m_bDisible == false) {
					if (!DrawImage(hDC, (LPCTSTR)m_sThumbPushedImage, (LPCTSTR)m_sImageModify)) m_sThumbPushedImage.Empty();
					else return;
				}	
				else {
					if (!DrawImage(hDC, (LPCTSTR)m_sThumbDisibleImage, (LPCTSTR)m_sImageModify)) m_sThumbDisibleImage.Empty();
					else return;
				}
			}
		}
		else if( (m_uButtonState & UISTATE_HOT) != 0 ) {
			if( !m_sThumbHotImage.IsEmpty() ) {
				m_sImageModify.Empty();
				m_sImageModify.SmallFormat(_T("dest='%d,%d,%d,%d'"), rcThumb.left, rcThumb.top, rcThumb.right, rcThumb.bottom);
				if (m_bDisible == false) {
					if (!DrawImage(hDC, (LPCTSTR)m_sThumbHotImage, (LPCTSTR)m_sImageModify)) m_sThumbHotImage.Empty();
					else return;
				}
				else {
					if (!DrawImage(hDC, (LPCTSTR)m_sThumbDisibleImage, (LPCTSTR)m_sImageModify)) m_sThumbDisibleImage.Empty();
					else return;
				}
			}
		}

		if( !m_sThumbImage.IsEmpty() ) {
			m_sImageModify.Empty();
			m_sImageModify.SmallFormat(_T("dest='%d,%d,%d,%d'"), rcThumb.left, rcThumb.top, rcThumb.right, rcThumb.bottom);
			if (m_bDisible == false) {
				if (!DrawImage(hDC, (LPCTSTR)m_sThumbImage, (LPCTSTR)m_sImageModify)) m_sThumbImage.Empty();
				else return;
			}
			else {
				if(!DrawImage(hDC, (LPCTSTR)m_sThumbDisibleImage, (LPCTSTR)m_sImageModify)) m_sThumbDisibleImage.Empty();
				else return;
			}
		}
	}
}
