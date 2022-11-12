project('autoupnp', 'c',
  license : 'BSD-3-Clause',
  version : '0.4.7')

libminiupnp = dependency('miniupnpc')
libnotify = dependency('libtinynotify', required : get_option('libnotify'))
pthread = dependency('threads')

if libnotify.found()
  add_global_arguments('-DHAVE_LIBNOTIFY=1', language : 'c')
endif

autoupnp_files = files([
  'src/autoupnp.c',
  'src/notify.c',
  'src/registry.c',
  'src/upnp.c',
])

shared_library('autoupnp', autoupnp_files,
  dependencies : [libminiupnp, libnotify, pthread],
  install : true)

install_data('autoupnp',
  install_dir : get_option('bindir'),
  install_mode : 'rwxr-xr-x')
