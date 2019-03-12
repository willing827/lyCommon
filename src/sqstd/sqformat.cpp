#include <sqstd/sqformat.h>
#include <stdarg.h>

namespace snqu { namespace fmt {

std::string FormatEx(const char* src, ...)
{
    int final_n, n = ((int)strlen(src)) * 2;
    std::unique_ptr<char[]> formatted;
    va_list ap;
    while (1)
    {
        formatted.reset(new char[n]);
        va_start(ap, src);
        final_n = vsnprintf(&formatted[0], n, src, ap);
        va_end(ap);
        if (final_n < 0 || final_n >= n)
            n += abs(final_n - n + 1);
        else
            break;
    }
    if (final_n < n)
        formatted[final_n] = '\0';
    return std::string(formatted.get());
}

}
}