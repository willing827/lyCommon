/*---------------------------------------------------------------------------*/
/*  sqhardwarecode.h                                                         */
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
#pragma once
#include <string>

namespace snqu{

    // Ӳ�̱���
    std::string get_disk_code();

    // cpu����
    std::string get_cpu_code();

    // �������
    std::string get_baseboard_code();

    // ����Ӳ�� cpu �������ɵ�Ψһ��ʶ
    std::string get_machine_key();

    // ���㱾����hashֵ ����ip mac��������������
    std::string local_machine_hash();
}