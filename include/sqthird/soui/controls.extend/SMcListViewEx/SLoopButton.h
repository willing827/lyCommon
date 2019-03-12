#pragma once
#include "core\Swnd.h"

#define EVT_BTNSEL_CHANGING EVT_EXTERNAL_BEGIN+100
#define EVT_BTNSEL_CHANGED EVT_BTNSEL_CHANGING+1

class SOUI_EXP EventBtnSelChanging : public TplEventArgs<EventBtnSelChanging>
{
	SOUI_CLASS_NAME(EventTCSelChanging, L"on_btn_sel_changing")
public:
	EventBtnSelChanging(SObject *pSender) :TplEventArgs<EventBtnSelChanging>(pSender), bCancel(FALSE) {}
	enum {
		EventID = EVT_BTNSEL_CHANGING
	};
	int iOldSel;
	int iNewSel;
	BOOL bCancel;
};
class SOUI_EXP EventBtnSelChanged : public TplEventArgs<EventBtnSelChanged>
{
	SOUI_CLASS_NAME(EventTCSelChanging, L"on_btn_sel_changed")
public:
	EventBtnSelChanged(SObject *pSender) :TplEventArgs<EventBtnSelChanged>(pSender), bCancel(FALSE) {}
	enum {
		EventID = EVT_BTNSEL_CHANGED
	};
	int iOldSel;
	int iNewSel;
	BOOL bCancel;
};
//һ��
class SLoopButton :
	public SImageButton
{
	SOUI_CLASS_NAME(SLoopButton, L"loopbtn")
public:
	SLoopButton();
	~SLoopButton();

	SOUI_ATTRS_BEGIN()
		ATTR_INT(L"skinStates",m_iSkinStates,FALSE)
		ATTR_INT(L"curState",m_iCurState,FALSE)
		ATTR_INT(L"states",m_iState,FALSE)
	SOUI_ATTRS_END()

protected:
	virtual void OnStateChanged(DWORD dwOldState, DWORD dwNewState);
	virtual BOOL NeedRedrawWhenStateChange()
	{
		return m_iSkinStates>1?TRUE:FALSE;
	}
	void OnPaint(IRenderTarget * pRT);
	void OnLButtonUp(UINT nFlags, CPoint pt);
	void Next()
	{
		if (++m_iCurState>=m_iState)
			m_iCurState = 0;
	}
	int GetNext()
	{
		int iNext = m_iCurState;
		if (++iNext >= m_iState)
			return 0;
		return iNext;
	}
	int GetSkinState()
	{
		int state = IIF_STATE4(GetState(), 0, 1, 2, 3);
		state = min(m_iSkinStates - 1, state);
		return (m_iCurState*m_iSkinStates + state);
	}
	SOUI_MSG_MAP_BEGIN()
		MSG_WM_PAINT_EX(OnPaint)
		MSG_WM_LBUTTONDOWN(OnLButtonDown)
		MSG_WM_LBUTTONUP(OnLButtonUp)
	SOUI_MSG_MAP_END()
private:
	//��ǰ��ʾ��״̬,ע��״̬��0��ʼ���
	int m_iCurState;
	//�ܹ��м���״̬
	int m_iState;
	//��ͼ��״̬���������ܹ��ж��ٸ���ͼ������ÿ��״̬�µ���ͼ��
	int m_iSkinStates;
};

