/*
 * xpl_es.c
 *
 *  Created on: 2012-12-13
 *      Author: Hume
 */

#include <assert.h>

#include "uthash.h"

#include "xpl_es.h"

typedef struct es_component {
	xpl_entity          eid;
	xpl_type_id         type_id;
	void                *data;

	UT_hash_handle      hh_main_by_ptr;
	UT_hash_handle      hh_entity_by_type_id;
	UT_hash_handle      hh_type_id_by_eid;
} es_component_t;

typedef struct es_component_by_type_id {
	xpl_type_id         type_id;
	es_component_t      *by_eid_table;
	UT_hash_handle      hh;
} es_component_by_type_id_t;

typedef struct es_components {
	es_component_t      *by_ptr_table;
	es_component_by_type_id_t *by_type_id_table;
} es_components_t;

typedef struct es_entity {
	xpl_entity          id;
	es_component_t      *component_table;
	UT_hash_handle      hh;
} es_entity_t;

typedef struct es_entities {
	xpl_entity          next_id;
	es_entity_t         *entity_table;
} es_entities_t;

typedef struct es_allocator {
    xpl_type_id         type_id;
    xpl_component_allocator allocator;
    xpl_component_destructor destructor;
    UT_hash_handle      hh;
} es_allocator_t;

struct xpl_es_context {
	es_components_t     *components;
	es_entities_t       *entities;
    es_allocator_t      *allocators;
};

static void entity_destroy(xpl_es_t *context, es_entity_t *entity) {
	es_component_t *component, *tmp;
	HASH_ITER(hh_entity_by_type_id, entity->component_table, component, tmp) {
		xpl_entity_component_destroy(context, entity->id, component->data);
	}

	HASH_DEL(context->entities->entity_table, entity);
    xpl_free(entity);
}

xpl_es_t *xpl_es_new() {
	xpl_es_t *context = xpl_calloc_type(xpl_es_t);

	context->entities = xpl_calloc_type(es_entities_t);
	context->entities->next_id = XPL_ENTITY_NONE + 1;
	context->components = xpl_calloc_type(es_components_t);

	return context;
}

void xpl_es_destroy(xpl_es_t **ppcontext) {
	assert(ppcontext);
	xpl_es_t *context = *ppcontext;
	assert(context);

	es_entity_t *entity, *etmp;
	HASH_ITER(hh, context->entities->entity_table, entity, etmp) {
		entity_destroy(context, entity);
	}

	// We don't purge the type_id table when its lists are empty for performance,
	// so we have to do it now.
	es_component_by_type_id_t *by_type_id, *tmp;
	HASH_ITER(hh, context->components->by_type_id_table, by_type_id, tmp) {
		HASH_DEL(context->components->by_type_id_table, by_type_id);
		assert(by_type_id->by_eid_table == NULL);
		xpl_free(by_type_id);
	}

    es_allocator_t *allocator, *atmp;
    HASH_ITER(hh, context->allocators, allocator, atmp) {
        HASH_DEL(context->allocators, allocator);
        xpl_free(allocator);
    }
    
	xpl_free(context->entities);
	xpl_free(context->components);
	xpl_free(context);

	*ppcontext = NULL;
}

xpl_entity xpl_entity_new(xpl_es_t *context) {
	assert(context);
	xpl_entity eid = context->entities->next_id++;
	es_entity_t *entity = xpl_alloc_type(es_entity_t);
	entity->id = eid;
	entity->component_table = NULL;
	HASH_ADD_INT(context->entities->entity_table, id, entity);

	return eid;
}

void xpl_entity_destroy(xpl_es_t *context, xpl_entity entity) {
	assert(context);
	es_entity_t *entry;
	HASH_FIND_INT(context->entities->entity_table, &entity, entry);
	assert(entry);
	entity_destroy(context, entry);
}

xpl_entity xpl_entity_transfer(xpl_es_t *source, xpl_entity source_entity, xpl_es_t *dest) {
    xpl_entity destination_entity = xpl_entity_new(dest);
    
    es_entity_t *entity;
    HASH_FIND_INT(source->entities->entity_table, &source_entity, entity);
    assert(entity);
    es_component_t *component, *tmp;
    HASH_ITER(hh_entity_by_type_id, entity->component_table, component, tmp) {
        void *data = component->data;
        const xpl_type_id type_id = component->type_id;
        xpl_component_remove_type_id(source, source_entity, data, type_id);
        xpl_component_assign_type_id(dest, destination_entity, data, type_id);
    }
    xpl_entity_destroy(source, source_entity);
    
    return destination_entity;
}

void *xpl_component_assign_type_id(xpl_es_t *context, const xpl_entity entity, void *component, const xpl_type_id type_id) {
    if (xpl_entity_component_with_type_id(context, entity, type_id)) {
    	LOG_ERROR("Duplicate component assignment to %u!", entity);
    	assert(0);
    }
    
	assert(context);
	es_component_t *esc = xpl_alloc_type(es_component_t);
	esc->eid = entity;
	esc->data = component;
	esc->type_id = type_id;

	HASH_ADD(hh_main_by_ptr, context->components->by_ptr_table, data, sizeof(void *), esc);

	es_component_by_type_id_t *by_type_id;
	HASH_FIND_INT(context->components->by_type_id_table, &type_id, by_type_id);
	if (! by_type_id) {
		by_type_id = xpl_alloc_type(es_component_by_type_id_t);
		by_type_id->type_id = type_id;
		by_type_id->by_eid_table = NULL;
		HASH_ADD_INT(context->components->by_type_id_table, type_id, by_type_id);
	}
	HASH_ADD(hh_type_id_by_eid, by_type_id->by_eid_table, eid, sizeof(xpl_entity), esc);

	es_entity_t *ee;
	HASH_FIND_INT(context->entities->entity_table, &entity, ee);
	assert(ee);
    HASH_ADD(hh_entity_by_type_id, ee->component_table, type_id, sizeof(type_id), esc);

	return component;
}

void *xpl_component_remove_type_id(xpl_es_t *context, const xpl_entity entity, void *component, const xpl_type_id type_id) {
	assert(context);
    assert(type_id);
	es_component_t *esc;
	HASH_FIND(hh_main_by_ptr, context->components->by_ptr_table, &component, sizeof(void *), esc);
	assert(esc);
	HASH_DELETE(hh_main_by_ptr, context->components->by_ptr_table, esc);

	es_component_by_type_id_t *by_type_id;
	HASH_FIND_INT(context->components->by_type_id_table, &type_id, by_type_id);
	assert(by_type_id);
	HASH_DELETE(hh_type_id_by_eid, by_type_id->by_eid_table, esc);

	es_entity_t *ee;
	HASH_FIND_INT(context->entities->entity_table, &esc->eid, ee);
	assert(ee);
	HASH_DELETE(hh_entity_by_type_id, ee->component_table, esc);
    
    xpl_free(esc);

	return component;
}

void xpl_component_allocator_for_type_id(xpl_es_t *context, const xpl_type_id type_id, xpl_component_allocator allocator, xpl_component_destructor destructor) {
    assert(context);
    
    es_allocator_t *entry;
    HASH_FIND_INT(context->allocators, &type_id, entry);
    if (! entry) {
        entry = xpl_calloc_type(es_allocator_t);
        entry->type_id = type_id;
        HASH_ADD_INT(context->allocators, type_id, entry);
    }
    entry->allocator = allocator;
    entry->destructor = destructor;
}

static void *component_create_with_type_id(xpl_es_t *context, const xpl_type_id type_id, size_t size) {
    assert(context);
    es_allocator_t *allocator;
    HASH_FIND_INT(context->allocators, &type_id, allocator);
    if (allocator) return allocator->allocator();
    
    return xpl_calloc(size);
}

void xpl_component_destroy_with_type_id(xpl_es_t *context, void *component, const xpl_type_id type_id) {
    assert(component);
    
	// This looks dangerous. We actually do the free FIRST so that any
	// last second eid lookups in registered destructors will work.  THEN
	// we destroy the association with the entity, after the component
	// is dead and gone.
    es_allocator_t *allocator;
    HASH_FIND_INT(context->allocators, &type_id, allocator);
    if (allocator) {
        allocator->destructor(component);
    } else {
        xpl_free(component);
    }
	
	es_component_t *esc;
    HASH_FIND(hh_main_by_ptr, context->components->by_ptr_table, &component, sizeof(void *), esc);
    if (esc) {
        // Not necessary for the component to be registered.
        // The ES allocates the component and allows it to be detached, so it must
        // allow destroy after detach.
        xpl_component_remove_type_id(context, esc->eid, esc->data, esc->type_id);
    }
    

}

void * xpl_component_new_with_type_id(xpl_es_t *context, xpl_entity entity, const xpl_type_id type_id, const size_t size) {
    void *component = component_create_with_type_id(context, type_id, size);
    xpl_component_assign_type_id(context, entity, component, type_id);
    return component;
}

void xpl_entity_component_destroy(xpl_es_t *context, xpl_entity entity, void *component_data) {
    const xpl_type_id type_id = xpl_component_type_id(context, component_data);
    xpl_component_destroy_with_type_id(context, xpl_component_remove_type_id(context, entity, component_data, type_id), type_id);
}

xpl_entity xpl_entity_with_component(xpl_es_t *context, const void *component) {
    assert(context);
    assert(component);
    es_component_t *esc;
    HASH_FIND(hh_main_by_ptr, context->components->by_ptr_table, &component, sizeof(void *), esc);
    return esc ? esc->eid : XPL_ENTITY_NONE;
}

xpl_type_id xpl_component_type_id(xpl_es_t *context, const void *component) {
	assert(context);
	assert(component);
	es_component_t *esc;
	HASH_FIND(hh_main_by_ptr, context->components->by_ptr_table, &component, sizeof(void *), esc);
	return esc ? esc->type_id : 0;
}

xpl_component_result_set_t *xpl_component_result_set_new() {
    size_t max_size = 2;
	xpl_component_result_set_t *set = xpl_alloc_type(xpl_component_result_set_t);
	set->result = xpl_alloc((max_size) * sizeof(xpl_component_result_t));
	set->max_size = max_size;
	set->size = 0;
	return set;
}

void xpl_component_result_set_destroy(xpl_component_result_set_t **ppset) {
	assert(ppset);
	xpl_component_result_set_t *set = *ppset;
	assert(set);

	xpl_free(set->result);

	*ppset = NULL;
}

size_t xpl_components_with_type_id(xpl_es_t *context, const xpl_type_id type_id, xpl_component_result_set_t *results) {
    assert(results);
	assert(results->max_size > 0);
	results->size = 0;

	es_component_by_type_id_t *by_type_id;
	HASH_FIND_INT(context->components->by_type_id_table, &type_id, by_type_id);
	if (by_type_id) {
		es_component_t *esc, *tmp;
		HASH_ITER(hh_type_id_by_eid, by_type_id->by_eid_table, esc, tmp) {
			size_t index = results->size++;
			results->result[index].eid = esc->eid;
			results->result[index].component = esc->data;

			// Resize in case of imminent overflow
			if (results->size == results->max_size) {
				results->max_size *= 1.75 + 1;
				results->result = xpl_realloc(results->result, (1 + results->max_size) * sizeof(xpl_component_result_t));
			}
		}
	}

	// We pad the allocated array so that this is always safe.
	results->result[results->size].eid = XPL_ENTITY_NONE;
	results->result[results->size].component = NULL;

	return results->size;
}

void * xpl_component_with_type_id(xpl_es_t *context, const xpl_type_id type_id, xpl_entity *entity_out) {
    es_component_by_type_id_t *by_type_id;
    HASH_FIND_INT(context->components->by_type_id_table, &type_id, by_type_id);
    if (by_type_id) {
        es_component_t *esc, *tmp;
        HASH_ITER(hh_type_id_by_eid, by_type_id->by_eid_table, esc, tmp) {
        	if (entity_out) *entity_out = esc->eid;
            return esc->data;
        }
    }

    if (entity_out) *entity_out = XPL_ENTITY_NONE;
    return NULL;    
}

void * xpl_entity_component_with_type_id(xpl_es_t *context, xpl_entity entity, const xpl_type_id type_id) {
	es_entity_t *ee;
	HASH_FIND_INT(context->entities->entity_table, &entity, ee);
	if (! ee) {
		assert(ee); // Entity doesn't exist
	}

	es_component_t *ec;
    HASH_FIND(hh_entity_by_type_id, ee->component_table, &type_id, sizeof(type_id), ec);
	return ec ? ec->data : NULL;
}

xpl_entity xpl_only_entity_with_component_type_id(xpl_es_t *context, const xpl_type_id type_id) {
    xpl_entity e;
    xpl_component_with_type_id(context, type_id, &e);
    return e;
}

