#ifndef BINEX_BASIC_WINDOW_HPP
#define BINEX_BASIC_WINDOW_HPP

#include <binex/widgets.hpp>

#include <lak/system/windowing/window.hpp>

#include <lak/imgui/basic_window.hpp>

namespace bex
{
	template<typename DERIVED>
	struct basic_window : public lak::basic_window<DERIVED>
	{
		lak::path_getter pgetter;
		MemoryEditor editor;
		bex::memory_viewer viewer;

		void file_menu()
		{
			if (auto res = pgetter(); res)
				static_cast<DERIVED *>(this)->open_file(*res);

			if (ImGui::BeginMenu("File"))
			{
				if (ImGui::MenuItem("Open...", nullptr, false))
					pgetter.open_file(static_cast<DERIVED *>(this)->file_path());
				ImGui::EndMenu();
			}
		}

		void menu_bar(float)
		{
			static_cast<DERIVED *>(this)->file_menu();
			bex::debug_menu();
		}

		void left_region(float)
		{
			lak::span<byte_t> binary = static_cast<DERIVED *>(this)->file_data();
			editor.DrawContents(reinterpret_cast<uint8_t *>(binary.data()),
			                    binary.size());
		}

		void right_region(float)
		{
			lak::span<byte_t> binary = static_cast<DERIVED *>(this)->file_data();
			viewer.draw(binary, static_cast<DERIVED *>(this)->update());
		}
	};
}

#endif
