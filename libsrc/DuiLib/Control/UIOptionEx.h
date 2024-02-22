#ifndef __UIOPTIONEX_H__
#define __UIOPTIONEX_H__

#pragma once

namespace DuiLib
{
	class UILIB_API COptionExUI : public COptionUI
	{
	public:
		COptionExUI();
		~COptionExUI();

		LPCTSTR GetClass() const;
		LPVOID GetInterface(LPCTSTR pstrName);

		LPCTSTR GetForeNormalImage();
		void SetForeNormalimage(LPCTSTR pStrImage);

		LPCTSTR GetForeHotImage();
		void SetForeHotImage(LPCTSTR pStrImage);

		LPCTSTR GetForePushedImage();
		void SetForePushedImage(LPCTSTR pStrImage);

		LPCTSTR GetForeDisableImage();
		void SetForeDisableImage(LPCTSTR pStrImage);

		void SetBtlineNormalcolor(DWORD dcolor);
		void SetBtlineHotcolor(DWORD dcolor);
		void SetBtlineSelectedcolor(DWORD dcolor);
		void SetBtlinePushedcolor(DWORD dcolor);
		void SetBtlineDisablecolor(DWORD dcolor);
		void SetDrawBtLine(bool bdraw);

		void SetLineHeight(int nlineheight);
		void SetAttribute(LPCTSTR pstrName, LPCTSTR pstrValue);
		void SetVisibleForeImage(bool bVisible);
		bool GetVisibleForeImage();
	
		void SetText2Color(DWORD dwColor);
		DWORD GetText2Color() const;
		void SetText2Text(LPCTSTR pstrtext);
		CDuiString GetText2Text() const;
		void SetTextPadding2(RECT &rect);

		void SetForeTipImage(LPCTSTR pStrImage);
		void PaintStatusImage(HDC hDC);
		void PaintText2(HDC hDC);
		void PaintBkImage(HDC hDC);

		void DoEvent(TEventUI& event);

		HICON LoadIconImage(LPCTSTR bitmap, int cx, int cy);
		CDuiString ParsingIconImageString(LPCTSTR pStrImage);
		bool FileExist(LPCTSTR strPath);
	
	protected:	
		CDuiString		m_sForeNormalImage;
		CDuiString		m_sForeHotImage;
		CDuiString		m_sForePushedImage;
		CDuiString		m_sForeDisableImage;
		CDuiString      m_sForeTipImage;
		CDuiString		m_sIconImage;
		HICON			m_hIcon;

	protected:
		DWORD			m_dwtext2Color;
		DWORD			m_dwBtlineNormalColor;
		DWORD			m_dwBtlineHotColor;
		DWORD			m_dwBtlinePushedColor;
		DWORD			m_dwBtlineSelectedColor;
		DWORD			m_dwBtlineDisableColor;
		bool            m_bDrawBottomline;
		int				m_nlineheight;
		bool            m_bVisibleForeImage;

		CDuiString		m_strtext2;
		CDuiRect		m_rcText2Padding;
		CDuiRect        m_rcIconDestRc;
		CDuiRect		m_rcClickEventRc;
	};

} // namespace DuiLib

#endif // __UIOPTION_H__