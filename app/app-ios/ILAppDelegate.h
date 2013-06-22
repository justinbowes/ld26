//
//  ILAppDelegate.h
//  UltraPew-IOS
//
//  Created by Justin Bowes on 2013-05-13.
//  Copyright (c) 2013 Informi Software Inc. All rights reserved.
//

#import <UIKit/UIKit.h>

@class ILViewController;

@interface ILAppDelegate : UIResponder <UIApplicationDelegate>

@property (strong, nonatomic) UIWindow *window;

@property (strong, nonatomic) ILViewController *viewController;

@end
