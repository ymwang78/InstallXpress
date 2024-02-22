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
		//�����ƶ�����;
		void SetMoveStep(int nlen);
		int GetMoveStep() const;

		//�����ƶ�����(0:ˮƽ,1:��ֱ);
		void SetMoveType(int ntype);
		int GetMoveType();		
		
		//���ò���ʱ��;
		void SetPalyTimer(int nlen);
		
		//���Ŷ���;
		void PlayAnim();

		//�رն���;
		void ClosePlay();

		//�����Զ�����;
		void SetAutoPlay(bool bpaly);
		bool GetAutoPlay();

		//�Ƿ�ز�;
		void SetPlayback(bool bplayback);
		bool GetPlayback();

		//������ֹ����(0�������1�����У�2���ײ���3������);
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
		int m_nMovetype;		//0:ˮƽ��1:��ֱ;
		int m_nPlayTimerLen;	//���ò���ʱ��;
		bool m_bplay;
		bool m_bAutoPlay;		//�Զ�����;
		bool m_bPlayback;		//�Ƿ�ز�;
		int m_imgalign;			//ͼƬ���䷽ʽ;
		int m_nPlayingLen;		
		int m_nPlayMode;		//������ֹ����;
		int m_nbkImageWidth;	//����ͼƬ���;
		int m_nbkImageHeight;	//����ͼƬ�߶�;
		int m_nbkImageNum;		//��������;
		int m_nbkChangeNum;		//������ʾ����;
		int m_nchangecount;		//�����л���������;
	};
}

#endif // __UILABEL_H__