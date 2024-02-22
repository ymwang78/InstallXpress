#include "stdafx.h"
#include "UICheckBox.h"

namespace DuiLib
{
	CCheckBoxUI::CCheckBoxUI()
	{
		m_bUseTwoModle = false;
		m_bRedrawText = false;
		m_dwtwoTextColor = 0xFF000000;
		m_textpadding1 = CDuiRect(0, 0, 0, 0);
		m_textpadding2 = CDuiRect(0, 0, 0, 0);
		m_sSelectedImage1 = _T("");
		m_sNormalImage1 = _T("");
	}

	LPCTSTR CCheckBoxUI::GetClass() const
	{
		return _T("CheckBoxUI");
	}

	LPVOID CCheckBoxUI::GetInterface(LPCTSTR pstrName)
	{
		if (_tcscmp(pstrName, DUI_CTR_CHECKBOX) == 0) 
			return static_cast<CCheckBoxUI*>(this);
		return __super::GetInterface(pstrName);
	}

	void CCheckBoxUI::SetCheck(bool bCheck, bool bEvent)
	{
		Selected(bCheck, bEvent);
	}

	bool  CCheckBoxUI::GetCheck() const
	{
		return IsSelected();
	}

	void CCheckBoxUI::SetTwoTextColor(DWORD dwColor)
	{
		m_dwtwoTextColor = dwColor;
		Invalidate();
	}

	void CCheckBoxUI::SetUseTwoModle(bool bUser)
	{
		m_bUseTwoModle = bUser;
		Invalidate();
	}

	void CCheckBoxUI::SetRedrawText(bool bDraw)
	{
		m_bRedrawText = bDraw;
		Invalidate();
	}

	void CCheckBoxUI::SetNormalImage1(LPCTSTR pstrValue)
	{
		m_sNormalImage1 = pstrValue;
		Invalidate();
	}

	void CCheckBoxUI::Setselectedimage1(LPCTSTR pstrValue)
	{
		m_sSelectedImage1 = pstrValue;
		Invalidate();
	}

	void CCheckBoxUI::SetAttribute(LPCTSTR pstrName, LPCTSTR pstrValue)
	{
		__super::SetAttribute(pstrName, pstrValue);
	    if (_tcscmp(pstrName, _T("textcolor2")) == 0)
		{
			if (*pstrValue == _T('#')) pstrValue = ::CharNext(pstrValue);
			LPTSTR pstr = NULL;
			DWORD clrColor = _tcstoul(pstrValue, &pstr, 16);
			SetTwoTextColor(clrColor);
		}
		else if (_tcscmp(pstrName, _T("textpadding1"))==0)
		{
			LPTSTR pstr = NULL;
			m_textpadding1.left = _tcstol(pstrValue, &pstr, 10);  ASSERT(pstr);
			m_textpadding1.top = _tcstol(pstr + 1, &pstr, 10);    ASSERT(pstr);
			m_textpadding1.right = _tcstol(pstr + 1, &pstr, 10);  ASSERT(pstr);
			m_textpadding1.bottom = _tcstol(pstr + 1, &pstr, 10); ASSERT(pstr);
		}
		else if (_tcscmp(pstrName, _T("textpadding2")) == 0)
		{
			LPTSTR pstr = NULL;
			m_textpadding2.left = _tcstol(pstrValue, &pstr, 10);  ASSERT(pstr);
			m_textpadding2.top = _tcstol(pstr + 1, &pstr, 10);    ASSERT(pstr);
			m_textpadding2.right = _tcstol(pstr + 1, &pstr, 10);  ASSERT(pstr);
			m_textpadding2.bottom = _tcstol(pstr + 1, &pstr, 10); ASSERT(pstr);
		}
		else if (_tcscmp(pstrName, _T("usetwomodle")) == 0)
		{
			SetUseTwoModle(_tcscmp(pstrValue, _T("true")) == 0);
		}
		else if (_tcscmp(pstrName, _T("redrawtext")) == 0)
		{
			SetRedrawText(_tcscmp(pstrValue, _T("true")) == 0);
		}
		else if (_tcscmp(pstrName, _T("normalimage1")) == 0) SetNormalImage1(pstrValue);
		else if (_tcscmp(pstrName, _T("selectedimage1")) == 0) Setselectedimage1(pstrValue);
	}

	void CCheckBoxUI::PaintStatusImage(HDC hDC)
	{
		if (m_bUseTwoModle)
		{
			if ((m_uButtonState & UISTATE_PUSHED) != 0 && IsSelected() && !m_sSelectedPushedImage.IsEmpty()) {
				if (!DrawImage(hDC, (LPCTSTR)m_sSelectedPushedImage))
					m_sSelectedPushedImage.Empty();
				else goto Label_ForeImage2;
			}
			else if ((m_uButtonState & UISTATE_HOT) != 0 && IsSelected() && !m_sSelectedHotImage.IsEmpty()) {
				if (!DrawImage(hDC, (LPCTSTR)m_sSelectedHotImage))
					m_sSelectedHotImage.Empty();
				else goto Label_ForeImage2;
			}
			else if ((m_uButtonState & UISTATE_SELECTED) != 0) {
				if (!m_sSelectedImage1.IsEmpty()) {
					if (!DrawImage(hDC, (LPCTSTR)m_sSelectedImage1)) m_sSelectedImage1.Empty();
					else goto Label_ForeImage2;
				}
				else if (m_dwSelectedBkColor != 0) {
					CRenderEngine::DrawColor(hDC, m_rcPaint, GetAdjustColor(m_dwSelectedBkColor));
					return;
				}
			}
			if (!m_sNormalImage1.IsEmpty()) {
				if (!DrawImage(hDC, (LPCTSTR)m_sNormalImage1)) m_sNormalImage1.Empty();
				else goto Label_ForeImage2;
			}
			CButtonUI::PaintStatusImage(hDC);

		Label_ForeImage2:
			if (IsSelected() && !m_sSelectedForeImage.IsEmpty())
			{
				if (!DrawImage(hDC, (LPCTSTR)m_sSelectedForeImage)) m_sSelectedForeImage.Empty();
			}
			else if (!m_sForeImage.IsEmpty())
			{
				if (!DrawImage(hDC, (LPCTSTR)m_sForeImage)) m_sForeImage.Empty();
			}
		}
		else
		{
			if ((m_uButtonState & UISTATE_PUSHED) != 0 && IsSelected() && !m_sSelectedPushedImage.IsEmpty()) {
				if (!DrawImage(hDC, (LPCTSTR)m_sSelectedPushedImage))
					m_sSelectedPushedImage.Empty();
				else goto Label_ForeImage;
			}
			else if ((m_uButtonState & UISTATE_HOT) != 0 && IsSelected() && !m_sSelectedHotImage.IsEmpty()) {
				if (!DrawImage(hDC, (LPCTSTR)m_sSelectedHotImage))
					m_sSelectedHotImage.Empty();
				else goto Label_ForeImage;
			}
			else if ((m_uButtonState & UISTATE_SELECTED) != 0) {
				if (!m_sSelectedImage.IsEmpty()) {
					if (!DrawImage(hDC, (LPCTSTR)m_sSelectedImage)) m_sSelectedImage.Empty();
					else goto Label_ForeImage;
				}
				else if (m_dwSelectedBkColor != 0) {
					CRenderEngine::DrawColor(hDC, m_rcPaint, GetAdjustColor(m_dwSelectedBkColor));
					return;
				}
			}

			CButtonUI::PaintStatusImage(hDC);

		Label_ForeImage:
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

	void CCheckBoxUI::PaintText(HDC hDC)
	{
		if (!m_bRedrawText)
		{
			COptionUI::PaintText(hDC);
		}
		else
		{
			if ((m_uButtonState & UISTATE_SELECTED) != 0)
			{
				DWORD oldTextColor = m_dwTextColor;
				if (m_dwSelectedTextColor != 0) m_dwTextColor = m_dwSelectedTextColor;

				if (m_dwTextColor == 0) m_dwTextColor = m_pManager->GetDefaultFontColor();
				if (m_dwDisabledTextColor == 0) m_dwDisabledTextColor = m_pManager->GetDefaultDisabledColor();
				if (m_bUseTwoModle)	m_dwTextColor = m_dwtwoTextColor;

				int nLinks = 0;
				RECT rc = m_rcItem;
				if (m_bUseTwoModle)
				{
					rc.left += m_textpadding2.left;
					rc.top += m_textpadding2.top;
					rc.bottom -= m_textpadding2.bottom;
				}
				else
				{
					rc.left += m_textpadding1.left;					
					rc.top += m_textpadding1.top;
					rc.bottom -= m_textpadding1.bottom;
				}
				CRenderEngine::DrawText(hDC, m_pManager, rc, m_sText, IsEnabled() ? m_dwTextColor : m_dwDisabledTextColor, \
					m_iFont, m_uTextStyle);

				m_dwTextColor = oldTextColor;
			}	
			else
			{
				DWORD oldTextColor = m_dwTextColor;
				if (m_dwDisabledTextColor != 0) m_dwTextColor = m_dwDisabledTextColor;
				
				if (m_dwTextColor == 0) m_dwTextColor = m_pManager->GetDefaultFontColor();
				if (m_dwDisabledTextColor == 0) m_dwDisabledTextColor = m_pManager->GetDefaultDisabledColor();

				if (m_sText.IsEmpty()) return;

				int nLinks = 0;
				RECT rc = m_rcItem;
				if (m_bUseTwoModle)
				{
					rc.left += m_textpadding2.right;					
					rc.top += m_textpadding2.top;
					rc.bottom -= m_textpadding2.bottom;
				}
				else
				{
					rc.left += m_textpadding1.right;
					rc.top += m_textpadding1.top;
					rc.bottom -= m_textpadding1.bottom;
				}
				CRenderEngine::DrawText(hDC, m_pManager, rc, m_sText, IsEnabled() ? m_dwTextColor : m_dwDisabledTextColor, \
					m_iFont, m_uTextStyle);

				m_dwTextColor = oldTextColor;
			}
		}
	}

}
