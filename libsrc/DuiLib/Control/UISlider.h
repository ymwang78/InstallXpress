//===========================================================================================
//===========================================================================================
#ifndef __UISLIDER_H__
#define __UISLIDER_H__

#pragma once

namespace DuiLib
{
	class UILIB_API CSliderUI : public CProgressUI
	{
	public:
		CSliderUI();

		LPCTSTR GetClass() const;
		UINT GetControlFlags() const;
		LPVOID GetInterface(LPCTSTR pstrName);

		void SetEnabled(bool bEnable = true);
		void SetDisible(bool bDisible = false);
		int GetChangeStep();
		void SetChangeStep(int step);
		void SetThumbSize(SIZE szXY);
		RECT GetThumbRect() const;
		LPCTSTR GetThumbImage() const;
		void SetThumbImage(LPCTSTR pStrImage);
		LPCTSTR GetThumbHotImage() const;
		void SetThumbHotImage(LPCTSTR pStrImage);
		LPCTSTR GetThumbPushedImage() const;
		void SetThumbPushedImage(LPCTSTR pStrImage);
		LPCTSTR GetThumbDisibleImage() const;
		void SetThumbDisibleImage(LPCTSTR pStrImage);
		LPCTSTR GetForeDisibleImage() const;
		void SetForeDisibleImage(LPCTSTR pStrImage);
		

		void DoEvent(TEventUI& event);//2014.7.28  
		void SetAttribute(LPCTSTR pstrName, LPCTSTR pstrValue);
		void PaintStatusImage(HDC hDC);

		void SetValue(int nValue);//2014.7.28  
		void SetCanSendMove(bool bCanSend);//2014.7.28  
		bool GetCanSendMove() const;//2014.7.28  
	protected:
		SIZE m_szThumb;
		CDuiRect m_szForeImageIndent;
		UINT m_uButtonState;
		int m_nStep;

		CDuiString m_sThumbImage;
		CDuiString m_sThumbHotImage;
		CDuiString m_sThumbPushedImage;
		CDuiString m_sThumbDisibleImage;
		CDuiString m_sTForeDisibleImage;
		CDuiString m_sImageModify;

		bool	   m_bSendMove;//2014.7.28  
		bool       m_bDisible;
	};
}

#endif // __UISLIDER_H__