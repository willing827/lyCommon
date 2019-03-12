#ifndef _RAII_H_
#define _RAII_H_

#include <type_traits>
#include <functional>

namespace snqu{

/* Ԫģ�壬�����const������ȥ��const���η� */
// template<typename T>
// struct no_const 
// {
//     using type=typename std::conditional<std::is_const<T>::value,typename std::remove_const<T>::type,T>::type;
// };
/*
 * RAII��ʽ����������ͷ���Դ����
 * ���󴴽�ʱ,ִ��acquire(������Դ)����(����Ϊ�պ���[]{})
 * ��������ʱ,ִ��release(�ͷ���Դ)����
 * ��ֹ���󿽱��͸�ֵ
 */
class raii
{
public:
    typedef std::function<int()> fun_type;
    /* release: ����ʱִ�еĺ���
     * acquire: ���캯��ִ�еĺ���
     * default_com:_commit,Ĭ��ֵ,����ͨ��commit()������������
     */
    explicit raii(fun_type release, fun_type acquire = []()->int {}, bool default_com = true)
        : _commit(default_com), _release(release) 
            
    {
        acquire();
    }

    /* ��������ʱ����_commit��־ִ��_release���� */
    ~raii() 
    {
        if (_commit)
            _release();
    }
    /* �ƶ����캯�� ������ֵ��ֵ */
    raii(raii&& rv)
        : _commit(rv._commit),_release(rv._release)
    {
        rv._commit=false;
    };

    /* ����_commit��־ */
    raii& commit(bool c = true) 
    { 
        _commit = c; 
        return *this; 
    };

private:
    /* Ϊtrueʱ��������ִ��_release */
    bool _commit;

    /* ���ÿ������캯�� */
    raii(const raii&);

    /* ���ø�ֵ������ */
    raii& operator=(const raii&);
protected:  
    /* ����ʱִ���к��� */
    std::function<int()> _release;
}; /* raii */

} 

#endif//_RAII_H_
