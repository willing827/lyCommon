#pragma once
#include "Print.h"

namespace snqu{

class GridPrinter : public Printer
{
public:
    GridPrinter();
    ~GridPrinter();

    bool set_direct();  // …Ë÷√∫·œÚ¥Ú”°

private:
    struct impl;
    std::unique_ptr<impl> impl_;

    virtual bool before_print();
    virtual bool after_print();
    virtual bool print_job_unit(job_unit_t &job_unit);

};

}