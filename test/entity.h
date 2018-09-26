//
// Created by Kim on 26-09-2018.
//

#ifndef PLAZA_ENTITY_H
#define PLAZA_ENTITY_H

#include "named.h"
#include "owned.h"

typedef struct {
    owned_t ownership;
    named_t naming;
} entity_t;

extern component_t entity_cpt;

void use_entity();

#endif //PLAZA_ENTITY_H
