#ifndef SQFMTERROR_H
#define SQFMTERROR_H

#include <sqstd/sqinc.h>
#include <sqstd/singleton.h>
#include <sqstd/sqstringhelper.h>
#include <map>
#include <sqthird/codec/sqcodec.h>

namespace snqu {
    class SQErrorMsg;
    #define sglSQErrorMsg SQSingleton<SQErrorMsg>::instance

    class SQErrorMsg
	{
	public:
		SQErrorMsg() {}
		~SQErrorMsg() 
		{
			m_error_map.clear();
		}

	public:
		inline void add_error_message(int32 code, std::string msg)
		{
			if (m_error_map.count(code))
				return;

			m_error_map[code] = msg;
		}

		inline std::string format_error_msg(int32 code)
		{
			std::string nomsg = "找不到指定的错误";
			auto iter = m_error_map.find(code);
			if (iter != m_error_map.end())
				return iter->second;

			return nomsg;
		}

	private:
		std::map<int32, std::string> m_error_map;
	};

	inline void push_error_message(int32 code, std::string msg)
	{
		sglSQErrorMsg().add_error_message(code, msg);
	}

	inline std::string format_error_message(int32 code)
	{
		return sglSQErrorMsg().format_error_msg(code);
	}

    inline std::string format_error_message_utf8(int32 code)
    {
        return snqu::codec::A2U(sglSQErrorMsg().format_error_msg(code));
    }
}

#define SQDECLARE_VARIABLE(type, value, shorttype, name, tn)     \
	namespace aL##shorttype {                                    \
	   static type SQERROR_##name = value;                       \
    }                                                            \
	using aL##shorttype::SQERROR_##name

#define SQINVOKE_FUNC2(shortname, arg1, arg2)                    \
	namespace nL##shortname {                                    \
        class cL##arg1 {                                         \
		public:                                                  \
			inline cL##arg1() {snqu::push_error_message(arg1, arg2);} \
			inline ~cL##arg1() {}                                \
		};                                                       \
		                                                         \
		static cL##arg1 __##arg1;                                \
    }

#define SQMAPPING_ERROR_MESSAGE(errsymbol, code, msg)	         \
	SQDECLARE_VARIABLE(int, code, code, errsymbol, int);         \
	SQINVOKE_FUNC2(code, code, msg)
    
#define SQFORMAT_ERROR_MESSAGE(code)                             \
	snqu::format_error_message(code)

#define SQFORMAT_UTF8_ERR_MSG(code)                             \
    snqu::format_error_message_utf8(code)

SQMAPPING_ERROR_MESSAGE(ERR, 0x10, "操作失败")
SQMAPPING_ERROR_MESSAGE(OK,  0x00, "操作成功")


#define  CHECK_RETURN(code) if (SQERROR_OK != code) return code
#define  CHECK_BREAK(code) if (SQERROR_OK != code) break
#define  CHECK_FUNC(func) { int funret = func; if (SQERROR_OK != funret) return funret;}

#endif //SQFMTERROR_H