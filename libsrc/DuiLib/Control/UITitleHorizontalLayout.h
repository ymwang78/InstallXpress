#ifndef __TITLEHORIZONTALLAYOUT_H__
#define __TITLEHORIZONTALLAYOUT_H__

namespace DuiLib
{
	class CTitleHorizontalLayoutUI : public CHorizontalLayoutUI
	{
	public:
		CTitleHorizontalLayoutUI();
		~CTitleHorizontalLayoutUI();

		LPCTSTR GetClass() const;
		LPVOID GetInterface(LPCTSTR pstrName);
		UINT GetControlFlags() const;
	
		void SetOverLayoutWidth(bool bover);

		void SetPos(RECT rc);
		void DoPaint(HDC hDC, const RECT& rcPaint);	

	private:
		int m_nSubOverlayWidth;
		bool m_bOverlayWidth;
	};
}
#endif // __TITLEHORIZONTALLAYOUT_H__
