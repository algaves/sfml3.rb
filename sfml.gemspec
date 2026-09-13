require_relative 'lib/sfml/version'

Gem::Specification.new do |s|
    s.name        = 'sfml'
    s.version     = SFML::VERSION
    s.summary     = 'SFML wrapper for Ruby'
    s.description = 'Ruby bindings for SFML 3 via CSFML. SFML and CSFML are downloaded and ' \
                    'built from source at install time, so neither needs to be installed system-wide.'
    s.homepage    = 'https://github.com/algaves/sfml3.rb'

    s.authors     = ['Sealtiel Valderrama']
    s.email       = 'SealtielFreak@yandex.com'
    s.license     = '0BSD'

    s.required_ruby_version = '>= 2.5.0'

    s.metadata    = {
        'source_code_uri'   => 'https://github.com/algaves/sfml3.rb',
        'bug_tracker_uri'   => 'https://github.com/algaves/sfml3.rb/issues',
        'changelog_uri'     => 'https://github.com/algaves/sfml3.rb/blob/main/CHANGELOG.md',
    }

    s.add_dependency 'rice', '~> 4.0'

    s.files       = Dir.glob('ext/**/*.{h,c,rb,sh}') +
                    Dir.glob('lib/**/*.rb') +
                    ['README.md', 'LICENSE.md']

    s.extensions  = ['ext/extconf.rb']
end
