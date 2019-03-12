#pragma once

#include <exception>
#include <string>

namespace snqu{

    class CException: std::exception 
    {
    public:
        CException(int err, const std::string& msg="")
            : m_err(err), m_msg(msg)
        {
        }

        ~CException() throw(){}

        int getCode() const { return m_err; }
        const char* getMessage() const { return m_msg.c_str(); }

    protected:
        int m_err;
        std::string m_msg;
    };
}

