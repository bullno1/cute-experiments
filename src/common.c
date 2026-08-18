#include "common.h"
#include <bgame/reloadable.h>
#include <bgame/allocator/frame.h>
#include <bgame/scene.h>
#include <cute.h>
#include <dcimgui.h>

static bool debug_on = false;
BGAME_PERSIST_VAR_EX(common, debug_on)

void
common(void) {
	if (cf_key_just_pressed(CF_KEY_F12)) {
		debug_on = !debug_on;
	}

	if (debug_on) {
		if (ImGui_Begin("Debug", &debug_on, ImGuiWindowFlags_AlwaysAutoResize)) {
			static int scene_id;
			int num_scenes;
			bgame_scene_reg_t* scenes;
			bgame_list_scenes(&scenes, &num_scenes);

			const char** scene_names = bgame_alloc_for_frame(sizeof(char*) * num_scenes, _Alignof(char*));
			bgame_scene_reg_t current_scene = bgame_current_scene();
			for (int i = 0; i < num_scenes; ++i) {
				if (scenes[i].name == current_scene.name) {  // Names are interned
					scene_id = i;
				}

				scene_names[i] = scenes[i].name;
			}

			if (ImGui_ComboChar("Scene", &scene_id, scene_names, num_scenes)) {
				bgame_switch_scene(scene_names[scene_id]);
			}
		}
		ImGui_End();
	}
}
