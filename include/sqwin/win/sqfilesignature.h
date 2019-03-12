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
     @full_file_path	�ļ�����Ŀ¼
     @cert_chain_check	ǩ����֤���� �ӵ͵������� Ϊ���򲻼��֤���� ֻ����Ƿ�ǩ��
    */
    bool check_file_signature(const std::string& full_file_path, 
                              const std::vector<std::string>& cert_chain_check = std::vector<std::string>());
}
}


#endif//FILE_SIGNATURE_H