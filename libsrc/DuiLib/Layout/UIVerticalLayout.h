#ifndef __UIVERTICALLAYOUT_H__
#define __UIVERTICALLAYOUT_H__

#pragma once

namespace DuiLib
{
	class UILIB_API IItemPrePaintCallback
	{
	public:
		virtual void OnPrePaint(CControlUI* pList, int iItem) = 0;
		virtual void OnEndPaint() = 0;
	};

	class UILIB_API CVerticalLayoutUI : public CContainerUI
	{
	public:
		CVerticalLayoutUI();

		LPCTSTR GetClass() const;
		LPVOID GetInterface(LPCTSTR pstrName);
		UINT GetControlFlags() const;

		void SetSepHeight(int iHeight);
		int GetSepHeight() const;
		void SetSepImmMode(bool bImmediately);
		bool IsSepImmMode() const;
		void SetAttribute(LPCTSTR pstrName, LPCTSTR pstrValue);
		void DoEvent(TEventUI& event);

		void DoPaint(HDC hDC, const RECT& rcPaint);
		void PaintBkImage(HDC hDC);
		void PaintBkConverImage(HDC hDC);

		void SetVisibleScroll(bool bVisible);
		bool GetVisibleScroll();

		void SetOnlyPage(bool bVisible);
		bool GetOnlyPage();
		void SetDrawBk(bool bDrawBk);

		void SetPos(RECT rc);
		void DoPostPaint(HDC hDC, const RECT& rcPaint);

		RECT GetThumbRect(bool bUseNew = false) const;
		
	protected:
		IItemPrePaintCallback* m_pPrePaintCallback;

		int  m_iSepHeight;		
		UINT m_uButtonState;
		POINT ptLastMouse;
		RECT m_rcNewPos;
		bool m_bImmMode;
		bool m_bVisibleScroll;
		bool m_bOnlyPage;
		bool m_bDrawBk;
	};
}
#endif // __UIVERTICALLAYOUT_H__
