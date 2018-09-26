//
// Created by Kim on 26-09-2018.
//

#include "entity.h"

#include <stddef.h>

component_t entity_cpt;

static const field_t entity_fields[] = {
    { "Ownership", component_type(owned_cpt), offsetof(entity_t, ownership), member_size(entity_t, ownership), false },
    { "Naming",    component_type(named_cpt), offsetof(entity_t, naming), member_size(entity_t, naming), false },
    fields_end
};

void use_entity() {
    use_owned();
    use_named();

    component_register(&entity_cpt, "Entity", entity_fields, sizeof(entity_t));
};
