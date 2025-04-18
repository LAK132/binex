project(
	'binex',
	['c', 'cpp'],
	default_options: [
		'b_vscrt=static_from_buildtype',
	],
)

lak_subprj = subproject('lak')
lak_dep = lak_subprj.get_variable('lak_dep')
lak_libs = lak_subprj.get_variable('system_libraries')

c_cpp_args = lak_subprj.get_variable('c_cpp_args')
cpp_args = lak_subprj.get_variable('cpp_args')
c_args = lak_subprj.get_variable('c_args')

override_options = lak_subprj.get_variable('override_options')
override_options_w0 = lak_subprj.get_variable('override_options_w0')
override_options_w1 = lak_subprj.get_variable('override_options_w1')
override_options_w2 = lak_subprj.get_variable('override_options_w2')
override_options_w3 = lak_subprj.get_variable('override_options_w3')
override_options_werror = lak_subprj.get_variable('override_options_werror')

add_project_arguments(c_cpp_args + cpp_args, language: ['cpp'])
add_project_arguments(c_cpp_args + c_args, language: ['c'])

if host_machine.system() == 'windows'
	git_header = custom_target(
		'generate_git_file',
		output: 'git.hpp',
		command: ['generate_git_file.bat', '@OUTPUT@'] ,
		build_by_default: true,
	)
else
	git_header = custom_target(
		'generate_git_file',
		output: 'git.hpp',
		command: ['./generate_git_file.sh', '@OUTPUT@'] ,
		build_by_default: true,
	)
endif

install_directory = host_machine.cpu_family()

subdir('src')

executable(
	'binex',
	[
		'src'/'main.cpp',
		git_header,
	],
	install: true,
	install_dir: install_directory,
	override_options: override_options_werror,
	include_directories: binex_includes,
	dependencies: [
		binex_core_dep,
	],
)
