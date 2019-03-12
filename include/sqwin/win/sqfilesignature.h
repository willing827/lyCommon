/*---------------------------------------------------------------------------*/
/*  sqfilesignature.h                                                        */
/*                                                                           */
/*  History                                                                  */
/*      05/26/2017  create                                                   */
/*                                                                           */
/*  Author                                                                   */
/*       feng hao                                                            */
/*                                                                           */
/*  Copyright (C) 2015 by SNQU network technology Inc.                       */
/*  All rights reserved                                                      */
/*---------------------------------------------------------------------------*/
#ifndef FILE_SIGNATURE_H
#define FILE_SIGNATURE_H
#include <string>
#include <vector>

namespace snqu { namespace os{
    /*
     @full_file_path	文件完整目录
     @cert_chain_check	签名者证书链 从低到高排列 为空则不检查证书链 只检测是否签名
    */
    bool check_file_signature(const std::string& full_file_path, 
                              const std::vector<std::string>& cert_chain_check = std::vector<std::string>());
}
}


#endif//FILE_SIGNATURE_H