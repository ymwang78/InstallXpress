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

    // 以下函数中的颜色参数alpha值无效
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
/// @brief 文本转图片;
///
/// 将文本字符串转成透明图片;
//================================================================================
class UILIB_API CTextToImage
{
public:
	CTextToImage();
	CTextToImage(CDuiString strText, CDuiString pFontName = L"宋体", float fontSize = 12, DWORD pColor = 0x000000FF, int fontStyle = 0);
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
	CDuiString		m_pStrText;		//文本;
	CDuiString		m_pFontName;	//字体名称;
	Gdiplus::Color  *m_pFontColor;  //文本的颜色;
	float            m_FontSize;	//字号;
	int				 m_FontStyle;	//字体效果规则、粗体、斜体、粗斜体、下划线、强调线;
};

} // namespace DuiLib

#endif // __UIRENDER_H__
