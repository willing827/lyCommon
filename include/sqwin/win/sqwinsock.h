#pragma once
  

namespace snqu{ namespace net{

/**@class       SockInit 
  *@brief       初始化和清理winsock库 
*/  
class WinSockInit  
{  
public:  
    static WinSockInit& instance();

public:  
    /**@brief   返回是否加载成功 */  
    bool isloaded() const;  

    /**@brief   返回使用的版本 */  
	unsigned short useversion() const;

    /**@brief   初始化 
      *@param[in] version                   加载的版本 
    */  
    bool init(unsigned short version);

    /**@brief   初始化  
      *@param[in] major     主版本 
      *@param[in] minor     副版本 
    */  
    bool init(BYTE major = 2, BYTE minor = 2);

    /**@brief   清理 */ 
    void uninit();

private: 
    WinSockInit();
    ~WinSockInit();
    bool m_is_loaded;             // 是否成功加载了库  
	unsigned short m_use_version; // 使用的版本  
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


