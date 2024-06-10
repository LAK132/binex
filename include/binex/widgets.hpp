#ifndef BINEX_WIDGETS_HPP
#define BINEX_WIDGETS_HPP

#include <lak/image.hpp>
#include <lak/span.hpp>
#include <lak/stdint.hpp>
#include <lak/window.hpp>

#include <lak/opengl/texture.hpp>
#include <misc/memory_editor/imgui_memory_editor.h>
#include <misc/softraster/texture.h>

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

	using texture =
	  lak::variant<lak::monostate, lak::opengl::texture, texture_color32_t>;

	bex::texture create_texture(const lak::image4_t &bitmap,
	                            const lak::graphics_mode mode);

	bex::texture create_texture(const lak::image<float> &bitmap,
	                            const lak::graphics_mode mode);

	void image_view(const bex::texture &texture, const float scale);

	void memory_image_view(lak::span<byte_t> data,
	                       lak::graphics_mode graphics_mode,
	                       bool update);

	void memory_byte_pairs_view(lak::span<byte_t> data,
	                            lak::graphics_mode graphics_mode,
	                            bool update);

	void memory_view(lak::span<byte_t> data,
	                 lak::graphics_mode graphics_mode,
	                 bool update);

	void debug_log_view();

	void debug_menu();
}

#endif
