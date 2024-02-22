#ifndef __UIICONBUTTON_H__
#define __UIICONBUTTON_H__

#pragma once

namespace DuiLib
{
	class UILIB_API CIconButtonUI : public CLabelUI
	{
	public:
		CIconButtonUI();
		~CIconButtonUI();

		LPCTSTR GetClass() const;
		LPVOID GetInterface(LPCTSTR pstrName);
		UINT GetControlFlags() const;

		bool Activate();
		void SetEnabled(bool bEnable = true);
		void DoEvent(TEventUI& event);

		LPCTSTR GetNormalImage();
		void SetNormalImage(LPCTSTR pStrImage);
		LPCTSTR GetHotImage();
		void SetHotImage(LPCTSTR pStrImage);
		LPCTSTR GetPushedImage();
		void SetPushedImage(LPCTSTR pStrImage);
		LPCTSTR GetFocusedImage();
		void SetFocusedImage(LPCTSTR pStrImage);
		LPCTSTR GetDisabledImage();
		void SetDisabledImage(LPCTSTR pStrImage);
		LPCTSTR GetForeImage();
		void SetForeImage(LPCTSTR pStrImage);
		LPCTSTR GetHotForeImage();
		void SetHotForeImage(LPCTSTR pStrImage);

		void BindTabIndex(int _BindTabIndex);
		void BindTabLayoutName(LPCTSTR _TabLayoutName);
		void BindTriggerTabSel(int _SetSelectIndex = -1);
		void RemoveBindTabIndex();
		int	 GetBindTabLayoutIndex();
		CDuiString GetBindTabLayoutName();

		void SetHotBkColor(DWORD dwColor);
		DWORD GetHotBkColor() const;
		void SetDisiableBkColor(DWORD dwColor);
		DWORD GetDisiableBkColor() const;
		void SetHotTextColor(DWORD dwColor);
		DWORD GetHotTextColor() const;
		void SetPushedTextColor(DWORD dwColor);
		DWORD GetPushedTextColor() const;
		void SetFocusedTextColor(DWORD dwColor);
		DWORD GetFocusedTextColor() const;
		SIZE EstimateSize(SIZE szAvailable);
		void SetAttribute(LPCTSTR pstrName, LPCTSTR pstrValue);

		void SetMouseEnterEvent(bool bEnter);
		void SetMouseLeaveEvent(bool bLeave);
		void SetMouserEnterLeaveTimerlen(int nentertimer, int nleavetimer);

		void SetUseMidLayoutColor(bool buse);
		void SetColorUseValue(int nvalue);
		void SetMidlayoutColor(DWORD dwColor);
		void SetDisDbClickEvent(bool bdisible);

		HICON LoadIconImage(LPCTSTR bitmap, int cx, int cy);
		CDuiString ParsingIconImageString(LPCTSTR pStrImage);
		bool FileExist(LPCTSTR strPath);

		void SetBkIconImage(LPCTSTR sImage);

		void PaintBkImage(HDC hDC);
		void PaintText(HDC hDC);
		void PaintStatusImage(HDC hDC);

	protected:
		HICON m_hIcon;
		UINT m_uButtonState;

		DWORD m_dwHotBkColor;
		DWORD m_dwDisabledBkColor;
		DWORD m_dwHotTextColor;
		DWORD m_dwPushedTextColor;
		DWORD m_dwFocusedTextColor;

		CDuiString m_sNormalImage;
		CDuiString m_sHotImage;
		CDuiString m_sHotForeImage;
		CDuiString m_sPushedImage;
		CDuiString m_sPushedForeImage;
		CDuiString m_sFocusedImage;
		CDuiString m_sDisabledImage;
		CDuiString m_sBkIcomImage;

		int			m_iBindTabIndex;
		CDuiString	m_sBindTabLayoutName;
		CDuiRect    m_rcIconDestRc;

		bool       m_bMouseEnter;
		bool       m_bMouseLeave;
		bool       m_bUseMidLayoutColor;
		int        m_nColorUseValue;
		int        m_nEntertimerlen;
		int        m_nLeavetimerlen;
		bool       m_bOpenleavetimer;
		bool       m_bDbClick;
		bool       m_bDisbleDbClick;
		DWORD      m_dwMidLayoutColor;
	};

}	// namespace DuiLib

#endif // __UIICONBUTTON_H__