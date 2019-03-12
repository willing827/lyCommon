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

    // 硬盘编码
    std::string get_disk_code();

    // cpu编码
    std::string get_cpu_code();

    // 主板编码
    std::string get_baseboard_code();

    // 根据硬盘 cpu 主板生成的唯一标识
    std::string get_machine_key();

    // 计算本机的hash值 根据ip mac（电脑名）生成
    std::string local_machine_hash();
}