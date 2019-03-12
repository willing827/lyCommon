#pragma once

namespace SOUI{
//ʵ��һ��ֻ�����������Ƥ��
/*img format: 1-normal, 2-hover, 3-pushdown, 4-disable
1 2 3 4 //for thumb
1 2 3 4 //for rail
*/
class SSkinNewScrollbar : public SSkinScrollbar
{
    SOUI_CLASS_NAME(SSkinNewScrollbar, L"newScrollbar")

public:

    SSkinNewScrollbar():m_pImgVert(NULL),m_pImgHorz(NULL)
    {
        m_nStates=4;
    }

    ~SSkinNewScrollbar()
    {
        if(m_pImgVert) m_pImgVert->Release();
        if(m_pImgHorz) m_pImgHorz->Release();
    }

    //��֧����ʾ���¼�ͷ
    virtual BOOL HasArrow(){return FALSE;}
    virtual int GetIdealSize(){
        SASSERT(m_pImgHorz && m_pImgVert);
        return m_pImgVert->Width()/m_nStates;
    }
protected:
	virtual void OnColorize(COLORREF cr){

	}

    virtual void _Draw(IRenderTarget *pRT, LPCRECT prcDraw, DWORD dwState,BYTE byAlpha)
    {
        if(!m_pImgHorz || !m_pImgVert) return;
        int nSbCode=LOWORD(dwState);
        int nState=LOBYTE(HIWORD(dwState));
        BOOL bVertical=HIBYTE(HIWORD(dwState));

        CRect rcSour=GetPartRect(nSbCode,nState,bVertical);
        CRect rcMargin(0,0,0,0);
        if(bVertical)
        {
            rcMargin.top=m_nMargin,rcMargin.bottom=m_nMargin;
            pRT->DrawBitmap9Patch(prcDraw,m_pImgVert,&rcSour,&rcMargin,m_bTile?EM_TILE:EM_STRETCH,byAlpha);
        }
        else
        {
            rcMargin.left=m_nMargin,rcMargin.right=m_nMargin;
            pRT->DrawBitmap9Patch(prcDraw,m_pImgHorz,&rcSour,&rcMargin,m_bTile?EM_TILE:EM_STRETCH,byAlpha);
        }

    }


    //����Դָ��������ԭλͼ�ϵ�λ�á�
    virtual CRect GetPartRect(int nSbCode, int nState,BOOL bVertical)
    {
        CRect rc;
        if(
            nSbCode==SB_LINEDOWN 
            || nSbCode==SB_LINEUP 
            || nSbCode == SB_CORNOR 
            || nSbCode==SB_THUMBGRIPPER) 
            return rc;
        
        if(bVertical)
        {
            SASSERT(m_pImgVert);
            rc.right = m_pImgVert->Width()/m_nStates;
            rc.bottom = m_pImgVert->Height()/2;

            rc.OffsetRect(rc.Width()*nState,0);
            if(nSbCode == SB_PAGEUP || nSbCode == SB_PAGEDOWN)
            {
                rc.OffsetRect(0,rc.Height());
            }

        }else
        {
            SASSERT(m_pImgHorz);
            rc.bottom = m_pImgHorz->Height()/m_nStates;
            rc.right = m_pImgHorz->Width()/2;

            rc.OffsetRect(0,rc.Height()*nState);
            if(nSbCode == SB_PAGEUP || nSbCode == SB_PAGEDOWN)
            {
                rc.OffsetRect(rc.Width(),0);
            }

        }
        return rc;
    }

    IBitmap *m_pImgVert,        //��ֱ������ͼƬ,��ͼ��ֱ����
            *m_pImgHorz;        //ˮƽ������ͼƬ,��ͼˮƽ����

    SOUI_ATTRS_BEGIN()
        ATTR_IMAGE(L"srcVert", m_pImgVert,FALSE)    //skinObj���õ�ͼƬ�ļ�������uires.idx�е�name���ԡ�
        ATTR_IMAGE(L"srcHorz", m_pImgHorz,FALSE)    //skinObj���õ�ͼƬ�ļ�������uires.idx�е�name���ԡ�
    SOUI_ATTRS_END()
};

}