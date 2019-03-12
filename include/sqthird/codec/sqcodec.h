#ifndef __CODEC_H__
#define __CODEC_H__
#include <string>

namespace snqu { namespace codec {
    
    std::string MD5(const std::string&);
    std::string MD5File(const std::string& full_path);

    std::string B64Encode(const std::string&);
    std::string B64Decode(const std::string&);
    
    std::string RC4Encode(const std::string& input, const std::string& key);

    unsigned long Adler32(const char* data, size_t len);
    inline unsigned long Adler32(const std::string& strData) { return Adler32(strData.c_str(), strData.length()); }

    unsigned long CRC32(const char* buf, int nLength);
    inline unsigned long CRC32(const std::string& strData) { return CRC32(strData.c_str(), strData.length()); }

    std::string URLEncode(const std::string&);
    std::string URLDecode(const std::string&);

    // utf8 -> ansi
    std::string U2A(const std::string&);
    // ansi -> utf8
    std::string A2U(const std::string&);

    // unicode -> ansi
    std::string W2S(const std::wstring&);
    // ansi -> unicode
    std::wstring S2W(const std::string&);

    // unicode -> UTF8
    std::string W2U(const std::wstring&);
    // UTF8 -> unicode
    std::wstring U2W(const std::string&);
    
    bool IsUTF8(const std::string&);
    bool IsUTF8(const char* str, int size);
}
}

#endif