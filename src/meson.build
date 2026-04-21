binex_includes = include_directories('..'/'include')

binex_core = static_library(
	'binex_core',
	[
		'core.cpp',
		'widgets.cpp',
	],
	include_directories: binex_includes,
	dependencies: [
		lak_dep,
		imgui_color_text_edit_dep,
		imgui_memory_editor_dep,
	],
)

binex_core_dep = declare_dependency(
	link_with: binex_core,
	include_directories: binex_includes,
	dependencies: [
		lak_dep,
		imgui_color_text_edit_dep,
		imgui_memory_editor_dep,
	],
)
