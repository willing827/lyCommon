/////////////////////////////////////////////////////////////////////////////// 
// FileName:    Firewall_util.h
// Created:     2016/11/03
// Author:      luhaijun 
//----------------------------------------------------------------------------- 
/////////////////////////////////////////////////////////////////////////////// 

#pragma once



namespace snqu { namespace os{

    //pe_file_name : App full pathname , fwName : Your App Name
    bool AddFireWallApp(const char* pe_file_name, const char* app_name);
    bool DelFireWallApp(const char* pe_file_name);

}}


