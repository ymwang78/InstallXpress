#ifndef __UIHORIZONTALLAYOUT_H__
#define __UIHORIZONTALLAYOUT_H__

#pragma once

namespace DuiLib
{
	class UILIB_API CHorizontalLayoutUI : public CContainerUI
	{
	public:
		CHorizontalLayoutUI();

		LPCTSTR GetClass() const;
		LPVOID GetInterface(LPCTSTR pstrName);
		UINT GetControlFlags() const;

		void SetSepWidth(int iWidth);
		int GetSepWidth() const;
		void SetSepImmMode(bool bImmediately);
		bool IsSepImmMode() const;
		void SetAttribute(LPCTSTR pstrName, LPCTSTR pstrValue);
		void SetMouserEnterLeaveTimerlen(int nentertimer, int nleavetimer);
		void DoEvent(TEventUI& event);
		void PaintBkImage(HDC hDC);
		void PaintBkConverImage(HDC hDC);
		
		void SetPos(RECT rc);
		void DoPostPaint(HDC hDC, const RECT& rcPaint);

		RECT GetThumbRect(bool bUseNew = false) const;

		void SetBkCenterDraw(bool bcenter);
		void SetBkImageWidth(int nwidth);
		void SetBkImageHeight(int nwidth);
		void SetMouseEnterEvent(bool bEnter);
		void SetMouseLeaveEvent(bool bLeave);
		void SetEventRectValue(int nSize);
		void SetEventRectType(bool bAsktype);
		void SetDrawBk(bool bDrawBk);
		void SetTextPadding(RECT rc);
		void SetFont(int nfont);
		void SetTextColor(DWORD dtextcolor);
		void SetExText(LPCTSTR pstrValue);

	protected:
		int m_iFont;
		CDuiString m_extext;
		DWORD m_dwTextColor;
		RECT	m_rcTextPadding;
		int m_iSepWidth;
		UINT m_uButtonState;
		POINT ptLastMouse;
		RECT m_rcNewPos;
		bool m_bDrawBk;
		bool m_bImmMode;
		bool m_bBkCenterDraw;	//取原图与背景同等大小绘制(m_nBkImageWidth,m_nBkImageHeight必须设置);
		bool m_bMouseEnter;
		bool m_bMouseLeave;
		int  m_nEntertimerlen;
		int  m_nLeavetimerlen;
		int  m_nBkImageWidth;	//背景图片宽度;
		int  m_nBkImageHeight;	//背景图片高度;
		int  m_nEventRectValue;
		bool m_bxposAsktype;
		bool m_bOpenleavetimer;
	};
}
#endif // __UIHORIZONTALLAYOUT_H__
