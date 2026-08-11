#include <bgame/scene.h>
#include <bgame/allocator/tracked.h>
#include <cute.h>

#define SCENE_NAME scratch
#define SCENE_VAR(TYPE, NAME) BGAME_PRIVATE_VAR(SCENE_NAME, TYPE, NAME)
BGAME_DECLARE_SCENE_ALLOCATOR(SCENE_NAME)

static void
init(void) {
	cf_clear_color(0.5f, 0.5f, 0.5f, 0.5f);
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
	cf_app_draw_onto_screen(true);
}

BGAME_SCENE(scratch) = {
	.init = init,
	.update = update,
	.cleanup = cleanup,
};
