//
//  audio.c
//  ld26
//
//  Created by Justin Bowes on 2013-04-25.
//  Copyright (c) 2013 Informi Software Inc. All rights reserved.
//

#include <assert.h>

#include "fmod.h"
#include "fmod_errors.h"
#include "uthash.h"

#include "audio/audio.h"

#include "xpl_platform.h"
#include "xpl_file.h"

#define RESOURCE_ID_MAX 256

typedef struct audio_resource {
	char				resource_id[RESOURCE_ID_MAX];
	
	FMOD_SOUND			*sound;
	
	FMOD_SOUND_TYPE		type;
	FMOD_SOUND_FORMAT	format;
	int					channels;
	int					bits;
	float				frequency;
	
	uint32_t			tick;
	
	UT_hash_handle		hh;
	
} audio_resource_t;

typedef struct audio_system {
	
	FMOD_SYSTEM			*system;
	
	audio_resource_t	*resource_table;
	audio_t				*instance_table;
	
	uint32_t			instance_count;
	uint32_t			tick;
	
} audio_system_t;

struct audio_impl {
	FMOD_CHANNEL		*channel;
	audio_resource_t	*resource;
	
	audio_state_t		state;
	float				frequency;

	bool				retain;
	
	uint32_t			tick;
};

static audio_system_t *s_audio_system = NULL;

static FMOD_RESULT result;
#define FMOD_ERRCHECK(x) \
	do { \
		result = x; \
		if (result != FMOD_OK) { \
			LOG_ERROR("FMOD error: %d %s", result, FMOD_ErrorString(result)); \
			fmod_error_breakpoint_func(); \
		} \
	} while(0)

static void fmod_error_breakpoint_func() {
    LOG_TRACE("Put a breakpoint here to stop on FMOD errors");
}

static audio_resource_t *load_resource(const char *short_name) {
	bool as_stream = (xpl_file_has_extension(short_name, "ogg") ||
					  xpl_file_has_extension(short_name, "mp3"));
	
	char subpath[PATH_MAX];
	snprintf(subpath, PATH_MAX, "audio/%s", short_name);
	
	char resource_path[PATH_MAX];
	if (! xpl_resolve_resource(resource_path, subpath, PATH_MAX)) {
		LOG_ERROR("Couldn't load %s", subpath);
		return NULL;
	}
	
	audio_resource_t *resource = xpl_calloc_type(audio_resource_t);
	
	int flags = FMOD_3D;
	if (as_stream) {
		FMOD_ERRCHECK(FMOD_System_CreateStream(s_audio_system->system, resource_path, flags, NULL, &resource->sound));
	} else {
		FMOD_ERRCHECK(FMOD_System_CreateSound(s_audio_system->system, resource_path, flags, NULL, &resource->sound));
	}
	
	FMOD_Sound_GetFormat(resource->sound, &resource->type, &resource->format, &resource->channels, &resource->bits);
	
	strncpy(resource->resource_id, short_name, RESOURCE_ID_MAX);
	
	return resource;
}

static void destroy_instance(audio_t **ppinstance) {
	audio_t *instance = *ppinstance;

	HASH_DEL(s_audio_system->instance_table, instance);
	
	if (instance->impl->channel) {
		FMOD_Channel_Stop(instance->impl->channel);
		FMOD_Channel_SetUserData(instance->impl->channel, NULL);
		instance->impl->channel = 0;
	}
	xpl_free(instance->impl);
	xpl_free(instance);
	*ppinstance = NULL;
}

static void destroy_resource(audio_resource_t **ppres) {
	audio_resource_t *res = *ppres;

	audio_t *instance_el, *instance_tmp;
	HASH_ITER(hh, s_audio_system->instance_table, instance_el, instance_tmp) {
		if (instance_el->impl->resource == res) {
			HASH_DEL(s_audio_system->instance_table, instance_el);
			destroy_instance(&instance_el);
		}
	}
	
	if (res->sound) {
		FMOD_Sound_Release(res->sound);
		res->sound = NULL;
	}

	HASH_DEL(s_audio_system->resource_table, res);
	
	*ppres = NULL;
}

static audio_resource_t *get_resource(const char *short_name) {
	audio_resource_t *res;
	HASH_FIND_STR(s_audio_system->resource_table, short_name, res);
	if (! res) {
		res = load_resource(short_name);
	}
	assert(res);
	
	return res;
}


audio_t *audio_create(const char *resource_name) {
	assert(s_audio_system);
	
	audio_resource_t *resource = get_resource(resource_name);
	
	audio_t *audio = xpl_calloc_type(audio_t);
	audio->impl = xpl_calloc_type(struct audio_impl);

	audio->loop = false;
	audio->pitch = 1.0f;
	audio->position = xvec3_all(0.f);
	audio->action = aa_stop;
	audio->volume = 1.0f;
	
	audio->impl->state = as_idle;
	audio->impl->retain = true;
	audio->impl->resource = resource;
	
	audio->instance_id = s_audio_system->instance_count++;
	
	HASH_ADD_INT(s_audio_system->instance_table, instance_id, audio);
	
	return audio;
}

void audio_quickplay(const char *resource_name, float volume, xvec3 position) {
	audio_t *audio = audio_create(resource_name);
	audio->volume = volume;
	audio->position = position;
	audio->action = aa_play;
	audio->impl->retain = false;
}

static void ensure_stopped(audio_t *instance) {
	if (instance->impl->state == as_idle) return;
	
	if (instance->impl->channel) {
		FMOD_ERRCHECK(FMOD_Channel_Stop(instance->impl->channel));
	}
	instance->impl->state = as_idle;
}


static F_CALLBACK FMOD_RESULT fmod_channel_callback(FMOD_CHANNEL *channel, FMOD_CHANNEL_CALLBACKTYPE type, void *cdata1, void *cdata2) {
	audio_t *instance;
	FMOD_Channel_GetUserData(channel, ((void **)&instance));
	
	switch (type) {
		case FMOD_CHANNEL_CALLBACKTYPE_END:
			ensure_stopped(instance);
			break;
			
		default:
			break;
	}
	
	return FMOD_OK;
}

static void update_live(audio_t *instance) {
	float volume;
	FMOD_ERRCHECK(FMOD_Channel_GetVolume(instance->impl->channel, &volume));
	if (volume != instance->volume)	FMOD_ERRCHECK(FMOD_Channel_SetVolume(instance->impl->channel, instance->volume));
	
	instance->impl->frequency = instance->pitch * instance->impl->resource->frequency;
	float frequency;
	FMOD_ERRCHECK(FMOD_Channel_GetFrequency(instance->impl->channel, &frequency));
	if (fabsf(frequency - instance->impl->frequency) > 1.0f) {
		FMOD_ERRCHECK(FMOD_Channel_SetFrequency(instance->impl->channel, instance->impl->frequency));
	}
	
	FMOD_ERRCHECK(FMOD_Channel_Set3DAttributes(instance->impl->channel, (FMOD_VECTOR *)(&instance->position.data), NULL));
}

static void ensure_playing(audio_t *instance) {
	if (instance->impl->state == as_playing) return;
	
	FMOD_ERRCHECK(FMOD_System_PlaySound(s_audio_system->system, FMOD_CHANNEL_FREE, instance->impl->resource->sound, TRUE, &instance->impl->channel));
	if (instance->loop) {
		FMOD_ERRCHECK(FMOD_Channel_SetMode(instance->impl->channel, FMOD_LOOP_NORMAL));
		// Documented workaround for very short sounds
		FMOD_ERRCHECK(FMOD_Channel_SetPosition(instance->impl->channel, 0, FMOD_TIMEUNIT_MS));
	} else {
		FMOD_ERRCHECK(FMOD_Channel_SetMode(instance->impl->channel, FMOD_LOOP_OFF));
	}
	
	FMOD_ERRCHECK(FMOD_Channel_SetUserData(instance->impl->channel, instance));
	FMOD_ERRCHECK(FMOD_Channel_SetCallback(instance->impl->channel, fmod_channel_callback));
	
	if (instance->impl->resource->frequency == 0.f) {
		FMOD_ERRCHECK(FMOD_Channel_GetFrequency(instance->impl->channel, &instance->impl->resource->frequency));
	}

	update_live(instance);
	
	FMOD_ERRCHECK(FMOD_Channel_SetPaused(instance->impl->channel, FALSE));
	instance->impl->state = as_playing;
}

static void update_ticks(audio_t *instance) {
	instance->impl->resource->tick = s_audio_system->tick;
	instance->impl->tick = s_audio_system->tick;
	
}

static void update_playing(audio_t *instance) {
	FMOD_BOOL is_playing;
	if (instance->impl->channel == 0) {
		is_playing = FALSE;
	} else {
		FMOD_ERRCHECK(FMOD_Channel_IsPlaying(instance->impl->channel, &is_playing));
	}
	
	if (is_playing) {
		update_live(instance);
	}
	
	update_ticks(instance);
}

static void check_retain_instance(audio_t *instance) {
	if (! instance->impl->retain) {
		if (instance->impl->tick != s_audio_system->tick) {
			destroy_instance(&instance);
			return;
		}
	}
	
	// Resource still bound
	update_ticks(instance);
}

static void check_retain_resource(audio_resource_t *res) {
	if (res->tick != s_audio_system->tick) {
		HASH_DEL(s_audio_system->resource_table, res);
		destroy_resource(&res);
	}
}

static void process_instances(void) {
	s_audio_system->tick++;
	audio_t *iel, *itmp;
	HASH_ITER(hh, s_audio_system->instance_table, iel, itmp) {
		switch (iel->action) {
			case aa_play:
				ensure_playing(iel);
				break;
				
			case aa_stop:
				ensure_stopped(iel);
				break;
				
			default:
				break;
		}
		iel->action = aa_continue;
		
		if (iel->impl->state == as_playing) update_playing(iel);
		if (iel->impl->state == as_idle)	check_retain_instance(iel);
	}
	
	audio_resource_t *rel, *rtmp;
	HASH_ITER(hh, s_audio_system->resource_table, rel, rtmp) {
		check_retain_resource(rel);
	}
}

bool audio_is_playing(audio_t *audio) {
	return audio->impl->state == as_playing;
}

void audio_update(void) {
	assert(s_audio_system);
	process_instances();
	FMOD_ERRCHECK(FMOD_System_Update(s_audio_system->system));
}

void audio_destroy(audio_t **ppaudio) {
	destroy_instance(ppaudio);
}

void audio_startup(void) {
	assert(! s_audio_system);
	s_audio_system = xpl_calloc_type(audio_system_t);
	
	FMOD_ERRCHECK(FMOD_System_Create(&s_audio_system->system));
	int flags = (FMOD_INIT_NORMAL |
				 FMOD_INIT_3D_RIGHTHANDED |
				 FMOD_INIT_VOL0_BECOMES_VIRTUAL);
	FMOD_ERRCHECK(FMOD_System_Init(s_audio_system->system, 100, flags, 0));
}

void audio_shutdown(void) {
	assert(s_audio_system);
	
	// Cascade-deletes instances too
	audio_resource_t *el, *tmp;
	HASH_ITER(hh, s_audio_system->resource_table, el, tmp) {
		HASH_DEL(s_audio_system->resource_table, el);
		destroy_resource(&el);
	}
	
	FMOD_ERRCHECK(FMOD_System_Release(s_audio_system->system));
	s_audio_system->system = NULL;
	
	xpl_free(s_audio_system);
	s_audio_system = NULL;
}



