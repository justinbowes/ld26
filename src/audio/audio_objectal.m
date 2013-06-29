//
//  audio_objectal
//  ld26
//
//  Created by Justin Bowes on 2013-04-25.
//  Copyright (c) 2013 Informi Software Inc. All rights reserved.
//

#include <assert.h>
#include <sys/stat.h>

#include "uthash.h"

#include <xpl_platform.h>
#include <xpl_file.h>
#include <xpl_hash.h>

#include "audio.h"

#import "ObjectAL.h"

#define RESOURCE_ID_MAX					256
#define PLATFORM_AUDIO_EXTENSION		"aac"

typedef struct audio_impl {
	id<ALSoundSource> source;
	NSString *file_path;
} audio_impl_t;

static audio_t *audio_table = NULL;

NSString *get_resource_ns(const char *resource) {
	char with_extension[PATH_MAX];
	char resolved[PATH_MAX];
	snprintf(with_extension, PATH_MAX, "audio/%s.%s", resource, PLATFORM_AUDIO_EXTENSION);
	xpl_resolve_resource(resolved, with_extension, PATH_MAX);
	
	return [NSString stringWithCString:resolved encoding:NSUTF8StringEncoding];
}

static audio_t *audio_create_default(void) {
	audio_t *audio = xpl_calloc_type(audio_t);
	audio->impl = xpl_calloc_type(audio_impl_t);
	audio->impl->source = nil;
	audio->impl->file_path = nil;
	audio->instance_id = 0;
	audio->volume = 1.f;
	audio->pitch = 1.f;
	
	return audio;
}

audio_t *audio_create(const char *resource, bool as_bgm) {
	if (as_bgm) return audio_bgm_create(resource, 1.f, 0.f, true);
	
	NSString *ns_resource = get_resource_ns(resource);
	id<ALSoundSource> source = [[OALSimpleAudio sharedInstance] playEffect:ns_resource];
	
	[source setPaused:YES];
	
	audio_t *audio = audio_create_default();
	audio->impl = xpl_calloc_type(audio_impl_t);
	audio->impl->source = source;
	audio->impl->file_path = [ns_resource retain];
	
	int hash = xpl_hashp(audio->impl->source, XPL_HASH_INIT);
	audio->instance_id = hash;
	HASH_ADD_INT(audio_table, instance_id, audio);
	
	return audio;
}

audio_t *audio_bgm_create(const char *resource, float volume, float pan, bool loop) {
	NSString *ns_resource = get_resource_ns(resource);
	[[OALSimpleAudio sharedInstance] playBg:ns_resource volume:volume pan:pan loop:loop];
	[[OALSimpleAudio sharedInstance] setBgPaused:YES];

	audio_t *audio = audio_create_default();
	HASH_ADD_INT(audio_table, instance_id, audio);
	
	return audio;
}

void audio_destroy(audio_t **ppaudio) {
	audio_t *audio = *ppaudio;
	HASH_DEL(audio_table, audio);
	if (audio->impl->source) {
		[audio->impl->source stop];
		[[OALSimpleAudio sharedInstance] unloadEffect:audio->impl->file_path];
		[audio->impl->file_path release];
	} else {
		[[OALSimpleAudio sharedInstance] stopBg];
	}
	xpl_free(audio->impl);
	xpl_free(audio);
	*ppaudio = NULL;
}

bool audio_is_playing(audio_t *audio) {
	if (audio->impl->source) {
		return [audio->impl->source playing];
	} else {
		return [[OALSimpleAudio sharedInstance] bgPlaying];
	}
}

void audio_quickplay_pan(const char *resource, float volume, float pan) {
	NSString *ns_resource = get_resource_ns(resource);
	id<ALSoundSource> source = [[OALSimpleAudio sharedInstance] playEffect:ns_resource];
	[source setVolume:volume];
	[source setPan:pan];
}
void audio_quickplay_position(const char *resource, float volume, xvec3 position) {
	NSString *ns_resource = get_resource_ns(resource);
	id<ALSoundSource> source = [[OALSimpleAudio sharedInstance] playEffect:ns_resource];
	[source setVolume:volume];
	ALPoint al_position = { position.x, position.y, position.z };
	[source setPosition:al_position];
}

void audio_startup(void) {
//	[OALSimpleAudio sharedInstance].honorSilentSwitch = YES;
//	[OALSimpleAudio sharedInstance].allowIpod = YES;
}

void audio_shutdown(void) {
	audio_t *el, *tmp;
	HASH_ITER(hh, audio_table, el, tmp) {
		audio_destroy(&el);
	}
	[[OALSimpleAudio sharedInstance] stopEverything];
	[[OALSimpleAudio sharedInstance] unloadAllEffects];
}

static void audio_update_instance(audio_t *el) {
	el->volume = xclamp(el->volume, 0.f, 2.f);
	id<ALSoundSource> source = el->impl->source;
	if (source) {
		switch (el->action) {
			case aa_play:
				[source setPaused:NO];
				break;
			case aa_stop:
				[source setPaused:YES];
				break;
			case aa_continue:
			default:
				break;
		}
		el->action = aa_continue;
		[source setPitch:el->pitch];
		ALPoint point = { el->position.x, el->position.y, el->position.z };
		[source setPosition:point];
		[source setVolume:el->volume];
		if (source.looping != el->loop) {
			[source setLooping:el->loop];
		}
	} else {
		// background
		switch (el->action) {
			case aa_play:
				[[OALSimpleAudio sharedInstance] setBgPaused:NO];
				break;
			case aa_stop:
				[[OALSimpleAudio sharedInstance] setBgPaused:YES];
				break;
			case aa_continue:
			default:
				break;
		}
		el->action = aa_continue;
		[[OALSimpleAudio sharedInstance]setBgVolume:el->volume];
	}
}

void audio_update(void) {
	audio_t *el, *tmp;
	HASH_ITER(hh, audio_table, el, tmp) {
		audio_update_instance(el);
	}
}