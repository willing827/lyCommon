/*---------------------------------------------------------------------------*/
/*  sqformat.h                                                               */
/*                                                                           */
/*  History                                                                  */
/*      05/26/2017                                                           */
/*                                                                           */
/*  Author                                                                   */
/*       feng hao                                                            */
/*                                                                           */
/*  Copyright (C) 2015 by SNQU network technology Inc.                       */
/*  All rights reserved                                                      */
/*---------------------------------------------------------------------------*/
#ifndef SNQU_FORMAT_H
#define SNQU_FORMAT_H
#include <tuple>
#include <type_traits>
#include <string>
#include <cctype>
#include <mutex>


namespace snqu { namespace fmt {

namespace detail {

    inline void FormatArg(std::string& buf, int i) { buf += std::to_string(i); }
    inline void FormatArg(std::string& buf, unsigned int i) { buf += std::to_string(i); }
    inline void FormatArg(std::string& buf, unsigned long i) { buf += std::to_string(i); }
    inline void FormatArg(std::string& buf, double i) { buf += std::to_string(i); }
    inline void FormatArg(std::string& buf, int64_t i) { buf += std::to_string(i); }
    inline void FormatArg(std::string& buf, uint64_t i) { buf += std::to_string(i); }
    inline void FormatArg(std::string& buf, const char* p) { buf.append(p); }
    inline void FormatArg(std::string& buf, std::string& s) { buf += s; }

#ifndef DISABLE_FORMAT //toolset < 140����֧�ֱ��ģ�棬�붨������
	template<size_t k, typename Tuple>
	typename std::enable_if < (k == std::tuple_size<Tuple>::value)>::type
		inline GetArgByIndex(size_t, Tuple&, std::string&)
	{
		throw std::invalid_argument("arg index out of range");
	}

	template<size_t k = 0, typename Tuple>
	typename std::enable_if < (k < std::tuple_size<Tuple>::value)>::type
		inline GetArgByIndex(size_t index, Tuple& tp, std::string& p)
	{
		if (k == index) { FormatArg(p, std::get<k>(tp)); }
		else { GetArgByIndex<k + 1>(index, tp, p); }
	}

	inline int GetIndex(const char*& p)
	{
		char temp[3] = {};
		int i = 0;
		while (*p != '}'&&*p != '\0')
		{
			if (i >= 2)
				throw std::invalid_argument("index is out of range.");

			if (std::isdigit(*p))
			{
				//push digit
				temp[i++] = *p;
				char next = *(p + 1);
				if (std::isdigit(next))
				{
					temp[i++] = next;
					p += 2;
					continue;
				}

				//validate arg
				if (!std::isspace(next) && next != '}')
				{
					throw std::invalid_argument("invalid argument.");
				}
			}

			p++;
		}

		return i == 0 ? -1 : std::atoi(temp);
	}

	template<typename... Args>
	inline void format(std::string& ss, const char* src, Args... args)
	{
		auto tp = std::tuple<Args...>(args...);

		const char* p = src;
		const char* original = p;
		int len = strlen(src) + 1;
		int last = 0;
		while (true)
		{
			if (*p == '{')
			{
				//copy content befor {
				last = p - original;
				ss.append(original, last);

				//format args
				int index = GetIndex(p);
				if (index >= 0)
				{
					GetArgByIndex<0>(index, tp, ss);
				}

				//skip }
				original = p + 1;
			}
			else if (*p == '\0')
			{
				last = p - original;
				ss.append(original, last);
				break;
			}
			p++;
		}
	}
}

template<typename str, typename... Args>
inline std::string Format(const str& src, Args... args)
{
    return Format(src.c_str(), args...);
}

/********************************************
����:	snqu::fmt::Format
Ȩ��:	public 
����ֵ:	snqu::string
˵����	�滻��ռλ����{0}��ʾ�����ִ�0��ʼ������
		ʾ����"db err{0} errmsg{1}"
		����һЩ��Ҫ��%08ld��%.2f���ܵģ�����FormatEx
		ע���˺������ʺ�ƴ��json��ص����ݣ�����FormatEx
���� src: Դ��ʽ�ַ���
���� args:�滻�ı��
********************************************/
template<typename... Args>
inline std::string Format(const char* src, Args... args)
{
	using namespace detail;
	std::string ss;
	ss.reserve(2048);

	detail::format(ss, src, args...);
	
	return std::move(ss);
}
#else
}
#endif //DISABLE_FORMAT

}}

namespace snqu { namespace fmt {
/********************************************
����:	snqu::fmt::FormatEx
Ȩ��:	public
����ֵ:	snqu::string
˵����	printf��ʽ�ĸ�ʽ���������Լ��������ڴ�
�������һЩ��Ҫ��%08ld��%.2f���ܵ�����
���� src: Դ��ʽ�ַ���
���� ...: �滻�ı��
********************************************/
std::string FormatEx(const char* src, ...);
    
}
}

#endif //SNQU_FORMAT_H