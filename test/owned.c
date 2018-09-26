//
// Created by Kim on 26-09-2018.
//


#include "owned.h"
#include <stddef.h>

def_comp(ownership_t)
    def_field(ownership_t, entity, owner)
def_end

void use_ownership() {
    use_component();

    reg_comp(ownership_t);
};
