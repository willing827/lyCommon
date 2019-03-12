/*---------------------------------------------------------------------------*/
/*  sqjsontosql.h                                                            */
/*                                                                           */
/*  History                                                                  */
/*      08/04/2017 create                                                    */
/*                                                                           */
/*  Author                                                                   */
/*      feng hao                                                             */
/*                                                                           */
/*  Copyright (C) 2015 by SNQU network technology Inc.                       */
/*  All rights reserved                                                      */
/*---------------------------------------------------------------------------*/
#ifndef SQDB_PB_SQL_H
#define SQDB_PB_SQL_H
#include <vector>
#include <memory>
#include <codec/pbjson.h>

namespace snqu {

    template <class MsgT>
    class PBDataSet : public std::vector<std::shared_ptr<MsgT>>
    {
        bool ParseJson(const std::string& jsonstr)
        {
            return pbjson::json2pb(jsonstr, *this);
        }

    };

}
#endif//SQDB_PB_SQL_H