#ifndef BINEX_WIDGETS_HPP
#define BINEX_WIDGETS_HPP

#include <lak/image.hpp>
#include <lak/span.hpp>
#include <lak/stdint.hpp>

#include <lak/system/windowing/window.hpp>

#include <imgui_memory_editor.h>

namespace bex
{
	struct memory_region_selector
	{
		uint64_t view_begin = 0;
		uint64_t view_size  = lak::dynamic_extent;
		bool fixed_size     = true;

		bool draw(lak::span<byte_t> data,
		          lak::span<byte_t> &view_data,
		          bool force_update);
	};

	enum class pixel_layout : uint8_t
	{
		mono,
		r,
		rg,
		rgb,
		bgr,
		rgbx,
		bgrx,
		xrgb,
		xbgr,
		bayer_rggb,
		bayer_bggr,
		bayer_grbg,
		bayer_gbrg,
	};

	struct memory_image_viewer
	{
		lak::vec2u64_t image_size = {256, 256};
		lak::vec3u64_t block_skip = {0, 1, 0};
		ImTextureRef texture;
		float scale                       = 1.0f;
		lak::array<int, 4> rgbx_bit_count = {8, 8, 8, 0};
		bex::pixel_layout pixel_layout    = bex::pixel_layout::rgb;
		lak::span<byte_t> old_data;
		lak::span<byte_t> image_data;
		bex::memory_region_selector view;

		~memory_image_viewer();

		void draw(lak::span<byte_t> data, bool force_update);
	};

	struct memory_byte_pairs_viewer
	{
		ImTextureRef texture;
		float scale = 1.0f;
		lak::span<byte_t> old_data;
		lak::span<byte_t> image_data;
		bex::memory_region_selector view;

		~memory_byte_pairs_viewer();

		void draw(lak::span<byte_t> data, bool force_update);
	};

	struct memory_viewer
	{
		enum memory_view_content_mode : int
		{
			VIEW_DATA_BINARY,
			VIEW_DATA_BYTE_PAIRS,
			VIEW_DATA_IMAGE,
		};

		memory_view_content_mode content_mode;
		MemoryEditor editor;
		memory_image_viewer image_viewer;
		memory_byte_pairs_viewer byte_pairs_viewer;

		void draw(lak::span<byte_t> data, bool force_update);
	};

	void image_view(ImTextureRef texture, const float scale);

	void debug_log_view();

	void debug_menu();
}

#endif
