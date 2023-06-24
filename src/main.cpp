#include "main.hpp"

#include "binex/basic_window.hpp"

#include <lak/imgui/widgets.hpp>

#include <lak/file.hpp>
#include <lak/strconv.hpp>
#include <lak/test.hpp>

#include <filesystem>

int opengl_major, opengl_minor;
lak::graphics_mode graphics_mode;
bool force_only_error = false;

struct main_window : bex::basic_window<main_window>
{
	static void menu_bar(float frame_time) { ImGui::Text("%f", frame_time); }
};

lak::optional<int> basic_window_preinit(int argc, char **argv)
{
	if (argc == 2 && argv[1] == lak::astring("--version"))
	{
		std::cout << APP_NAME << "\n";
		return lak::optional<int>(0);
	}

	lak::debugger.std_out(u8"", u8"" APP_NAME "\n");

	for (int arg = 1; arg < argc; ++arg)
	{
		if (argv[arg] == lak::astring("-h") || argv[arg] == lak::astring("--help"))
		{
			std::cout << "binex.exe "
			             "[--help] "
			             "[--nogl] "
			             "[--onlyerr] "
			             "[--listtests | --laktestall | --laktests \"test1;test2\"] "
			             "[<filepath>]\n";

			return lak::optional<int>(0);
		}
		else if (argv[arg] == lak::astring("--nogl"))
		{
			basic_window_force_software = true;
		}
		else if (argv[arg] == lak::astring("--onlyerr"))
		{
			force_only_error = true;
		}
		else if (argv[arg] == lak::astring("--listtests"))
		{
			lak::debugger.std_out(lak::u8string(),
			                      lak::u8string(u8"Available tests:\n"));
			for (const auto &[name, func] : lak::registered_tests())
			{
				lak::debugger.std_out(lak::u8string(),
				                      lak::to_u8string(name) + u8"\n");
			}
		}
		else if (argv[arg] == lak::astring("--laktestall"))
		{
			return lak::optional<int>(lak::run_tests());
		}
		else if (argv[arg] == lak::astring("--laktests") ||
		         argv[arg] == lak::astring("--laktest"))
		{
			++arg;
			if (arg >= argc) FATAL("Missing tests");
			return lak::optional<int>(lak::run_tests(
			  lak::as_u8string(lak::astring_view::from_c_str(argv[arg]))));
		}
		else
		{
			if (lak::path_exists(argv[arg]).UNWRAP())
			{
				// :TODO: do something with the file
			}
			else
				FATAL("file ", argv[arg], " does not exists");
		}
	}

#ifdef LAK_OS_APPLE
	basic_window_force_software = true;
#endif

	basic_window_target_framerate      = 30;
	basic_window_opengl_settings.major = 3;
	basic_window_opengl_settings.minor = 2;
	basic_window_clear_colour          = {0.0f, 0.0f, 0.0f, 1.0f};

	return lak::nullopt;
}

void basic_window_init(lak::window &window)
{
	lak::debugger.crash_path =
	  std::filesystem::current_path() /
	  "ATTACH-TO-ISSUE-ON-SOURCE-EXPLORER-GITHUB-REPO.txt";

	lak::debugger.live_output_enabled = true;

	graphics_mode = window.graphics();

	DEBUG("Graphics: ", graphics_mode);
	if (!lak::debugger.live_output_enabled || lak::debugger.live_errors_only)
		std::cout << "Graphics: " << graphics_mode << "\n";

	switch (graphics_mode)
	{
		case lak::graphics_mode::OpenGL:
		{
			opengl_major = lak::opengl::get_uint(GL_MAJOR_VERSION).UNWRAP();
			opengl_minor = lak::opengl::get_uint(GL_MINOR_VERSION).UNWRAP();
		}
		break;

		default:
			break;
	}
}

void basic_window_handle_event(lak::window &, lak::event &event)
{
	switch (event.type)
	{
		case lak::event_type::dropfile:
			// :TODO: do something with the file
			// event.dropfile().path;
			break;

		default:
			break;
	}
}

void basic_window_loop(lak::window &, uint64_t counter_delta)
{
	const float frame_time = (float)counter_delta / lak::performance_frequency();

	main_window::draw(frame_time);
}

int basic_window_quit(lak::window &) { return 0; }
