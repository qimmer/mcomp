//
// Created by Kim on 26-09-2018.
//

#include "../mcomp.h"
#include "entity.h"

#include <string.h>
#include <assert.h>
#include <stdio.h>

void entity_changed(const entity_t *o, const entity_t *n) {

}

void naming_changed(const named_t *o, const named_t *n) {

}

void print_component(component_t *cpt) {
    printf("%s: {\n", cpt->name);
    for(int i = 0; cpt->fields[i].size; ++i) {
        if(cpt->fields[i].type < t_MAX) {
            printf("    %s%s %s\n", type_names[cpt->fields[i].type], cpt->fields[i].is_array ? "[]" : "", cpt->fields[i].name);
        } else {
            printf("    %s%s %s\n", ((component_t*)cpt->fields[i].type)->name, cpt->fields[i].is_array ? "[]" : "", cpt->fields[i].name);
        }
    }
    printf("}\n");
}

int main() {
    component_error_proc = printf;

    use_entity();

    register_listener(&entity_cpt, (void*)entity_changed);
    register_listener(&named_cpt, (void*)naming_changed);

    ref_t ref = comp_new(&entity_cpt);

    entity_t entity, entity2;
    component_get(ref, &entity);

    strcpy(entity.naming.name, "Hello");

    comp_update(ref, &entity);

    component_get(ref, &entity2);

    assert(strcmp(entity2.naming.name, "Hello") == 0);


    print_component(&component_cpt);

    return 0;
}