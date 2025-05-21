import lit.formats

config.name = 'Mapsc'
config.test_format = lit.formats.ShTest(True)

config.suffixes = ['.mapsci']

config.test_source_root = os.path.dirname(__file__)
config.test_exec_root = os.path.join(config.build_dir, 'test')

config.substitutions.append(('%mapsci',
    os.path.join(config.build_dir, 'mapsci --quit-on-error --no-history -q')))
