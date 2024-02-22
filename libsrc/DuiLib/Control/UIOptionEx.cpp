#include "stdafx.h"
#include "UIOptionEx.h"

namespace DuiLib
{
	COptionExUI::COptionExUI() 
		: m_dwtext2Color(0x00000000)
		, m_strtext2(_T(""))
		, m_hIcon(NULL)
		, m_sIconImage(_T(""))
	{
		m_rcText2Padding = CDuiRect(0, 0, 0, 0);	
		m_rcClickEventRc = CDuiRect(0, 0, 0, 0);
		m_rcIconDestRc = CDuiRect(0, 0, 0, 0);
		m_dwBtlineNormalColor = 0xFFE4E4E4;
		m_dwBtlineHotColor = 0xFFE4E4E4;
		m_dwBtlineSelectedColor = 0xFFE4E4E4;
		m_dwBtlinePushedColor = 0xFFE4E4E4;
		m_dwBtlineDisableColor = 0xFFE4E4E4;
		m_bDrawBottomline = false;
		m_bVisibleForeImage = false;
	}

	COptionExUI::~COptionExUI()
	{
		if (m_hIcon) ::DestroyIcon(m_hIcon);		
		if( !m_sGroupName.IsEmpty() && m_pManager ) m_pManager->RemoveOptionGroup(m_sGroupName, this);
	}

	LPCTSTR COptionExUI::GetClass() const
	{
		return _T("OptionExUI");
	}

	LPVOID COptionExUI::GetInterface(LPCTSTR pstrName)
	{
		if( _tcscmp(pstrName, DUI_CTR_OPTIONEX) == 0 ) return static_cast<COptionExUI*>(this);
		return COptionUI::GetInterface(pstrName);
	}

	LPCTSTR COptionExUI::GetForeNormalImage()
	{
		return m_sForeNormalImage;
	}

	void COptionExUI::SetForeNormalimage(LPCTSTR pStrImage)
	{
		m_sForeNormalImage = pStrImage;
		Invalidate();
	}

	LPCTSTR COptionExUI::GetForeHotImage()
	{
		return m_sForeHotImage;
	}

	void COptionExUI::SetForeHotImage(LPCTSTR pStrImage)
	{
		m_sForeHotImage = pStrImage;
		Invalidate();
	}

	LPCTSTR COptionExUI::GetForePushedImage()
	{
		return m_sForePushedImage;
	}

	void COptionExUI::SetForePushedImage(LPCTSTR pStrImage)
	{
		m_sForePushedImage = pStrImage;
		Invalidate();
	}

	LPCTSTR COptionExUI::GetForeDisableImage()
	{
		return m_sForeDisableImage;
	}

	void COptionExUI::SetForeDisableImage(LPCTSTR pStrImage)
	{
		m_sForeDisableImage = pStrImage;
		Invalidate();
	}

	void COptionExUI::SetBtlineNormalcolor(DWORD dcolor)
	{
		m_dwBtlineNormalColor = dcolor;
		if(m_bDrawBottomline) Invalidate();
	}

	void COptionExUI::SetBtlineHotcolor(DWORD dcolor)
	{
		m_dwBtlineHotColor = dcolor;
		if (m_bDrawBottomline) Invalidate();
	}

	void COptionExUI::SetBtlineSelectedcolor(DWORD dcolor)
	{
		m_dwBtlineSelectedColor = dcolor;
		if (m_bDrawBottomline) Invalidate();
	}

	void COptionExUI::SetBtlinePushedcolor(DWORD dcolor)
	{
		m_dwBtlinePushedColor = dcolor;
		if (m_bDrawBottomline) Invalidate();
	}

	void COptionExUI::SetDrawBtLine(bool bdraw)
	{
		m_bDrawBottomline = bdraw;
		if (m_bDrawBottomline) Invalidate();
	}

	void COptionExUI::SetBtlineDisablecolor(DWORD dcolor)
	{
		m_dwBtlineDisableColor = dcolor;
		if (m_bDrawBottomline) Invalidate();
	}

	void COptionExUI::SetLineHeight(int nlineheight)
	{
		m_nlineheight = nlineheight;
	}

	void COptionExUI::SetVisibleForeImage(bool bVisible)
	{
		m_bVisibleForeImage = bVisible;
		Invalidate();
	}

	bool COptionExUI::GetVisibleForeImage()
	{
		return m_bVisibleForeImage;
	}

	void COptionExUI::SetForeTipImage(LPCTSTR pStrImage)
	{
		m_sForeTipImage = pStrImage;
		Invalidate();
	}

	void COptionExUI::SetText2Color(DWORD dwColor)
	{
		m_dwtext2Color = dwColor;
	}

	DWORD COptionExUI::GetText2Color() const
	{
		return m_dwtext2Color;
	}

	void COptionExUI::SetText2Text(LPCTSTR pstrtext)
	{
		m_strtext2 = pstrtext;
	}

	CDuiString COptionExUI::GetText2Text() const
	{
		return m_strtext2;
	}

	void COptionExUI::SetTextPadding2(RECT &rect)
	{
		m_rcText2Padding.left = rect.left;
		m_rcText2Padding.right = rect.right;
		m_rcText2Padding.top = rect.top;
		m_rcText2Padding.bottom = rect.bottom;
	}

	void COptionExUI::SetAttribute(LPCTSTR pstrName, LPCTSTR pstrValue)
	{
		if( _tcscmp(pstrName, _T("forenormalimage")) == 0 ) SetForeNormalimage(pstrValue);
		else if( _tcscmp(pstrName, _T("forehotimage")) == 0 ) SetForeHotImage(pstrValue);
		else if( _tcscmp(pstrName, _T("forepushedimage")) == 0 ) SetForePushedImage(pstrValue);
		else if( _tcscmp(pstrName, _T("foredisabledimage")) == 0 ) SetForeDisableImage(pstrValue);
		else if (_tcscmp(pstrName, _T("foretipimage")) == 0) SetForeTipImage(pstrValue);
		else if (_tcscmp(pstrName, _T("btlinenormalcolor")) == 0) {
			if (*pstrValue == _T('#')) pstrValue = ::CharNext(pstrValue);
			LPTSTR pstr = NULL;
			DWORD clrColor = _tcstoul(pstrValue, &pstr, 16);
			SetBtlineNormalcolor(clrColor);
		}
		else if (_tcscmp(pstrName, _T("btlinehotcolor")) == 0) {
			if (*pstrValue == _T('#')) pstrValue = ::CharNext(pstrValue);
			LPTSTR pstr = NULL;
			DWORD clrColor = _tcstoul(pstrValue, &pstr, 16);
			SetBtlineHotcolor(clrColor);
		}
		else if (_tcscmp(pstrName, _T("btlineselectedcolor")) == 0) {
			if (*pstrValue == _T('#')) pstrValue = ::CharNext(pstrValue);
			LPTSTR pstr = NULL;
			DWORD clrColor = _tcstoul(pstrValue, &pstr, 16);
			SetBtlineSelectedcolor(clrColor);
		}
		else if (_tcscmp(pstrName, _T("btlinepushedcolor")) == 0) {
			if (*pstrValue == _T('#')) pstrValue = ::CharNext(pstrValue);
			LPTSTR pstr = NULL;
			DWORD clrColor = _tcstoul(pstrValue, &pstr, 16);
			SetBtlinePushedcolor(clrColor);
		}
		else if (_tcscmp(pstrName, _T("btlinedisablecolor")) == 0) {
			if (*pstrValue == _T('#')) pstrValue = ::CharNext(pstrValue);
			LPTSTR pstr = NULL;
			DWORD clrColor = _tcstoul(pstrValue, &pstr, 16);
			SetBtlineDisablecolor(clrColor);
		}
		else if (_tcscmp(pstrName, _T("drawbtline")) == 0) {
			SetDrawBtLine(_tcscmp(pstrValue, _T("true")) == 0);
		}
		else if (_tcscmp(pstrName, _T("btlineheight")) == 0) {
			LPTSTR pstr = NULL;
			SetLineHeight(_tcstol(pstrValue, &pstr, 10));
		}
		else if (_tcscmp(pstrName, _T("foretipimg")) == 0)
		{
			SetVisibleForeImage(_tcscmp(pstrValue, _T("true")) == 0);
		}
		else if (_tcscmp(pstrName, _T("textcolor2")) == 0)
		{
			if (*pstrValue == _T('#')) pstrValue = ::CharNext(pstrValue);
			LPTSTR pstr = NULL;
			DWORD clrColor = _tcstoul(pstrValue, &pstr, 16);
			SetText2Color(clrColor);
		}
		else if (_tcscmp(pstrName, _T("text2")) == 0)
		{
			SetText2Text(pstrValue);
		}
		else if (_tcscmp(pstrName, _T("textpadding2")) == 0)
		{
			RECT rcTextPadding = { 0 };
			LPTSTR pstr = NULL;
			rcTextPadding.left = _tcstol(pstrValue, &pstr, 10);  ASSERT(pstr);
			rcTextPadding.top = _tcstol(pstr + 1, &pstr, 10);    ASSERT(pstr);
			rcTextPadding.right = _tcstol(pstr + 1, &pstr, 10);  ASSERT(pstr);
			rcTextPadding.bottom = _tcstol(pstr + 1, &pstr, 10); ASSERT(pstr);
			SetTextPadding2(rcTextPadding);
		}
		else if (_tcscmp(pstrName, _T("clickeventrc")) == 0)
		{
			LPTSTR pstr = NULL;
			m_rcClickEventRc.left = _tcstol(pstrValue, &pstr, 10);  ASSERT(pstr);
			m_rcClickEventRc.top = _tcstol(pstr + 1, &pstr, 10);    ASSERT(pstr);
			m_rcClickEventRc.right = _tcstol(pstr + 1, &pstr, 10);  ASSERT(pstr);
			m_rcClickEventRc.bottom = _tcstol(pstr + 1, &pstr, 10); ASSERT(pstr);
		}
		else if (_tcscmp(pstrName, _T("bkiconimg")) == 0)
		{
			m_sIconImage = pstrValue;
		}
		else COptionUI::SetAttribute(pstrName, pstrValue);
	}

	void COptionExUI::DoEvent(TEventUI& event)
	{
		bool bselect = IsSelected();
		__super::DoEvent(event);
		if (event.Type == UIEVENT_BUTTONUP && bselect)
		{
			if (m_rcClickEventRc.bottom == 0 || m_rcClickEventRc.top == m_rcClickEventRc.bottom)
				return;

			RECT rc = m_rcPaint;
			rc.top = rc.bottom - (m_rcClickEventRc.bottom - m_rcClickEventRc.top);
			rc.left = rc.left + m_rcClickEventRc.left;
			rc.right = rc.left + m_rcClickEventRc.right;
			if (::PtInRect(&rc, event.ptMouse))
			{
				if (m_pManager) 
				{
					m_pManager->SendNotify(this, DUI_MSGTYPE_PTRCCLICK);
				}
			}
		}
	}

	HICON COptionExUI::LoadIconImage(LPCTSTR bitmap, int cx, int cy)
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

	CDuiString COptionExUI::ParsingIconImageString(LPCTSTR pStrImage)
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

	bool COptionExUI::FileExist(LPCTSTR strPath)
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

	void COptionExUI::PaintText2(HDC hDC)
	{
		if (m_strtext2.IsEmpty())
			return;

		RECT rc = m_rcItem;
		rc = m_rcItem;
		rc.left += m_rcText2Padding.left;
		rc.right -= m_rcText2Padding.right;
		rc.top += m_rcText2Padding.top;
		rc.bottom -= m_rcText2Padding.bottom;

		DWORD clr2Color = IsEnabled() ? m_dwtext2Color : m_dwDisabledTextColor;

		CRenderEngine::DrawText(hDC, m_pManager, rc, m_strtext2, clr2Color, \
			m_iFont, m_uTextStyle);
	}

	void COptionExUI::PaintBkImage(HDC hDC)
	{
		__super::PaintBkImage(hDC);

		if (!m_pManager) return;
		if (m_sIconImage.IsEmpty() == false)
		{
			if (!m_hIcon)
			{
				m_hIcon = LoadIconImage(m_sIconImage, m_rcIconDestRc.right - m_rcIconDestRc.left, m_rcIconDestRc.bottom - m_rcIconDestRc.top);
			}
			if (m_hIcon)
			{
				DrawIconEx(hDC, m_rcItem.left + m_rcIconDestRc.left, m_rcItem.top + m_rcIconDestRc.top, m_hIcon, m_rcIconDestRc.right - m_rcIconDestRc.left,
					m_rcIconDestRc.bottom - m_rcIconDestRc.top, 0, NULL, DI_NORMAL);
			}
		}	
		PaintText2(hDC);
	}

	void COptionExUI::PaintStatusImage(HDC hDC)
	{
		COptionUI::PaintStatusImage(hDC);
		
		RECT BottomRect = m_rcPaint;
		BottomRect.top = BottomRect.bottom - 2;

		if ((m_uButtonState & UISTATE_DISABLED) != 0) {
			if (!m_sForeDisableImage.IsEmpty())
			{
				if (!CenterDrawImage(hDC, (LPCTSTR)m_sForeDisableImage)) m_sForeDisableImage.Empty();
				else goto Label_ForeImage;
			}
			if (m_bDrawBottomline) {
				CRenderEngine::DrawColor(hDC, BottomRect, GetAdjustColor(m_dwBtlineDisableColor));
				goto Label_ForeImage;
			}
		}
		else if ((m_uButtonState & UISTATE_PUSHED) != 0) {
			if (!m_sForePushedImage.IsEmpty())
			{
				if (!CenterDrawImage(hDC, (LPCTSTR)m_sForePushedImage)) 	m_sForePushedImage.Empty();
				else goto Label_ForeImage;
			}
			if (m_bDrawBottomline) {
				CRenderEngine::DrawColor(hDC, BottomRect, GetAdjustColor(m_dwBtlinePushedColor));
				goto Label_ForeImage;
			}
		}
		else if ((m_uButtonState & UISTATE_HOT) != 0) {
			if (!m_sForeHotImage.IsEmpty()) 
			{
				if (!CenterDrawImage(hDC, (LPCTSTR)m_sForeHotImage)) 	m_sForeHotImage.Empty();
				else goto Label_ForeImage;
			}	
			if (m_bDrawBottomline) {
				CRenderEngine::DrawColor(hDC, BottomRect, GetAdjustColor(m_dwBtlineHotColor));
				goto Label_ForeImage;
			}
		}
		else if ((m_uButtonState & UISTATE_SELECTED) != 0) 
		{
			if (!m_sForePushedImage.IsEmpty())
			{
				if (!CenterDrawImage(hDC, (LPCTSTR)m_sForePushedImage)) 	m_sForePushedImage.Empty();
				else goto Label_ForeImage;
			}
			if (m_bDrawBottomline) {
				CRenderEngine::DrawColor(hDC, BottomRect, GetAdjustColor(m_dwBtlineSelectedColor));
				goto Label_ForeImage;
			}
		}
		if (!m_sForeNormalImage.IsEmpty()) {
			if (!CenterDrawImage(hDC, (LPCTSTR)m_sForeNormalImage)) m_sForeNormalImage.Empty();
		}
		if (m_bDrawBottomline) {
			CRenderEngine::DrawColor(hDC, BottomRect, GetAdjustColor(m_dwBtlineNormalColor));
			goto Label_ForeImage;
		}	
	Label_ForeImage:
		if (m_bVisibleForeImage && !m_sForeTipImage.IsEmpty()) {
			if (!DrawImage(hDC, (LPCTSTR)m_sForeTipImage)) m_sForeTipImage.Empty();
		}
		return;
	}
}