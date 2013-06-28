//
//  xpl_input_ios.h
//  protector
//
//  Created by Justin Bowes on 2013-06-22.
//  Copyright (c) 2013 Informi Software Inc. All rights reserved.
//

#ifndef protector_xpl_input_ios_h
#define protector_xpl_input_ios_h

#include <CoreGraphics/CoreGraphics.h>
#include <UIKit/UIKit.h>

#include "xpl_platform.h"

@interface XPLKeyInputView : UIView <UIKeyInput>
@property(nonatomic, assign) UIView *root_view;
-(id) initWithParentView:(UIView *)view;
@end

void xpl_input_ios_init(UIView *view);
void xpl_input_ios_destroy(void);

void xpl_input_ios_set_touch_began(CGPoint *point);
void xpl_input_ios_set_touch_moved(CGPoint *point);
void xpl_input_ios_set_touch_ended(CGPoint *point);

#endif
