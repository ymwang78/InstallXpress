/********************************************************
*文件描叙:树形列表控件;
*文件名: UITreeListView.h  ;
*创建时间:2015-08-18;
*********************************************************/

#ifndef __UITREELISTVIEW_H__
#define __UITREELISTVIEW_H__

namespace DuiLib {
	/////////////////////////////////////////////////////////////////////////////////////
	//
	class UILIB_API CTreeListViewUI : public CListUI	//树形列表控件;
	{
	public:
		CTreeListViewUI();
		LPCTSTR GetClass() const;
		LPVOID GetInterface(LPCTSTR pstrName);

		bool Add(CControlUI* pControl);
		bool AddAt(CControlUI* pControl, int iIndex);
		bool Remove(CControlUI* pControl);
		bool RemoveAt(int iIndex);
		void RemoveAll();

		CControlUI* GetItemAt(int iIndex) const;
		int GetItemIndex(CControlUI* pControl) const;
		bool SetItemIndex(CControlUI* pControl, int iIndex, bool bupdate = true);
		int GetCount() const;
		bool SelectItem(int iIndex, bool bTakeFocus  = false, bool bsendmsg = false);
	protected:
		CStdPtrArray m_stdArrayChilds;
	};

	class CLabelIconUI;

	class UILIB_API CTreeListViewHeaderUI : public CListContainerElementUI, public IListOwnerUI
	{
	public:
		CTreeListViewHeaderUI();
		
	public:
		LPCTSTR GetClass() const;
		LPVOID	GetInterface(LPCTSTR pstrName);
		UINT GetControlFlags() const;

		void SetVisible(bool bVisible);

		IListOwnerUI* GetOwner();
		void SetOwner(CControlUI* pOwner);
		
		void SetText(LPCTSTR pstrText);
		CDuiString GetText();
		CLabelUI* GetTitleLabel();

		int GetIndex() const;
		void SetIndex(int iIndex);

		void SetIconImage(LPCTSTR strImage);
		CDuiString GetIconImage();

		void SetEventName(LPCTSTR strname);
		void SetEventUserData(LPCTSTR struserdate);
		
		void SetExButtonText(LPCTSTR strname);
		void SetExButtonName(LPCTSTR strname);
		void SetExButtonUserData(LPCTSTR struserdate);
		void SetExButtonEnabled(bool bEnabled);

		bool IsExpanded() const;
		bool Expand(bool bExpand = true);
		void SetExpandBkImage(LPCTSTR strname);
		void SetExpandSelectedImg(LPCTSTR strImage);
		void SetExpandEnabled(bool bEnabled);

		bool Activate();

		TListInfoUI* GetListInfo();
		int GetCurSel() const;
		bool SelectItem(int iIndex, bool bTakeFocus = false, bool bsendmsg=false);
		bool OnItemActive(void* event);

		bool Add(CControlUI* pControl);
		bool AddAt(CControlUI* pControl, int iIndex);
		bool Remove(CControlUI* pControl);
		bool RemoveAt(int iIndex);
		void RemoveAll();
		CControlUI* GetItemAt(int iIndex) const;
		int GetItemIndex(CControlUI* pControl) const;
		bool SetItemIndex(CControlUI* pControl, int iIndex, bool bupdate = true);
		int GetCount() const;
		int FindSelectable(int iIndex, bool bForward = true) const;

		void DoEvent(TEventUI& event);
		void DrawItemText(HDC hDC, const RECT& rcItem, bool bParent = false);
		void DrawItemBk(HDC hDC, const RECT& rcItem);
		void PaintGribLine(HDC hDC) {};

		void SetAttribute(LPCTSTR pstrName, LPCTSTR pstrValue);

		void SortChildItems(PULVCompareFunc pfnCompare, UINT_PTR dwData);

	protected:
		static int __cdecl ItemComareFunc(void *pvlocale, const void *item1, const void *item2);
		int __cdecl ItemComareFunc(const void *item1, const void *item2);

	private:
		PULVCompareFunc m_pCompareFunc;
		UINT_PTR m_compareData;

		int m_iIndex;
		int m_iCurSel;
		bool m_bSelected;
		bool m_bExpanded;

		DWORD m_dwHotBkcolor;
		DWORD m_dwBkcolor;
		DWORD m_dwSelectedBkcolor;
		CDuiString m_sDisabledImage;
		CDuiString m_sSelectedImage;
		CDuiString m_sHotImage;
		CDuiString m_sBkImage;

		CStdPtrArray m_childs;

		CDuiString m_stext;

		CHorizontalLayoutUI* m_pHorizontallayout; //布局;
		COptionUI* m_pExpandOption;               //扩展;
		CLabelIconUI* m_pIconLabel;				   //图标;
		CLabelUI*  m_pTitleText;				  //标题;
		CButtonUI* m_pFistEventBtn;				  //扩展按钮;
		CButtonUI* m_pEventBtn;					  //末尾图标;
	};
	
	class UILIB_API CTreeListContainerElementUI : public CListContainerElementUI
	{
	public:
		CTreeListContainerElementUI();

		LPCTSTR GetClass() const;
		UINT GetControlFlags() const;
		LPVOID GetInterface(LPCTSTR pstrName);
		void SetPos(RECT rc);
	};
} // namespace DuiLib

#endif // __UITREELISTVIEW_H__
