//
// Created by Kim on 26-09-2018.
//

#ifndef PLAZA_OWNED_H
#define PLAZA_OWNED_H

#include "../mcomp.h"

component(ownership_t, {
    entity_t owner;
})

void use_ownership();

#endif //PLAZA_OWNED_H
