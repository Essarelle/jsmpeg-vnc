#ifndef LIST_H
#define LIST_H

#define LIST_HEADER(list_name, list_type, elements_name, elements_type) \
\
typedef struct list_type list_type; \
\
list_type *list_name##_new(); \
\
void list_name##_drop(list_type *list_name); \
\
size_t list_name##_get_length(list_type *list_name); \
\
elements_type *list_name##_get(list_type *list_name, size_t index); \
\
void list_name##_insert(list_type *list_name, elements_type *elements_name);

#define LIST_SOURCE(list_name, list_type, elements_name, elements_type) \
\
struct list_type { \
    elements_type *data; \
    size_t length; \
}; \
\
list_type *list_name##_new() { \
    list_type *list_name = malloc(sizeof(list_type)); \
    list_name->data = malloc(0); \
    return list_name; \
} \
\
void list_name##_drop(list_type *list_name) { \
    for (size_t i = 0; i < list_name##_get_length(list_name); i++) { \
        elements_type *elements_name##_copy = malloc(sizeof(elements_type)); \
        memcpy(elements_name##_copy, list_name##_get(list_name, i), sizeof(elements_type)); \
        elements_name##_drop(elements_name##_copy); \
    } \
    free(list_name->data); \
    free(list_name); \
} \
\
size_t list_name##_get_length(list_type *list_name) { \
    return list_name->length; \
} \
\
elements_type *list_name##_get(list_type *list_name, size_t index) { \
    return list_name->data + index; \
} \
\
void list_name##_insert(list_type *list_name, elements_type *elements_name) { \
    size_t index = list_name->length; \
    list_name->length++; \
    list_name->data = realloc(list_name->data, sizeof(elements_type) * list_name->length); \
    elements_type *elements_name##_copy = elements_name##_clone(elements_name); \
    memcpy(list_name##_get(list_name, index), elements_name##_copy, sizeof(elements_type)); \
    free(elements_name##_copy); \
}

#endif
