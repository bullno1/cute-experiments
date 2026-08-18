#include <cute.h>
#include "../assets.h"
#include "../common.h"
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

SCENE_VAR(dungeon_pos_t, enemy_pos)

SCENE_VAR(CF_DrawList, dungeon_mesh)

static CF_Sprite** floor_set[] = {
	&spr_floor_2,
	&spr_floor_3,
	&spr_floor_4,
	&spr_floor_5,
	&spr_floor_6,
	&spr_floor_7,
};

static uint64_t
fnv1a(const void *data, size_t len, uint64_t seed) {
	const uint8_t *p = (const uint8_t *)data;
	uint64_t hash = seed;

	for (size_t i = 0; i < len; ++i) {
		hash ^= p[i];
		hash *= UINT64_C(1099511628211);
	}

	return hash;
}

static void
init(void) {
	cf_clear_color(0.5f, 0.5f, 0.5f, 0.5f);
	cf_draw3d_mips(4);
	cf_draw3d_anisotropy(4);

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
		should_rebuild_dungeon = true;
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
	CF_V3 dir_forward = { 0 };
	switch (char_dir) {
		case DIR_NORTH:
			dir_forward.z = -1;
			break;
		case DIR_SOUTH:
			dir_forward.z = 1;
			break;
		case DIR_EAST:
			dir_forward.x = 1;
			break;
		case DIR_WEST:
			dir_forward.x = -1;
			break;
	}
	// Put the camera slightly backward from the center so it can see the tile boundary
	cam_pos_target = cf_add(cam_pos_target, cf_mul(dir_forward, -0.3f * TILE_SIZE));

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
	// Look slightly down so we can see the seam of the current tile
	CF_Quat cam_rot_target = cf_mul(
		cf_quat_from_axis_angle(cf_v3(0, -1, 0), angle),
		cf_quat_from_axis_angle(cf_v3(-1, 0, 0), 0.02f)
	);
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

	common();

	int w, h;
	cf_app_get_size(&w, &h);

	if (should_rebuild_dungeon) {
		cf_draw_list_begin(dungeon_mesh);

		// Draw outer wall too
		for (int y = -1; y <= dungeon->height; ++y) {
			for (int x = -1; x <= dungeon->width; ++x) {
				BGAME_SCOPE(cf_draw3d_push(), cf_draw3d_pop()) {
					cf_draw3d_translate(cf_v3(x * TILE_SIZE, 0, y * TILE_SIZE));

					dungeon_pos_t pos = { x, y };
					switch (get_tile(dungeon, pos)) {
						case DUNGEON_TILE_FLOOR: {
							// Pick sprite based on coordinate so it's random but still consistent
							CF_Sprite floor_sprite = **floor_set[fnv1a(&pos, sizeof(pos), 0) % CF_ARRAY_SIZE(floor_set)];
							floor_sprite.scale = cf_v2(TILE_SIZE / (float)floor_sprite.w);

							BGAME_SCOPE(cf_draw3d_push(), cf_draw3d_pop()) {
								cf_draw3d_rotate(cf_quat_from_axis_angle(cf_v3(-1.f, 0.f, 0.f), CF_PI * 0.5f));
								cf_draw3d_sprite(&floor_sprite, cf_v3(0.f, 0.f, 0.f));
							}

							CF_Sprite ceiling_sprite = **floor_set[fnv1a(&pos, sizeof(pos), 1) % CF_ARRAY_SIZE(floor_set)];
							ceiling_sprite.scale = cf_v2(TILE_SIZE / (float)ceiling_sprite.w);
							BGAME_SCOPE(cf_draw3d_push(), cf_draw3d_pop()) {
								cf_draw3d_translate(cf_v3(0.f, (float)TILE_SIZE, 0.f));;
								cf_draw3d_rotate(cf_quat_from_axis_angle(cf_v3(1.f, 0.f, 0.f), CF_PI * 0.5f));
								cf_draw3d_sprite(&ceiling_sprite, cf_v3(0.f, 0.f, 0.f));
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

		CF_V3 enemy_3d_pos = {
			.x = enemy_pos.x * TILE_SIZE,
			.y = 0.5f * TILE_SIZE + 0.05 + cf_sin(CF_SECONDS * 2.5f) * 0.02f,
			.z = enemy_pos.y * TILE_SIZE,
		};
		CF_Sprite enemy_sprite = *spr_enemy;
		enemy_sprite.scale = cf_v2(TILE_SIZE / 32.f);
		cf_draw3d_billboard(&enemy_sprite, enemy_3d_pos);

		// Shadow to make the position clearer
		BGAME_SCOPE(cf_draw3d_push_color(cf_make_color_rgba(0, 0, 0, 150)), cf_draw3d_pop_color()) {
			cf_draw3d_circle_fill(
				cf_v3(
					enemy_pos.x * TILE_SIZE,
					0.01f,  // Slightly above the floor
					enemy_pos.y * TILE_SIZE
				),
				cf_v3(0, 1, 0),
				TILE_SIZE * 0.3
			);
		}
	}

	// Overlay
	if (cf_key_down(CF_KEY_LSHIFT)) {
		BGAME_SCOPE(cf_draw_push_color(cf_make_color_rgba(255, 255, 255, 128)), cf_draw_pop_color())
		{
			float margin = 10;
			float available_height = (float)h - margin * 2.f;
			float cell_size = available_height / (float)dungeon->height;

			float start_x = -cell_size * 0.5f * (float)dungeon->width;
			float start_y = h * 0.5f - margin;

			for (int y = 0; y < dungeon->height; ++y) {
				for (int x = 0; x < dungeon->width; ++x) {
					CF_Aabb cell_box = cf_make_aabb_from_top_left(
						cf_v2(
							start_x + x * cell_size,
							start_y - y * cell_size
						),
						cell_size,
						cell_size
					);

					switch (get_tile(dungeon, (dungeon_pos_t){ x, y })) {
						case DUNGEON_TILE_WALL:
							cf_draw_box_fill(cell_box, 0.1f);
							break;
						case DUNGEON_TILE_FLOOR:
							cf_draw_box(cell_box, 0.1f, 0.1f);
							break;
					}
				}
			}

			CF_V2 arrow[] = {
				{  0.f             ,  cell_size * 0.35f },
				{ -cell_size * 0.3f, -cell_size * 0.3f },
				{  cell_size * 0.3f, -cell_size * 0.3f },
			};

			BGAME_SCOPE(cf_draw_push(), cf_draw_pop())
			{
				cf_draw_translate(
					start_x + cell_size * char_pos.x + cell_size * 0.5f,
					start_y - cell_size * char_pos.y - cell_size * 0.5f
				);
				switch (char_dir) {
					case DIR_NORTH:
						cf_draw_rotate(CF_PI * 0.0f);
						break;
					case DIR_EAST:
						cf_draw_rotate(CF_PI * 0.5f);
						break;
					case DIR_SOUTH:
						cf_draw_rotate(CF_PI * 1.0f);
						break;
					case DIR_WEST:
						cf_draw_rotate(CF_PI * 1.5f);
						break;
				}
				cf_draw_polygon_fill(arrow, CF_ARRAY_SIZE(arrow), 0.1f);
			}
		}
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
	dungeon_pos_t coord_in_front = {
		.x = char_pos.x + dir_forward.x,
		.y = char_pos.y + dir_forward.y,
	};
	dungeon_tile_t tile_in_front = get_tile(dungeon, coord_in_front);

	if (cf_key_just_pressed(CF_KEY_SPACE)) {
		should_rebuild_dungeon = set_tile(
			dungeon,
			coord_in_front,
			tile_in_front == DUNGEON_TILE_WALL ? DUNGEON_TILE_FLOOR : DUNGEON_TILE_WALL
		);
	}

	// Move enemy
	if (cf_key_just_pressed(CF_KEY_Z)) {
		if (tile_in_front == DUNGEON_TILE_FLOOR) {
			enemy_pos = coord_in_front;
		}
	}

	cf_app_draw_onto_screen(true);
}

SCENE {
	.init = init,
	.update = update,
	.cleanup = cleanup,
};
