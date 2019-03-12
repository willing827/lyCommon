#include "sqstd/sqconvert.h"

namespace snqu { namespace str {

    bool IsNum(const char* param)
    {
        auto pcur = param;
        auto len = strlen(param);
        if (len <= 0 || len >= 20)
            return false;

        bool has_dot = false;
        for (size_t i = 0; i < len; i++)
        {
            //负数
            if (0 == i && '-' == pcur[i])
                continue;

            if (!isdigit(pcur[i]))
            {
                if ('.' == pcur[i])
                {//小数
                    if (!has_dot)
                    {
                        has_dot = true;
                        continue;
                    }
                    else
                        return false;
                }
                return false;
            }
        }
        return true;
    }

    short ToShort(const char* str_val)
    {
#undef min
#undef max
        try
        {
            auto ret = std::atoi(str_val);
            if (ret < std::numeric_limits<short>::min() ||
                ret > std::numeric_limits<short>::max())
            {
                return 0;
            }
            return unsigned short(ret);
        }
        catch (std::exception& e)
        {}
        return 0;
    }

    unsigned short ToUShort(const char* str_val)
    {
#undef min
#undef max
        try
        {
            auto ret = std::atoi(str_val);
            if (ret < std::numeric_limits<unsigned short>::min() ||
                ret > std::numeric_limits<unsigned short>::max())
            {
                return 0;
            }
            return short(ret);
        }
        catch (std::exception& e)
        {}
        return 0;
    }

}}