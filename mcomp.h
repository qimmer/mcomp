//
// Created by Kim on 26-09-2018.
//

#ifndef PLAZA_COMPONENT_H
#define PLAZA_COMPONENT_H

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
    t_ref,
    t_type,
    t_string,
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
    "ref",
    "type",
    "string"
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
    sizeof(u64) * 2,
    sizeof(type_t),
    sizeof(const char*)
};

#define idx_invalid 0xFFFFFFFF

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


#define member_size(type, member) sizeof(((type *)0)->member)
#define component_type(cpt_type) ((u64)&cpt_type)
#define fields_end { 0, 0, 0, 0, 0 }

typedef void(*listener_t)(const void *, const void *);
typedef int(*error_proc)(const char *format, ...);

extern error_proc component_error_proc;

typedef struct {
    char *instances;
    u32 *generations;
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

typedef struct {
    component_t *component;
    u32 index, gen;
} ref_t;

void component_register(
    component_t *cpt,
    const char *name,
    const field_t *fields,
    u32 size);

ref_t component_new(component_t *cpt);

void component_free(ref_t ref);
bool component_get(ref_t ref, void *data);
bool component_set(ref_t ref, const void *data);

void register_listener(component_t *cpt, listener_t func);

bool ref_valid(ref_t ref);

extern component_t component_cpt;
extern component_t field_cpt;

void use_component();

#endif //PLAZA_COMPONENT_H
