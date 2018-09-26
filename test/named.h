//
// Created by Kim on 26-09-2018.
//

#ifndef PLAZA_NAMED_H
#define PLAZA_NAMED_H

#include "../mcomp.h"

typedef struct {
    char name[64];
} named_t;

extern component_t named_cpt;

void use_named();

#endif //PLAZA_NAMED_H
