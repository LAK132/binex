#include "main.hpp"

#include "binex/basic_window.hpp"
#include "binex/widgets.hpp"

#include <lak/imgui/texture.hpp>
#include <lak/imgui/widgets.hpp>

#include <lak/future.hpp>
#include <lak/strconv.hpp>
#include <lak/string_literals/string.hpp>
#include <lak/string_literals/view.hpp>
#include <lak/system/file.hpp>
#include <lak/test.hpp>

#include <filesystem>

#define LAK_BASIC_PROGRAM_IMGUI_WINDOW_IMPL
#include <lak/basic_program.inl>

struct main_window : bex::basic_window<main_window>
{
	using super_window = bex::basic_window<main_window>;

	lak::fs::path binary_path;
	lak::optional<lak::future<void>> binary_load;
	lak::array<byte_t> binary;
	bool binary_update = false;
	float time_acc     = 0.0f;

	void _load_binary(lak::fs::path path)
	{
		if (auto res = lak::read_file(path); res.is_ok())
			binary = lak::move(res.unsafe_unwrap());
		else
			ERROR(res.unsafe_unwrap_err());
		binary_path = lak::move(path);
	}

	void open_file(const lak::fs::path &path)
	{
		binary_load =
		  lak::async([this](const lak::fs::path &p) { _load_binary(p); }, path);
	}

	const lak::fs::path &file_path() { return binary_path; }

	lak::span<byte_t> file_data() { return lak::span(binary); }

	bool update() { return binary_update; }

	void main_region(float frame_time)
	{
		if (binary_load)
		{
			ImGui::BeginChild(
			  "Mid", {-1, -1}, true, ImGuiWindowFlags_NoSavedSettings);
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

struct my_window : virtual public LAK_BASIC_PROGRAM(window_api)
{
	my_window() : LAK_BASIC_PROGRAM(window_api)() {}

	main_window bex_window;

	virtual void init() override final { window().set_title(L"" APP_NAME); }

	virtual ~my_window() {}

	virtual void handle_event(lak::event &event) override final
	{
		switch (event.type)
		{
			case lak::event_type::close_window: destroy(); break;
			case lak::event_type::dropfile:
				bex_window.open_file(event.dropfile().path);
				break;
		}
	}

	virtual void loop(uint64_t counter_delta) override final
	{
		const float frame_time =
		  (float)counter_delta / lak::performance_frequency();
		bex_window.draw(frame_time);
		if (bex_window.binary_update)
		{
			window().set_title(L"" APP_NAME " " +
			                   bex_window.binary_path.generic_wstring());
			bex_window.binary_update = false;
		}
	}
};

lak::graphics_mode forced_graphics_mode = lak::graphics_mode::None;

lak::error_code<int> LAK_BASIC_PROGRAM(program_preinit)(lak::span<char *> args)
{
	if (!args.empty()) args = args.subspan(1U);

	if (args.size() == 1U && args[0] == "--version"_str)
	{
		std::cout << APP_NAME << "\n";
		return lak::err_t{EXIT_SUCCESS};
	}

	lak::debugger.std_out(u8"", u8"" APP_NAME "\n");

	lak::debugger.crash_path = std::filesystem::current_path() /
	                           "ATTACH-TO-ISSUE-ON-BINEX-GITHUB-REPO.txt";

	lak::debugger.live_output_enabled = true;

	for (size_t arg = 0U; arg < args.size(); ++arg)
	{
		if (args[arg] == "-h"_str || args[arg] == "--help"_str)
		{
			std::cout << "binex "
			             "[--help] "
			             "[--software | --opengl | --cobalt] "
			             "[--onlyerr] "
			             "[--listtests | --laktestall | --laktests \"test1;test2\"] "
			             "[<filepath>]\n";

			return lak::err_t{0};
		}
		else if (args[arg] == "--software"_str)
		{
			forced_graphics_mode = lak::graphics_mode::Software;
		}
		else if (args[arg] == "--opengl"_str)
		{
			forced_graphics_mode = lak::graphics_mode::OpenGL;
		}
		else if (args[arg] == "--cobalt"_str)
		{
			forced_graphics_mode = lak::graphics_mode::Cobalt;
		}
		else if (args[arg] == "--onlyerr"_str)
		{
			lak::debugger.live_errors_only = true;
		}
		else if (args[arg] == "--listtests"_str)
		{
			lak::debugger.std_out(lak::u8string(),
			                      lak::u8string(u8"Available tests:\n"));
			for (const auto &[name, func] : lak::registered_tests())
			{
				lak::debugger.std_out(lak::u8string(),
				                      lak::to_u8string(name) + u8"\n");
			}
		}
		else if (args[arg] == "--laktestall"_str)
		{
			return lak::err_t{lak::run_tests()};
		}
		else if (args[arg] == "--laktests"_str || args[arg] == "--laktest"_str)
		{
			++arg;
			if (arg >= args.size()) FATAL("Missing tests");
			return lak::err_t{lak::run_tests(
			  lak::as_u8string(lak::astring_view::from_c_str(args[arg])))};
		}
		else
		{
			if (lak::path_exists(args[arg]).UNWRAP())
			{
				// :TODO: do something with the file
			}
			else
				FATAL("file ", args[arg], " does not exists");
		}
	}

	basic_window_target_framerate = 30;

	return lak::ok_t{};
}

lak::weak_ptr<LAK_BASIC_PROGRAM(window_instance<my_window>)> my_window_ptr;

lak::error_code<int> LAK_BASIC_PROGRAM(program_init)()
{
	auto map_str_err = [](lak::u8string err) -> int
	{
		ERROR(err);
		return EXIT_FAILURE;
	};

	switch (forced_graphics_mode)
	{
		case lak::graphics_mode::None:
		{
			RES_TRY_ASSIGN(
			  my_window_ptr =,
			  LAK_BASIC_PROGRAM(create_window<my_window>)().map_err(map_str_err));
		}
		break;
#ifdef LAK_ENABLE_SOFTRENDER
		case lak::graphics_mode::Software:
		{
			RES_TRY_ASSIGN(my_window_ptr =,
			               LAK_BASIC_PROGRAM(create_window<my_window>)(
			                 LAK_BASIC_PROGRAM(window_software_settings))
			                 .map_err(map_str_err));
		}
		break;
#endif
#ifdef LAK_ENABLE_OPENGL
		case lak::graphics_mode::OpenGL:
		{
			RES_TRY_ASSIGN(my_window_ptr =,
			               LAK_BASIC_PROGRAM(create_window<my_window>)(
			                 LAK_BASIC_PROGRAM(window_opengl_settings))
			                 .map_err(map_str_err));
		}
		break;
#endif
#ifdef LAK_ENABLE_COBALT
		case lak::graphics_mode::Cobalt:
		{
			RES_TRY_ASSIGN(my_window_ptr =,
			               LAK_BASIC_PROGRAM(create_window<my_window>)(
			                 LAK_BASIC_PROGRAM(window_cobalt_settings))
			                 .map_err(map_str_err));
		}
		break;
#endif
		default:
			ERROR(
			  lak::fmt<u8"Graphics mode {} not available">(forced_graphics_mode));
			return lak::err_t{EXIT_FAILURE};
	}

	DEBUG_EXPR(my_window_ptr.get()->window().graphics());

	return lak::ok_t{};
}

void LAK_BASIC_PROGRAM(program_handle_event)(lak::event &event)
{
	switch (event.type)
	{
		case lak::event_type::quit_program:
			for (auto &inst : basic_window_instances()) inst->destroy();
			break;

		default: break;
	}
}

bool LAK_BASIC_PROGRAM(program_loop)(uint64_t counter_delta)
{
	LAK_UNUSED(counter_delta);
	return !LAK_BASIC_PROGRAM(window_instances)().empty();
}

int LAK_BASIC_PROGRAM(program_quit)()
{
	my_window_ptr.reset();
	return EXIT_SUCCESS;
}
