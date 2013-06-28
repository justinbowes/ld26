//
//  game_center.h
//  app
//
//  Created by Justin Bowes on 2013-06-26.
//  Copyright (c) 2013 Informi Software Inc. All rights reserved.
//

#include <xpl_texture.h>

#import <Foundation/Foundation.h>
#import <GameKit/GameKit.h>


typedef struct gc_user {
	bool authenticated;
	char *identifier;
	char *name;
	char *alias;
	xpl_texture_t *portrait;
} gc_user_t;

typedef void (* gccb_success_callback)(gc_user_t user);
typedef void (* gccb_failure_callback)(const char *error_message);

@interface ILGameCenter : NSObject {
	BOOL gameCenterAvailable;
	NSString *error;
	BOOL userAuthenticated;
	
	gc_user_t *user;
}

@property (assign, readonly) BOOL gameCenterAvailable;
@property (assign) gccb_failure_callback failureCallback;
@property (assign) gccb_success_callback successCallback;

bool game_center_init(void);
void game_center_authenticate(gccb_success_callback, gccb_failure_callback);

@end
