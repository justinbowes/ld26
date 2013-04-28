//
//  xpl_preprocessor_hash.h
//  p1
//
//  Created by Justin Bowes on 2013-02-20.
//  Copyright (c) 2013 Informi Software Inc. All rights reserved.
//

#ifndef p1_xpl_preprocessor_hash_h
#define p1_xpl_preprocessor_hash_h

// http://lol.zoy.org/blog/2011/12/20/cpp-constant-string-hash

#include <string.h>
#include <stdint.h>
#include <stdio.h>

#define H1(s,i,x)   (x*65599u+(uint8_t)s[(i)<strlen(s)?strlen(s)-1-(i):strlen(s)])
#define H4(s,i,x)   H1(s,i,H1(s,i+1,H1(s,i+2,H1(s,i+3,x))))
#define H16(s,i,x)  H4(s,i,H4(s,i+4,H4(s,i+8,H4(s,i+12,x))))
#define H64(s,i,x)  H16(s,i,H16(s,i+16,H16(s,i+32,H16(s,i+48,x))))
#define H256(s,i,x) H64(s,i,H64(s,i+64,H64(s,i+128,H64(s,i+192,x))))

//#define PS_HASH(s)    ((uint32_t)(H256(s,0,0)^(H256(s,0,0)>>16)))
//#ifdef DEBUG
//#define PS_HASH(s)    xpl_hashs(s, XPL_HASH_INIT)
//#else
#define PS_HASH(s)    ((uint32_t)(H16(s,0,0)^(H16(s,0,0)>>16)))
//#endif


#endif
