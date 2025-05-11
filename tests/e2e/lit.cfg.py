import lit.formats

config.name = 'Mapsc'
config.test_format = lit.formats.ShTest(True)

config.suffixes = ['.maps']

config.test_source_root = os.path.dirname(__file__)
config.test_exec_root = os.path.join(config.build_dir, 'test')

config.substitutions.append(('%mapsc',
    os.path.join(config.build_dir, 'mapsc')))