#pragma once
#include "SRadioBox2.h"
namespace SOUI
{

class SRadioEx: public SRadioBox2
{
    SOUI_CLASS_NAME(SRadioEx, L"radioex")
    
public:
    SRadioEx();
    ~SRadioEx();
    
    virtual void OnStateChanged(DWORD dwOldState, DWORD dwNewState);
    
};

}