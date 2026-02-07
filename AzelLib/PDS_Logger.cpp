#include "PDS.h"

#ifndef SHIPPING_BUILD
sPDS_Logger PDS_Logger[eLogCategories::log_max];

void cleanupLoggers()
{
    for (int i = 0; i < eLogCategories::log_max; i++)
    {
        PDS_Logger[i].Clear();
    }
}
#endif
