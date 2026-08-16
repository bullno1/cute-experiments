#include <cute.h>
#include "../assets.h"
#define BGAME_SCENE_NAME dungeon
#include <bgame/utils.h>

#define DUNGEON_WIDTH  16
#define DUNGEON_HEIGHT 16

#define TILE_SIZE 3.f

#define INTERPOLATION_SPED 15.f

typedef enum {
	DUNGEON_TILE_FLOOR = 0,
	DUNGEON_TILE_WALL,
} dungeon_tile_t;

typedef struct {
	int width;
	int height;

	dungeon_tile_t tiles[];
} dungeon_t;

typedef struct {
	int x;
	int y;
} dungeon_pos_t;

typedef enum {
	DIR_NORTH,
	DIR_EAST,
	DIR_SOUTH,
	DIR_WEST,
} direction_t;

SCENE_VAR(CF_V3, cam_pos)
SCENE_VAR(dungeon_t*, dungeon)

SCENE_VAR(dungeon_pos_t, char_pos)
SCENE_VAR(direction_t, char_dir)
static bool should_rebuild_dungeon = true;  // Not a scene var so it rebuilds on each reload

SCENE_VAR(CF_DrawList, dungeon_mesh)

static void
init(void) {
	cf_clear_color(0.5f, 0.5f, 0.5f, 0.5f);

	if (bgame_current_scene_state() == BGAME_SCENE_INITIALIZING) {
		dungeon = bgame_malloc(sizeof(dungeon_t) + sizeof(dungeon_tile_t) * DUNGEON_WIDTH * DUNGEON_HEIGHT, scene_allocator);
		dungeon->width = DUNGEON_WIDTH;
		dungeon->height = DUNGEON_HEIGHT;
		for (int y = 0; y < DUNGEON_HEIGHT; ++y) {
			for (int x = 0; x < DUNGEON_WIDTH; ++x) {
				dungeon->tiles[x + y * dungeon->width] = DUNGEON_TILE_FLOOR;
			}
		}

		dungeon_mesh = cf_make_draw_list();
	}

	cam_pos.y = TILE_SIZE * 0.5f;
}

static void
cleanup(void) {
	bgame_free(dungeon, scene_allocator);
	cf_destroy_draw_list(dungeon_mesh);
}

static void
draw_wall(float rotate) {
	CF_Sprite sprite = *spr_wall;
	sprite.scale = cf_v2(TILE_SIZE / 32.f);
	BGAME_SCOPE(cf_draw3d_push(), cf_draw3d_pop()) {
		cf_draw3d_rotate(cf_quat_from_axis_angle(cf_v3(0.f, 1.f, 0.f), rotate));
		cf_draw3d_sprite(&sprite, cf_v3(0, TILE_SIZE * 0.5f, -TILE_SIZE * 0.5f));
	}
}

static void
draw_north_wall(void) {
	draw_wall(0.f);
}

static void
draw_south_wall(void) {
	draw_wall(CF_PI);
}

static void
draw_west_wall(void) {
	draw_wall(CF_PI * 0.5f);
}

static void
draw_east_wall(void) {
	draw_wall(CF_PI * -0.5f);
}

static void
fixed_update(void* userdata) {
	CF_V3 cam_target = {
		.x = char_pos.x * TILE_SIZE,
		.y = TILE_SIZE * 0.5f,
		.z = char_pos.y * TILE_SIZE,
	};

	CF_V3 cam_delta = cf_mul(cf_sub(cam_target, cam_pos), 1.f - cf_exp(-INTERPOLATION_SPED * CF_DELTA_TIME_FIXED));
	cam_pos = cf_add(cam_pos, cam_delta);
}

static void
update(void) {
	cf_app_update(fixed_update);

	int w, h;
	cf_app_get_size(&w, &h);

	if (should_rebuild_dungeon) {
		cf_draw_list_begin(dungeon_mesh);

		for (int y = 0; y < dungeon->width; ++y) {
			for (int x = 0; x < dungeon->height; ++x) {
				BGAME_SCOPE(cf_draw3d_push(), cf_draw3d_pop()) {
					cf_draw3d_translate(cf_v3(x * TILE_SIZE, 0, y * TILE_SIZE));
					if (x == 0) { draw_west_wall(); }
					if (x == dungeon->width - 1) { draw_east_wall(); }

					if (y == 0) { draw_north_wall(); }
					if (y == dungeon->height - 1) { draw_south_wall(); }

					switch (dungeon->tiles[x + y * dungeon->width]) {
						case DUNGEON_TILE_FLOOR: {
							CF_Sprite sprite = *spr_floor;
							sprite.scale = cf_v2(TILE_SIZE / (float)sprite.w);
							BGAME_SCOPE(cf_draw3d_push(), cf_draw3d_pop()) {
								cf_draw3d_rotate(cf_quat_from_axis_angle(cf_v3(-1.f, 0.f, 0.f), CF_PI * 0.5f));
								cf_draw3d_sprite(&sprite, cf_v3(0.f, 0.f, 0.f));
							}
						} break;
						case DUNGEON_TILE_WALL: {
						} break;
					}
				}
			}
		}

		cf_draw_list_end();

		should_rebuild_dungeon = false;
	}

	CF_V3 cam_look_dir;

	switch (char_dir) {
		case DIR_NORTH:
			cam_look_dir = cf_v3(0, 0, -1);
			break;
		case DIR_SOUTH:
			cam_look_dir = cf_v3(0, 0, 1);
			break;
		case DIR_EAST:
			cam_look_dir = cf_v3(1, 0, 0);
			break;
		case DIR_WEST:
			cam_look_dir = cf_v3(-1, 0, 0);
			break;
	}
	cam_look_dir.y = -0.04f;

	BGAME_SCOPE(cf_draw3d_push_projection(cf_perspective(CF_PI * 0.5f, (float)w / (float)h, 0.1f, 100.f)), cf_draw3d_pop_projection())
	BGAME_SCOPE(cf_draw3d_push_view(cf_look_at(cam_pos, cf_add(cam_pos, cam_look_dir), cf_v3(0, 1, 0))), cf_draw3d_pop_view())
	BGAME_SCOPE(cf_draw3d_push_shader(shd_default), cf_draw3d_pop_shader())
	{
		cf_draw_list(dungeon_mesh);
	}

	dungeon_pos_t dir_forward = { 0 };
	dungeon_pos_t dir_right = { 0 };
	switch (char_dir) {
		case DIR_NORTH:
			dir_forward.y = -1;
			dir_right.x = 1;
			break;
		case DIR_SOUTH:
			dir_forward.y = 1;
			dir_right.x = -1;
			break;
		case DIR_EAST:
			dir_forward.x = 1;
			dir_right.y = 1;
			break;
		case DIR_WEST:
			dir_forward.x = -1;
			dir_right.y = -1;
			break;
	}

	dungeon_pos_t move_dir = { 0 };

	if (cf_key_just_pressed(CF_KEY_W)) {
		move_dir = dir_forward;
	}

	if (cf_key_just_pressed(CF_KEY_S)) {
		move_dir = (dungeon_pos_t){
			.x = -dir_forward.x,
			.y = -dir_forward.y,
		};
	}

	if (cf_key_just_pressed(CF_KEY_A)) {
		move_dir = (dungeon_pos_t){
			.x = -dir_right.x,
			.y = -dir_right.y,
		};
	}

	if (cf_key_just_pressed(CF_KEY_D)) {
		move_dir = dir_right;
	}

	if (cf_key_just_pressed(CF_KEY_Q)) {
		char_dir = (char_dir - 1 + 4) % 4;
	}

	if (cf_key_just_pressed(CF_KEY_E)) {
		char_dir = (char_dir + 1) % 4;
	}

	char_pos.x += move_dir.x;
	char_pos.y += move_dir.y;

	char_pos.x = cf_clamp(char_pos.x, 0, DUNGEON_WIDTH - 1);
	char_pos.y = cf_clamp(char_pos.y, 0, DUNGEON_HEIGHT - 1);

	cf_app_draw_onto_screen(true);
}

SCENE {
	.init = init,
	.update = update,
	.cleanup = cleanup,
};
