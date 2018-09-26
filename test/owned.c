//
// Created by Kim on 26-09-2018.
//


#include "owned.h"
#include <stddef.h>

component_t owned_cpt;

static const field_t owned_fields[] = {
        { "Owner", t_ref, offsetof(owned_t, owner), member_size(owned_t, owner), false },
        fields_end
};

void use_owned() {
    use_component();

    component_register(&owned_cpt, "Owned", owned_fields, sizeof(owned_t));
};
