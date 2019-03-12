#pragma once

// 该单例模板会在程序初始化之前就会调用，但是没有互斥可能会重复操作!!
template <typename T>
struct Singleton
{
	struct object_creator
	{
		object_creator()
		{ 
			Singleton<T>::instance(); 
		}

		inline void do_nothing()const 
		{}
	};

	static object_creator create_object;

public:
	typedef T object_type;
	static object_type& instance()
	{
        static object_type obj;
		create_object.do_nothing();
		return obj;
	}

private:
	// 日志模块有继承此类，这个暂时不处理
    //Singleton(const Singleton&);
    //const Singleton& operator=(const Singleton&);
};

template <typename T>
typename Singleton<T>::object_creator Singleton<T>::create_object;
