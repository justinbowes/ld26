//
//  audio.h
//  ld26
//
//  Created by Justin Bowes on 2013-04-25.
//  Copyright (c) 2013 Informi Software Inc. All rights reserved.
//

#ifndef ld26_audio_h
#define ld26_audio_h

#include <stdbool.h>

#include "xpl_vec3.h"

#include "uthash.h"

#define RESOURCE_ID_MAX 256

struct audio_resource_impl;
struct audio_system_impl;
struct audio_channel_impl;


typedef enum audio_action {
	aa_stop,
	aa_play,
	aa_continue
} audio_action_t;

typedef enum audio_state {
	as_idle,
	as_playing
} audio_state_t;

typedef struct audio {
	uint32_t			instance_id;
	bool				loop;
	float				pitch;
	float				volume;
	xvec3				position;
	
	audio_action_t		action;
	
	struct	audio_impl	*impl;
	
	UT_hash_handle		hh;
	
} audio_t;

void audio_startup(void);
void audio_shutdown(void);

bool audio_is_playing(audio_t *audio);

audio_t *audio_create(const char *resource, bool as_bgm);
#ifdef XPL_PLATFORM_IOS
audio_t *audio_bgm_create(const char *resource, float volume, float pan, bool loop);
#endif
void audio_destroy(audio_t **ppaudio);
void audio_quickplay_pan(const char *resource, float volume, float pan);
void audio_quickplay_position(const char *resource, float volume, xvec3 position);

void audio_update(void);

#endif
