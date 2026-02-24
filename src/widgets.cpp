#include "binex/widgets.hpp"

#include <lak/bit_reader.hpp>
#include <lak/defer.hpp>

#include <lak/imgui/backend.hpp>
#include <lak/imgui/widgets.hpp>

#include <imgui_memory_editor.h>

bool bex::memory_region_selector::draw(lak::span<byte_t> data,
                                       lak::span<byte_t> &view_data,
                                       bool force_update)
{
	ImGui::PushID((const void *)this);
	DEFER(ImGui::PopID());

	uint64_t view_end = view_begin + view_size;

	const uint64_t range_min = 0;
	const uint64_t range_max = data.size();

	bool updated = force_update;

	ImGui::Checkbox("Fixed Size", &fixed_size);
	ImGui::SameLine();
	if (ImGui::Button("Reset View"))
	{
		view_begin = 0;
		view_end   = range_max;
		view_size  = range_max;
		updated    = true;
	}

	uint64_t old_size = view_end - view_begin;
	if (ImGui::DragScalar("View Begin",
	                      ImGuiDataType_U64,
	                      &view_begin,
	                      1.0f,
	                      &range_min,
	                      &range_max))
	{
		view_begin = std::min(view_begin, range_max);
		if (fixed_size)
			view_end = std::min(view_begin + old_size, range_max);
		else
			view_end = std::min(std::max(view_begin, view_end), range_max);
		view_size = view_end - view_begin;

		updated = true;
	}

	if (fixed_size)
	{
		if (uint64_t max_size = range_max - view_begin;
		    ImGui::DragScalar("View Size",
		                      ImGuiDataType_U64,
		                      &view_size,
		                      1.0f,
		                      &range_min,
		                      &max_size))
		{
			view_end = view_begin + view_size;

			updated = true;
		}
	}
	else
	{
		if (ImGui::DragScalar("View End",
		                      ImGuiDataType_U64,
		                      &view_end,
		                      1.0f,
		                      &range_min,
		                      &range_max))
		{
			view_end = std::min(view_end, range_max);
			if (fixed_size)
				view_begin =
				  std::min(view_end - std::min(old_size, view_end), range_max);
			else
				view_begin = std::min(view_begin, range_max);
			view_size = view_end - view_begin;

			updated = true;
		}
	}

	const uint64_t bound_begin = std::min(view_begin, range_max);
	const uint64_t bound_end =
	  std::min(std::max(view_begin, view_end), range_max);

	auto vdata = data.subspan(static_cast<size_t>(bound_begin),
	                          static_cast<size_t>(bound_end - bound_begin));
	updated |= data.empty() != view_data.empty() ||
	           !lak::same_span<byte_t>(vdata, view_data);

	if (updated)
	{
		view_begin = bound_begin;
		view_end   = bound_end;
		view_size  = view_end - view_begin;
		view_data  = vdata;
	}

	return updated;
}

void bex::image_view(ImTextureRef texture, const float scale)
{
	ImGui::BeginChild("Image View",
	                  ImVec2(0, 0),
	                  false,
	                  ImGuiWindowFlags_NoSavedSettings |
	                    ImGuiWindowFlags_AlwaysVerticalScrollbar |
	                    ImGuiWindowFlags_AlwaysHorizontalScrollbar);

	lak::vec2s_t size = lak::TextureSize(texture);

	ImGui::Image(texture, ImVec2(scale * size.x, scale * size.y));

	ImGui::EndChild();
}

void image_memory_view_impl(lak::span<byte_t> data,
                            lak::vec2u64_t &image_size,
                            lak::vec3u64_t &block_skip,
                            lak::span<int, 4> rgbx_bit_count,
                            bex::pixel_layout &pixel_layout,
                            ImTextureRef &texture,
                            float &scale,
                            bool &update)
{
	{
		if (ImGui::Button("Reset View"))
		{
			image_size = {256, 256};
			block_skip = {0, 1, 0};
			update     = true;
		}

		const static uint64_t sizeMin = 0;
		const static uint64_t sizeMax = 10000;
		update |= ImGui::DragScalarN("Image Size (Width/Height)",
		                             ImGuiDataType_U64,
		                             &image_size,
		                             2,
		                             1.0f,
		                             &sizeMin,
		                             &sizeMax);

		ImGui::Separator();

		if (ImGui::Button("MONO8"))
		{
			rgbx_bit_count[0U] = 8U;
			rgbx_bit_count[1U] = 0U;
			rgbx_bit_count[2U] = 0U;
			rgbx_bit_count[3U] = 0U;
			pixel_layout       = bex::pixel_layout::mono;
			update             = true;
		}
		ImGui::SameLine();
		if (ImGui::Button("RGB15"))
		{
			rgbx_bit_count[0U] = 5U;
			rgbx_bit_count[1U] = 5U;
			rgbx_bit_count[2U] = 5U;
			rgbx_bit_count[3U] = 1U;
			pixel_layout       = bex::pixel_layout::rgbx;
			update             = true;
		}
		ImGui::SameLine();
		if (ImGui::Button("RGB16"))
		{
			rgbx_bit_count[0U] = 5U;
			rgbx_bit_count[1U] = 6U;
			rgbx_bit_count[2U] = 5U;
			rgbx_bit_count[3U] = 0U;
			pixel_layout       = bex::pixel_layout::rgb;
			update             = true;
		}
		ImGui::SameLine();
		if (ImGui::Button("RGB24"))
		{
			rgbx_bit_count[0U] = 8U;
			rgbx_bit_count[1U] = 8U;
			rgbx_bit_count[2U] = 8U;
			rgbx_bit_count[3U] = 0U;
			pixel_layout       = bex::pixel_layout::rgb;
			update             = true;
		}
		ImGui::SameLine();
		if (ImGui::Button("BGR24"))
		{
			rgbx_bit_count[0U] = 8U;
			rgbx_bit_count[1U] = 8U;
			rgbx_bit_count[2U] = 8U;
			rgbx_bit_count[3U] = 0U;
			pixel_layout       = bex::pixel_layout::bgr;
			update             = true;
		}
		ImGui::SameLine();
		if (ImGui::Button("RGBX32"))
		{
			rgbx_bit_count[0U] = 8U;
			rgbx_bit_count[1U] = 8U;
			rgbx_bit_count[2U] = 8U;
			rgbx_bit_count[3U] = 8U;
			pixel_layout       = bex::pixel_layout::rgbx;
			update             = true;
		}
		ImGui::SameLine();
		if (ImGui::Button("RGGB10"))
		{
			rgbx_bit_count[0U] = 10U;
			rgbx_bit_count[1U] = 10U;
			rgbx_bit_count[2U] = 10U;
			rgbx_bit_count[3U] = 0U;
			pixel_layout       = bex::pixel_layout::bayer_rggb;
			update             = true;
		}
		ImGui::SameLine();
		if (ImGui::Button("RGGB12"))
		{
			rgbx_bit_count[0U] = 12U;
			rgbx_bit_count[1U] = 12U;
			rgbx_bit_count[2U] = 12U;
			rgbx_bit_count[3U] = 0U;
			pixel_layout       = bex::pixel_layout::bayer_rggb;
			update             = true;
		}
		ImGui::SameLine();
		if (ImGui::Button("RGGB14"))
		{
			rgbx_bit_count[0U] = 14U;
			rgbx_bit_count[1U] = 14U;
			rgbx_bit_count[2U] = 14U;
			rgbx_bit_count[3U] = 0U;
			pixel_layout       = bex::pixel_layout::bayer_rggb;
			update             = true;
		}
		ImGui::SameLine();
		if (ImGui::Button("RGGB16"))
		{
			rgbx_bit_count[0U] = 16U;
			rgbx_bit_count[1U] = 16U;
			rgbx_bit_count[2U] = 16U;
			rgbx_bit_count[3U] = 0U;
			pixel_layout       = bex::pixel_layout::bayer_rggb;
			update             = true;
		}

		update |=
		  ImGui::DragInt4("RGBX Bit Count", rgbx_bit_count.data(), 0.05f, 0, 16);

		int layout = static_cast<int>(pixel_layout);
		// update |= ImGui::SliderInt("Layout",
		//                            &layout,
		//                            (int)bex::pixel_layout::mono,
		//                            (int)bex::pixel_layout::bayer_rggb);
		update |= ImGui::Combo("Channel Layout",
		                       &layout,
		                       "Monochrome\0"
		                       "R\0"
		                       "RG\0"
		                       "RGB\0"
		                       "BGR\0"
		                       "RGBX\0"
		                       "BGRX\0"
		                       "XRGB\0"
		                       "XBGR\0"
		                       "Bayer RGGB\0"
		                       "Bayer BGGR\0"
		                       "Bayer GRBG\0"
		                       "Bayer GBRG\0"
		                       "\0");
		pixel_layout = static_cast<bex::pixel_layout>(layout);

		ImGui::Separator();

		const static uint64_t skipMax = 1000000;
		update |= ImGui::DragScalarN("For Every X * Y Pixels Skip Z Bytes (X/Y/Z)",
		                             ImGuiDataType_U64,
		                             &block_skip,
		                             3,
		                             0.05f,
		                             &sizeMin,
		                             &skipMax);
	}

	update |= texture.GetTexID() == ImTextureID_Invalid;

	if (update)
	{
		static lak::image4_t image{}; // static so we can reuse the memory
		image.resize(
		  {static_cast<size_t>(image_size.x), static_cast<size_t>(image_size.y)});

		image.fill({0, 0, 0, 255});

		lak::bit_reader reader{data};

		auto read = [&](uint8_t bit_count) -> uint8_t
		{
			return uint8_t(reader.read_bits(bit_count).unwrap_or(0U) >>
			               (bit_count - std::min<uint8_t>(bit_count, 8U)));
		};
		auto read_r = [&]() -> uint8_t
		{ return read(uint8_t(rgbx_bit_count[0U])); };
		auto read_g = [&]() -> uint8_t
		{ return read(uint8_t(rgbx_bit_count[1U])); };
		auto read_b = [&]() -> uint8_t
		{ return read(uint8_t(rgbx_bit_count[2U])); };
		auto read_x = [&]() -> uint8_t
		{ return read(uint8_t(rgbx_bit_count[3U])); };

		const size_t to_read = size_t(block_skip.x * block_skip.y);
		const size_t to_skip = size_t(block_skip.z);
		const bool do_skips  = to_read != 0 && to_skip != 0;

		switch (pixel_layout)
		{
			case bex::pixel_layout::mono:
				for (size_t i = 0; i < image.contig_size(); ++i)
				{
					auto mono{read_r()};
					image[i].r = mono;
					image[i].g = mono;
					image[i].b = mono;
					if (do_skips && (i + 1) % to_read == 0)
						reader.skip_bytes(to_skip).discard();
				}
				break;

			case bex::pixel_layout::r:
				for (size_t i = 0; i < image.contig_size(); ++i)
				{
					image[i].r = read_r();
					if (do_skips && (i + 1) % to_read == 0)
						reader.skip_bytes(to_skip).discard();
				}
				break;

			case bex::pixel_layout::rg:
				for (size_t i = 0; i < image.contig_size(); ++i)
				{
					image[i].r = read_r();
					image[i].g = read_g();
					if (do_skips && (i + 1) % to_read == 0)
						reader.skip_bytes(to_skip).discard();
				}
				break;

			case bex::pixel_layout::rgb:
				for (size_t i = 0; i < image.contig_size(); ++i)
				{
					image[i].r = read_r();
					image[i].g = read_g();
					image[i].b = read_b();
					if (do_skips && (i + 1) % to_read == 0)
						reader.skip_bytes(to_skip).discard();
				}
				break;

			case bex::pixel_layout::bgr:
				for (size_t i = 0; i < image.contig_size(); ++i)
				{
					image[i].b = read_b();
					image[i].g = read_g();
					image[i].r = read_r();
					if (do_skips && (i + 1) % to_read == 0)
						reader.skip_bytes(to_skip).discard();
				}
				break;

			case bex::pixel_layout::rgbx:
				for (size_t i = 0; i < image.contig_size(); ++i)
				{
					image[i].r = read_r();
					image[i].g = read_g();
					image[i].b = read_b();
					read_x();
					if (do_skips && (i + 1) % to_read == 0)
						reader.skip_bytes(to_skip).discard();
				}
				break;

			case bex::pixel_layout::bgrx:
				for (size_t i = 0; i < image.contig_size(); ++i)
				{
					image[i].b = read_b();
					image[i].g = read_g();
					image[i].r = read_r();
					read_x();
					if (do_skips && (i + 1) % to_read == 0)
						reader.skip_bytes(to_skip).discard();
				}
				break;

			case bex::pixel_layout::xrgb:
				for (size_t i = 0; i < image.contig_size(); ++i)
				{
					read_x();
					image[i].r = read_r();
					image[i].g = read_g();
					image[i].b = read_b();
					if (do_skips && (i + 1) % to_read == 0)
						reader.skip_bytes(to_skip).discard();
				}
				break;

			case bex::pixel_layout::xbgr:
				for (size_t i = 0; i < image.contig_size(); ++i)
				{
					read_x();
					image[i].b = read_b();
					image[i].g = read_g();
					image[i].r = read_r();
					if (do_skips && (i + 1) % to_read == 0)
						reader.skip_bytes(to_skip).discard();
				}
				break;

			case bex::pixel_layout::bayer_rggb:
				for (size_t y = 0, i = 0; y < image.size().y; ++y)
				{
					for (size_t x = 0; x < image.size().x; ++x, ++i)
					{
						image.at(lak::vec2s_t{x, y}).r = read_r();
						image.at(lak::vec2s_t{x, y}).g = read_g();
						if (do_skips && (i + 1) % to_read == 0)
							reader.skip_bytes(to_skip).discard();
					}
					for (size_t x = 0; x < image.size().x; ++x, ++i)
					{
						image.at(lak::vec2s_t{x, y}).g =
						  (image.at(lak::vec2s_t{x, y}).g + read_g()) / 2U;
						image.at(lak::vec2s_t{x, y}).b = read_b();
						if (do_skips && (i + 1) % to_read == 0)
							reader.skip_bytes(to_skip).discard();
					}
				}
				break;

			case bex::pixel_layout::bayer_bggr:
				for (size_t y = 0, i = 0; y < image.size().y; ++y)
				{
					for (size_t x = 0; x < image.size().x; ++x, ++i)
					{
						image.at(lak::vec2s_t{x, y}).b = read_b();
						image.at(lak::vec2s_t{x, y}).g = read_g();
						if (do_skips && (i + 1) % to_read == 0)
							reader.skip_bytes(to_skip).discard();
					}
					for (size_t x = 0; x < image.size().x; ++x, ++i)
					{
						image.at(lak::vec2s_t{x, y}).g =
						  (image.at(lak::vec2s_t{x, y}).g + read_g()) / 2U;
						image.at(lak::vec2s_t{x, y}).r = read_r();
						if (do_skips && (i + 1) % to_read == 0)
							reader.skip_bytes(to_skip).discard();
					}
				}
				break;

			case bex::pixel_layout::bayer_grbg:
				for (size_t y = 0, i = 0; y < image.size().y; ++y)
				{
					for (size_t x = 0; x < image.size().x; ++x, ++i)
					{
						image.at(lak::vec2s_t{x, y}).g = read_g();
						image.at(lak::vec2s_t{x, y}).r = read_r();
						if (do_skips && (i + 1) % to_read == 0)
							reader.skip_bytes(to_skip).discard();
					}
					for (size_t x = 0; x < image.size().x; ++x, ++i)
					{
						image.at(lak::vec2s_t{x, y}).b = read_b();
						image.at(lak::vec2s_t{x, y}).g =
						  (image.at(lak::vec2s_t{x, y}).g + read_g()) / 2U;
						if (do_skips && (i + 1) % to_read == 0)
							reader.skip_bytes(to_skip).discard();
					}
				}
				break;

			case bex::pixel_layout::bayer_gbrg:
				for (size_t y = 0, i = 0; y < image.size().y; ++y)
				{
					for (size_t x = 0; x < image.size().x; ++x, ++i)
					{
						image.at(lak::vec2s_t{x, y}).g = read_g();
						image.at(lak::vec2s_t{x, y}).b = read_b();
						if (do_skips && (i + 1) % to_read == 0)
							reader.skip_bytes(to_skip).discard();
					}
					for (size_t x = 0; x < image.size().x; ++x, ++i)
					{
						image.at(lak::vec2s_t{x, y}).r = read_r();
						image.at(lak::vec2s_t{x, y}).g =
						  (image.at(lak::vec2s_t{x, y}).g + read_g()) / 2U;
						if (do_skips && (i + 1) % to_read == 0)
							reader.skip_bytes(to_skip).discard();
					}
				}
				break;

			default: ASSERT_NYI(); break;
		}

#if 0
			const auto begin = data.begin();
			const auto end   = data.end();
			auto it          = begin;

			auto get_next_channel = [&,
			                         i = size_t(0),
			                         stride =
			                           uint64_t(block_skip.x) *
			                           uint64_t(block_skip.y)]() mutable -> uint8_t
			{
				const uint8_t value = static_cast<uint8_t>(*(it++));
				if (stride > 0 && ((i + 1) % stride) == 0U) it += block_skip.z;
				++i;
				return value;
			};

			auto get_next_pixel = [&]()
			{
				lak::color4_t result{0, 0, 0, 255};
				size_t i = 0;
				for (; i < channels_per_pixel && i < 3; ++i)
					result[i] = get_next_channel();
				for (; i < channels_per_pixel; ++i) (void)get_next_channel();
				return result;

				switch (colour_size)
				{
					case 1:
					{
						const auto rgb{get_next()};
						return lak::color4_t(rgb, rgb, rgb, static_cast<uint8_t>(0xFFU));
					}

					case 2:
					{
						uint16_t rgb = static_cast<uint16_t>(get_next()) << 8U;
						rgb |= static_cast<uint16_t>(get_next());
						return lak::color4_t(
						  static_cast<uint8_t>((rgb & 0xF800) >> 8), // 1111 1000 0000 0000
						  static_cast<uint8_t>((rgb & 0x07E0) >> 3), // 0000 0111 1110 0000
						  static_cast<uint8_t>((rgb & 0x001F) << 3), // 0000 0000 0001 1111
						  static_cast<uint8_t>(0xFFU));
					}

					default:
					{
						const uint8_t r = get_next();
						const uint8_t g = get_next();
						const uint8_t b = get_next();
						it += colour_size - 3;
						return lak::color4_t(r, g, b, static_cast<uint8_t>(0xFFU));
					}
				}
			};
#endif

		texture = lak::CreateTexture(image);
	}

	if (texture.GetTexID() != ImTextureID_Invalid)
	{
		ImGui::Separator();
		ImGui::DragFloat("Scale", &scale, 0.1f, 0.1f, 10.0f);
		ImGui::Separator();
		// DERIVED::view_image(texture, scale);
		bex::image_view(texture, scale);
	}
}

bex::memory_image_viewer::~memory_image_viewer()
{
	if (texture.GetTexID() != ImTextureID_Invalid) lak::DestroyTexture(texture);
}

void bex::memory_image_viewer::draw(lak::span<byte_t> data, bool update)
{
	if (data.empty() && old_data.empty()) return;

	if (!data.empty() && !lak::same_span<byte_t>(data, old_data))
	{
		old_data   = data;
		image_data = data;
		update     = true;
	}

	// if (update)
	// {
	// 	if (SrcExp.view != nullptr && SrcExp.state.file != nullptr &&
	// 	    data == SrcExp.state.file->data())
	// 	{
	// 		auto ref_span = SrcExp.view->ref_span;
	// 		while (ref_span._source && ref_span._source != SrcExp.state.file)
	// 			ref_span = ref_span.parent_span();
	// 		if (!ref_span.empty())
	// 		{
	// 			from = ref_span.position().UNWRAP();
	// 			to   = from + ref_span.size();
	// 		}
	// 		else
	// 		{
	// 			from = 0;
	// 			to   = SIZE_MAX;
	// 		}
	// 	}
	// 	else
	// 	{
	// 		from = 0;
	// 		to   = SIZE_MAX;
	// 	}
	// }

	update |= view.draw(data, image_data, update);

	ImGui::Separator();

	image_memory_view_impl(image_data,
	                       image_size,
	                       block_skip,
	                       rgbx_bit_count,
	                       pixel_layout,
	                       texture,
	                       scale,
	                       update);
}

void byte_pairs_memory_view_impl(lak::span<byte_t> data,
                                 ImTextureRef &texture,
                                 float &scale,
                                 bool &update)
{
	update |= texture.GetTexID() == ImTextureID_Invalid;

	if (update)
	{
		thread_local static lak::image<float> image{lak::vec2s_t{256, 256}};

		image.fill(0.0f);

		const auto begin = data.begin();
		const auto end   = data.end();
		auto it          = begin;

		const float step = 1.0f / (data.size() / float(image.contig_size()));
		for (uint8_t prev = (it != end ? uint8_t(*it) : 0); it != end;
		     prev         = uint8_t(*(it++)))
      image[{prev, uint8_t(*it)}] += step;

		texture = lak::CreateTexture(image);
	}

	if (texture.GetTexID() != ImTextureID_Invalid)
	{
		ImGui::DragFloat("Scale", &scale, 0.1f, 0.1f, 10.0f);
		ImGui::Separator();
		bex::image_view(texture, scale);
	}
}

bex::memory_byte_pairs_viewer::~memory_byte_pairs_viewer()
{
	if (texture.GetTexID() != ImTextureID_Invalid) lak::DestroyTexture(texture);
}

void bex::memory_byte_pairs_viewer::draw(lak::span<byte_t> data, bool update)
{
	if (data.empty() && old_data.empty()) return;

	if (!data.empty() && !lak::same_span<byte_t>(data, old_data))
	{
		old_data = data;
		update   = true;
	}

	// if (update)
	// {
	// 	if (SrcExp.view != nullptr && SrcExp.state.file != nullptr &&
	// 	    data == SrcExp.state.file->data())
	// 	{
	// 		auto ref_span = SrcExp.view->ref_span;
	// 		while (ref_span._source && ref_span._source != SrcExp.state.file)
	// 			ref_span = ref_span.parent_span();
	// 		if (!ref_span.empty())
	// 		{
	// 			from = ref_span.position().UNWRAP();
	// 			to   = from + ref_span.size();
	// 		}
	// 		else
	// 		{
	// 			from = 0;
	// 			to   = SIZE_MAX;
	// 		}
	// 	}
	// 	else
	// 	{
	// 		from = 0;
	// 		to   = SIZE_MAX;
	// 	}
	// }

	update |= view.draw(data, image_data, update);

	ImGui::Separator();

	byte_pairs_memory_view_impl(image_data, texture, scale, update);
}

void bex::memory_viewer::draw(lak::span<byte_t> data, bool update)
{
	update |=
	  ImGui::RadioButton("Binary", (int *)&content_mode, VIEW_DATA_BINARY);
	ImGui::SameLine();
	update |= ImGui::RadioButton(
	  "Byte Pairs", (int *)&content_mode, VIEW_DATA_BYTE_PAIRS);
	ImGui::SameLine();
	update |=
	  ImGui::RadioButton("Data Image", (int *)&content_mode, VIEW_DATA_IMAGE);
	ImGui::Separator();

	switch (content_mode)
	{
		case VIEW_DATA_BINARY:
			editor.DrawContents(reinterpret_cast<uint8_t *>(data.data()),
			                    data.size());
			break;

		case VIEW_DATA_BYTE_PAIRS: byte_pairs_viewer.draw(data, update); break;

		case VIEW_DATA_IMAGE: image_viewer.draw(data, update); break;

		default: content_mode = VIEW_DATA_BINARY; break;
	}
}

void bex::debug_log_view()
{
	static lak::u8string log_str;
	static const char *log_cstr = nullptr;

	if (ImGui::Button("Refresh"))
	{
		log_str  = lak::to_u8string(lak::debugger.str());
		log_cstr = (const char *)log_str.c_str();
	}
	ImGui::SameLine();
	if (ImGui::Button("Clear"))
	{
		lak::debugger.clear();
		log_str.clear();
		log_cstr = nullptr;
	}

	if (log_cstr != nullptr && log_str.size() > 0)
	{
		if (ImGui::BeginChild("view debug log"))
		{
			ImGui::TextUnformatted(log_cstr, log_cstr + log_str.size());
		}
		ImGui::EndChild();
	}
}

void bex::debug_menu()
{
	if (ImGui::BeginMenu("Debug"))
	{
		ImGui::Checkbox("Debug console", &lak::debugger.live_output_enabled);
		if (lak::debugger.live_output_enabled)
		{
			ImGui::Checkbox("Only errors", &lak::debugger.live_errors_only);
			ImGui::Checkbox("Developer mode", &lak::debugger.line_info_enabled);
		}
		ImGui::EndMenu();
	}
}
