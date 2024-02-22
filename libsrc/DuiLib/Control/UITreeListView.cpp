#include "StdAfx.h"

namespace DuiLib {

	CTreeListViewUI::CTreeListViewUI()
	{

	}

	LPCTSTR CTreeListViewUI::GetClass() const
	{
		return _T("TreeListViewUI");
	}

	LPVOID CTreeListViewUI::GetInterface(LPCTSTR pstrName)
	{
		if (_tcscmp(pstrName, DUI_CTR_TREELISTVIEW) == 0)  return static_cast<CTreeListViewUI*>(this);
		return CListUI::GetInterface(pstrName);
	}

	int CTreeListViewUI::GetCount() const
	{
		return m_stdArrayChilds.GetSize();
	}

	bool CTreeListViewUI::Add(CControlUI* pControl)
	{
		ASSERT(pControl);
		CTreeListViewHeaderUI* plistHeader = static_cast<CTreeListViewHeaderUI*>(pControl->GetInterface(DUI_CTR_TREELISTVIEWHEAD));
		if (plistHeader == NULL)
			return false;
		if (!m_pList->Add(pControl))
			return false;
		for (int i = 0;i < plistHeader->GetCount();i++)
		{
			if (!m_pList->Add(plistHeader->GetItemAt(i)))
			{
				return false;
			}
		}
		plistHeader->SetOwner(this);
		plistHeader->SetIndex(GetCount());

		m_stdArrayChilds.Add(plistHeader);
		return true;
	}

	bool CTreeListViewUI::AddAt(CControlUI* pControl, int iIndex)
	{
		ASSERT(pControl);
		CTreeListViewHeaderUI* pHeader = static_cast<CTreeListViewHeaderUI*>(pControl->GetInterface(DUI_CTR_TREELISTVIEWHEAD));
		if (pHeader == NULL) return false;
		int iItemIndex = 0;

		if (GetCount() == iIndex)
		{
			if (!m_pList->Add(pControl)) return FALSE;
		}
		else
		{
			iItemIndex = m_pList->GetItemIndex(GetItemAt(iIndex));
			if (!m_pList->AddAt(pControl, iItemIndex)) return false;
		}

		for (int i = 0;i < pHeader->GetCount();i++)
		{
			if (!m_pList->AddAt(pControl, iItemIndex + i + 1)) return false;
		}

		pHeader->SetOwner(this);
		pHeader->SetIndex(iIndex);

		m_stdArrayChilds.InsertAt(iIndex, pHeader);
		for (int i = iIndex + 1; i < GetCount(); ++i)
		{
			CControlUI* p = GetItemAt(i);
			IListItemUI* pItem = static_cast<IListItemUI*>(p->GetInterface(DUI_CTR_LISTITEM));
			if (pItem != NULL)
			{
				pItem->SetIndex(i);
			}
		}
		return true;
	}

	bool CTreeListViewUI::Remove(CControlUI* pControl)
	{
		ASSERT(pControl);
		CTreeListViewHeaderUI* pHeader = static_cast<CTreeListViewHeaderUI*>(pControl->GetInterface(DUI_CTR_TREELISTVIEWHEAD));
		if (pHeader == NULL) return false;

		int iIndex = GetItemIndex(pHeader);
		if (iIndex == -1) return false;

		for (int i = 0;i < pHeader->GetCount();i++)
		{
			CControlUI* pItem = pHeader->GetItemAt(i);
			if (!m_pList->Remove(pItem)) return false;
		}

		m_stdArrayChilds.Remove(pHeader);
		if (!m_pList->Remove(pHeader)) return false;

		for (int i = iIndex; i < GetCount(); ++i)
		{
			CControlUI* p = GetItemAt(i);
			IListItemUI* pListItem = static_cast<IListItemUI*>(p->GetInterface(DUI_CTR_LISTITEM));
			if (pListItem != NULL)
			{
				pListItem->SetIndex(i);
			}
		}
		return true;
	}

	bool CTreeListViewUI::RemoveAt(int iIndex)
	{
		if (iIndex < 0 || iIndex >= GetCount()) return false;

		CControlUI* pControl = GetItemAt(iIndex);
		ASSERT(pControl);

		CTreeListViewHeaderUI* pHeader = static_cast<CTreeListViewHeaderUI*>(pControl->GetInterface(DUI_CTR_TREELISTVIEWHEAD));
		if (pHeader == NULL) return false;
		for (int i = 0;i < pHeader->GetCount();i++)
		{
			CControlUI* pItem = pHeader->GetItemAt(i);
			if (!m_pList->Remove(pItem)) return false;
		}
		m_stdArrayChilds.Remove(iIndex);
		if (!m_pList->Remove(pHeader)) return false;

		for (int i = iIndex; i < GetCount(); ++i)
		{
			CControlUI* p = GetItemAt(i);
			IListItemUI* pListItem = static_cast<IListItemUI*>(p->GetInterface(DUI_CTR_LISTITEM));
			if (pListItem != NULL)
			{
				pListItem->SetIndex(i);
			}
		}
		return true;
	}

	void CTreeListViewUI::RemoveAll()
	{
		m_stdArrayChilds.Empty();
		return CListUI::RemoveAll();
	}

	CControlUI* CTreeListViewUI::GetItemAt(int iIndex) const
	{
		if (iIndex < 0 || iIndex >= m_stdArrayChilds.GetSize()) return NULL;
		return static_cast<CControlUI*>(m_stdArrayChilds[iIndex]);
	}

	int CTreeListViewUI::GetItemIndex(CControlUI* pControl) const
	{
		return m_stdArrayChilds.Find(pControl);
	}

	bool CTreeListViewUI::SetItemIndex(CControlUI* pControl, int iIndex, bool bupdate)
	{
		int iOrginIndex = GetItemIndex(pControl);
		if (iOrginIndex == -1) return false;
		if (iOrginIndex == iIndex) return true;

		IListItemUI* pSelectedListItem = NULL;
		if (m_iCurSel >= 0) pSelectedListItem = static_cast<IListItemUI*>(GetItemAt(m_iCurSel)->GetInterface(DUI_CTR_LISTITEM));
		CTreeListViewHeaderUI* pHeader = static_cast<CTreeListViewHeaderUI*>(pControl);
		int iHeaderIndex = m_pList->GetItemIndex(GetItemAt(iIndex));

		if (!m_pList->SetItemIndex(pControl, iIndex)) return false;
		for (int i = 0; i < pHeader->GetCount();i++)
		{
			if (!m_pList->SetItemIndex(pHeader->GetItemAt(i), iHeaderIndex + i + 1))
				return false;
		}

		for (int it = 0; it < m_stdArrayChilds.GetSize(); it++)
		{
			if (static_cast<CControlUI*>(m_stdArrayChilds[it]) == pControl)
			{
				NeedUpdate();
				m_stdArrayChilds.Remove(it);
				return m_stdArrayChilds.InsertAt(iIndex, pControl);
			}
		}

		int iMinIndex = min(iOrginIndex, iIndex);
		int iMaxIndex = max(iOrginIndex, iIndex);
		for (int i = iMinIndex; i < iMaxIndex + 1; ++i)
		{
			CControlUI* p = GetItemAt(i);
			IListItemUI* pListItem = static_cast<IListItemUI*>(p->GetInterface(DUI_CTR_LISTITEM));
			if (pListItem != NULL)
			{
				pListItem->SetIndex(i);
			}
		}

		if (m_iCurSel >= 0 && pSelectedListItem != NULL)
			m_iCurSel = pSelectedListItem->GetIndex();

		return true;
	}

	bool CTreeListViewUI::SelectItem(int iIndex, bool bTakeFocus, bool bsendmsg)
	{
		if (iIndex == m_iCurSel) return true;

		// We should first unselect the currently selected item;
		if (m_iCurSel >= 0)
		{
			CControlUI* pControl = GetItemAt(m_iCurSel);
			if (pControl != NULL) {
				IListItemUI* pListItem = static_cast<IListItemUI*>(pControl->GetInterface(_T("ListItem")));
				if (pListItem != NULL)
				{
					pListItem->Select(false);
				}
			}
			m_iCurSel = -1;
		}

		if (iIndex < 0)
			return false;

		if (!__super::SelectItem(iIndex, bTakeFocus))
			return false;

		return true;
	}
	////////////////////////////////////////////////////
	CTreeListContainerElementUI::CTreeListContainerElementUI()
	{
	}

	LPCTSTR CTreeListContainerElementUI::GetClass() const
	{
		return _T("TreeListContainerElementUI");
	}

	UINT CTreeListContainerElementUI::GetControlFlags() const
	{
		return UIFLAG_WANTRETURN;
	}

	LPVOID CTreeListContainerElementUI::GetInterface(LPCTSTR pstrName)
	{
		if (_tcscmp(pstrName, DUI_CTR_TREELISTCONTAINERELEMENT) == 0) return static_cast<CTreeListContainerElementUI*>(this);
		return __super::GetInterface(pstrName);
	}

	void CTreeListContainerElementUI::SetPos(RECT rc)
	{
		CHorizontalLayoutUI::SetPos(rc);
		if (m_pOwner == NULL) return;

		CTreeListViewHeaderUI* pListViewHead = static_cast<CTreeListViewHeaderUI*>(m_pOwner);
		if (pListViewHead == NULL)  return;

		CListUI* pList = static_cast<CListUI*>(pListViewHead->GetOwner());
		if (pList == NULL) return;

		CListHeaderUI *pHeader = pList->GetHeader();
		if (pHeader == NULL || !pHeader->IsVisible())
			return;

		int nCount = m_items.GetSize();
		for (int i = 0; i < nCount; i++)
		{
			CControlUI *pListItem = static_cast<CControlUI*>(m_items[i]);

			CControlUI *pHeaderItem = pHeader->GetItemAt(i);
			if (pHeaderItem == NULL)
				return;

			RECT rcHeaderItem = pHeaderItem->GetPos();
			if (pListItem != NULL && !(rcHeaderItem.left == 0 && rcHeaderItem.right == 0))
			{
				RECT rt = pListItem->GetPos();
				rt.left = rcHeaderItem.left;
				rt.right = rcHeaderItem.right;
				pListItem->SetPos(rt);
			}

		}
	}


	////////////////////////////////////////////////////
	CTreeListViewHeaderUI::CTreeListViewHeaderUI()
	{
		m_iIndex = -1;
		m_iCurSel = -1;
		m_pOwner = NULL;
		m_bSelected = false;
		m_bExpanded = false;

		m_dwHotBkcolor = 0;
		m_dwBkcolor = 0;
		m_dwSelectedBkcolor = 0;
		m_sDisabledImage = _T("");
		m_sSelectedImage = _T("");
		m_sHotImage = _T("");
		m_sBkImage = _T("");
		m_stext = _T("");
		m_pCompareFunc = NULL;

		m_pHorizontallayout = new CHorizontalLayoutUI();  //布局;
		m_pExpandOption = new COptionUI();                //扩展;
		m_pIconLabel = new CLabelIconUI();				  //图标;
		m_pTitleText = new CLabelUI();				      //标题;
		m_pFistEventBtn = new CButtonUI();				  //中间button;
		m_pEventBtn = new CButtonUI();				      //末尾图标;

		m_pExpandOption->SetVisible(false);
		m_pIconLabel->SetVisible(false);
		m_pTitleText->SetVisible(false);
		m_pFistEventBtn->SetVisible(false);
		m_pEventBtn->SetVisible(false);
	
		m_pHorizontallayout->SetBkColor(0x00220000);
		m_pHorizontallayout->Add(m_pExpandOption);
		m_pHorizontallayout->Add(m_pIconLabel);
		m_pHorizontallayout->Add(m_pTitleText);
		m_pHorizontallayout->Add(m_pFistEventBtn);
		m_pHorizontallayout->Add(m_pEventBtn);

		CListContainerElementUI::Add(m_pHorizontallayout);
	}

	LPVOID CTreeListViewHeaderUI::GetInterface(LPCTSTR pstrName)
	{
		if (_tcscmp(pstrName, DUI_CTR_TREELISTVIEWHEAD) == 0)
			return static_cast<CTreeListViewHeaderUI*>(this);
		else if (_tcscmp(pstrName, _T("IListOwner")) == 0)
			return static_cast<IListOwnerUI*>(this);
		return NULL;
	}

	LPCTSTR CTreeListViewHeaderUI::GetClass() const
	{
		return _T("TreeListViewHeaderUI");
	}

	UINT CTreeListViewHeaderUI::GetControlFlags() const
	{
		return UIFLAG_WANTRETURN;
	}

	IListOwnerUI* CTreeListViewHeaderUI::GetOwner()
	{
		return m_pOwner;
	}

	void CTreeListViewHeaderUI::SetOwner(CControlUI* pOwner)
	{
		m_pOwner = static_cast<IListOwnerUI*>(pOwner->GetInterface(_T("IListOwner")));
	}

	void CTreeListViewHeaderUI::SetVisible(bool bVisible)
	{
		CControlUI::SetVisible(bVisible);
		for (int i = 0;i < GetCount();i++)
		{
			CControlUI* pControl = static_cast<CControlUI*>(m_childs[i]);
			if (pControl)
			{
				if (!m_bExpanded && bVisible)
					pControl->SetVisible(false);
				else
					pControl->SetVisible(bVisible);
			}
		}
		if (!IsVisible() && m_bSelected)
		{
			m_bSelected = false;
			if (m_pOwner != NULL)
				m_pOwner->SelectItem(-1);
		}
	}

	void CTreeListViewHeaderUI::SetText(LPCTSTR pstrText)
	{
		if (_tcsicmp(pstrText, m_stext.GetData()) != 0)
		{
			if (m_pTitleText)
			{
				m_pTitleText->SetText(pstrText);
				m_pTitleText->Invalidate();
			}
			m_stext = pstrText;
		}
	}

	CLabelUI* CTreeListViewHeaderUI::GetTitleLabel()
	{
		return m_pTitleText;
	}

	CDuiString CTreeListViewHeaderUI::GetText()
	{
		return m_stext;
	}

	int CTreeListViewHeaderUI::GetIndex() const
	{
		return m_iIndex;
	}

	void CTreeListViewHeaderUI::SetIndex(int iIndex)
	{
		m_iIndex = iIndex;
	}

	void CTreeListViewHeaderUI::SetIconImage(LPCTSTR strImage)
	{
		if (m_pIconLabel)
		{
			m_pIconLabel->SetBkImage(strImage);
		}
	}

	CDuiString CTreeListViewHeaderUI::GetIconImage()
	{
		CDuiString strImage = _T("");
		if (m_pIconLabel)
		{
			strImage = m_pIconLabel->GetBkImage();
		}
		return strImage;
	}

	void CTreeListViewHeaderUI::SetEventName(LPCTSTR strname)
	{
		if (m_pEventBtn) m_pEventBtn->SetName(strname);
	}

	void CTreeListViewHeaderUI::SetEventUserData(LPCTSTR struserdate)
	{
		if (m_pEventBtn) m_pEventBtn->SetUserData(struserdate);		
	}

	void CTreeListViewHeaderUI::SetExButtonText(LPCTSTR strname)
	{
		if (m_pFistEventBtn) m_pFistEventBtn->SetText(strname);
	}

	void CTreeListViewHeaderUI::SetExButtonName(LPCTSTR strname)
	{
		if (m_pFistEventBtn) m_pFistEventBtn->SetName(strname);
	}

	void CTreeListViewHeaderUI::SetExButtonUserData(LPCTSTR struserdate)
	{
		if (m_pFistEventBtn) m_pFistEventBtn->SetUserData(struserdate);
	}

	void CTreeListViewHeaderUI::SetExButtonEnabled(bool bEnabled)
	{
		if (m_pFistEventBtn) m_pFistEventBtn->SetEnabled(bEnabled);
	}

	void CTreeListViewHeaderUI::SetExpandBkImage(LPCTSTR strImage)
	{
		if (m_pExpandOption)
		{
			m_pExpandOption->SetBkImage(strImage);
		}
	}

	void CTreeListViewHeaderUI::SetExpandSelectedImg(LPCTSTR strImage)
	{
		if (m_pExpandOption)
		{
			m_pExpandOption->SetSelectedImage(strImage);
		}
	}

	void CTreeListViewHeaderUI::SetExpandEnabled(bool bEnabled)
	{
		if (m_pExpandOption)
		{
			m_pExpandOption->SetEnabled(bEnabled);
		}
	}

	bool CTreeListViewHeaderUI::IsExpanded() const
	{
		return m_bExpanded;
	}

	bool CTreeListViewHeaderUI::Activate()
	{
		return Expand(!m_bExpanded);
	}

	bool CTreeListViewHeaderUI::Expand(bool bExpand)
	{
		if (m_bExpanded == bExpand) return false;
		m_bExpanded = bExpand;

		if (m_bExpanded && m_pManager != NULL)
		{
			m_pManager->SendNotify(this, DUI_MSGTYPE_LISTHEADEXPAND);
		}
		else
		{
			m_iCurSel = -1;
		}

		Invalidate();

		for (int i = 0;i < GetCount();i++)
		{
			CControlUI* pControl = static_cast<CControlUI*>(m_childs[i]);
			if (pControl)
			{
				pControl->SetVisible(m_bExpanded);
			}
		}
		if (m_pExpandOption) m_pExpandOption->Selected(m_bExpanded);
		return m_bExpanded;
	}

	TListInfoUI* CTreeListViewHeaderUI::GetListInfo()
	{
		TListInfoUI* pListInfo = NULL;
		if (m_pOwner != NULL)
		{
			pListInfo = m_pOwner->GetListInfo();
		}
		return pListInfo;
	}

	int CTreeListViewHeaderUI::GetCurSel() const
	{
		return m_iCurSel;
	}

	bool CTreeListViewHeaderUI::SelectItem(int iIndex, bool bTakeFocus, bool bsendmsg)
	{
		if (iIndex == m_iCurSel)
		{
			if (bsendmsg && m_pManager != NULL)
			{				
				m_pManager->SendNotify(this, DUI_MSGTYPE_ITEMSELSCR, m_iCurSel, m_iCurSel);
			}
			return true;
		}		
		int iOldSel = m_iCurSel;

		if (m_iCurSel >= 0)
		{
			CControlUI* pControl = GetItemAt(m_iCurSel);
			if (pControl != NULL)
			{
				IListItemUI* pListItem = static_cast<IListItemUI*>(pControl->GetInterface(DUI_CTR_LISTITEM));
				if (pListItem != NULL) pListItem->Select(false);
			}
			m_iCurSel = -1;
		}
		if (iIndex < 0) return false;

		CControlUI* pControl = GetItemAt(iIndex);
		if (pControl == NULL) return false;
		if (!pControl->IsVisible()) return false;
		if (!pControl->IsEnabled()) return false;

		IListItemUI* pListItem = static_cast<IListItemUI*>(pControl->GetInterface(DUI_CTR_LISTITEM));
		if (pListItem == NULL) return false;
		m_iCurSel = iIndex;

		if (!pListItem->Select(true))
		{
			m_iCurSel = -1;
			return false;
		}

		CListUI* pList = static_cast<CListUI*>(m_pOwner);
		if (pList)
		{
			pList->EnsureVisible(m_iCurSel);
		}
		CTreeListViewUI* pTreeList = static_cast<CTreeListViewUI*>(m_pOwner);
		if (pTreeList)
		{
			for (int i = 0;i < pTreeList->GetCount();i++)
			{
				CTreeListViewHeaderUI* pFriends = static_cast<CTreeListViewHeaderUI*>(pTreeList->GetItemAt(i));
				if (pFriends&&pFriends != this) pFriends->SelectItem(-1);
			}
		}
		if (bTakeFocus) pControl->SetFocus();
		if (m_pManager != NULL)
		{
			if (bsendmsg)
				m_pManager->SendNotify(this, DUI_MSGTYPE_ITEMSELSCR, m_iCurSel, iOldSel);
			else
				m_pManager->SendNotify(this, DUI_MSGTYPE_ITEMSELECT, m_iCurSel, iOldSel);
		}
		return true;
	}

	bool CTreeListViewHeaderUI::Add(CControlUI* pControl)
	{
		ASSERT(pControl);
		IListItemUI* pListItem = static_cast<IListItemUI*>(pControl->GetInterface(DUI_CTR_LISTITEM));
		if (pListItem == NULL) return false;
		CTreeListViewUI* pList = static_cast<CTreeListViewUI*>(m_pOwner);
		if (pList != NULL)
		{
			int iIndex = pList->GetList()->GetItemIndex(this);
			iIndex += GetCount();
			iIndex++;
			if (!pList->GetList()->AddAt(pControl, iIndex))
			{
				return false;
			}
		}
		pListItem->SetIndex(GetCount());
		pControl->OnNotify += MakeDelegate(this, &CTreeListViewHeaderUI::OnItemActive);
		m_childs.Add(pControl);
		pControl->SetVisible(m_bExpanded);
		pListItem->SetOwner(this);
		return true;
	}

	bool CTreeListViewHeaderUI::AddAt(CControlUI* pControl, int iIndex)
	{
		ASSERT(pControl);
		if (iIndex<0 || iIndex>GetCount()) return false;
		IListItemUI* pListItem = static_cast<IListItemUI*>(pControl->GetInterface(DUI_CTR_LISTITEM));
		if (pListItem == NULL) return false;
		CTreeListViewUI* pList = static_cast<CTreeListViewUI*>(m_pOwner);
		if (pList != NULL)
		{
			int iNodeIndex = pList->GetList()->GetItemIndex(this);
			int itemIndex = iIndex + iNodeIndex + 1;
			if (!pList->GetList()->AddAt(pControl, itemIndex)) return false;
		}

		pControl->OnNotify += MakeDelegate(this, &CTreeListViewHeaderUI::OnItemActive);
		m_childs.InsertAt(iIndex, pControl);
		pControl->SetVisible(m_bExpanded);
		pListItem->SetOwner(this);
		pListItem->SetIndex(iIndex);

		for (int i = iIndex + 1; i < GetCount(); ++i)
		{
			CControlUI* p = GetItemAt(i);
			pListItem = static_cast<IListItemUI*>(p->GetInterface(DUI_CTR_LISTITEM));
			if (pListItem != NULL)
			{
				pListItem->SetIndex(i);
			}
		}
		if (m_iCurSel >= iIndex) m_iCurSel += 1;
		return true;
	}

	bool CTreeListViewHeaderUI::Remove(CControlUI* pControl)
	{
		ASSERT(pControl);
		int iIndex = GetItemIndex(pControl);
		if (iIndex == -1) return false;

		CTreeListViewUI* pList = static_cast<CTreeListViewUI*>(m_pOwner);
		if (pList != NULL)
			if (!pList->GetList()->Remove(pControl)) return false;

		pControl->OnNotify -= MakeDelegate(this, &CTreeListViewHeaderUI::OnItemActive);
		m_childs.Remove(pControl);
		for (int i = iIndex; i < GetCount(); ++i)
		{
			CControlUI* p = GetItemAt(i);
			IListItemUI* pListItem = static_cast<IListItemUI*>(p->GetInterface(DUI_CTR_LISTITEM));
			if (pListItem != NULL)
			{
				pListItem->SetIndex(i);
			}
		}

		if (iIndex == m_iCurSel && m_iCurSel >= 0)
		{
			int iSel = m_iCurSel;
			m_iCurSel = -1;
			SelectItem(FindSelectable(iSel, false));
		}
		else if (iIndex < m_iCurSel) m_iCurSel -= 1;
		return true;
	}

	bool CTreeListViewHeaderUI::RemoveAt(int iIndex)
	{
		if (iIndex < 0 || iIndex >= GetCount()) return false;
		CControlUI* pControl = GetItemAt(iIndex);
		ASSERT(pControl);

		CTreeListViewUI* pList = static_cast<CTreeListViewUI*>(m_pOwner);
		if (pList != NULL)
			if (!pList->GetList()->Remove(pControl)) return false;

		pControl->OnNotify -= MakeDelegate(this, &CTreeListViewHeaderUI::OnItemActive);
		m_childs.Remove(pControl);
		for (int i = iIndex; i < GetCount(); ++i)
		{
			CControlUI* p = GetItemAt(i);
			IListItemUI* pListItem = static_cast<IListItemUI*>(p->GetInterface(DUI_CTR_LISTITEM));
			if (pListItem != NULL) pListItem->SetIndex(i);
		}
		if (iIndex == m_iCurSel && m_iCurSel >= 0)
		{
			int iSel = m_iCurSel;
			m_iCurSel = -1;
			SelectItem(FindSelectable(iSel, false));
		}
		else if (iIndex < m_iCurSel) m_iCurSel -= 1;
		return true;
	}

	void CTreeListViewHeaderUI::RemoveAll()
	{
		m_iCurSel = -1;
		for (int i = 0;i < GetCount();i++)
		{
			CControlUI* pControl = GetItemAt(i);
			ASSERT(pControl);
			pControl->OnNotify -= MakeDelegate(this, &CTreeListViewHeaderUI::OnItemActive);
			CTreeListViewUI* pList = static_cast<CTreeListViewUI*>(m_pOwner);
			if (pList != NULL)
				pList->GetList()->Remove(pControl);
		}
		m_childs.Empty();
	}

	CControlUI* CTreeListViewHeaderUI::GetItemAt(int iIndex) const
	{
		if (iIndex >= GetCount() || iIndex < 0) return NULL;
		return static_cast<CControlUI*>(m_childs.GetAt(iIndex));
	}

	void CTreeListViewHeaderUI::SortChildItems(PULVCompareFunc pfnCompare, UINT_PTR dwData)
	{		
		if (!pfnCompare)
			return;

		if (m_childs.GetSize() < 2)
			return;

		m_pCompareFunc = pfnCompare;

		CStdPtrArray stchilds(m_childs);
		qsort_s(stchilds.GetData(), stchilds.GetSize(), sizeof(CControlUI*), CTreeListViewHeaderUI::ItemComareFunc, this);
		IListItemUI *pItem = NULL;
		for (int i = 0; i < stchilds.GetSize(); ++i)
		{
			pItem = (IListItemUI*)(static_cast<CControlUI*>(stchilds[i])->GetInterface(TEXT("ListItem")));
			if (pItem)
			{
				SetItemIndex(static_cast<CControlUI*>(stchilds[i]), i, false);
			}
		}
		SelectItem(-1, false, false);
		if (m_pManager)
		{
			SetPos(GetPos());
			Invalidate();
		}
		return;
	}

	int __cdecl CTreeListViewHeaderUI::ItemComareFunc(void *pvlocale, const void *item1, const void *item2)
	{
		CTreeListViewHeaderUI *pThis = (CTreeListViewHeaderUI*)pvlocale;
		if (!pThis || !item1 || !item2)
			return 0;
		return pThis->ItemComareFunc(item1, item2);
	}

	int __cdecl CTreeListViewHeaderUI::ItemComareFunc(const void *item1, const void *item2)
	{
		CControlUI *pControl1 = *(CControlUI**)item1;
		CControlUI *pControl2 = *(CControlUI**)item2;
		return m_pCompareFunc((UINT_PTR)pControl1, (UINT_PTR)pControl2, m_compareData);
	}

	int CTreeListViewHeaderUI::GetItemIndex(CControlUI* pControl) const
	{
		ASSERT(pControl);
		return m_childs.Find(pControl);
	}

	bool CTreeListViewHeaderUI::SetItemIndex(CControlUI* pControl, int iIndex, bool bupdate)
	{
		ASSERT(pControl);
		int iOrginIndex = GetItemIndex(pControl);
		if (iOrginIndex == -1) return false;
		if (iOrginIndex == iIndex) return true;
		IListItemUI* pSelectedListItem = NULL;
		if (m_iCurSel >= 0)
			pSelectedListItem = static_cast<IListItemUI*>(GetItemAt(m_iCurSel)->GetInterface(DUI_CTR_LISTITEM));

		CListUI* pList = static_cast<CListUI*>(m_pOwner);
		if (pList != NULL)
		{
			int iNodeIndex = pList->GetList()->GetItemIndex(this);
			pList->GetList()->SetItemIndex(pControl, iIndex + iNodeIndex + 1);
		}
		for (int it = 0; it < m_childs.GetSize(); it++)
		{
			if (static_cast<CControlUI*>(m_childs[it]) == pControl)
			{
				if (bupdate) NeedUpdate();
				m_childs.Remove(it);
				m_childs.InsertAt(iIndex, pControl);
				break;
			}
		}
		int iMinIndex = min(iOrginIndex, iIndex);	//当前位置和移动前的位置
		int iMaxIndex = max(iOrginIndex, iIndex);
		for (int i = iMinIndex; i < iMaxIndex + 1; ++i)
		{
			CControlUI* p = GetItemAt(i);
			IListItemUI* pListItem = static_cast<IListItemUI*>(p->GetInterface(DUI_CTR_LISTITEM));
			if (pListItem != NULL)
			{
				pListItem->SetIndex(i);
			}
		}
		if (m_iCurSel >= 0 && pSelectedListItem != NULL)
		{
			m_iCurSel = pSelectedListItem->GetIndex();
		}
		return true;
	}

	int CTreeListViewHeaderUI::GetCount() const
	{
		return m_childs.GetSize();
	}

	int CTreeListViewHeaderUI::FindSelectable(int iIndex, bool bForward) const
	{
		if (GetCount() == 0) return -1;
		iIndex = CLAMP(iIndex, 0, GetCount() - 1);
		if (bForward)
		{
			for (int i = iIndex; i < GetCount(); i++)
			{
				if (GetItemAt(i)->GetInterface(DUI_CTR_LISTITEM) != NULL
					&& GetItemAt(i)->IsVisible()
					&& GetItemAt(i)->IsEnabled()) return i;
			}
			return -1;
		}
		else {
			for (int i = iIndex; i >= 0; --i) {
				if (GetItemAt(i)->GetInterface(DUI_CTR_LISTITEM) != NULL
					&& GetItemAt(i)->IsVisible()
					&& GetItemAt(i)->IsEnabled()) return i;
			}
			return FindSelectable(0, true);
		}
	}

	bool CTreeListViewHeaderUI::OnItemActive(void* event)
	{
		return false;
	}
	
	void CTreeListViewHeaderUI::DrawItemText(HDC hDC, const RECT& rcItem, bool bParent)
	{

	}

	void CTreeListViewHeaderUI::DrawItemBk(HDC hDC, const RECT& rcItem)
	{
		ASSERT(m_pOwner);
		if (m_pOwner == NULL) return;

		DWORD iBackColor = 0;
		if ((m_uButtonState & UISTATE_HOT) != 0) {
			iBackColor = m_dwHotBkcolor;
		}

		if (IsSelected()) {
			iBackColor = m_dwSelectedBkcolor;
		}
		if (!IsEnabled()) {
			iBackColor = m_dwBkcolor;
		}
		if (iBackColor != 0) {
			CRenderEngine::DrawColor(hDC, m_rcItem, GetAdjustColor(iBackColor));
		}

		if (!IsEnabled()) {
			if (!m_sDisabledImage.IsEmpty()) {
				if (!DrawImage(hDC, (LPCTSTR)m_sDisabledImage)) m_sDisabledImage.Empty();
				else return;
			}
		}

		if (IsExpanded()) {
			if (!m_sSelectedImage.IsEmpty()) {
				if (!DrawImage(hDC, (LPCTSTR)m_sSelectedImage)) m_sSelectedImage.Empty();
				else return;
			}
		}

		if (!m_sBkImage.IsEmpty()) {
			if (!DrawImage(hDC, (LPCTSTR)m_sBkImage)) m_sBkImage.Empty();
			else return;
		}

		if ((m_uButtonState & UISTATE_HOT) != 0) {
			if (!m_sHotImage.IsEmpty()) {
				if (!DrawImage(hDC, (LPCTSTR)m_sHotImage)) m_sHotImage.Empty();
				else return;
			}
		}
	}

	void CTreeListViewHeaderUI::DoEvent(TEventUI& event)
	{
		if (!IsMouseEnabled() && event.Type > UIEVENT__MOUSEBEGIN && event.Type < UIEVENT__MOUSEEND)
		{
			if (m_pOwner != NULL) m_pOwner->DoEvent(event);
			CListContainerElementUI::DoEvent(event);
			return;
		}
		else if (event.Type == UIEVENT_KEYDOWN && IsEnabled())
		{
			if (event.chKey == VK_RETURN)
			{
				Activate();
				return;
			}
		}
		else if (event.Type == UIEVENT_BUTTONUP && IsEnabled())
		{
			Activate();
			return;
		}
		else if (event.Type == UIEVENT_MOUSEENTER)
		{
			if (IsEnabled())
			{
				m_uButtonState |= UISTATE_HOT;
				Invalidate();
			}
			return;
		}
		else if (event.Type == UIEVENT_MOUSELEAVE)
		{
			if (IsEnabled())
			{
				m_uButtonState &= ~UISTATE_HOT;
				Invalidate();
			}
			return;
		}
		else if (event.Type == UIEVENT_CONTEXTMENU)
		{
			if (IsContextMenuUsed()) {
				m_pManager->SendNotify(this, DUI_MSGTYPE_MENU, event.wParam, event.lParam);
				return;
			}
		}	

		if (m_pOwner != NULL)
			m_pOwner->DoEvent(event);
		else
			CListContainerElementUI::DoEvent(event);
	}

	void CTreeListViewHeaderUI::SetAttribute(LPCTSTR pstrName, LPCTSTR pstrValue)
	{
		if (_tcscmp(pstrName, _T("hlayoutattrib")) == 0)
		{
			m_pHorizontallayout->ApplyAttributeList(pstrValue);
		}
		else if (_tcscmp(pstrName, _T("expandattrib")) == 0)
		{
			CDuiString strvalue(pstrValue);
			strvalue.Replace(_T("&rarr;"), _T("="));
			strvalue.Replace(_T("&sub;"), _T("\""));
			m_pExpandOption->ApplyAttributeList(strvalue);
		}
		else if (_tcscmp(pstrName, _T("iconattrib")) == 0)
		{
			CDuiString strvalue(pstrValue);
			strvalue.Replace(_T("&rarr;"), _T("="));
			strvalue.Replace(_T("&sub;"), _T("\""));
			m_pIconLabel->ApplyAttributeList(strvalue);
		}
		else if (_tcscmp(pstrName, _T("titleattrib")) == 0)
		{
			CDuiString strvalue(pstrValue);
			strvalue.Replace(_T("&rarr;"), _T("="));
			strvalue.Replace(_T("&sub;"), _T("\""));
			m_pTitleText->ApplyAttributeList(strvalue);
		}
		else if (_tcscmp(pstrName, _T("exbuttonattrib")) == 0)
		{
			CDuiString strvalue(pstrValue);
			strvalue.Replace(_T("&rarr;"), _T("="));
			strvalue.Replace(_T("&sub;"), _T("\""));
			m_pFistEventBtn->ApplyAttributeList(strvalue);
		}
		else if (_tcscmp(pstrName, _T("buttonattrib")) == 0)
		{
			CDuiString strvalue(pstrValue);
			strvalue.Replace(_T("&rarr;"), _T("="));
			strvalue.Replace(_T("&sub;"), _T("\""));
			m_pEventBtn->ApplyAttributeList(strvalue);
		}		
		else if (_tcscmp(pstrName, _T("hotbkcolor")) == 0)
		{
			while (*pstrValue > _T('\0') && *pstrValue <= _T(' ')) pstrValue = ::CharNext(pstrValue);
			if (*pstrValue == _T('#')) pstrValue = ::CharNext(pstrValue);
			LPTSTR pstr = NULL;
			DWORD clrColor = _tcstoul(pstrValue, &pstr, 16);
			m_dwHotBkcolor = clrColor;
		}
		else if (_tcscmp(pstrName, _T("bkcolor")) == 0)
		{
			while (*pstrValue > _T('\0') && *pstrValue <= _T(' ')) pstrValue = ::CharNext(pstrValue);
			if (*pstrValue == _T('#')) pstrValue = ::CharNext(pstrValue);
			LPTSTR pstr = NULL;
			DWORD clrColor = _tcstoul(pstrValue, &pstr, 16);
			m_dwBkcolor = clrColor;
		}
		else if (_tcscmp(pstrName, _T("selectedbkcolor")) == 0)
		{
			while (*pstrValue > _T('\0') && *pstrValue <= _T(' ')) pstrValue = ::CharNext(pstrValue);
			if (*pstrValue == _T('#')) pstrValue = ::CharNext(pstrValue);
			LPTSTR pstr = NULL;
			DWORD clrColor = _tcstoul(pstrValue, &pstr, 16);
			m_dwSelectedBkcolor = clrColor;
		}
		else if (_tcscmp(pstrName, _T("bkimage")) == 0)
		{
			m_sBkImage = pstrValue;
		}
		else if (_tcscmp(pstrName, _T("hotbkimage")) == 0)
		{
			m_sHotImage = pstrValue;
		}
		else if (_tcscmp(pstrName, _T("selectedbkimage")) == 0)
		{
			m_sSelectedImage = pstrValue;
		}
		else if (_tcscmp(pstrName, _T("disablebkimage")) == 0)
		{
			m_sDisabledImage = pstrValue;
		}
		else
		{
			CListContainerElementUI::SetAttribute(pstrName, pstrValue);
		}
	}
} // namespace DuiLib
