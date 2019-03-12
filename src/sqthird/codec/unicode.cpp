#include "unicode.h"
#include <sqwin/sqwin.h>

namespace snqu { namespace codec {

    bool is_UTF8(const char* str, int size)
    {
        int i;
        DWORD nBytes = 0;//UFT8可用1-6个字节编码,ASCII用一个字节
        UCHAR chr;
        bool bAllAscii = true; //如果全部都是ASCII, 说明不是UTF-8
        int length = size;

        for (i = 0; i < length; i++)
        {
            chr = *(str + i);
            if ((chr & 0x80) != 0) // 判断是否ASCII编码,如果不是,说明有可能是UTF-8,ASCII用7位编码,但用一个字节存,最高位标记为0,o0xxxxxxx
            {
                bAllAscii = false;
            }

            if (nBytes == 0) //如果不是ASCII码,应该是多字节符,计算字节数
            {
                if (chr >= 0x80)
                {
                    if (chr >= 0xFC && chr <= 0xFD)
                        nBytes = 6;
                    else if (chr >= 0xF8)
                        nBytes = 5;
                    else if (chr >= 0xF0)
                        nBytes = 4;
                    else if (chr >= 0xE0)
                        nBytes = 3;
                    else if (chr >= 0xC0)
                        nBytes = 2;
                    else
                    {
                        return false;
                    }
                    nBytes--;
                }
            }
            else //多字节符的非首字节,应为 10xxxxxx
            {
                if ((chr & 0xC0) != 0x80)
                {
                    return false;
                }
                nBytes--;
            }
        }
        if (nBytes > 0) //违返规则
        {
            return false;
        }
        if (bAllAscii) //如果全部都是ASCII, 说明不是UTF-8
        {
            return false;
        }

        return true;
    }

    std::string convert_utf8_to_ansi(const std::string& utf8)
    {
        int     nRel = 0;
        LPWSTR  lpwszStr = NULL;
        LPSTR   lpszAnsiString = NULL;
        std::string  ansi = "";

        if (!is_UTF8(utf8.c_str(), utf8.length()))
            return utf8;

        nRel = MultiByteToWideChar(
            CP_UTF8,
            0,
            utf8.c_str(),
            utf8.size(),
            NULL,
            0
        );

        lpwszStr = (LPWSTR)LocalAlloc(LPTR, (nRel * sizeof(WCHAR)) + 2);
        if (!lpwszStr)
        {
            return ansi;
        }

        MultiByteToWideChar(
            CP_UTF8,
            0,
            utf8.c_str(),
            utf8.size(),
            lpwszStr,
            nRel
        );

        nRel = WideCharToMultiByte(CP_ACP,
            0,
            lpwszStr,
            -1,
            NULL,
            0,
            NULL,
            NULL
        );

        lpszAnsiString = (LPSTR)LocalAlloc(LPTR, nRel + 1);
        if (lpszAnsiString != NULL)
        {
            nRel = WideCharToMultiByte(CP_ACP,
                0,
                lpwszStr,
                -1,
                lpszAnsiString,
                nRel,
                NULL,
                NULL);
            ansi = lpszAnsiString;
        }

        if (lpwszStr) LocalFree(lpwszStr);
        if (lpszAnsiString) LocalFree(lpszAnsiString);
        return ansi;
    }


    std::string convert_ansi_to_utf8(const std::string& ansi)
    {
        int     nRel = 0;
        LPWSTR  lpwszStr = NULL;
        LPSTR   lpszUtf8 = NULL;
        string  utf8_str;
        int utf8_size = 0;

        nRel = MultiByteToWideChar(CP_ACP, 0, ansi.c_str(), -1, NULL, 0);
        lpwszStr = (LPWSTR)LocalAlloc(LPTR, (nRel * sizeof(WCHAR)) + 2);
        if (!lpwszStr)
        {
            return 0;
        }

        nRel = MultiByteToWideChar(CP_ACP, 0, ansi.c_str(), -1, lpwszStr, nRel);

        nRel = WideCharToMultiByte(CP_UTF8,
            0,
            lpwszStr,
            -1,
            NULL,
            0,
            NULL,
            NULL);

        lpszUtf8 = (LPSTR)LocalAlloc(LPTR, nRel + 1);
        if (lpszUtf8 != NULL)
        {
            nRel = WideCharToMultiByte(CP_UTF8,
                0,
                lpwszStr,
                -1,
                lpszUtf8,
                nRel,
                NULL,
                NULL);

            utf8_str = lpszUtf8;
            utf8_size = nRel;
        }

        if (lpwszStr) LocalFree(lpwszStr);
        if (lpszUtf8) LocalFree(lpszUtf8);
        return utf8_str;
    }

    std::wstring convert_ansi_to_unicode(const std::string& ansi)
    {
        std::wstring unicode = L"";
        wchar_t *wszstr = NULL;
        size_t size = ansi.size();
        if (size > 0)
        {
            wszstr = (wchar_t *)LocalAlloc(LPTR, sizeof(wchar_t)*(size + 1));
            if (wszstr != NULL)
            {
                auto bRel = (BOOL)MultiByteToWideChar(
                    CP_ACP,
                    0,
                    (LPSTR)ansi.c_str(),
                    -1,
                    wszstr,
                    size
                );
                unicode = wszstr;

                LocalFree((HLOCAL)wszstr);
            }
        }

        return unicode;
    }

    std::string convert_unicode_to_ansi(const std::wstring& unicode)
    {
        std::string ansistring = "";
        char *pstr = NULL;
        size_t size = wcslen((wchar_t*)unicode.c_str());
        if (size > 0)
        {
            size = sizeof(wchar_t)*(size + 1);
            pstr = (char *)LocalAlloc(LPTR, size);
            if (pstr != NULL)
            {
                auto bRel = (BOOL)WideCharToMultiByte(
                    CP_ACP,
                    0,
                    (LPWSTR)unicode.c_str(),
                    -1,
                    pstr,
                    size,
                    NULL,
                    NULL
                );
                ansistring = pstr;

                LocalFree((HLOCAL)pstr);
            }
        }

        return ansistring;
    }

    std::string convert_unicode_to_utf8(const std::wstring& unicode)
    {
        std::string utf8string = "";
        size_t size = unicode.size();
        if (size > 0)
        {
            char	*pBuf = NULL;
            int		nLen = WideCharToMultiByte(CP_UTF8, 0, unicode.c_str(), -1, NULL, 0, NULL, NULL);
            pBuf = (char*)LocalAlloc(LPTR, (nLen + 1) * 2);
            nLen = WideCharToMultiByte(CP_UTF8, 0, unicode.c_str(), -1, pBuf, nLen, NULL, NULL);
            pBuf[nLen] = 0;
            utf8string = pBuf;
            LocalFree((HLOCAL)pBuf);
        }
        return utf8string;
    }

    std::wstring convert_utf8_to_unicode(const std::string& utf8)
    {
        std::wstring unicodestring = L"";

        size_t size = utf8.size();
        if (size > 0)
        {
            wchar_t	*pBuf = NULL;

            int		nLen = MultiByteToWideChar(CP_UTF8, 0, utf8.c_str(), -1, NULL, 0);

            pBuf = (wchar_t*)LocalAlloc(LPTR, (nLen + 1) * 2);
            nLen = MultiByteToWideChar(CP_UTF8, 0, utf8.c_str(), -1, pBuf, nLen);
            pBuf[nLen] = 0;
            unicodestring = pBuf;
            LocalFree((HLOCAL)pBuf);
        }

        return unicodestring;
    }

    static unsigned char		utf8_bom[3] = { 0x0ef, 0x0bb, 0x0bf };
    static unsigned char		unic_bom[2] = { 0x0ff, 0x0fe };

    bool is_unicode(const char* bom_str)
    {
        return memcmp(bom_str, unic_bom, sizeof(unic_bom)) == 0;
    }
    bool is_utf8(const char* bom_str)
    {
        return memcmp(bom_str, utf8_bom, sizeof(utf8_bom)) == 0;
    }
    bool is_ansi(const char* bom_str)
    {
        return !is_unicode(bom_str) && !is_utf8(bom_str);
    }

    std::string bom_str_to_ansi(const char* bom_str, size_t len)
    {
        std::string	ret(bom_str, len);

        if (len > 3)
        {
            if (is_unicode(bom_str))	ret = convert_unicode_to_ansi((const wchar_t*)&bom_str[2]);
            else if (is_utf8(bom_str))	ret = convert_utf8_to_ansi(&bom_str[3]);
        }

        return ret;
    }
    std::string bom_str_to_utf8(const char* bom_str, size_t len)
    {
        std::string	ret(convert_ansi_to_utf8(bom_str));

        if (len > 3)
        {
            if (is_unicode(bom_str))	ret = convert_unicode_to_utf8((const wchar_t*)&bom_str[2]);
            else if (is_utf8(bom_str))	ret = &bom_str[3];
        }

        return ret;
    }
    std::wstring bom_str_to_unicode(const char* bom_str, size_t len)
    {
        std::wstring	ret(convert_ansi_to_unicode(bom_str));

        if (len > 3)
        {
            if (is_unicode(bom_str))	ret = (const wchar_t*)&bom_str[2];
            else if (is_utf8(bom_str))	ret = convert_utf8_to_unicode(&bom_str[3]);
        }

        return ret;
    }

    std::string bom_str_from_ansi(const char* ansi_str)
    {
        return ansi_str;
    }
    std::string bom_str_from_utf8(const char* utf8_str)
    {
        std::string	str((char*)utf8_bom, sizeof(utf8_bom));

        str += utf8_str;

        return str;
    }
    std::string bom_str_from_unicode(const wchar_t* unic_str)
    {
        std::string	str((char*)unic_bom, sizeof(unic_bom));

        str += std::string((const char*)unic_str, lstrlenW(unic_str) * 2);

        return str;
    }
}
}