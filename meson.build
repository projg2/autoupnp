project('autoupnp', 'c',
        license: 'GPL-2.0-or-later',
        version: '0.5.0')

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
