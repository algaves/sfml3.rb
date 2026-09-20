# Headless smoke test for sfml3-rb compiled to wasm32-unknown-emscripten.
# Window/Graphics are not part of this build, so only System + Audio are exercised.

$LOAD_PATH.unshift('/usr/local/lib')
require 'sfml'
raise 'no SFML module' unless defined?(SFML)

def ok(name)
  warn "ok: #{name}"
end

# --- System: Clock / Time / sleep ------------------------------------------
clock = SFML::Clock.new
raise 'clock not running' unless clock.running?
raise 'elapsed negative' unless clock.elapsed_time.as_microseconds >= 0
SFML.sleep(SFML::Time.milliseconds(2))
raise 'sleep did not advance clock' unless clock.elapsed_time.as_microseconds >= 2000
ok 'clock.running? and sleep advances elapsed_time'
clock.restart!
ok 'clock.restart!'

t = SFML::Time.seconds(1.5)
raise 'seconds->milliseconds' unless t.as_milliseconds == 1500
raise 'microseconds conversion' unless SFML::Time.microseconds(3).as_milliseconds == 0
raise 'time addition' unless (SFML::Time.zero + SFML::Time.microseconds(250)).as_microseconds == 250
raise 'time multiplication' unless (SFML::Time.milliseconds(1) * 3).as_microseconds == 3000
raise 'time comparison' unless SFML::Time.microseconds(2) > SFML::Time.microseconds(1)
ok 'Time conversions, arithmetic, comparison'

# --- System: Vector2 / Vector3 ---------------------------------------------
v = SFML::Vector2.new(1, 2)
v2 = SFML::Vector2.new(10, 20)
raise 'vector2 to_a' unless v.to_a == [1, 2]
raise 'vector2 add' unless (v + v2).to_a == [11, 22]
raise 'vector2 mul' unless (v * 3).to_a == [3, 6]
v3 = SFML::Vector3.new(3, 4, 0)
raise 'vector3 length' unless (v3.length - 5).abs < 1e-9
ok 'Vector2/Vector3 arithmetic and length'

# --- System: Buffer ---------------------------------------------------------
b = SFML::Buffer.new
raise 'buffer not empty' unless b.empty?
raise 'buffer data' unless b.data == ''
ok 'Buffer.empty?'

# --- Audio: SoundBuffer from in-memory WAV ----------------------------------
sr = 44100
n = sr
pcm = Array.new(n) { |i| (Math.sin(2 * Math::PI * 440 * i / sr) * 32_767).round }.pack('s<*')
data_size = pcm.bytesize
wav = ['RIFF', 36 + data_size, 'WAVE', 'fmt ', 16, 1, 1, sr, sr * 2, 2, 16, 'data', data_size]
      .pack('A4VA4A4VvvVVvvA4V') + pcm

sb = SFML::SoundBuffer.from_memory(wav)
raise 'wav sample_count' unless sb.sample_count == n
raise 'wav sample_rate' unless sb.sample_rate == sr
raise 'wav channel_count' unless sb.channel_count == 1
raise 'wav duration' unless sb.duration.as_milliseconds >= 999
ok 'SoundBuffer.from_memory decodes 1s mono 440Hz WAV'

# --- Audio: SoundBuffer via InputStream -------------------------------------
class Reader
  attr_reader :pos

  def initialize(bytes)
    @bytes = bytes
    @pos = 0
  end

  def size
    @bytes.bytesize
  end
  alias length size

  def read(n)
    return nil if @pos >= @bytes.bytesize

    from = @pos
    @pos = [@pos + n, @bytes.bytesize].min
    @bytes.byteslice(from, @pos - from)
  end

  def seek(offset, whence)
    base = whence.zero? ? 0 : (whence == 1 ? @pos : @bytes.bytesize)
    @pos = [base + offset, 0, @bytes.bytesize].sort[1]
    0
  end

  def tell
    @pos
  end
end

stream = SFML::InputStream.new(Reader.new(wav))
sb2 = SFML::SoundBuffer.from_stream(stream)
raise 'stream sample_count' unless sb2.sample_count == n
raise 'stream duration' unless sb2.duration.as_milliseconds >= 999
ok 'SoundBuffer.from_stream through InputStream'

# --- Audio: Listener + SoundSourceCone (device-free state) ------------------
SFML::Listener.position = SFML::Vector3.new(5, 6, 7)
pos = SFML::Listener.position
raise 'listener position roundtrip' unless pos.to_a == [5, 6, 7]
raise 'listener velocity' unless SFML::Listener.velocity.to_a == [0, 0, 0]
ok 'Listener position set/get roundtrip'

cone = SFML::SoundSourceCone.new(30, 90, 0.5)
raise 'cone roundtrip' unless cone.inner_angle == 30 && cone.outer_angle == 90 && (cone.outer_gain - 0.5).abs < 1e-9
ok 'SoundSourceCone getters/setters'

warn 'ALL SFML WASM TESTS PASS'
