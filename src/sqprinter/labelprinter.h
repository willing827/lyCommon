#pragma once
#include "ReceiptPrinter.h"

namespace snqu{

class LabelPrinter : public ReceiptPrinter
{
private:
    virtual bool after_print() { return 0 != printer_dc_.EndPage(); }
};

}