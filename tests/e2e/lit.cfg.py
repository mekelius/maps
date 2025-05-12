import lit.formats

config.name = 'Mapsc'
config.test_format = lit.formats.ShTest(True)

config.suffixes = ['.mapsci'] # type: ignore

config.test_source_root = os.path.dirname(__file__)
config.test_exec_root = os.path.join(config.build_dir, 'test')

config.substitutions.append(('%mapsc',
    os.path.join(config.build_dir, 'mapsc'))) # type: ignore

config.substitutions.append(('%mapsc',
    os.path.join(config.build_dir, 'mapsc')))

config.substitutions.append(('%mapsci',
    os.path.join(config.build_dir, 'mapsci')))

config.substitutions.append(('%mapsc-verify',
    os.path.join(config.build_dir, 'mapsc-verify')))