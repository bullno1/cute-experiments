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
SCENE_VAR(CF_Quat, cam_rot)
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
		cam_rot = cf_quat_identity();
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
		cf_draw3d_sprite(&sprite, cf_v3(0, TILE_SIZE * 0.5f, TILE_SIZE * 0.5f));
	}
}

static void
draw_north_wall(void) {
	draw_wall(CF_PI * 1.f);
}

static void
draw_south_wall(void) {
	draw_wall(CF_PI * 0.f);
}

static void
draw_west_wall(void) {
	draw_wall(CF_PI * -0.5f);
}

static void
draw_east_wall(void) {
	draw_wall(CF_PI * 0.5f);
}

static void
fixed_update(void* userdata) {
	// Smooth camera movement
	float t = 1.f - cf_exp(-INTERPOLATION_SPED * CF_DELTA_TIME_FIXED);

	CF_V3 cam_pos_target = {
		.x = char_pos.x * TILE_SIZE,
		.y = TILE_SIZE * 0.5f,
		.z = char_pos.y * TILE_SIZE,
	};
	cam_pos = cf_lerp(cam_pos, cam_pos_target, t);

	float angle;
	switch (char_dir) {
		case DIR_NORTH:
			angle = CF_PI * 0.f;
			break;
		case DIR_SOUTH:
			angle = CF_PI * 1.f;
			break;
		case DIR_EAST:
			angle = CF_PI * 0.5f;
			break;
		case DIR_WEST:
			angle = CF_PI * 1.5f;
			break;
	}
	CF_Quat cam_rot_target = cf_quat_from_axis_angle(cf_v3(0, -1, 0), angle);
	cam_rot = cf_quat_norm(cf_quat_slerp(cam_rot, cam_rot_target, t));
}

static dungeon_tile_t
get_tile(dungeon_t* dgn, dungeon_pos_t pos) {
	// Outer wall
	if (pos.x < 0 || pos.x >= dgn->width || pos.y < 0 || pos.y >= dgn->height) {
		return DUNGEON_TILE_WALL;
	}

	return dgn->tiles[pos.x + pos.y * dgn->width];
}

static bool
set_tile(dungeon_t* dgn, dungeon_pos_t pos, dungeon_tile_t tile) {
	if (pos.x < 0 || pos.x >= dgn->width || pos.y < 0 || pos.y >= dgn->height) {
		return false;
	}

	dgn->tiles[pos.x + pos.y * dgn->width] = tile;
	return true;
}

static void
update(void) {
	cf_app_update(fixed_update);

	int w, h;
	cf_app_get_size(&w, &h);

	if (should_rebuild_dungeon) {
		cf_draw_list_begin(dungeon_mesh);

		// Draw outer wall too
		for (int y = -1; y <= dungeon->height; ++y) {
			for (int x = -1; x <= dungeon->width; ++x) {
				BGAME_SCOPE(cf_draw3d_push(), cf_draw3d_pop()) {
					cf_draw3d_translate(cf_v3(x * TILE_SIZE, 0, y * TILE_SIZE));

					switch (get_tile(dungeon, (dungeon_pos_t){ x, y })) {
						case DUNGEON_TILE_FLOOR: {
							CF_Sprite sprite = *spr_floor;
							sprite.scale = cf_v2(TILE_SIZE / (float)sprite.w);
							BGAME_SCOPE(cf_draw3d_push(), cf_draw3d_pop()) {
								cf_draw3d_rotate(cf_quat_from_axis_angle(cf_v3(-1.f, 0.f, 0.f), CF_PI * 0.5f));
								cf_draw3d_sprite(&sprite, cf_v3(0.f, 0.f, 0.f));
							}
						} break;
						case DUNGEON_TILE_WALL: {
							if (get_tile(dungeon, (dungeon_pos_t){ x - 1, y    }) != DUNGEON_TILE_WALL) {
								draw_west_wall();
							}

							if (get_tile(dungeon, (dungeon_pos_t){ x + 1, y    }) != DUNGEON_TILE_WALL) {
								draw_east_wall();
							}

							if (get_tile(dungeon, (dungeon_pos_t){ x    , y + 1 }) != DUNGEON_TILE_WALL) {
								draw_south_wall();
							}

							if (get_tile(dungeon, (dungeon_pos_t){ x    , y - 1 }) != DUNGEON_TILE_WALL) {
								draw_north_wall();
							}
						} break;
					}
				}
			}
		}

		cf_draw_list_end();

		should_rebuild_dungeon = false;
	}

	CF_M4x4 cam_transform = cf_m4_from_trs(cam_pos, cam_rot, cf_v3(1, 1, 1));
	float aspect = (float)w / (float)h;
	float hfov   = 1.6581f;                      /* 95 degrees */
	float vfov   = 2.0f * atanf(tanf(hfov * 0.5f) / aspect);
	BGAME_SCOPE(cf_draw3d_push_projection(cf_perspective(vfov, aspect, 0.1f, 100.f)), cf_draw3d_pop_projection())
	BGAME_SCOPE(cf_draw3d_push_view(cf_m4_invert(cam_transform)), cf_draw3d_pop_view())
	BGAME_SCOPE(cf_draw3d_push_shader(shd_default), cf_draw3d_pop_shader())
	{
		cf_draw_list(dungeon_mesh);
	}

	// Overlay
	if (cf_key_down(CF_KEY_LSHIFT)) {
	}

	// Camera control
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

	dungeon_pos_t target_pos = {
		.x = char_pos.x + move_dir.x,
		.y = char_pos.y + move_dir.y,
	};
	if (get_tile(dungeon, target_pos) == DUNGEON_TILE_FLOOR) {
		char_pos = target_pos;
	}

	// Wall/Floor toggle
	if (cf_key_just_pressed(CF_KEY_SPACE)) {
		dungeon_pos_t coord_in_front = {
			.x = char_pos.x + dir_forward.x,
			.y = char_pos.y + dir_forward.y,
		};
		dungeon_tile_t tile_in_front = get_tile(dungeon, coord_in_front);
		should_rebuild_dungeon = set_tile(
			dungeon,
			coord_in_front,
			tile_in_front == DUNGEON_TILE_WALL ? DUNGEON_TILE_FLOOR : DUNGEON_TILE_WALL
		);
	}

	cf_app_draw_onto_screen(true);
}

SCENE {
	.init = init,
	.update = update,
	.cleanup = cleanup,
};
