//
//  xpl_input.c
//  protector
//
//  Created by Justin Bowes on 2013-06-22.
//  Copyright (c) 2013 Informi Software Inc. All rights reserved.
//

#include <stdio.h>
#include <assert.h>

#include <uthash.h>

#include "xpl_hash.h"
#include "xpl_log.h"

#include "xpl_input.h"

typedef struct {
	int id;
	
	xpl_input_character_func func;
	void *context;
	
	UT_hash_handle hh;
	
} xpl_character_listener_t;
static xpl_character_listener_t *character_listeners = NULL;

bool xpl_input_is_character(int key) {
	return key > XPL_KEY_SPECIAL;
}

int xpl_input_add_character_listener(xpl_input_character_func func, void *context) {
	if (character_listeners == NULL) {
		xpl_input_enable_characters();
	}
	
	int hash = xpl_hashp(func, XPL_HASH_INIT);
	hash = xpl_hashp(context, hash);

	xpl_character_listener_t *listener;
	HASH_FIND_INT(character_listeners, &hash, listener);
	if (listener) {
		LOG_WARN("Listener already registered");
		return hash;
	}
	
	listener = xpl_calloc_type(xpl_character_listener_t);
	listener->id = hash;
	listener->func = func;
	listener->context = context;
	
	HASH_ADD_INT(character_listeners, id, listener);
	
	return listener->id;
}

void xpl_input_remove_character_listener(int listener_id) {
	xpl_character_listener_t *listener = NULL;
	HASH_FIND_INT(character_listeners, &listener_id, listener);
	assert(listener);
	HASH_DEL(character_listeners, listener);
	
	if (character_listeners == NULL) {
		xpl_input_disable_characters();
	}
}

void xpl_input__internal__dispatch_character(int character) {
	xpl_character_listener_t *el, *tmp;
	HASH_ITER(hh, character_listeners, el, tmp) {
		if (el->func(character, el->context)) return;
	}
}
