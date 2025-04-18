binex_includes = include_directories('..'/'include')

binex_core = static_library(
	'binex_core',
	[
		'core.cpp',
		'widgets.cpp',
	],
	override_options: override_options_werror,
	include_directories: binex_includes,
	dependencies: lak_dep,
)

binex_core_dep = declare_dependency(
	link_with: binex_core,
	include_directories: binex_includes,
	dependencies: lak_dep,
)
