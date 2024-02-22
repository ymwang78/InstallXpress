#ifndef __UILABELICON_H__
#define __UILABELICON_H__

namespace DuiLib
{
	class UILIB_API CLabelIconUI : public CLabelUI
	{
	public:
		CLabelIconUI(void);
		~CLabelIconUI(void);

	public:
		LPCTSTR GetClass() const;
		LPVOID GetInterface(LPCTSTR pstrName);
		void PaintBkImage(HDC hDC);
		void SetAttribute(LPCTSTR pstrName, LPCTSTR pstrValue);
		void SetBkImage(LPCTSTR szImage);
		void SetForegImage(LPCTSTR szImage);
		CDuiString GetBkImage() const;
		HICON LoadIconImage(LPCTSTR bitmap, int cx, int cy);
		CDuiString ParsingIconImageString(LPCTSTR pStrImage);
		bool FileExist(LPCTSTR strPath);

	protected:
		CDuiString m_sIconImage;
		CDuiString m_strForegImage;
		HICON      m_hIcon;
	};
}

#endif // __UILABEL_H__