//
//  hud_sheet.h
//  p1
//
//  Created by Justin Bowes on 2013-01-22.
//  Copyright (c) 2013 Informi Software Inc. All rights reserved.
//

#ifndef p1_hud_sheet_h
#define p1_hud_sheet_h

#include "xpl_sprite.h"

XPL_UNUSED static xpl_sprite_sheet_def_t hud_sheet = {
    "hud_icons.png",
    {{ 1024, 1024 }},
    {{ 8, 8 }},
    TRUE
};

typedef enum hud_sheet_id {
    hud_0_0,    hud_0_1,    hud_0_2,    hud_0_3,    hud_0_4,    hud_0_5,    hud_0_6,    hud_0_7,
    hud_1_0,    hud_1_1,    hud_1_2,    hud_1_3,    hud_1_4,    hud_1_5,    hud_1_6,    hud_1_7,
    hud_2_0,    hud_2_1,    hud_2_2,    hud_2_3,    hud_2_4,    hud_2_5,    hud_2_6,    hud_2_7,
    hud_dir,    hud_back,   hud_query,  hud_yes,    hud_no,     hud_3_5,    hud_3_6, 	hud_3_7,
    hud_mech,   hud_batt,   hud_shield, hud_co2_o2, hud_thrust, hud_gyro,   hud_4_6,    hud_4_7,
    hud_power,  hud_slrpwr, hud_pwrbatt,hud_pwrmech,hud_5_4,    hud_5_5,    hud_5_6,    hud_5_7,
    hud_warn,   hud_info,   hud_rad,    hud_heat,   hud_6_4,    hud_fusion, hud_fission,hud_6_7,
    hud_undock, hud_dock,   hud_7_2,    hud_deploy, hud_o2,     hud_co2,    hud_h2,     hud_u
} hud_sheet_id_t;

#endif
