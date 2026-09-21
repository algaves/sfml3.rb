# Rubyesque (Matz-like)

One script per slice of the Rubyesque (Matz-like) layer in
`lib/sfml/rubyesque.rb`. The other examples *use* that layer; these are written
to *teach* it, so each one is small and its comments name the methods it is
demonstrating. The API itself is in the
[main README](../../README.md#rubyesque-matz-like-layer) and the
[ROADMAP](../../ROADMAP.md#rubyesque-matz-like-layer).

Run them from the repository root like any other example (`bundle exec` keeps
the checkout's freshly built extension from being shadowed by an installed gem):

```
bundle exec ruby -Ilib examples/rubyesque/window.rb
```

The ones that open a window need a display; headless machines can set the
`SFML_EXAMPLE_FRAMES` hook (see [`../support.rb`](../support.rb)) or prefix
`xvfb-run -a`:

```
SFML_EXAMPLE_FRAMES=120 xvfb-run -a bundle exec ruby -Ilib examples/rubyesque/events.rb
```

`system.rb` and `deprecations.rb` print to the console and exit on their own.

## The scripts

| File | What it teaches |
| --- | --- |
| `window.rb` | Scoped `Window.open`, `poll_events!` as a block and as an `Enumerator`, the one-call `render!` frame, `open?`/`focused?`/`visible?`, `request_focus!` |
| `events.rb` | The `event.*?` predicates and `event.code`, including the focus/mouse/text/touch/joystick kinds |
| `audio.rb` | `play!`/`pause!`/`stop!` with `playing?`/`paused?`/`stopped?` on `Sound` and `Music`, and the scoped `SoundBufferRecorder.record!` |
| `input.rb` | `Keyboard.key_pressed?`, `Joystick.axis?`, `Sensor.enable!`/`disable!`/`value`, `Touch.down?`/`Touch.position(relative_to:)` |
| `system.rb` | `Clock.measure`, `Clock#restart!`/`running?`, `SFML.sleep!`/`SFML::Sleep.sleep!`, the `Clipboard.content` pair with `has_text?`/`clear!` |
| `deprecations.rb` | The pre-Rubyesque names, still working and printing their deprecation warning |

## Name cheat sheet

`?` asks a question, `!` changes state. The old spelling keeps working but
warns, so there is nothing to migrate in a hurry.

| Before (deprecated)            | Now (primary)                          |
| ------------------------------ | -------------------------------------- |
| `window.is_open?`              | `window.open?`                         |
| `window.focus?`                | `window.focused?`                      |
| `window.request_focus`         | `window.request_focus!`                |
| `window.clear`                 | `window.clear!`                        |
| `window.display`               | `window.display!`                      |
| `window.poll_event!(event)`    | `window.poll_events! { \|event\| }`    |
| `sound.play` / `pause` / `stop` | `sound.play!` / `pause!` / `stop!`     |
| `sound.status == :playing`     | `sound.playing?` (also `paused?`, `stopped?`) |
| `Keyboard.pressed?`            | `Keyboard.key_pressed?`                |
| `Joystick.has_axis?`           | `Joystick.axis?`                       |
| `Clipboard.string` / `string=` | `Clipboard.content` / `content=`       |
| `SFML.sleep`                   | `SFML.sleep!` (or `SFML::Sleep.sleep!`) |

New in the Rubyesque layer, with no pre-Rubyesque equivalent: `WindowBase.open` /
`Window.open`, `window.render!`, `SoundBufferRecorder.record!`, `Clock.measure`,
`Sensor.enable!` / `disable!`, `Clipboard.has_text?` / `clear!`,
`Touch.position(finger, relative_to: window)` and every `event.*?` predicate.
