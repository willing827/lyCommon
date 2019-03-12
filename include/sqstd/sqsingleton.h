#ifndef SQ_SINGLETON_H
#define SQ_SINGLETON_H

#include <mutex>

// 注意该单例模板不能在程序初始化之前就调用，否则可能会引起程序死锁!!
template<typename T>
class SQSingleton 
{
public:
	~SQSingleton()
	{
        Release();
	}

	static T& instance() 
    {
		std::call_once(initDataFlag, [&](){
			m_obj = new T();
		});
		return *m_obj;
	}

	static void Release() 
    {
        if (NULL != m_obj)
        {
            delete m_obj;
            m_obj = NULL;
        }
            
	}

private:
    static std::once_flag initDataFlag;
    static T* m_obj;
    SQSingleton() {}
    SQSingleton(const SQSingleton&);
    const SQSingleton& operator=(const SQSingleton&);
};

template<typename T>
std::once_flag SQSingleton<T>::initDataFlag;

template<typename T>
T* SQSingleton<T>::m_obj = NULL;


#endif // SQ_SINGLETON_H