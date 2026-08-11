#include <bgame/scene.h>
#include <bgame/allocator/tracked.h>
#include <bgame/utils.h>
#include <cute.h>
#include "../assets.h"

#define SCENE_NAME dungeon
#define SCENE_VAR(TYPE, NAME) BGAME_PRIVATE_VAR(SCENE_NAME, TYPE, NAME)
BGAME_DECLARE_SCENE_ALLOCATOR(SCENE_NAME)

#define DUNGEON_WIDTH  64
#define DUNGEON_HEIGHT 64

#define TILE_SIZE 3.f

typedef enum {
	DUNGEON_TILE_FLOOR,
	DUNGEON_TILE_WALL,
} dungeon_tile_t;

SCENE_VAR(CF_V3, cam_pos)

static void
init(void) {
	cf_clear_color(0.5f, 0.5f, 0.5f, 0.5f);

	cam_pos = cf_v3(0.f, 1.5f, 0.f);
}

static void
cleanup(void) {
}

static void
fixed_update(void* userdata) {
}

static void
update(void) {
	cf_app_update(fixed_update);

	int w, h;
	cf_app_get_size(&w, &h);

	BGAME_SCOPE(cf_draw3d_push_projection(cf_perspective(CF_PI * 0.5f, (float)w / (float)h, 1.f, 100.f)), cf_draw3d_pop_projection())
	BGAME_SCOPE(cf_draw3d_push_view(cf_look_at(cam_pos, cf_v3(0, 1.5f, -1), cf_v3(0, 1, 0))), cf_draw3d_pop_view())
	BGAME_SCOPE(cf_draw3d_push_shader(shd_default), cf_draw3d_pop_shader())
	{
		CF_Sprite sprite = *spr_wall;
		sprite.scale = cf_v2(TILE_SIZE / (float)sprite.w);
		cf_draw3d_sprite(&sprite, cf_v3(0, 0, -3));
	}

	cf_app_draw_onto_screen(true);
}

BGAME_SCENE(SCENE_NAME) = {
	.init = init,
	.update = update,
	.cleanup = cleanup,
};
