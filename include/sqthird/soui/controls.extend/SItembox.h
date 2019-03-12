/**
 * Copyright (C) 2014-2050 SOUI�Ŷ�
 * All rights reserved.
 * 
 * @file       SItembox.h
 * @brief      
 * @version    v1.0      
 * @author     soui      
 * @date       2014-07-03
 * 
 * Describe     
 */
#pragma once
#include <core/SPanel.h>

namespace SOUI
{

/**
 * @class      SItemBox
 * @brief      ItemBox
 * 
 * Describe    ItemBox
 */
class SItemBox
    : public SScrollView
{
    SOUI_CLASS_NAME(SItemBox, L"itembox")
public:
    /**
     * SItemBox::SItemBox
     * @brief    ���캯��
     *
     * Describe  ���캯��  
     */
    SItemBox();

    /**
     * SItemBox::~SItemBox
     * @brief    ��������
     *
     * Describe  ��������  
     */
    virtual ~SItemBox() {}
    
    /**
     * SItemBox::InsertItem
     * @brief    ��������
     * @param    LPCWSTR pszXml -- xml�����ļ�
     * @param    int iItem -- ����
     * @param    BOOL bEnsureVisible -- �Ƿ���ʾ
     * @return   ����SWindow
     *
     * Describe  ��������  
     */
    SWindow* InsertItem(LPCWSTR pszXml,int iItem=-1,BOOL bEnsureVisible=FALSE);

    /**
     * SItemBox::InsertItem
     * @brief    ��������
     * @param    LPCWSTR pszXml -- xml�����ļ�
     * @param    int iItem -- ����
     * @param    BOOL bEnsureVisible -- �Ƿ���ʾ
     * @return   ����SWindow
     *
     * Describe  ��������  
     */
    SWindow* InsertItem(pugi::xml_node xmlNode, int iItem=-1,BOOL bEnsureVisible=FALSE);

    /**
     * SItemBox::RemoveItem
     * @brief    ɾ����
     * @param    UINT iItem -- ����
     * @return   ����BOOL
     *     
     * Describe  ɾ����  
     */
    BOOL RemoveItem(UINT iItem);

    /**
     * SItemBox::RemoveItem
     * @brief    ɾ����
     * @param    SWindow * pChild -- ���ڽڵ�
     * @return   ����BOOL
     *
     * Describe  ɾ����  
     */
    BOOL RemoveItem(SWindow * pChild);

    /**
     * SItemBox::SetNewPosition
     * @brief    ��������
     * @param    SWindow * pChild -- �ڵ�
     * @param    DWORD nPos -- λ��
     * @param    BOOL bEnsureVisible -- �Ƿ���ʾ
     * @return   ����BOOL
     *     
     * Describe  ��������  
     */
    BOOL SetNewPosition(SWindow * pChild, DWORD nPos, BOOL bEnsureVisible = TRUE);

    /**
     * SItemBox::RemoveAllItems
     * @brief    ɾ������
     *
     * Describe  ɾ������  
     */
    void RemoveAllItems();

    /**
     * SItemBox::GetItemCount
     * @brief    ��ȡ�����
     * @return   UINT
     *
     * Describe  ��ȡ�����  
     */
    UINT GetItemCount();

    /**
     * SItemBox::PageUp
     * @brief    ��һҳ
     *
     * Describe  ��һҳ  
     */
    void PageUp();

    /**
     * SItemBox::PageDown
     * @brief    ��һҳ
     *
     * Describe  ��һҳ  
     */
    void PageDown();

    /**
     * SItemBox::EnsureVisible
     * @brief    ������ʾ
     * @param    SWindow *pItem  -- ĳ��ָ��
     *
     * Describe  ������ʾ  
     */
    void EnsureVisible(SWindow *pItem);

    /**
     * SItemBox::GetItemPos
     * @brief    ��ȡĳ�������
     * @return   ����int
     *
     * Describe  ��ȡĳ�������  
     */
    int GetItemPos(SWindow * lpCurItem);

protected:
    int m_nItemWid; /**< Item��� */
    int m_nItemHei; /**< Item�߶�*/
    int m_nSepWid;  /**< */
    int m_nSepHei;  /**< */

    /**
     * SItemBox::UpdateScroll
     * @brief    ���¹�����
     *
     * Describe  ���¹�����  
     */
    void UpdateScroll();

    /**
     * SItemBox::GetItemRect
     * @brief    ��ȡĳ��λ��
     * @param    int iItem -- ĳ������
     * @return   ����int
     *
     * Describe  ��ȡĳ�������  
     */
    
    CRect GetItemRect(int iItem);

    /**
     * SItemBox::BringWindowAfter
     * @brief    �����½ڵ�
     * @param    SWindow * pChild -- �½ڵ�
     * @param    SWindow * pInsertAfter -- λ�ýڵ�
     *
     * Describe  ��ĳ���ڵ������½ڵ�  
     */
    void BringWindowAfter(SWindow * pChild, SWindow * pInsertAfter);

    /**
     * SItemBox::OnSize
     * @brief    ��Ϣ��Ӧ����
     * @param    UINT nType --
     * @param    CSize size -- 
     *
     * Describe  ��ȡĳ�������  
     */
    void OnSize(UINT nType, CSize size);

    /**
     * SItemBox::UpdateChildrenPosition
     * @brief    �����ӽڵ�λ��
     *
     * Describe  �����ӽڵ�λ��  
     */
    virtual void UpdateChildrenPosition(){}//leave it empty

    /**
     * SItemBox::ReLayout
     * @brief    ���²���
     *
     * Describe  ���²���  
     */
    void ReLayout();

    /**
     * SItemBox::OnScroll
     * @brief    �����¼�
     * @param    BOOL bVertical -- �Ƿ�����ֱ
     * @param    UINT uCode -- ��Ϣ��
     * @param    int nPos -- λ��
     * @retur    ����int
     *
     * Describe  ��ȡĳ�������  
     */
    virtual BOOL OnScroll(BOOL bVertical,UINT uCode,int nPos);

    /**
     * SItemBox::GetScrollLineSize
     * @brief    ��ȡ��������С
     * @param    BOOL bVertical -- �Ƿ�����ֱ����
     * @retur    ����int
     *
     * Describe  ��ȡ��������С  
     */
    virtual int GetScrollLineSize(BOOL bVertical);

    /**
     * SItemBox::CreateChildren
     * @brief    ��������
     * @param    pugi::xml_node xmlNode
     * @return   ����BOOL
     *
     * Describe  ��ȡĳ�������  
     */
    virtual BOOL CreateChildren(pugi::xml_node xmlNode);

    SOUI_ATTRS_BEGIN()
        ATTR_INT(L"itemWidth", m_nItemWid, TRUE)
        ATTR_INT(L"itemHeight", m_nItemHei, TRUE)
        ATTR_INT(L"sepWidth", m_nSepWid, TRUE)
        ATTR_INT(L"sepHeight", m_nSepHei, TRUE)
    SOUI_ATTRS_END()

    SOUI_MSG_MAP_BEGIN()
        MSG_WM_SIZE(OnSize)
    SOUI_MSG_MAP_END()

};

}//namespace SOUI
