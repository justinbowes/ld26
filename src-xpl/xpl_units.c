//
//  xpl_units.c
//  p1
//
//  Created by Justin Bowes on 2013-03-04.
//  Copyright (c) 2013 Informi Software Inc. All rights reserved.
//

#include <stdio.h>

#include "xpl_units.h"

static const char *metric_prefix(float q) {
    const char *smaller[] = { "m", "µ", "n", "p", "f", "a", "z", "y" };
    const char *larger[] = { "k", "M", "G", "T", "P", "E", "Z", "Y" };
    
    if (q < 0.001) {
        int i = 0;
        while (q < 0.001) {
            q *= 1000.0;
            i++;
        }
        return smaller[i];
    }
    
    if (q > 1000.0f) {
        int i = 0;
        while (q > 1000.0f) {
            q *= 0.001f;
            i++;
        }
        return larger[i];
    }
    
    return "";
}

static char format[64];
const char *xpl_metric_format(float qty, const char *unit) {
    snprintf(format, 64, "%.3f %s%s", qty, metric_prefix(qty), unit);
    return format;
}