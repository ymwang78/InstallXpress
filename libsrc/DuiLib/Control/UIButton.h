#ifndef __UIBUTTON_H__
#define __UIBUTTON_H__

#pragma once

namespace DuiLib
{
	class UILIB_API CButtonUI : public CLabelUI
	{
	public:
		CButtonUI();
		~CButtonUI();

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

		void SetClickUpEvent(bool bNeed);
		void SetMouseEnterEvent(bool bEnter);
		void SetMouseLeaveEvent(bool bLeave);
		void SetMouserEnterLeaveTimerlen(int nentertimer, int nleavetimer);
		void BkPlayAnimotion(LPCTSTR pstrImage, int uElapse);

		void SetUseMidLayoutColor(bool buse);
		void SetColorUseValue(int nvalue);
		void SetMidlayoutColor(DWORD dwColor);
		void SetDisDbClickEvent(bool bdisible);
		void SetIgnoreEnable(bool bIgnore);
		void PaintText(HDC hDC);
		void PaintStatusImage(HDC hDC);

	protected:
		UINT m_uButtonState;

		DWORD m_dwHotBkColor;
		DWORD m_dwDisabledBkColor;
		DWORD m_dwHotTextColor;
		DWORD m_dwPushedTextColor;
		DWORD m_dwFocusedTextColor;

		CDuiString m_sPlayBkImage;
		CDuiString m_sNormalImage;
		CDuiString m_sHotImage;
		CDuiString m_sHotForeImage;
		CDuiString m_sPushedImage;
		CDuiString m_sPushedForeImage;
		CDuiString m_sFocusedImage;
		CDuiString m_sDisabledImage;

		int			m_iBindTabIndex;
		CDuiString	m_sBindTabLayoutName;

		bool       m_bMouseEnter;
		bool       m_bMouseLeave;
		bool       m_bUseMidLayoutColor;
		bool	   m_bBkAnimotionPlay;
		int		   m_nBkPlayCount;
		int		   m_nBkPlayIndex;
		int        m_nColorUseValue;
		int        m_nEntertimerlen;
		int        m_nLeavetimerlen;
		bool       m_bOpenleavetimer;
		bool       m_bDbClick;
		bool       m_bDisbleDbClick;
		bool	   m_bIgnoreEnable;
		bool	   m_bNeedUpEvent;
		DWORD      m_dwMidLayoutColor;
	};

}	// namespace DuiLib

#endif // __UIBUTTON_H__