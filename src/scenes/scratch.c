#include <cute.h>
#define BGAME_SCENE_NAME scratch
#include <bgame/utils.h>
#include "../common.h"

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
	common();

	cf_app_draw_onto_screen(true);
}

SCENE {
	.init = init,
	.update = update,
	.cleanup = cleanup,
};
