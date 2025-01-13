cmake_minimum_required(VERSION 3.28)
add_subdirectory(engine)

project(game
	DESCRIPTION "a really good game for sure"
	LANGUAGES CXX
)
set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)


topaz_add_executable(
	TARGET game
	FAVICON game.ico
	SOURCES
		src/main.cpp
		src/card.cpp
		src/deck.cpp
		src/entity.cpp
		src/level.cpp
		src/player.cpp
		src/prefab.cpp
		src/render.cpp
		src/script.cpp
		src/ui.cpp
		src/save.cpp
	SHADERS
		src/quad_bloom.fragment.tzsl
		src/bloom_blur.vertex.tzsl
		src/bloom_blur.fragment.tzsl
	BUNDLE_DIRECTORIES
		res/
		script/
	DEFINE_PACKAGE
)
target_link_libraries(game PRIVATE topaz)
