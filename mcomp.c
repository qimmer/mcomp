//
// Created by Kim on 26-09-2018.
//

#include <memory.h>
#include <string.h>
#include <assert.h>
#include <stddef.h>
#include <malloc.h>
#include <memory.h>

#include "mcomp.h"

static const u64 magic_number = 0xdeadbeefdeadbeef;
static component_t **components = null;

error_proc component_error_proc = null;

static const int vec_alignment = 16;
static const int vec_min_alloc = 16;

static char large_null[4096];
static entity_t invalid_entity = {
    0,
    0
};

unsigned long upper_pow_two(unsigned long v)
{
    v--;
    v |= v >> 1;
    v |= v >> 2;
    v |= v >> 4;
    v |= v >> 8;
    v |= v >> 16;
    v++;
    return v;
}

int align(int number, int multiple)
{
    if (multiple == 0)
        return number;

    int remainder = abs(number) % multiple;
    if (remainder == 0)
        return number;

    if (number < 0)
        return -(abs(number) - remainder);
    else
        return number + multiple - remainder;
}

bool vec_resize(void **vec_data, unsigned int count, unsigned short elem_size) {
    // If the vector is gone and count is 0, everything is as it should be
    if(!*vec_data && !count) {
        return true;
    }

    if(!*vec_data) {
        // initialization of array
        unsigned int cap = upper_pow_two(max(count, vec_min_alloc));
        //elem_size = align(elem_size, vec_alignment);

        vec_t *vec = calloc(cap * elem_size + sizeof(vec_t), 1);
        *vec_data = &vec->data[0];
        vec->cap = cap;
        vec->count = count;
        vec->elem_size = elem_size;
    } else if(count == 0) {
        // free array on zero count
        vec_t *vec = &((vec_t*)*vec_data)[-1];
        free(vec);
        *vec_data = null;
    } else {
        // resize array
        vec_t *vec = &((vec_t*)*vec_data)[-1];

        if(elem_size != vec->elem_size) {
            // Element size has to be the same
            if(component_error_proc) component_error_proc("inconsistent element size: %d (original: %d)", elem_size, vec->elem_size);
            return false;
        }

        unsigned int new_cap = upper_pow_two(max(count, vec_min_alloc));

        if(new_cap <= vec->cap) {
            // capacity is still the same or smaller, just modify count and keep capacity to reduce re-allocations
            vec->count = count;
            return true;
        }

        vec = realloc(vec, new_cap * elem_size + sizeof(vec_t));

        // initialize new elements to null
        memset(&vec->data[vec->count * vec->elem_size], 0, (count - vec->count) * vec->elem_size);
        vec->count = count;
        vec->cap = new_cap;
        *vec_data = &vec->data[0];
    }

    return true;
}

unsigned int vec_push(void **vec_data, const void *data, unsigned short elem_size) {
    if(!*vec_data) {
        vec_resize(vec_data, 1, elem_size);
        return 0;
    }

    vec_t *vec = &((vec_t*)*vec_data)[-1];

    if(!vec_resize(vec_data, vec->count + 1, elem_size)) {
        if(component_error_proc) component_error_proc("could not add to vector: resize failed");
        return idx_invalid;
    }

    return vec->count - 1;
}

bool vec_remove(void **vec_data, unsigned int idx) {
    if(!*vec_data) {
        if(component_error_proc) component_error_proc("index out of range: %d", idx);
        return false;
    }

    vec_t *vec = &((vec_t*)*vec_data)[-1];

    // Move last element into slot of removed entity
    if(!vec_get(vec_data, vec->count - 1, &vec->data[vec->elem_size * idx])) {
        if(component_error_proc) component_error_proc("index out of range: %d", idx);
        return false;
    }

    return vec_resize(vec_data, vec->count - 1, vec->elem_size);
}

bool vec_pop(void **vec_data, void *data) {
    if(!*vec_data) {
        return false;
    }

    vec_t *vec = &((vec_t*)*vec_data)[-1];
    memcpy(data, &vec->data[vec->elem_size * (vec->count - 1)], vec->elem_size);

    return vec_resize(vec_data, vec->count - 1, vec->elem_size);
}

bool vec_get(void **vec_data, unsigned int idx, void *data) {
    if(!*vec_data) {
        return false;
    }

    vec_t *vec = &((vec_t*)*vec_data)[-1];
    if(idx >= vec->count) {
        if(component_error_proc) component_error_proc("index out of range: %d", idx);
        return false;
    }

    memcpy(data, &vec->data[vec->elem_size * idx], vec->elem_size);

    return 0;
}

bool vec_set(void **vec_data, unsigned int idx, const void *data) {
    if(!*vec_data) {
        return false;
    }

    vec_t *vec = &((vec_t*)*vec_data)[-1];
    if(idx >= vec->count) {
        if(component_error_proc) component_error_proc("index out of range: %d", idx);
        return false;
    }

    memcpy(&vec->data[vec->elem_size * idx], data, vec->elem_size);

    return true;
}

void *vec_at(void **vec_data, unsigned int idx) {
    if(!*vec_data) {
        if(component_error_proc) component_error_proc("index out of range: %d", idx);
        return false;
    }

    vec_t *vec = &((vec_t*)*vec_data)[-1];
    if(idx >= vec->count) {
        if(component_error_proc) component_error_proc("index out of range: %d", idx);
        return false;
    }

    return &vec->data[vec->count * vec->elem_size];
}

unsigned int vec_count(void **vec_data) {
    if(!*vec_data) {
        return 0;
    }

    vec_t *vec = &((vec_t*)*vec_data)[-1];
    return vec->count;
}

u32 *entity_free_slots = null;
u32 *entity_generations = null;

entity_t create() {
    entity_t result;

    if(vec_pop(&entity_free_slots, &result.index)) {
        result.gen = entity_generations[result.index];
    } else {
        u32 cnt = vec_count(&entity_generations);
        result.index = cnt;

        vec_resize(&entity_generations, cnt + 1, sizeof(result.gen));
    }

    entity_generations[result.index]++;
    result.gen = entity_generations[result.index];

    return result;
}

void destroy(entity_t entity) {
    if(!is_valid(entity)) return;

    u32 num_components = vec_count(component_t_cpt.manager.instances);
    component_t* components = (component_t*)component_t_cpt.manager.instances;

    for(u32 i = 0; i < num_components; ++i) {
        update(&components[i], entity, large_null);
    }

    entity_generations[entity.index]++;
    vec_push(&entity_free_slots, &entity.index, sizeof(entity.index));
}

bool is_valid(entity_t entity) {
    return entity.gen
        && entity.index < vec_count(&entity_generations)
        && entity_generations[entity.index] == entity.gen;
};

void component_register(component_t *cpt, const char *name, const field_t *fields, u32 size) {
    if(cpt->magic == magic_number) {
        return;
    }

    // Limit a single component to maximally occupy a whole memory page of 4kb
    assert(size <= 4096);

    // Just need to do this somewhere.
    memset(large_null, 0, sizeof(large_null));

    memset(cpt, 0, sizeof(component_t));

    cpt->magic = magic_number;
    cpt->size = size;

    strncpy(cpt->name, name, sizeof(cpt->name));
    cpt->fields = fields;

    vec_push((void**)&components, cpt, sizeof(cpt));
}

bool get(component_t *cpt, entity_t entity, void *data) {
    assert(cpt->magic == magic_number);

    if(vec_count(&cpt->manager.entity_to_component_index_map) <= entity.index
    || !is_valid(entity)) {
        memset(data, 0, cpt->size);
        return false;
    }

    u32 comp_idx = cpt->manager.entity_to_component_index_map[entity.index];
    bool has_component = comp_idx != idx_invalid;

    if(!has_component) {
        memset(data, 0, cpt->size);
        return false;
    }

    memcpy(data, &cpt->manager.instances[comp_idx * cpt->size], cpt->size);

    return true;
}

static u32 comp_alloc(component_t *cpt, entity_t entity) {
    u32 idx = 0;
    if(!vec_pop(&cpt->manager.free_indices, &idx)) {
        idx = vec_count(&cpt->manager.instances);
        vec_resize(&cpt->manager.instances, idx + 1, cpt->size);
    }

    if(vec_count(&cpt->manager.component_to_entity_map) <= idx) {
        vec_resize(&cpt->manager.component_to_entity_map, idx + 1, sizeof(entity_t));
    }

    cpt->manager.component_to_entity_map[idx] = entity;
    cpt->manager.entity_to_component_index_map[entity.index] = idx;

    return idx;
}

static void comp_free(component_t *cpt, u32 idx) {
    entity_t entity = cpt->manager.component_to_entity_map[idx];

    cpt->manager.component_to_entity_map[idx] = invalid_entity;
    cpt->manager.entity_to_component_index_map[entity.index] = idx_invalid;

    vec_push(&cpt->manager.free_indices, &idx, sizeof(u32));
}

bool update(component_t *cpt, entity_t entity, const void *data) {
    if(!is_valid(entity)) return false;

    bool is_null = memcmp(data, large_null, cpt->size) == 0;

    u32 cnt = vec_count(&cpt->manager.entity_to_component_index_map);
    if(cnt <= entity.index) {
        vec_resize(&cpt->manager.entity_to_component_index_map, entity.index + 1, sizeof(u32));
        memset(&cpt->manager.entity_to_component_index_map[cnt], 0xFF, sizeof(u32) * (entity.index + 1 - cnt));
    }

    u32 comp_idx = cpt->manager.entity_to_component_index_map[entity.index];
    bool has_component = comp_idx != idx_invalid;

    // If this entity has no data and data is null, everything is already as it should be
    if(is_null && !has_component) {
        return false;
    }

    // We should free the component data
    if(is_null && has_component) {
        u32 num_listeners = vec_count(&cpt->listeners);
        for(u32 i = 0; i < num_listeners; ++i) {
            cpt->listeners[i](
                &cpt->manager.instances[comp_idx * cpt->size],
                data
            );
        }
        comp_free(cpt, comp_idx);
        return false;
    }

    if(!is_null && !has_component) {
        comp_idx = comp_alloc(cpt, entity);
    }

    void *temp = alloca(cpt->size);
    memcpy(temp, &cpt->manager.instances[comp_idx * cpt->size], cpt->size);

    memcpy(&cpt->manager.instances[comp_idx * cpt->size], data, cpt->size);

    u32 num_listeners = vec_count(&cpt->listeners);
    for(u32 i = 0; i < num_listeners; ++i) {
        cpt->listeners[i](
            temp,
            data
        );
    }

    return true;
}

void register_listener(component_t *cpt, listener_t func) {
    assert(cpt->magic == magic_number);
}

void unregister_listener(component_t *cpt, listener_t func) {
    assert(cpt->magic == magic_number);
}

const component_t *get_components() {
    return *components;
}

def_comp(field_t)
  def_field(field_t, string, name)
  def_field(field_t, type, type)
  def_field(field_t, u16, size)
  def_field(field_t, u16, offset)
  def_field(field_t, bool, is_array)
def_end

def_comp(component_t)
    def_field(component_t, string, name)
    def_field(component_t, u32, size)
    def_children(component_t, field_t, fields)
def_end

void use_component() {
    reg_comp(field_t);
    reg_comp(component_t);
}

