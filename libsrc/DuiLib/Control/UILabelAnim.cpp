#include "StdAfx.h"
#include "UILabelAnim.h"

#include <atlconv.h>
namespace DuiLib
{
#define EVENT_ATIEM_ID	224
#define EVENT_DELAYNOTIF_ID 225

	CLabelAnimUI::CLabelAnimUI(void)
	{
		m_nMovesteplen = 0;
		m_nMovetype = 0;
		m_nPlayTimerLen = 0;
		m_bplay = false;
		m_bAutoPlay = false;
		m_nPlayingLen = 0;
		m_bPlayback = false;
		m_nPlayMode = 0;
		m_nbkImageNum = 0;
		m_nbkChangeNum = 0;
		m_nchangecount = 0;
		m_imgalign = 0;
		m_bdelaynotify = false;
	}

	CLabelAnimUI::~CLabelAnimUI(void)
	{
	}

	LPCTSTR CLabelAnimUI::GetClass() const
	{
		return _T("LabelAnimUI");
	}

	LPVOID CLabelAnimUI::GetInterface(LPCTSTR pstrName)
	{
		if (_tcscmp(pstrName, _T("LabelAnim")) == 0) return this;
		return CLabelUI::GetInterface(pstrName);
	}

	void CLabelAnimUI::SetAttribute(LPCTSTR pstrName, LPCTSTR pstrValue)
	{
		//新增属性-----| movestep, movetype, playtimer, autoplay, playback |;
		if (_tcscmp(pstrName, _T("movestep")) == 0) {
			SetMoveStep(_ttoi(pstrValue));
		}
		else if (_tcscmp(pstrName, _T("movetype")) == 0) {
			SetMoveType(_ttoi(pstrValue));
		}
		else if (_tcscmp(pstrName, _T("playtimer")) == 0) {
			SetPalyTimer(_ttoi(pstrValue));
		}
		else if (_tcscmp(pstrName, _T("autoplay")) == 0) {
			SetAutoPlay(_tcscmp(pstrValue, _T("true")) == 0);
		}
		else if (_tcscmp(pstrName, _T("playback")) == 0) {
			SetPlayback(_tcscmp(pstrValue, _T("true")) == 0);
		}
		else if (_tcscmp(pstrName, _T("playmode")) == 0) {
			SetPlayMode(_ttoi(pstrValue));
		}
		else if (_tcscmp(pstrName, _T("playimagew")) == 0) {
			SetBkImageWidth(_ttoi(pstrValue));
		}
		else if (_tcscmp(pstrName, _T("playimageh")) == 0) {
			SetBkImageHeight(_ttoi(pstrValue));
		}
		else if (_tcscmp(pstrName, _T("bkimagecount")) == 0) {
			m_nbkImageNum = _ttoi(pstrValue);
		}
		else if (_tcscmp(pstrName, _T("bkchangenum")) == 0) {
			m_nbkChangeNum = _ttoi(pstrValue);
		}
		else if (_tcscmp(pstrName, _T("bkimagefmt")) == 0){
			SetBkImageFmt(pstrValue);
		}
		else if (_tcscmp(pstrName,_T("imgalign")) == 0){
			m_imgalign = _ttoi(pstrValue);
		}
		else if (_tcscmp(pstrName, _T("delaynotify")) == 0) {
			m_bdelaynotify = (_tcscmp(pstrValue, _T("true")) == 0);
		}
		else
			CLabelUI::SetAttribute(pstrName, pstrValue);
	}

	//设置移动步长;
	void CLabelAnimUI::SetMoveStep(int nlen)
	{
		m_nMovesteplen = nlen;
	}

	int CLabelAnimUI::GetMoveStep() const
	{
		return m_nMovesteplen;
	}

	//设置移动类型(0:水平,1:垂直);
	void CLabelAnimUI::SetMoveType(int ntype)
	{
		m_nMovetype = ntype;
	}

	int CLabelAnimUI::GetMoveType()
	{
		return m_nMovetype;
	}

	//设置播放时长;
	void CLabelAnimUI::SetPalyTimer(int nlen)
	{
		m_nPlayTimerLen = nlen;
	}

	//播放动画;
	void CLabelAnimUI::PlayAnim()
	{
		ClosePlay();
		if (m_bplay || m_nPlayTimerLen == 0) 
			return;

		m_pManager->SetTimer(this, EVENT_ATIEM_ID, m_nPlayTimerLen);

		m_bplay = true;
	}

	//关闭动画;
	void CLabelAnimUI::ClosePlay()
	{
		m_bplay = false;
		m_nPlayingLen = 0;
		m_nchangecount = 0;
		m_pManager->KillTimer(this, EVENT_ATIEM_ID);
	}

	//设置自动播放;
	void CLabelAnimUI::SetAutoPlay(bool bpaly)
	{
		m_bAutoPlay = bpaly;
	}

	bool CLabelAnimUI::GetAutoPlay()
	{
		return m_bAutoPlay;
	}

	//是否回播;
	void CLabelAnimUI::SetPlayback(bool bplayback)
	{
		m_bPlayback = bplayback;
	}

	bool CLabelAnimUI::GetPlayback()
	{
		return m_bPlayback;
	}

	//播放终止类型(0：随机，1：居中，2：底部，3，顶部);
	void CLabelAnimUI::SetPlayMode(int nmode)
	{
		m_nPlayMode = nmode;
	}

	int	CLabelAnimUI::GetPlayMode()
	{
		return m_nPlayMode;
	}

	void CLabelAnimUI::SetBkImageWidth(int nWidth)
	{
		m_nbkImageWidth = nWidth;
	}

	void CLabelAnimUI::SetBkImageHeight(int nHeight)
	{
		m_nbkImageHeight = nHeight;
	}

	void CLabelAnimUI::SetBkImageFmt(LPCTSTR pvalue)
	{
		m_BkImageFmt = pvalue;
	}

	void CLabelAnimUI::OnTimer(UINT_PTR idEvent, TEventUI& event)
	{
		if (idEvent != EVENT_ATIEM_ID && idEvent != EVENT_DELAYNOTIF_ID)
			return;

		if (EVENT_DELAYNOTIF_ID == idEvent && m_pManager) {
			m_pManager->KillTimer(this, EVENT_DELAYNOTIF_ID);
			m_pManager->SendNotify(this, DUI_MSGTYPE_PLAYFINISH, event.wParam, event.lParam);
			return;
		}

		m_nPlayingLen += m_nMovesteplen;

		bool bInalidate = true;
		bool bFinish = false;
		if (m_nMovetype == 0) 
		{
			switch (m_nPlayMode)
			{
			case 0:
			{
				if (m_rcItem.left + m_nPlayingLen > (m_rcItem.right - (m_rcItem.right - m_rcItem.left) / 5)) {
					m_pManager->KillTimer(this, EVENT_ATIEM_ID);
					bInalidate = false;
					bFinish = true;					
				}
				break;
			}			
			case 1: 
			{
				int ncenterleft = 0;
				ncenterleft = ((m_rcItem.right - m_rcItem.left) - m_nbkImageWidth) / 2;
				if (m_nPlayingLen > ncenterleft) {
					m_nPlayingLen = ncenterleft;
					m_pManager->KillTimer(this, EVENT_ATIEM_ID);
					bFinish = true;					
				}
				break;
			}				
			case 3:
				break;
			default:
				break;
			}			
		}
		else if (m_nMovetype == 1) 
		{
			switch (m_nPlayMode)
			{
			case 0:
				if (m_rcItem.top + m_nPlayingLen > (m_rcItem.bottom - (m_rcItem.bottom - m_rcItem.top) / 5)) {
					m_pManager->KillTimer(this, EVENT_ATIEM_ID);	
					bInalidate = false;
					bFinish = true;					
				}
				break;
			case 2:
			{
				int nbottom = (m_rcItem.bottom - m_rcItem.top) - m_nbkImageHeight;
				if (m_nPlayingLen > nbottom) {
					m_nPlayingLen = nbottom;
					m_pManager->KillTimer(this, EVENT_ATIEM_ID);
					bFinish = true;					
				}
				break;
			}
			case 3:
				break;
			default:
				break;
			}			
		}
		else if (m_nMovetype == 3) //背景切换;
		{
			m_nchangecount++;
			if (m_nchangecount >= m_nbkChangeNum*m_nbkImageNum) {
				m_nchangecount = 0;
				m_pManager->KillTimer(this, EVENT_ATIEM_ID);
				bFinish = true;
			}
			else {
				CDuiString strBkimage = _T("");
				int index = m_nchangecount % m_nbkImageNum;
				strBkimage.Format(m_BkImageFmt.GetData(), index == 0 ? 1 : index);
				SetBkImage(strBkimage);
			}
		}
		else if (m_nMovetype == 5) //背景放大;
		{
			int nMoveHeight = m_nPlayingLen + m_nbkImageHeight;
			int nMoveWidth = m_nPlayingLen*m_nMovesteplen + m_nbkImageWidth;
			if (nMoveWidth >= GetWidth()) {
				m_pManager->KillTimer(this, EVENT_ATIEM_ID);
				bFinish = true;
			}
			else {
				CDuiString strBkimage = _T("");
				if (m_imgalign == 0) {
					strBkimage.Format(m_BkImageFmt.GetData(), 0, GetHeight() - nMoveHeight, nMoveWidth, GetHeight());
				}
				else if (m_imgalign == 1) {
					strBkimage.Format(m_BkImageFmt.GetData(), GetWidth() - nMoveWidth, GetHeight() - nMoveHeight,	GetWidth(), GetHeight());
				}
				SetBkImage(strBkimage);
			}
		}

		if (bInalidate) {
			this->Invalidate();
		}
		if (bFinish) {
			if(!m_bdelaynotify)	m_pManager->SendNotify(this, DUI_MSGTYPE_PLAYFINISH, event.wParam, event.lParam);
			else m_pManager->SetTimer(this, EVENT_DELAYNOTIF_ID, 500);
		}
	}

	void CLabelAnimUI::DoInit()
	{
		if (m_bAutoPlay)
		{
			PlayAnim();
		}
	}

	void CLabelAnimUI::DoEvent(TEventUI& event)
	{
		if (event.Type == UIEVENT_TIMER)
			OnTimer((UINT_PTR)event.wParam, event);
	}

	void CLabelAnimUI::PaintBkImage(HDC hDC)
	{
		int nxdest = 0;
		int nydest = 0;

		RECT rcSource;
		rcSource.bottom = rcSource.top = 0;
		rcSource.left = rcSource.right = 0;

		if (m_nMovesteplen != 0)
		{
			if (m_nMovetype == 0) nxdest = m_bPlayback ? 0 - m_nPlayingLen : m_nPlayingLen;
			if (m_nMovetype == 1) nydest = m_bPlayback ? 0 - m_nPlayingLen : m_nPlayingLen;
		}
		CRenderEngine::DrawImageString(hDC, m_pManager, m_rcItem, m_rcPaint, (LPCTSTR)m_sBkImage, nxdest,nydest, rcSource, NULL);
	}

	void CLabelAnimUI::PaintText(HDC hDC)
	{
		if (m_dwTextColor == 0) m_dwTextColor = m_pManager->GetDefaultFontColor();
		if (m_dwDisabledTextColor == 0) m_dwDisabledTextColor = m_pManager->GetDefaultDisabledColor();

		RECT rc = m_rcItem;
		rc.left += m_rcTextPadding.left;
		rc.right -= m_rcTextPadding.right;
		rc.top += m_rcTextPadding.top;
		rc.bottom -= m_rcTextPadding.bottom;

		if (m_nMovetype)
		{
			switch (m_nPlayMode)
			{
			case 2:
			{
				rc.top += m_nPlayingLen;
				break;
			}
			case 3:
				break;
			default:
				break;
			}
		}

		if (!GetEnabledEffect())
		{
			if (m_sText.IsEmpty()) return;
			int nLinks = 0;
			if (IsEnabled()) {
				if (m_bShowHtml)
					CRenderEngine::DrawHtmlText(hDC, m_pManager, rc, m_sText, m_dwTextColor, \
						NULL, NULL, nLinks, DT_SINGLELINE | m_uTextStyle);
				else {
					CRenderEngine::DrawText(hDC, m_pManager, rc, m_sText, m_dwTextColor, \
						m_iFont, DT_SINGLELINE | m_uTextStyle);
				}				
			}
			else {
				if (m_bShowHtml)
					CRenderEngine::DrawHtmlText(hDC, m_pManager, rc, m_sText, m_dwDisabledTextColor, \
						NULL, NULL, nLinks, DT_SINGLELINE | m_uTextStyle);
				else
					CRenderEngine::DrawText(hDC, m_pManager, rc, m_sText, m_dwDisabledTextColor, \
						m_iFont, DT_SINGLELINE | m_uTextStyle);
			}
		}
		else
		{
			//不使用GDI+，不使用特效;
			ASSERT(FALSE);
		}
	}
}