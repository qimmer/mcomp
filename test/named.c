//
// Created by Kim on 26-09-2018.
//

#include "named.h"
#include <stddef.h>

def_comp(naming_t)
    def_field(naming_t, string, name)
def_end

void use_naming() {
    use_component();

    reg_comp(naming_t);
};
