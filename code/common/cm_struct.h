//
// Created by carrot on 18-3-19.
//

#ifndef RSYNCTOOL_CM_STRUCT_H
#define RSYNCTOOL_CM_STRUCT_H

#include <strings.h>

struct ST_PackageHeader
{
    int op = 0;

    ST_PackageHeader(){reset();}

    void reset()
    { bzero(this, sizeof(*this)); }
};

#endif //RSYNCTOOL_CM_STRUCT_H
