#pragma once
  

namespace snqu{ namespace net{

/**@class       SockInit 
  *@brief       ��ʼ��������winsock�� 
*/  
class WinSockInit  
{  
public:  
    static WinSockInit& instance();

public:  
    /**@brief   �����Ƿ���سɹ� */  
    bool isloaded() const;  

    /**@brief   ����ʹ�õİ汾 */  
	unsigned short useversion() const;

    /**@brief   ��ʼ�� 
      *@param[in] version                   ���صİ汾 
    */  
    bool init(unsigned short version);

    /**@brief   ��ʼ��  
      *@param[in] major     ���汾 
      *@param[in] minor     ���汾 
    */  
    bool init(BYTE major = 2, BYTE minor = 2);

    /**@brief   ���� */ 
    void uninit();

private: 
    WinSockInit();
    ~WinSockInit();
    bool m_is_loaded;             // �Ƿ�ɹ������˿�  
	unsigned short m_use_version; // ʹ�õİ汾  
    int err;

    WinSockInit(WinSockInit const &);
    WinSockInit& operator= (WinSockInit const &);
};  
  
inline bool WinSockInit::isloaded() const  
{  
    return m_is_loaded;  
}  
 
inline unsigned short WinSockInit::useversion() const  
{  
    return m_use_version;  
}

}}


