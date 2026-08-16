#ifndef ASSETS_H
#define ASSETS_H

#include <bgame/asset/sprite.h>
#include <cute_graphics.h>

extern struct CF_Sprite* spr_wall;
extern struct CF_Sprite* spr_enemy;
extern struct CF_Sprite* spr_floor_1;
extern struct CF_Sprite* spr_floor_2;
extern struct CF_Sprite* spr_floor_3;
extern struct CF_Sprite* spr_floor_4;
extern struct CF_Sprite* spr_floor_5;
extern struct CF_Sprite* spr_floor_6;
extern struct CF_Sprite* spr_floor_7;
extern struct CF_Sprite* spr_floor_8;
extern CF_Shader shd_default;

void
load_assets(void);

void
unload_assets(void);

void
check_assets(void);

#endif
