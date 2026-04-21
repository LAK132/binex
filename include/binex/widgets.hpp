#ifndef BINEX_WIDGETS_HPP
#define BINEX_WIDGETS_HPP

#include <lak/image.hpp>
#include <lak/span.hpp>
#include <lak/stdint.hpp>

#include <lak/system/windowing/window.hpp>

#include <lak/imgui/texture.hpp>

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

	struct memory_region_selector2
	{
		lak::span<byte_t> data;
		bex::memory_region_selector view;

		bool draw(lak::span<byte_t> data, bool update);
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

	using texture = lak::ImUniqueTexture;

	struct image_viewer
	{
		bex::texture texture;
		float scale = 1.0f;
		void draw();
	};

	struct memory_image_viewer
	{
		bex::image_viewer img_view;
		bex::memory_region_selector2 mem_view;

		lak::vec2u64_t image_size         = {256, 256};
		lak::vec3u64_t block_skip         = {0, 1, 0};
		lak::array<int, 4> rgbx_bit_count = {8, 8, 8, 0};
		bex::pixel_layout pixel_layout    = bex::pixel_layout::rgb;

		void draw(lak::span<byte_t> data, bool update);
	};

	struct memory_byte_pairs_viewer
	{
		bex::image_viewer img_view;
		bex::memory_region_selector2 mem_view;

		void draw(lak::span<byte_t> data, bool update);
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
		bex::memory_image_viewer image_viewer;
		bex::memory_byte_pairs_viewer byte_pairs_viewer;

		void draw(lak::span<byte_t> data, bool update);
	};

	void debug_log_view();

	void debug_menu();
}

#endif
