#include "stdafx.h"
#include "UITitleHorizontalLayout.h"
#include "UITabOption.h"

namespace DuiLib
{
	CTitleHorizontalLayoutUI::CTitleHorizontalLayoutUI()
	{		
		m_nSubOverlayWidth = 12;
		m_bOverlayWidth = true;
	}

	CTitleHorizontalLayoutUI::~CTitleHorizontalLayoutUI()
	{
	
	}

	LPCTSTR CTitleHorizontalLayoutUI::GetClass() const
	{
		return _T("TitleHorizontalLayoutUI");
	}

	LPVOID CTitleHorizontalLayoutUI::GetInterface(LPCTSTR pstrName)
	{
		if( _tcscmp(pstrName, DUI_CTR_TITLEHORIZONTALLAYOUT) == 0 ) return static_cast<CTitleHorizontalLayoutUI*>(this);
		return CHorizontalLayoutUI::GetInterface(pstrName);
	}

	UINT CTitleHorizontalLayoutUI::GetControlFlags() const
	{
		if( IsEnabled()) return UIFLAG_SETCURSOR;
		else return 0;
	}

	void CTitleHorizontalLayoutUI::SetOverLayoutWidth(bool bover)
	{
		m_bOverlayWidth = bover;
	}

	void CTitleHorizontalLayoutUI::SetPos(RECT rc)
	{
		CControlUI::SetPos(rc);
		rc = m_rcItem;

		// Adjust for inset
		rc.left += m_rcInset.left;
		rc.top += m_rcInset.top;
		rc.right -= m_rcInset.right;
		rc.bottom -= m_rcInset.bottom;

		if (m_items.GetSize() == 0) {
			ProcessScrollBar(rc, 0, 0);
			return;
		}

		if (m_pVerticalScrollBar && m_pVerticalScrollBar->IsVisible()) rc.right -= m_pVerticalScrollBar->GetFixedWidth();
		if (m_pHorizontalScrollBar && m_pHorizontalScrollBar->IsVisible()) rc.bottom -= m_pHorizontalScrollBar->GetFixedHeight();

		// Determine the width of elements that are sizeable
		SIZE szAvailable = { rc.right - rc.left, rc.bottom - rc.top };
		if (m_pHorizontalScrollBar && m_pHorizontalScrollBar->IsVisible())
			szAvailable.cx += m_pHorizontalScrollBar->GetScrollRange();

		int nAdjustables = 0;
		int cxFixed = 0;
		int nEstimateNum = 0;
		for (int it1 = 0; it1 < m_items.GetSize(); it1++) {
			CControlUI* pControl = static_cast<CControlUI*>(m_items[it1]);
			if (!pControl->IsVisible()) continue;
			if (pControl->IsFloat()) continue;
			SIZE sz = pControl->EstimateSize(szAvailable);
			if (sz.cx == 0) {
				nAdjustables++;
			}
			else {
				if (sz.cx < pControl->GetMinWidth()) sz.cx = pControl->GetMinWidth();
				if (sz.cx > pControl->GetMaxWidth()) sz.cx = pControl->GetMaxWidth();
			}
			cxFixed += sz.cx + pControl->GetPadding().left + pControl->GetPadding().right;
			nEstimateNum++;
		}
		cxFixed += (nEstimateNum - 1) * m_iChildPadding;

		int cxExpand = 0;
		int cxNeeded = 0;
		int noffsetsum = (nAdjustables - 2) * m_nSubOverlayWidth;
		if (noffsetsum < 0)  noffsetsum = 0;
		if (nAdjustables > 0) cxExpand = MAX(0, (szAvailable.cx - cxFixed + noffsetsum) / (nAdjustables));
		// Position the elements
		SIZE szRemaining = szAvailable;
		int iPosX = rc.left;
		if (m_pHorizontalScrollBar && m_pHorizontalScrollBar->IsVisible()) {
			iPosX -= m_pHorizontalScrollBar->GetScrollPos();
		}
		int iAdjustable = 0;
		int cxFixedRemaining = cxFixed;
		for (int it2 = 0; it2 < m_items.GetSize(); it2++) {
			CControlUI* pControl = static_cast<CControlUI*>(m_items[it2]);
			if (!pControl->IsVisible()) continue;
			if (pControl->IsFloat()) {
				SetFloatPos(it2);
				continue;
			}
			RECT rcPadding = pControl->GetPadding();
			szRemaining.cx -= rcPadding.left;
			SIZE sz = pControl->EstimateSize(szRemaining);
			if (sz.cx == 0) {
				iAdjustable++;
				sz.cx = cxExpand;				
				if (sz.cx < pControl->GetMinWidth()) sz.cx = pControl->GetMinWidth();
				if (sz.cx > pControl->GetMaxWidth()) sz.cx = pControl->GetMaxWidth();
			}
			else {
				if (sz.cx < pControl->GetMinWidth()) sz.cx = pControl->GetMinWidth();
				if (sz.cx > pControl->GetMaxWidth()) sz.cx = pControl->GetMaxWidth();
			}

			sz.cy = pControl->GetFixedHeight();
			if (sz.cy == 0) sz.cy = rc.bottom - rc.top - rcPadding.top - rcPadding.bottom;
			if (sz.cy < 0) sz.cy = 0;
			if (sz.cy < pControl->GetMinHeight()) sz.cy = pControl->GetMinHeight();
			if (sz.cy > pControl->GetMaxHeight()) sz.cy = pControl->GetMaxHeight();

			RECT rcCtrl = { iPosX + rcPadding.left, rc.top + rcPadding.top, iPosX + sz.cx + rcPadding.left , rc.top + rcPadding.top + sz.cy };
			if (it2 != 0 && m_bOverlayWidth) {
				rcCtrl.left = rcCtrl.left - m_nSubOverlayWidth;
				rcCtrl.right = rcCtrl.right - m_nSubOverlayWidth;
			}
			pControl->SetPos(rcCtrl);
			iPosX += sz.cx + m_iChildPadding + rcPadding.left + rcPadding.right;
			if (it2 != 0 && m_bOverlayWidth) iPosX -= m_nSubOverlayWidth;
			cxNeeded += sz.cx + rcPadding.left + rcPadding.right;
			szRemaining.cx -= sz.cx + m_iChildPadding + rcPadding.right;
		}
		cxNeeded += (nEstimateNum - 1) * m_iChildPadding;
		//reddrain
		if (m_pHorizontalScrollBar != NULL) {
			if (cxNeeded > rc.right - rc.left) {
				if (m_pHorizontalScrollBar->IsVisible()) {
					m_pHorizontalScrollBar->SetScrollRange(cxNeeded - (rc.right - rc.left));
				}
				else {
					m_pHorizontalScrollBar->SetVisible(true);
					m_pHorizontalScrollBar->SetScrollRange(cxNeeded - (rc.right - rc.left));
					m_pHorizontalScrollBar->SetScrollPos(0);
					rc.bottom -= m_pHorizontalScrollBar->GetFixedHeight();
				}
			}
			else {
				if (m_pHorizontalScrollBar->IsVisible()) {
					m_pHorizontalScrollBar->SetVisible(false);
					m_pHorizontalScrollBar->SetScrollRange(0);
					m_pHorizontalScrollBar->SetScrollPos(0);
					rc.bottom += m_pHorizontalScrollBar->GetFixedHeight();
				}
			}
		}
		ProcessScrollBar(rc, cxNeeded, 0);
	}
		
	void CTitleHorizontalLayoutUI::DoPaint(HDC hDC, const RECT& rcPaint)
	{
		CHorizontalLayoutUI::PaintBkImage(hDC);

		RECT rcTemp = { 0 };
		if (!::IntersectRect(&rcTemp, &rcPaint, &m_rcItem)) return;

		CRenderClip clip;
		CRenderClip::GenerateClip(hDC, rcTemp, clip);
		CControlUI::DoPaint(hDC, rcPaint);

		if (m_items.GetSize() == 0)
			return;

		RECT rc = m_rcItem;
		rc.left += m_rcInset.left;
		rc.top += m_rcInset.top;
		rc.right -= m_rcInset.right;
		rc.bottom -= m_rcInset.bottom;

		if (m_pVerticalScrollBar && m_pVerticalScrollBar->IsVisible()) 
			rc.right -= m_pVerticalScrollBar->GetFixedWidth();

		if (m_pHorizontalScrollBar && m_pHorizontalScrollBar->IsVisible())
			rc.bottom -= m_pHorizontalScrollBar->GetFixedHeight();

		if (!::IntersectRect(&rcTemp, &rcPaint, &rc))
		{
			for (int it = 0; it < m_items.GetSize(); it++)
			{
				CControlUI* pControl = static_cast<CControlUI*>(m_items[it]);
				if (!pControl->IsVisible()) continue;
				if (!::IntersectRect(&rcTemp, &rcPaint, &pControl->GetPos())) 
					continue;
				if (pControl->IsFloat()) 
				{
					if (!::IntersectRect(&rcTemp, &m_rcItem, &pControl->GetPos()))
						continue;
					pControl->DoPaint(hDC, rcPaint);
				}
			}
		}
		else 
		{
			CRenderClip childClip;
			CRenderClip::GenerateClip(hDC, rcTemp, childClip);
			int nSize = m_items.GetSize();

			CTabOptionUI* pSelOption = NULL;

			for (int it = nSize - 1; it >= 0; it--)
			{
				CControlUI* pControl = static_cast<CControlUI*>(m_items[it]);
				if (!pControl->IsVisible())
					continue;
				if (!::IntersectRect(&rcTemp, &rcPaint, &pControl->GetPos()))
					continue;
				if (pControl->IsFloat())
				{
					if (!::IntersectRect(&rcTemp, &m_rcItem, &pControl->GetPos()))
						continue;
					CRenderClip::UseOldClipBegin(hDC, childClip);
					pControl->DoPaint(hDC, rcPaint);
					CRenderClip::UseOldClipEnd(hDC, childClip);
				}
				else
				{
					if (!::IntersectRect(&rcTemp, &rc, &pControl->GetPos()))
						continue;
					pControl->DoPaint(hDC, rcPaint);

					CTabOptionUI* ptempSelOption = dynamic_cast<CTabOptionUI*>(pControl);
					if (ptempSelOption && ptempSelOption->IsSelected()) pSelOption = ptempSelOption;
				}
			}
			if (pSelOption && m_bOverlayWidth)
			{
				if (::IntersectRect(&rcTemp, &rc, &pSelOption->GetPos()))				
					pSelOption->DoPaint(hDC, rcPaint);
			}
		}
	}
}
