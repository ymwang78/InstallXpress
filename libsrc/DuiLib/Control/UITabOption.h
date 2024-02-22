#ifndef __UITABOPTION_H__
#define __UITABOPTION_H__

namespace DuiLib
{
	class UILIB_API CTabOptionUI : public CButtonUI
	{
	public:
		CTabOptionUI();
		~CTabOptionUI();

		LPCTSTR GetClass() const;
		LPVOID GetInterface(LPCTSTR pstrName);

		void SetManager(CPaintManagerUI* pManager, CControlUI* pParent, bool bInit = true);

		virtual bool Activate();
		void SetEnabled(bool bEnable = true);

		LPCTSTR GetSelectedImage();
		void SetSelectedImage(LPCTSTR pStrImage);

		LPCTSTR GetSelectedHotImage();
		void SetSelectedHotImage(LPCTSTR pStrImage);

		LPCTSTR GetSelectedPushedImage();
		void SetSelectedPushedImage(LPCTSTR pStrImage);

		void SetSelectedTextColor(DWORD dwTextColor);
		DWORD GetSelectedTextColor();

		void SetSelectedBkColor(DWORD dwBkColor);
		DWORD GetSelectBkColor();

		LPCTSTR GetForeImage();
		void SetForeImage(LPCTSTR pStrImage);

		LPCTSTR GetSelectedForedImage();
		void SetSelectedForedImage(LPCTSTR pStrImage);

		LPCTSTR GetCloseBtnImage();
		void SetCloseBtnImage(LPCTSTR pStrImage);

		LPCTSTR GetCloseBtnHotImage();
		void SetCloseBtnHotImage(LPCTSTR pStrImage);

		LPCTSTR GetCloseBtnPushedImage();
		void SetCloseBtnPushedImage(LPCTSTR pStrImage);

		CDuiRect GetCloseBtnPositon();

		void SetForeImageAnim(bool bAnim);

		void SetForeImageAnimCount(int nAnim);

		void SetShowForeImage(bool bShow);

		LPCTSTR GetGroup() const;
		void SetGroup(LPCTSTR pStrGroupName = NULL);
		virtual bool IsSelected() const;
		virtual void Selected(bool bSelected);
		virtual void Selected(bool bSelected,bool bSendEenvet);

		SIZE EstimateSize(SIZE szAvailable);
		void SetAttribute(LPCTSTR pstrName, LPCTSTR pstrValue);
		void DoEvent(TEventUI& event);

		void PaintStatusImage(HDC hDC);
		void PaintText(HDC hDC);
		void SetPos(RECT rc);

	protected:
		UINT			m_uCloseButtonState;

		int				m_mAnimEsp;
		int				m_nCurAnimcount;
		int             m_iSelfont;
		int				m_nforimageAnim;	//前景图片动画播放次数;
		int				m_nclosebtnrightpadding;
		bool			m_bSelected;
		bool			m_bForAnimo;		//前景是否动画播放;
		bool			m_bShowForeImage;	//是否显示前景;
		CDuiString		m_sGroupName;

		DWORD			m_dwSelectedBkColor;
		DWORD			m_dwSelectedTextColor;

		CDuiString		m_sSelectedImage;
		CDuiString		m_sSelectedHotImage;
		CDuiString		m_sSelectedPushedImage;
		CDuiString		m_sSelectedForeImage;
		CDuiString		m_sForeImage;

		CDuiString		m_sCloseBtnImage;
		CDuiString		m_sCloseBtnHotImage;
		CDuiString		m_sCloseBtnPushendImage;
		CDuiRect		m_rcCloseRect;
		RECT			m_rcCloseEvent;
		CDuiRect		m_rcForeDestRect;
		CDuiRect		m_rcSelTextPadding;
		CDuiString		m_sDrawText;
	};

} // namespace DuiLib

#endif // __UIOPTION_H__