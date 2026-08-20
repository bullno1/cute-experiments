#include <cute.h>
#include <blog.h>
#define BGAME_SCENE_NAME particle
#include <bgame/utils.h>
#include "../common.h"
#include <grain.h>

SCENE_VAR(grain_t*, grain)

static void
init(void) {
	cf_clear_color(0.5f, 0.5f, 0.5f, 0.5f);

	if (bgame_current_scene_state() == BGAME_SCENE_INITIALIZING) {
		grain = grain_create();
	}

	grain_emitter_t* point_emitter = grain_define_emitter(
		grain,
		"Module(Point)\n"
		"Requires(\n"
		"	vec2 position;\n"
		"	vec2 velocity;\n"
		")\n"
		"Params(\n"
		"	vec2 position;\n"
		"	float min_speed;\n"
		"	float max_speed;\n"
		"	float min_angle;\n"
		"	float max_angle;\n"
		")\n"
		"void process(out Particle particle, ModuleParams params, Ctx ctx) {\n"
		"	particle.position = params.position;\n"
		"	float speed = mix(params.min_speed, params.max_speed, rand());\n"
		"	float angle = mix(params.min_angle, params.max_angle, rand());\n"
		"	particle.velocity = vec2(cos(angle), sin(angle)) * speed;\n"
		"}"
	);
	if (point_emitter == NULL) {
		BLOG_ERROR("%s", grain_get_last_error(grain));
	}

	grain_emitter_t* age_init = grain_define_emitter(
		grain,
		"Module(Age)\n"
		"Requires(\n"
		"	float age;\n"
		")\n"
		"Params(\n"
		"	float min_age;\n"
		"	float max_age;\n"
		")\n"
		"void process(out Particle particle, ModuleParams params, Ctx ctx) {\n"
		"	particle.age = mix(params.min_age, params.max_age, rand());\n"
		"}"
	);
	if (age_init == NULL) {
		BLOG_ERROR("%s", grain_get_last_error(grain));
	}

	grain_affector_t* age_affector = grain_define_affector(
		grain,
		"Module(Age)\n"
		"Requires(\n"
		"	float age;\n"
		")\n"
		"Params(\n"
		")\n"
		"void process(inout Particle particle, ModuleParams params, Ctx ctx) {\n"
		"	particle.age -= ctx.dt;\n"
		"	if (particle.age <= 0.0) { destroy(); }\n"
		"}"
	);
	if (age_affector == NULL) {
		BLOG_ERROR("%s", grain_get_last_error(grain));
	}

	grain_renderer_t* renderer = grain_define_renderer(
		grain,
		"Module(Quad)\n"
		"Requires(\n"
		"	vec2 position;\n"
		")\n"
		"Params(\n"
		"	vec2 size;\n"
		"	uint color;\n"
		")\n"
		"#if GRAIN_SHADER_STAGE == GRAIN_SHADER_STAGE_VERTEX\n"
		"void process(in Particle particle, ModuleParams params, Ctx ctx) {\n"
		"	gl_Position.xy = particle.position + quad() * params.size;\n"
		"}\n"
		"#elif GRAIN_SHADER_STAGE == GRAIN_SHADER_STAGE_FRAGMENT\n"
		"void process(inout Particle particle, ModuleParams params, Ctx ctx) {\n"
		"	grain_Color = unpackUnorm4x8(params.color);\n"
		"}\n"
		"#endif"
	);
	if (renderer == NULL) {
		BLOG_ERROR("%s", grain_get_last_error(grain));
	}

	grain_archetype_t* archetype = grain_define_archetype(
		grain,
		"test",
		(grain_archetype_spec_t){
			.emitters = (grain_emitter_t*[]){
				point_emitter,
				age_init,
			},
			.num_emitters = 2,

			.affectors = &age_affector,
			.num_affectors = 1,

			.renderer = renderer,
		}
	);
	if (archetype == NULL) {
		BLOG_ERROR("%s", grain_get_last_error(grain));
	}
}

static void
cleanup(void) {
	grain_destroy(grain);
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
