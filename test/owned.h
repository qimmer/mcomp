//
// Created by Kim on 26-09-2018.
//

#ifndef PLAZA_OWNED_H
#define PLAZA_OWNED_H

#include "../mcomp.h"

typedef struct {
    ref_t owner;
} owned_t;

extern component_t owned_cpt;

void use_owned();

#endif //PLAZA_OWNED_H
