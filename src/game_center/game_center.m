//
//  ILGameCenter.m
//  app
//
//  Created by Justin Bowes on 2013-06-26.
//  Copyright (c) 2013 Informi Software Inc. All rights reserved.
//

#include <xpl_log.h>
#include <xpl_texture_cocoa.h>
#import "game_center.h"
#include "ILViewController.h"

#define IOS_VERSION_GE(v)  ([[[UIDevice currentDevice] systemVersion] compare:v options:NSNumericSearch] != NSOrderedAscending)


@implementation ILGameCenter

@synthesize gameCenterAvailable;

#pragma mark Initialization

static ILGameCenter * shared_instance = nil;
+ (ILGameCenter *) sharedInstance {
	if (! shared_instance) {
		shared_instance = [[ILGameCenter alloc] init];
	}
	return shared_instance;
}

- (BOOL) isGameCenterAvailable {
	Class game_center_class = (NSClassFromString(@"GKLocalPlayer"));
	BOOL os_version_supported = IOS_VERSION_GE(@"4.1");
	
	return (game_center_class && os_version_supported);
}

- (id) init {
	if ((self = [super init])) {
		user = xpl_calloc_type(gc_user_t);
		user->authenticated = false;
		user->identifier = NULL;
		user->name = NULL;
		user->portrait = NULL;
		
		gameCenterAvailable = [self isGameCenterAvailable];
		if (gameCenterAvailable) {
			NSNotificationCenter *nc = [NSNotificationCenter defaultCenter];
			[nc addObserver:self
				   selector:@selector(authenticationDidChange)
					   name:GKPlayerAuthenticationDidChangeNotificationName
					 object:nil];
		}
	}
	return self;
}

- (void) dealloc {
	xpl_free(user);
	[super dealloc];
}

- (void) authenticationCompleteWithPhoto:(UIImage *)photo error:(NSError *)err {
	if (photo && (nil == err)) {
		user->portrait = xpl_texture_new();
		xpl_texture_load_uiimage(user->portrait, photo);
	}
	self.successCallback(*user);
}

#pragma mark Notification target

- (void) authenticationDidChange {
	if ([GKLocalPlayer localPlayer].isAuthenticated == userAuthenticated) return;
	
	if (user->identifier) free(user->identifier);
	if (user->name) free(user->name);
	if (user->alias) free(user->alias);
	
	if ([GKLocalPlayer localPlayer].isAuthenticated) {
		LOG_DEBUG("Player authenticated");
		userAuthenticated = YES;
		GKLocalPlayer *player = [GKLocalPlayer localPlayer];

		user->identifier = strdup([player.playerID cStringUsingEncoding:NSUTF8StringEncoding]);
		user->name = strdup([player.displayName cStringUsingEncoding:NSUTF8StringEncoding]);
		user->alias = strdup([player.alias cStringUsingEncoding:NSUTF8StringEncoding]);
		user->authenticated = YES;
		user->portrait = nil;

		[player loadPhotoForSize:GKPhotoSizeNormal withCompletionHandler:^(UIImage *photo, NSError *err) {
			// Probably best that we get out of this block before I screw something up
			[self authenticationCompleteWithPhoto:photo error:err];
		}];

	} else {
		LOG_DEBUG("Player deauthenticated");
		userAuthenticated = NO;
		
		user->authenticated = false;
		user->identifier = NULL;
		user->name = NULL;
		user->portrait = NULL;
	}
	
}

#pragma mark User management

- (void) finishGameCenterAuthWithError:(NSError *)err {
	error = [err localizedDescription];
	self.failureCallback([error cStringUsingEncoding:NSUTF8StringEncoding]);
}

- (void) authenticateLocalUser {
	if (! gameCenterAvailable) {
		LOG_DEBUG("Game Center not available");
		self.failureCallback("Game Center not available");
	}
	
	GKLocalPlayer *local = [GKLocalPlayer localPlayer];
	BOOL authenticated = local.authenticated;
	if (authenticated) {
		LOG_DEBUG("Already authenticated");
		self.successCallback(*user);
	}
	
	if ([local respondsToSelector:@selector(setAuthenticateHandler:)]) {
		[local setAuthenticateHandler:^(UIViewController *vc, NSError *err) {
			if (vc) {
				[root_view_controller presentViewController:vc animated:YES completion:^{
					LOG_DEBUG("Waiting on user sign in");
				}];
			} else if (err) {
				[self finishGameCenterAuthWithError:err];
			}
		}];
	} else if ([local respondsToSelector:@selector(authenticateWithCompletionHandler:)]) {
		[local authenticateWithCompletionHandler:^(NSError *err) {
			[self finishGameCenterAuthWithError:err];
		}];
	}
}

@end

bool game_center_init(void) {
	return [ILGameCenter sharedInstance].gameCenterAvailable;
}

void game_center_authenticate(gccb_success_callback success, gccb_failure_callback failure) {
	[ILGameCenter sharedInstance].successCallback = success;
	[ILGameCenter sharedInstance].failureCallback = failure;
	[[ILGameCenter sharedInstance] authenticateLocalUser];
}

