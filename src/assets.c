#include "assets.h"
#include <cute_graphics.h>
#include <bgame/shader.h>
#include "gen/default_vert_bytecode.h"
#include "gen/default_frag_bytecode.h"

BGAME_VAR(bgame_asset_bundle_t*, predefined_assets) = { 0 };

BGAME_DEFINE_SPRITE(spr_wall) = { .path = "/assets/sprites/wall.png" };
BGAME_DEFINE_SPRITE(spr_enemy) = { .path = "/assets/sprites/skeleton-mage.png" };

BGAME_DEFINE_SPRITE(spr_floor_1) = { .path = "/assets/sprites/floor_1.png" };
BGAME_DEFINE_SPRITE(spr_floor_2) = { .path = "/assets/sprites/floor_2.png" };
BGAME_DEFINE_SPRITE(spr_floor_3) = { .path = "/assets/sprites/floor_3.png" };
BGAME_DEFINE_SPRITE(spr_floor_4) = { .path = "/assets/sprites/floor_4.png" };
BGAME_DEFINE_SPRITE(spr_floor_5) = { .path = "/assets/sprites/floor_5.png" };
BGAME_DEFINE_SPRITE(spr_floor_6) = { .path = "/assets/sprites/floor_6.png" };
BGAME_DEFINE_SPRITE(spr_floor_7) = { .path = "/assets/sprites/floor_7.png" };

CF_Shader shd_default;

void
load_assets(void) {
	bgame_asset_init(&predefined_assets, bgame_default_allocator);

	BGAME_FOREACH_DEFINED_ASSET(asset) {
		bgame_asset_load_def(predefined_assets, asset);
	}

	bgame_load_gfx_shader(&shd_default, default_vert_bytecode, default_frag_bytecode);
}

void
unload_assets(void) {
	bgame_asset_cleanup(&predefined_assets);
}

void
check_assets(void) {
	bgame_asset_check_bundle(predefined_assets);
}
