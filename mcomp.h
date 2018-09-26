//
// Created by Kim on 26-09-2018.
//

#ifndef PLAZA_COMPONENT_H
#define PLAZA_COMPONENT_H

#include <stddef.h>

typedef int(*error_proc)(const char *format, ...);
extern error_proc component_error_proc;

#define max(x, y) (((x) > (y)) ? (x) : (y))
#define min(x, y) (((x) < (y)) ? (x) : (y))
#define abs(x) (((x) > 0) ? x : -x)

typedef unsigned char u8;
typedef unsigned short u16;
typedef unsigned int u32;
typedef unsigned long long u64;
typedef signed char s8;
typedef signed short s16;
typedef signed int s32;
typedef signed long long s64;
typedef char bool;

typedef struct {
    float x, y;
} v2f;

typedef struct {
    float x, y, z;
} v3f;

typedef struct {
    int x, y;
} v2i;

typedef struct {
    int x, y, z;
} v3i;

#define true 1
#define false 0
#define null 0

typedef enum {
    t_none = 0,
    t_u8,
    t_u16,
    t_u32,
    t_u64,
    t_s8,
    t_s16,
    t_s32,
    t_s64,
    t_float,
    t_double,
    t_bool,
    t_entity,
    t_type,
    t_string,
    t_v2f,
    t_v3f,
    t_v2i,
    t_v3i,
    t_MAX
} pod_type_t;

typedef u64 type_t;

static const char * type_names[] = {
    "none",
    "u8",
    "u16",
    "u32",
    "u64",
    "s8",
    "s16",
    "s32",
    "s64",
    "float",
    "double",
    "bool",
    "entity",
    "type",
    "string",
    "v2f",
    "v3f",
    "v2i",
    "v3i",
};

static const u16 type_sizes[] = {
    0,
    sizeof(u8),
    sizeof(u16),
    sizeof(u32),
    sizeof(u64),
    sizeof(s8),
    sizeof(s16),
    sizeof(s32),
    sizeof(s64),
    sizeof(float),
    sizeof(double),
    sizeof(bool),
    sizeof(u64),
    sizeof(type_t),
    sizeof(const char*),
    sizeof(v2f),
    sizeof(v3f),
    sizeof(v2i),
    sizeof(v3i)
};

#define idx_invalid 0xFFFFFFFF

///
/// Simple Array Vector
///

typedef struct {
    unsigned short elem_size;
    unsigned int count;
    unsigned int cap;
    char data[0];
} vec_t;

bool vec_resize(void **vec_data, unsigned int count, unsigned short elem_size);

unsigned int vec_count(void **vec_data);

unsigned int vec_push(void **vec_data, const void *data, unsigned short elem_size);
bool vec_pop(void **vec_data, void *data);
bool vec_remove(void **vec_data, unsigned int idx);

bool vec_get(void **vec_data, unsigned int idx, void *data);
bool vec_set(void **vec_data, unsigned int idx, const void *data);
void *vec_at(void **vec_data, unsigned int idx);

///
/// Entity
///

typedef struct {
    u32 index, gen;
} entity_t;

///
/// \return A fresh new entity handle
entity_t create();

///
/// \param entity Entity to destroy and nullify
void destroy(entity_t entity);

///
/// \param entity The entity to check validity on
/// \return True if the entity is currently in a created state
bool is_valid(entity_t entity);

///
/// Component
///

typedef void(*listener_t)(const void *, const void *);

typedef struct {
    char *instances;
    u32 *entity_to_component_index_map;
    entity_t *component_to_entity_map;
    u32 *free_indices;
} manager_t;

typedef struct {
    char name[32];
    type_t type;
    u16 offset;
    u16 size;
    bool is_array;
} field_t;

typedef struct {
    u64 magic;
    char name[32];
    manager_t manager;
    const field_t *fields;
    u32 size;
    listener_t *listeners;
} component_t;

void component_register(
    component_t *cpt,
    const char *name,
    const field_t *fields,
    u32 size);

/// Retrieves the current component's state of an entity. An entity always
/// has a state for every component, but only non-default state is requiring memory.
/// \param cpt Component to look up with entity
/// \param entity The entity with a component
/// \param data The destination buffer for the component data to retrieve
/// \return True if component has non-default (zeroed) data
bool get(component_t *cpt, entity_t entity, void *data);

/// Updates an entity with new component data. If all data is null (default),
/// the memory is eventually freed, as default data does not need to be stored.
/// \param cpt Component to look up with entity
/// \param entity The entity with a component
/// \param data The source buffer for the component data to update
/// \return True if the component now has non-default (zeroed) data
bool update(component_t *cpt, entity_t entity, const void *data);

void register_listener(component_t *cpt, listener_t func);

extern component_t component_t_cpt;
extern component_t field_t_cpt;

void use_component();

#define member_size(type, member) sizeof(((type *)0)->member)
#define component_type(cpt_type) ((u64)&cpt_type)

#define component(C, S) \
    extern component_t C ## _cpt; \
    typedef struct S C; \
    static bool get_ ## C (entity_t entity, C *data) {\
        return get(& C ## _cpt, entity, (void*)data);\
    } \
    static bool update_ ## C (entity_t entity, C *data) {\
        return update(& C ## _cpt, entity, (const void*)data);\
    }

#define def_comp(C) \
    component_t C ## _cpt;\
    const field_t C ## _fields[] = {

#define def_field(C, T, N) { #N , t_ ## T, (u16)offsetof(C, N), (u16)member_size(C, N), false },
#define def_child(C, CC, N) { #N, component_type(CC ## _cpt), (u16)offsetof(C, N), (u16)member_size(C, N), false },
#define def_children(C, CC, N) { #N, component_type(CC ## _cpt), (u16)offsetof(C, N), (u16)member_size(C, N), true },
#define def_array(C, T, N) { #N, t_ ## T, (u16)offsetof(C, N), (u16)member_size(C, N), true },
#define def_end { 0, 0, 0, 0, 0 } };

#define reg_comp(C) component_register(&C ## _cpt, #C, C ## _fields, sizeof(C))

#endif //PLAZA_COMPONENT_H
