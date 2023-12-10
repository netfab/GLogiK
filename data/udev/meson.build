
udev_rules_dir = get_option('udev_rules_dir')
if udev_rules_dir == ''
  #! pkg-config --variable=udevdir udev
  udev_rules_dir = dependency('udev').get_variable(pkgconfig: 'udevdir')
  #! absolute path, otherwise meson prepends the prefix
  udev_rules_dir = udev_rules_dir / 'rules.d'

  if get_option('relative_udev_rules_dir')
	#! make the path relative, meson will prepend the prefix at install phase
	udev_rules_dir = udev_rules_dir.substring(1)
  endif
endif

configure_file(
  input : '99-GLogiK.rules.in',
  output : '99-GLogiK.rules',
  install_dir : udev_rules_dir,
  configuration : cdata
)

