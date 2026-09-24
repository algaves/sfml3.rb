# frozen_string_literal: true

require 'fileutils'
require 'zip'

# Builds the Examples section from the scripts under examples/: one page per
# top-level group, each inlining its scripts through the `{% example %}` tag and
# offering a ZIP of the folder plus its assets. Archives are written into the
# built site, so no binaries live in the repository.
module SFMLDocs
  # Jekyll generator that discovers examples and emits both the pages and the
  # downloadable archives.
  class ExampleArchive < Jekyll::Generator
    safe true
    priority :low

    ROOT = File.expand_path('../..', __dir__)
    EXAMPLES = File.join(ROOT, 'examples')
    DOWNLOADS = 'downloads'

    INTROS = {
      'hello_shapes' => 'The minimal walkthrough, with nothing else in the loop.',
      'bouncing_shapes' => 'The same parts made interactive: collision, dragging and spawning.',
      'namespace_tour' => 'A console map of the SF namespace and its five subsystem modules.',
      'graphics' => 'The drawing path: shapes, transforms, images, sprites, tilemaps and shaders.',
      'window' => 'The three window classes side by side.',
      'subsystems' => 'One script per window, audio, graphics and network surface.',
      'network' => 'Sockets, packets, TCP, UDP and file transfer -- console-only, no window.',
      'rubyesque' => 'The Rubyesque layer, one script per feature.',
      'games' => 'Eleven complete, playable games built from the same components.'
    }.freeze

    # The three groups a walkthrough sorts its parts into, in reading order. Each
    # is [kind, heading, one-line purpose for the group].
    GROUPS = [
      [:declaration, 'Definitions & declarations',
       'What the example needs before it runs: the binding it loads, the constants it fixes, ' \
       'and the data it lays out.'],
      [:routine, 'Routines',
       'The helper methods and classes it defines, so the main logic below reads as plain intent.'],
      [:logic, 'The example’s logic',
       'The setup and the frame loop — the behaviour the example exists to show.']
    ].freeze

    def generate(site)
      groups = discover
      return if groups.empty?

      keep = (site.config['keep_files'] ||= [])
      keep << DOWNLOADS unless keep.include?(DOWNLOADS)

      groups.each_value { |dirs| dirs.each { |dir| write_zip(site, dir) } }
      create_pages(site, groups)
    end

    private

    # Top-level folder under examples/ => sorted list of example folders in it.
    def discover
      groups = {}
      Dir.glob(File.join(EXAMPLES, '*', '**', '*.rb')).each do |script|
        dir = File.dirname(script)
        rel = dir.delete_prefix("#{EXAMPLES}/")
        (groups[rel.split('/').first] ||= []) << rel
      end
      groups.transform_values(&:uniq)
    end

    def write_zip(site, rel)
      dir = File.join(EXAMPLES, rel)
      path = File.join(site.dest, DOWNLOADS, "#{rel.tr('/', '-')}.zip")
      FileUtils.mkdir_p(File.dirname(path))
      FileUtils.rm_f(path)

      Zip::File.open(path, create: true) do |zip|
        Dir.chdir(dir) do
          Dir.glob('**/*').each do |entry|
            next if File.directory?(entry)

            zip.add("#{File.basename(rel)}/#{entry}", File.join(dir, entry))
          end
        end
      end
    end

    def create_pages(site, groups)
      groups.keys.sort.each_with_index do |group, index|
        page = Jekyll::PageWithoutAFile.new(site, site.source, File.join('examples', group), 'index.md')
        page.data.merge!(
          'layout' => 'default',
          'title' => titleize(group),
          'parent' => 'Examples',
          'nav_order' => index + 1
        )
        page.content = body(group, groups[group].sort)
        site.pages << page
      end
    end

    def body(group, dirs)
      parts = ["# #{titleize(group)}", '', INTROS.fetch(group, 'Self-contained example scripts.'), '']
      dirs.each { |rel| parts << section(rel) }
      parts.join("\n")
    end

    def section(rel)
      script = primary_script(rel)
      slug = rel.tr('/', '-')
      assets = assets(rel)
      walkthrough = ExampleWalkthrough.parts(script)
      lines = ["## #{titleize(File.basename(rel))}", '']
      description = description(script)
      lines << description << '' if description
      if walkthrough[:book]
        lines << 'Read the full walkthrough in the ' \
                 "[Book]({% link book/#{walkthrough[:book]} %})." << ''
      end
      lines.concat(parts(script, walkthrough[:parts])) if walkthrough[:parts].size >= 2
      lines << '### The complete script' << ''
      lines << code_block(script) << ''
      assets_note = assets.empty? ? 'no extra assets' : "assets: #{assets.join(', ')}"
      lines << "[Download #{slug}.zip]({{ '/downloads/#{slug}.zip' | relative_url }}) &middot; #{assets_note}" << ''
      lines.join("\n")
    end

    # Numbered subsections that break the script into the parts it is built from,
    # grouped into what it declares, the routines it defines, and the logic it runs.
    def parts(script, list)
      lines = ['### How the code works', '']
      lines << 'The code is broken into three groups: what it declares, the routines it defines, ' \
               'and the logic it runs.' << ''
      index = 0
      GROUPS.each do |kind, title, blurb|
        group = list.select { |part| part[:kind] == kind }
        next if group.empty?

        lines << "#### #{title}" << '' << blurb << ''
        group.each do |part|
          index += 1
          lines << "**#{index}. #{part[:title]}** — #{part[:note]}" << ''
          lines << code_block(script, part[:first], part[:last]) << ''
        end
      end
      lines
    end

    # The same highlighted markup the `{% example %}` tag emits, written
    # directly so raw Liquid never leaks into the rendered page (or the search
    # index). Pass first/last to render just one part of the file.
    def code_block(script, first = nil, last = nil)
      source = File.readlines(File.join(ROOT, script))
      source = source[(first - 1)..(last - 1)] if first
      CodeRenderer.render(source.join, 'ruby')
    end

    def primary_script(rel)
      dir = File.join(EXAMPLES, rel)
      preferred = File.join(dir, "#{File.basename(rel)}.rb")
      script = File.file?(preferred) ? preferred : Dir.glob(File.join(dir, '*.rb')).min
      script.delete_prefix("#{ROOT}/")
    end

    def assets(rel)
      dir = File.join(EXAMPLES, rel, 'assets')
      return [] unless Dir.exist?(dir)

      Dir.children(dir).sort
    end

    def description(script)
      lines = File.readlines(File.join(ROOT, script)).drop(1)
      index = lines.index { |line| line.start_with?('# ') }
      return nil unless index

      paragraph = lines[index..].take_while { |line| line.start_with?('#') }
      paragraph.map { |line| line.sub(/^#\s?/, '').rstrip }.join(' ').strip
    end

    def titleize(name)
      name.tr('_', ' ').split.map { |word| word[0].upcase + word[1..] }.join(' ')
    end
  end
end
