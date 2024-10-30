#include "tz/topaz.hpp"
#include "tz/os/window.hpp"

int main()
{
	tz::initialise();
	tz::os::open_window({.title = "My Amazing Game"});
	while(tz::os::window_is_open())
	{
		tz::os::window_update();
	}
	tz::terminate();
}