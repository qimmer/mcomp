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

bool ref_valid(ref_t ref) {
    if(!ref.component || ref.component->magic != magic_number) {
        return false;
    }

    u32 cnt = vec_count((void**)&ref.component->manager.generations);

    return cnt > ref.index && ref.component->manager.generations[ref.index] == ref.gen;
}

void component_register(component_t *cpt, const char *name, const field_t *fields, u32 size) {
    if(cpt->magic == magic_number) {
        return;
    }

    memset(cpt, 0, sizeof(component_t));

    cpt->magic = magic_number;
    cpt->size = size;

    strncpy(cpt->name, name, sizeof(cpt->name));
    cpt->fields = fields;

    vec_push((void**)&components, cpt, sizeof(cpt));
}

bool component_get(ref_t ref, void *data) {
    if(!ref_valid(ref)) return false;

    memcpy(data, &ref.component->manager.instances[ref.index * ref.component->size], ref.component->size);

    return true;
}

bool comp_update(ref_t ref, const void *data) {
    if(!ref_valid(ref)) return false;

    memcpy(&ref.component->manager.instances[ref.index * ref.component->size], data, ref.component->size);

    return true;
}

void register_listener(component_t *cpt, listener_t func) {
    assert(cpt->magic == magic_number);
}

void unregister_listener(component_t *cpt, listener_t func) {
    assert(cpt->magic == magic_number);
}

ref_t comp_new(component_t *cpt) {
    assert(cpt->magic == magic_number);

    u32 idx = -1;
    if(!vec_pop((void**)&cpt->manager.free_indices, &idx)) {
        idx = vec_count(&cpt->manager.instances);
        vec_resize(&cpt->manager.instances, idx + 1, cpt->size);
        vec_resize(&cpt->manager.generations, idx + 1, sizeof(u32));
    }

    cpt->manager.generations[idx]++; // Odd generations are valid

    ref_t ref;
    ref.component = cpt;
    ref.gen = cpt->manager.generations[idx];
    ref.index = idx;

    return ref;
}

void comp_free(ref_t ref) {
    if(!ref_valid(ref)) return;

    vec_push(&ref.component->manager.free_indices, &ref.index, sizeof(u32));

    ref.component->manager.generations[ref.index]++; // Even generations are invalid

    // Zero out component memory
    memset(&ref.component->manager.instances[ref.index * ref.component->size], 0, ref.component->size);
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
};
