//
// Created by Kim on 26-09-2018.
//

#include "named.h"
#include <stddef.h>

component_t named_cpt;

static const field_t named_fields[] = {
        { "Name", t_string, offsetof(named_t, name), member_size(named_t, name), false },
        fields_end
};

void use_named() {
    use_component();

    component_register(&named_cpt, "Named", named_fields, sizeof(named_t));
};
