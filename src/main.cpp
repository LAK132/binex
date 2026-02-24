#include "main.hpp"

#include "binex/basic_window.hpp"
#include "binex/widgets.hpp"

#include <lak/imgui/widgets.hpp>

#include <lak/future.hpp>
#include <lak/strconv.hpp>
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
	bool binary_update;

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
		binary_load = lak::async([](main_window *w, const lak::fs::path &p)
		                         { w->_load_binary(p); },
		                         this,
		                         path);
	}

	const lak::fs::path &file_path() { return binary_path; }

	lak::span<byte_t> file_data() { return lak::span(binary); }

	bool update() { return binary_update; }

	float time_acc = 0.0f;

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

lak::error_code<int> LAK_BASIC_PROGRAM(program_preinit)(lak::span<char *> args)
{
	if (args.size() == 2 && args[1] == lak::astring("--version"))
	{
		std::cout << APP_NAME << "\n";
		return lak::err_t{0};
	}

	lak::debugger.std_out(u8"", u8"" APP_NAME "\n");

	lak::debugger.crash_path = std::filesystem::current_path() /
	                           "ATTACH-TO-ISSUE-ON-BINEX-GITHUB-REPO.txt";

	lak::debugger.live_output_enabled = true;

	for (int arg = 1; arg < args.size(); ++arg)
	{
		if (args[arg] == lak::astring("-h") || args[arg] == lak::astring("--help"))
		{
			std::cout << "binex "
			             "[--help] "
			             "[--nogl] "
			             "[--onlyerr] "
			             "[--listtests | --laktestall | --laktests \"test1;test2\"] "
			             "[<filepath>]\n";

			return lak::err_t{0};
		}
		else if (args[arg] == lak::astring("--nogl"))
		{
			basic_window_force_software = true;
		}
		else if (args[arg] == lak::astring("--onlyerr"))
		{
			lak::debugger.live_errors_only = true;
		}
		else if (args[arg] == lak::astring("--listtests"))
		{
			lak::debugger.std_out(lak::u8string(),
			                      lak::u8string(u8"Available tests:\n"));
			for (const auto &[name, func] : lak::registered_tests())
			{
				lak::debugger.std_out(lak::u8string(),
				                      lak::to_u8string(name) + u8"\n");
			}
		}
		else if (args[arg] == lak::astring("--laktestall"))
		{
			return lak::err_t{lak::run_tests()};
		}
		else if (args[arg] == lak::astring("--laktests") ||
		         args[arg] == lak::astring("--laktest"))
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

#ifdef LAK_OS_APPLE
	basic_window_force_software = true;
#endif

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

	RES_TRY_ASSIGN(
	  my_window_ptr =,
	  LAK_BASIC_PROGRAM(create_window<my_window>)().map_err(map_str_err));

	{
		auto ptr = my_window_ptr.get();
		ASSERT(!!ptr);
		ptr->clear_colour = {0.0f, 0.0f, 0.0f, 1.0f};
		ptr->imgui_window_flags =
		  ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_NoScrollbar |
		  ImGuiWindowFlags_MenuBar | ImGuiWindowFlags_NoSavedSettings |
		  ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoMove;
		DEBUG_EXPR(ptr->window().graphics());
	}

	return lak::ok_t{};
}

void LAK_BASIC_PROGRAM(program_handle_event)(lak::event &event)
{
	switch (event.type)
	{
		case lak::event_type::quit_program:
			if (auto ptr = my_window_ptr.get(); ptr)
			{
				ptr->destroy();
				my_window_ptr.reset();
			}
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
