lak_subprj = subproject('lak')
lak_dep = lak_subprj.get_variable('lak_dep')
lak_libs = lak_subprj.get_variable('system_libraries')

binex_includes = include_directories('..'/'include')

binex_core = static_library(
	'binex_core',
	[
		'core.cpp',
	],
	override_options: [
		'cpp_std=@0@'.format(version),
		'warning_level=3',
		'werror=true',
	],
	include_directories: binex_includes,
	dependencies: lak_dep,
)

binex_core_dep = declare_dependency(
	link_with: binex_core,
	include_directories: binex_includes,
	dependencies: lak_dep,
)
