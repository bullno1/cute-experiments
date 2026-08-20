set(SOURCES
	"main.c"
	"assets.c"
	"common.c"

	"scenes/scratch.c"
	"scenes/dungeon.c"
	"scenes/particle.c"

	"gen/default_vert_bytecode.h"
	"gen/default_frag_bytecode.h"
)
add_bgame_app(cute-experiments "${SOURCES}")

target_link_libraries(cute-experiments PRIVATE grain)

compile_vertex_shader(
	${CMAKE_CURRENT_SOURCE_DIR}/glsl/default.vert.glsl
	default_vert_bytecode
	${CMAKE_CURRENT_SOURCE_DIR}/gen/default_vert_bytecode.h
)
compile_fragment_shader(
	${CMAKE_CURRENT_SOURCE_DIR}/glsl/default.frag.glsl
	default_frag_bytecode
	${CMAKE_CURRENT_SOURCE_DIR}/gen/default_frag_bytecode.h
)

if (EMSCRIPTEN)
	target_link_libraries(cute-experiments PRIVATE bgame-emscripten-shell)

	target_link_options(cute-experiments PRIVATE
		-sMALLOC=emmalloc
		--preload-file "${CMAKE_CURRENT_LIST_DIR}/../assets@/assets"
	)
endif ()
