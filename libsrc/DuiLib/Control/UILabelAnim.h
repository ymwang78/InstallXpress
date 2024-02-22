#ifndef __UILABELANIM_H__
#define __UILABELANIM_H__

namespace DuiLib
{
	class UILIB_API CLabelAnimUI : public CLabelUI
	{
	public:
		CLabelAnimUI(void);
		~CLabelAnimUI(void);

	public:
		LPCTSTR GetClass() const;
		LPVOID GetInterface(LPCTSTR pstrName);
		void SetAttribute(LPCTSTR pstrName, LPCTSTR pstrValue);		
		void PaintBkImage(HDC hDC);
		void PaintText(HDC hDC);
		void DoEvent(TEventUI& event);
		void DoInit();

	public:
		//设置移动步长;
		void SetMoveStep(int nlen);
		int GetMoveStep() const;

		//设置移动类型(0:水平,1:垂直);
		void SetMoveType(int ntype);
		int GetMoveType();		
		
		//设置播放时长;
		void SetPalyTimer(int nlen);
		
		//播放动画;
		void PlayAnim();

		//关闭动画;
		void ClosePlay();

		//设置自动播放;
		void SetAutoPlay(bool bpaly);
		bool GetAutoPlay();

		//是否回播;
		void SetPlayback(bool bplayback);
		bool GetPlayback();

		//播放终止类型(0：随机，1：居中，2：底部，3，顶部);
		void SetPlayMode(int nmode);
		int	GetPlayMode();

		void SetBkImageWidth(int nWidth);
		void SetBkImageHeight(int nHeight);

		void SetBkImageFmt(LPCTSTR pvalue);

	private:
		void OnTimer(UINT_PTR idEvent, TEventUI& event);

	private:
		CDuiString m_BkImageFmt;
		bool m_bdelaynotify;
		int m_nMovesteplen;
		int m_nMovetype;		//0:水平，1:垂直;
		int m_nPlayTimerLen;	//设置播放时长;
		bool m_bplay;
		bool m_bAutoPlay;		//自动播放;
		bool m_bPlayback;		//是否回播;
		int m_imgalign;			//图片对其方式;
		int m_nPlayingLen;		
		int m_nPlayMode;		//播放终止类型;
		int m_nbkImageWidth;	//背景图片宽度;
		int m_nbkImageHeight;	//背景图片高度;
		int m_nbkImageNum;		//背景数量;
		int m_nbkChangeNum;		//背景显示次数;
		int m_nchangecount;		//背景切换次数计算;
	};
}

#endif // __UILABEL_H__