//
// Created by Kim on 26-09-2018.
//

#include "../mcomp.h"
#include "named.h"
#include "owned.h"

#include <string.h>
#include <assert.h>
#include <stdio.h>

void naming_changed(const naming_t *o, const naming_t *n) {

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

    use_naming();
    use_ownership();

    register_listener(&naming_t_cpt, naming_changed);

    entity_t ref = create();

    naming_t data;
    get_naming_t(ref, &data);

    strcpy(data.name, "Hello");

    update_naming_t(ref, &data);

    get_naming_t(ref, &data);

    assert(strcmp(data.name, "Hello") == 0);


    print_component(&component_t_cpt);

    return 0;
}