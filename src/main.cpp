#include "main.hpp"

#include "binex/basic_window.hpp"
#include "binex/widgets.hpp"

#include <lak/imgui/widgets.hpp>

#include <lak/opengl/state.hpp>

#include <lak/file.hpp>
#include <lak/future.hpp>
#include <lak/strconv.hpp>
#include <lak/test.hpp>

#include <filesystem>

int opengl_major, opengl_minor;
lak::graphics_mode graphics_mode;
bool force_only_error = false;

lak::fs::path binary_path;
lak::optional<lak::future<void>> binary_load;
lak::array<byte_t> binary;
bool binary_update;

void load_binary(lak::fs::path path)
{
	if (auto res = lak::read_file(path); res.is_ok())
		binary = lak::move(res.unsafe_unwrap());
	else
		ERROR(res.unsafe_unwrap_err());
	binary_path = lak::move(path);
}

void load_binary_async(const lak::fs::path &path)
{
	binary_load = lak::async(load_binary, path);
}

struct main_window : bex::basic_window<main_window>
{
	using super_window = bex::basic_window<main_window>;

	static void file_menu()
	{
		static lak::path_getter pgetter;
		if (auto res = pgetter(); res) load_binary_async(*res);

		if (ImGui::BeginMenu(ASDFGHJKL))
		{
			if (ImGui::MenuItem("Open...", nullptr, false, !binary_load))
				pgetter.open_file(binary_path);
			ImGui::EndMenu();
		}
	}

	static void menu_bar(float)
	{
		file_menu();
		bex::debug_menu();
	}

	static void left_region(float)
	{
		static MemoryEditor editor;
		editor.DrawContents(reinterpret_cast<uint8_t *>(binary.data()),
		                    binary.size());
	}

	static void right_region(float)
	{
		bex::memory_view(binary, graphics_mode, binary_update);
	}

	static void main_region(float frame_time)
	{
		if (binary_load)
		{
			ImGui::BeginChild(
			  "Mid", {-1, -1}, true, ImGuiWindowFlags_NoSavedSettings);
			static float time_acc = 0.0f;
			time_acc += frame_time;
			if (time_acc > 3.0f) time_acc -= std::trunc(time_acc);
			if (time_acc > 2.0f)
				ImGui::Text("Loading...");
			else if (time_acc > 1.0f)
				ImGui::Text("Loading..");
			else
				ImGui::Text("Loading.");

			if (binary_load->has_value())
			{
				binary_load.reset();
				binary_update = true;
				time_acc      = 0.0f;
			}
			ImGui::EndChild();
		}
		else if (binary.empty())
		{
			ImGui::BeginChild(
			  "Mid", {-1, -1}, true, ImGuiWindowFlags_NoSavedSettings);
			ImGui::Text("No file");
			ImGui::EndChild();
		}
		else
			super_window::main_region(frame_time);
	}
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

	window.set_title(L"binex");
}

void basic_window_handle_event(lak::window &, lak::event &event)
{
	switch (event.type)
	{
		case lak::event_type::dropfile:
			load_binary_async(lak::fs::path(event.dropfile().path));
			break;

		default:
			break;
	}
}

void basic_window_loop(lak::window &window, uint64_t counter_delta)
{
	const float frame_time = (float)counter_delta / lak::performance_frequency();

	main_window::draw(frame_time);

	if (binary_update)
	{
		window.set_title(L"binex " + binary_path.generic_wstring());
		binary_update = false;
	}
}

int basic_window_quit(lak::window &) { return 0; }
