//
//  xpl_l10n.h
//  p1
//
//  Created by Justin Bowes on 2013-03-15.
//  Copyright (c) 2013 Informi Software Inc. All rights reserved.
//

#ifndef p1_xpl_l10n_h
#define p1_xpl_l10n_h

#define XPL_SHORT_L10N_FUNCTIONS

#ifdef XPL_SHORT_L10N_FUNCTIONS
#define xl(key) (xpl_l10n_get(key))
#endif

void xpl_l10n_set_locale(const char *locale);
void xpl_l10n_load_saved_locale(void);

void xpl_l10n_set_fallback_locale(const char *locale);

const char * xpl_l10n_get(const char *key);

#endif
