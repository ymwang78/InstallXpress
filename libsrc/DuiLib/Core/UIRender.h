#ifndef __UIRENDER_H__
#define __UIRENDER_H__

#pragma once

namespace DuiLib {
/////////////////////////////////////////////////////////////////////////////////////
//

class UILIB_API CRenderClip
{
public:
    ~CRenderClip();
    RECT rcItem;
    HDC hDC;
    HRGN hRgn;
    HRGN hOldRgn;

    static void GenerateClip(HDC hDC, RECT rc, CRenderClip& clip);
    static void GenerateRoundClip(HDC hDC, RECT rc, RECT rcItem, int width, int height, CRenderClip& clip);
    static void UseOldClipBegin(HDC hDC, CRenderClip& clip);
    static void UseOldClipEnd(HDC hDC, CRenderClip& clip);
};

/////////////////////////////////////////////////////////////////////////////////////
//

class UILIB_API CRenderEngine
{
	static HBITMAP WebpLoad(LPBYTE pData, DWORD dwSize, int& x, int& y, bool& bAlphaChannel, DWORD mask);
	static HBITMAP StbiLoad(LPBYTE pData, DWORD dwSize, int& x, int& y, bool& bAlphaChannel, DWORD mask);
public:
    static DWORD AdjustColor(DWORD dwColor, short H, short S, short L);
	static void DrawAlphaBitBlt(HDC hDC, int nDestX, int nDestY, int dwWidth, int dwHeight, HDC hSrcDC, int nSrcX, int nSrcY, int wSrc, int hSrc, BYTE uFade);
	static TImageInfo* LoadImage(STRINGorID bitmap, LPCTSTR type = NULL, DWORD mask = 0);
    static void FreeImage(const TImageInfo* bitmap);
    static void DrawImage(HDC hDC, HBITMAP hBitmap, const RECT& rc, const RECT& rcPaint, \
        const RECT& rcBmpPart, const RECT& rcCorners, bool alphaChannel, BYTE uFade = 255, 
        bool hole = false, bool xtiled = false, bool ytiled = false);
    static bool DrawImageString(HDC hDC, CPaintManagerUI* pManager, const RECT& rcItem, const RECT& rcPaint, 
        LPCTSTR pStrImage, LPCTSTR pStrModify = NULL);
	static bool DrawImageString(HDC hDC, CPaintManagerUI* pManager, const RECT& rcItem, const RECT& rcPaint,
		LPCTSTR pStrImage, int ndestx,int ndesty, const RECT& rcSource, LPCTSTR pStrModify = NULL);
	static bool CenterDrawImageString(HDC hDC, CPaintManagerUI* pManager, const RECT& rcItem, const RECT& rcPaint,
		LPCTSTR pStrImage, LPCTSTR pStrModify = NULL);
    static void DrawColor(HDC hDC, const RECT& rc, DWORD color);
    static void DrawGradient(HDC hDC, const RECT& rc, DWORD dwFirst, DWORD dwSecond, bool bVertical, int nSteps);

    // ���º����е���ɫ����alphaֵ��Ч
    static void DrawLine(HDC hDC, const RECT& rc, int nSize, DWORD dwPenColor,int nStyle = PS_SOLID);
    static void DrawRect(HDC hDC, const RECT& rc, int nSize, DWORD dwPenColor);
    static void DrawRoundRect(HDC hDC, const RECT& rc, int width, int height, int nSize, DWORD dwPenColor);
    static void DrawText(HDC hDC, CPaintManagerUI* pManager, RECT& rc, LPCTSTR pstrText, \
        DWORD dwTextColor, int iFont, UINT uStyle);
    static void DrawHtmlText(HDC hDC, CPaintManagerUI* pManager, RECT& rc, LPCTSTR pstrText, 
        DWORD dwTextColor, RECT* pLinks, CDuiString* sLinks, int& nLinkRects, UINT uStyle);
    static HBITMAP GenerateBitmap(CPaintManagerUI* pManager, CControlUI* pControl, RECT rc);
	static SIZE GetTextSize(HDC hDC, CPaintManagerUI* pManager , LPCTSTR pstrText, int iFont, UINT uStyle);
	static CDuiString GetLimitText(HDC hDC, CPaintManagerUI* pManager, LPCTSTR pstrText, int iFont, int iLimitLen);
	//alpha utilities
	static void CheckAlphaColor(DWORD& dwColor);
	static void ClearAlphaPixel(LPBYTE pBits, int bitsWidth, PRECT rc);
	static void RestoreAlphaColor(LPBYTE pBits, int bitsWidth, PRECT rc);

	static bool DDrawImage(HDC hDC, CPaintManagerUI* pManager, const RECT& rc, const RECT& rcPaint, const CDuiString& sImageName, \
		const CDuiString& sImageResType, RECT rcItem, RECT rcBmpPart, RECT rcCorner, DWORD dwMask, BYTE bFade, \
		bool bHole, bool bTiledX, bool bTiledY);
};

//==================================CTextToImage==================================
/// @brief �ı�תͼƬ;
///
/// ���ı��ַ���ת��͸��ͼƬ;
//================================================================================
class UILIB_API CTextToImage
{
public:
	CTextToImage();
	CTextToImage(CDuiString strText, CDuiString pFontName = L"����", float fontSize = 12, DWORD pColor = 0x000000FF, int fontStyle = 0);
	~CTextToImage();

public:
	void	SetString(CDuiString &strText);
	int		GetFontCount();
	void    GetFontNameArr(WCHAR **pFontNameArr, int numSought, int *pNumFound);
	void    SetFontName(CDuiString &fontName);
	void    SetFontSize(float fontSize);
	void    SetFontColor(DWORD  color);
	void    SetFontStyle(int fontStyle);
	void    SaveToImage(HWND hWnd, CDuiString &strPicPath);
	void    DrawText(HWND hWnd, HDC hdc, int xpos,int ypos);

private:
	CDuiString		m_pStrText;		//�ı�;
	CDuiString		m_pFontName;	//��������;
	Gdiplus::Color  *m_pFontColor;  //�ı�����ɫ;
	float            m_FontSize;	//�ֺ�;
	int				 m_FontStyle;	//����Ч�����򡢴��塢б�塢��б�塢�»��ߡ�ǿ����;
};

} // namespace DuiLib

#endif // __UIRENDER_H__
