#ifndef _RAII_H_
#define _RAII_H_

#include <type_traits>
#include <functional>

namespace snqu{

/* 元模板，如果是const类型则去除const修饰符 */
// template<typename T>
// struct no_const 
// {
//     using type=typename std::conditional<std::is_const<T>::value,typename std::remove_const<T>::type,T>::type;
// };
/*
 * RAII方式管理申请和释放资源的类
 * 对象创建时,执行acquire(申请资源)动作(可以为空函数[]{})
 * 对象析构时,执行release(释放资源)动作
 * 禁止对象拷贝和赋值
 */
class raii
{
public:
    typedef std::function<int()> fun_type;
    /* release: 析构时执行的函数
     * acquire: 构造函数执行的函数
     * default_com:_commit,默认值,可以通过commit()函数重新设置
     */
    explicit raii(fun_type release, fun_type acquire = []()->int {}, bool default_com = true)
        : _commit(default_com), _release(release) 
            
    {
        acquire();
    }

    /* 对象析构时根据_commit标志执行_release函数 */
    ~raii() 
    {
        if (_commit)
            _release();
    }
    /* 移动构造函数 允许右值赋值 */
    raii(raii&& rv)
        : _commit(rv._commit),_release(rv._release)
    {
        rv._commit=false;
    };

    /* 设置_commit标志 */
    raii& commit(bool c = true) 
    { 
        _commit = c; 
        return *this; 
    };

private:
    /* 为true时析构函数执行_release */
    bool _commit;

    /* 禁用拷贝构造函数 */
    raii(const raii&);

    /* 禁用赋值操作符 */
    raii& operator=(const raii&);
protected:  
    /* 析构时执的行函数 */
    std::function<int()> _release;
}; /* raii */

} 

#endif//_RAII_H_
