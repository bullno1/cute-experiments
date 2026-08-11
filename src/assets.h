#ifndef ASSETS_H
#define ASSETS_H

#include <bgame/asset/sprite.h>
#include <cute_graphics.h>

extern struct CF_Sprite* spr_wall;
extern struct CF_Sprite* spr_floor;
extern CF_Shader shd_default;

void
load_assets(void);

void
unload_assets(void);

void
check_assets(void);

#endif
