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
	SOURCES
		src/main.cpp
		src/card.cpp
		src/creature.cpp
		src/render.cpp
		src/script.cpp
	SHADERS
		src/shaders/quad.vertex.tzsl
		src/shaders/quad.fragment.tzsl
	BUNDLE_FILES
		res/images/smile.png
		res/images/frown.png
		res/images/bgforest.png
		res/images/hourglassh.png
		res/images/hourglassv.png
		res/images/cardbase.png

		res/images/creature/peasant/idle0.png
		res/images/creature/peasant/idle1.png
		res/images/creature/peasant/side0.png
		res/images/creature/peasant/side1.png
		script/creatures/peasant.lua

		res/images/creature/knight/idle0.png
		res/images/creature/knight/idle1.png
		res/images/creature/knight/side0.png
		res/images/creature/knight/side1.png
		script/creatures/knight.lua

		res/images/creature/skeleton/idle0.png
		res/images/creature/skeleton/idle1.png
		res/images/creature/skeleton/side0.png
		res/images/creature/skeleton/side1.png
		script/creatures/skeleton.lua

		res/images/creature/melistra/idle0.png
		res/images/creature/melistra/idle1.png
		res/images/creature/melistra/side0.png
		res/images/creature/melistra/side1.png
		script/creatures/melistra.lua

		res/images/creature/banshee/idle0.png
		res/images/creature/banshee/idle1.png
		script/creatures/banshee.lua
	DEFINE_PACKAGE
)
target_link_libraries(game PRIVATE topaz)