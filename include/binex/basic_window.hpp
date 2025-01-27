#ifndef BINEX_BASIC_WINDOW_HPP
#define BINEX_BASIC_WINDOW_HPP

#include <binex/widgets.hpp>

#include <lak/window.hpp>

#include <lak/imgui/basic_window.hpp>

namespace bex
{
	template<typename DERIVED>
	struct basic_window : public lak::basic_window<DERIVED>
	{
		static void file_menu()
		{
			static lak::path_getter pgetter;
			if (auto res = pgetter(); res) DERIVED::open_file(*res);

			if (ImGui::BeginMenu("File"))
			{
				if (ImGui::MenuItem("Open...", nullptr, false))
					pgetter.open_file(DERIVED::file_path());
				ImGui::EndMenu();
			}
		}

		static void menu_bar(float)
		{
			DERIVED::file_menu();
			bex::debug_menu();
		}

		static void left_region(float)
		{
			static MemoryEditor editor;
			lak::span<byte_t> binary = DERIVED::file_data();
			editor.DrawContents(reinterpret_cast<uint8_t *>(binary.data()),
			                    binary.size());
		}

		static void right_region(float)
		{
			lak::span<byte_t> binary = DERIVED::file_data();
			bex::memory_view(binary, DERIVED::graphics_mode(), DERIVED::update());
		}
	};
}

#endif
