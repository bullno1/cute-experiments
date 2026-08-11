set(SOURCES
	"main.c"

	"scenes/scratch.c"
)
add_bgame_app(cute-experiments "${SOURCES}")

if (EMSCRIPTEN)
	target_link_libraries(cute-experiments PRIVATE bgame-emscripten-shell)

	target_link_options(cute-experiments PRIVATE
		-sMALLOC=emmalloc
		--preload-file "${CMAKE_CURRENT_LIST_DIR}/../assets@/assets"
	)
endif ()
