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
	DEFINE_PACKAGE
)
target_link_libraries(game PRIVATE topaz)