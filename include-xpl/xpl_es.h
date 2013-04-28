/*
 * xpl_es.h
 *
 *  Created on: 2012-12-13
 *      Author: Justin
 */

#ifndef XPL_ES_H_
#define XPL_ES_H_

#include "uthash.h"
#include "utlist.h"

#include "xpl.h"
#include "xpl_preprocessor_hash.h"
#include "xpl_memory.h"

#define XPL_ENTITY_NONE 0

typedef uint32_t xpl_entity;
typedef uint32_t xpl_type_id;

typedef void *(* xpl_component_allocator)(void);
typedef void (* xpl_component_destructor)(void *);

typedef struct xpl_component_result {
	xpl_entity eid;
	void *component;
} xpl_component_result_t;

typedef struct xpl_component_result_set {
	xpl_component_result_t *result;
	size_t size;
	size_t max_size;
} xpl_component_result_set_t;

typedef struct xpl_es_context xpl_es_t;

xpl_es_t *xpl_es_new(void);
void xpl_es_destroy(xpl_es_t **ppcontext);

xpl_entity xpl_entity_new(xpl_es_t *context);
void xpl_entity_destroy(xpl_es_t *context, xpl_entity entity);

// Transfer an entity's components from the source entity system to a new entity
// in the destination entity system (without reallocation). Returns the new entity.
xpl_entity xpl_entity_transfer(xpl_es_t *source, xpl_entity source_entity, xpl_es_t *dest);

void xpl_component_allocator_for_type_id(xpl_es_t *context, const xpl_type_id type_id, xpl_component_allocator allocator, xpl_component_destructor destructor);

void *xpl_component_new_with_type_id(xpl_es_t *es, xpl_entity entity, const xpl_type_id type_id, const size_t size);
//void *xpl_component_create_with_type_id(xpl_es_t *es, const xpl_type_id type_id, const size_t size);
void xpl_component_destroy_with_type_id(xpl_es_t *es, void *component, const xpl_type_id type_id);
void xpl_entity_component_destroy(xpl_es_t *es, const xpl_entity entity, void *component_data);

xpl_entity xpl_entity_with_component(xpl_es_t *es, const void *component);

xpl_type_id xpl_component_type_id(xpl_es_t *context, const void *component);

void *xpl_component_assign_type_id(xpl_es_t *context, const xpl_entity entity, void *component, const xpl_type_id type_id);
void *xpl_component_remove_type_id(xpl_es_t *context, const xpl_entity entity, void *component, const xpl_type_id type_id);

size_t xpl_components_with_type_id(xpl_es_t *context, const xpl_type_id type_id, xpl_component_result_set_t *result);
void * xpl_component_with_type_id(xpl_es_t *context, const xpl_type_id type_id, xpl_entity *entity_out);

xpl_entity xpl_only_entity_with_component_type_id(xpl_es_t *context, const xpl_type_id type_id);
void * xpl_entity_component_with_type_id(xpl_es_t *context, const xpl_entity entity, const xpl_type_id type_id);

xpl_component_result_set_t *xpl_component_result_set_new(void);
void xpl_component_result_set_destroy(xpl_component_result_set_t **ppset);

#define xpl_component_new(context, entity, type) \
    ((type *)(xpl_component_new_with_type_id(context, entity, PS_HASH(#type), sizeof(type))))

#define xpl_component_allocator(context, type, allocator, destructor) \
    xpl_component_allocator_for_type_id(context, PS_HASH(#type), allocator, destructor);

#define xpl_component_destroy(context, component, type) \
    xpl_component_destroy_with_type_id(context, component, PS_HASH(#type));

#define xpl_component_assign(context, entity, component_data, type) \
    xpl_component_assign_type_id(context, entity, component_data, PS_HASH(#type))

#define xpl_component_remove(context, entity, component_data) \
    xpl_component_remove_type_id(context, entity, component_data, xpl_component_type_id(context, component_data))

#define xpl_components_of_type(context, type, result) \
	xpl_components_with_type_id(context, PS_HASH(#type), result)

#define xpl_component_of_type(context, type, entity_out) \
    ((type *)xpl_component_with_type_id(context, PS_HASH(#type), entity_out))

#define xpl_only_entity_with_component_type(context, type) \
    xpl_only_entity_with_component_type_id(context, PS_HASH(#type))

#define xpl_entity_component(context, entity, type) \
	((type *)xpl_entity_component_with_type_id(context, entity, PS_HASH(#type)))

#define xpl_component_foreach(results, idx, entry_eid, el) \
	for(idx = (el = results->result[0].component, entry_eid = results->result[0].eid, 0); \
        idx < results->size; \
        (el = results->result[++idx].component, entry_eid = results->result[idx].eid))

#endif /* XPL_ES_H_ */
