---
layout: default
title: Music & Event-Triggered Audio
parent: "Part IV — Advanced"
grand_parent: Book
nav_order: 4
---

# Recipe: Music & Event-Triggered Audio

A game's sound is mostly about **when** it plays. This recipe loops a music stream underneath and fires short effects from three kinds of trigger: a pickup (distance overlap), a bump (collision), and a chime (a timed "moment"), so you can see how each is wired to the game loop.

> **Advanced**

## Goal

An 820×560 window with a player, a gem, two patrolling blocks and a looping music bed. Collecting the gem plays a pickup sound, touching a patroller plays a bump, and every five seconds an independent chime fires. `M` pauses/resumes the music, `-`/`=` change its volume, escape quits.

## How the code works

### 1. Stream the music, buffer the effects

`Music.from_file` streams a track so long audio never sits fully in memory, while `SoundBuffer.from_file` decodes a short clip up front for a reusable `Sound`. The bed is set to loop at a low volume and started; playback is asynchronous, so the game loop keeps running underneath it.

{% example ruby examples/subsystems/music_triggers/music_triggers.rb 35-44 %}

### 2. Place the player and the gem

`CircleShape` objects for the player and the collectible gem use `origin=` to pivot about their centre and `position=` to place them. Both are ordinary drawables the loop can move and test against.

{% example ruby examples/subsystems/music_triggers/music_triggers.rb 46-54 %}

### 3. Describe the patrollers as data

A `Patroller` `Struct` pairs a shape with a velocity, and a small array maps plain numbers into those records so the loop can move and test them uniformly. The mutable counters and timer the loop updates are seeded just below.

{% example ruby examples/subsystems/music_triggers/music_triggers.rb 56-69 %}

### 4. Poll events to control the music

Every frame `window.poll_events!` drains the queue; `:m` toggles `music.play!`/`music.pause!`, and `-`/`=` adjust `music.volume` with a clamp, so the mix is driven entirely by input.

{% example ruby examples/subsystems/music_triggers/music_triggers.rb 71-86 %}

### 5. Move the player and fire the pickup

Arrow keys `move` the player, then a distance check against the gem acts as a trigger volume. On overlap the pickup `Sound` plays, the score rises, the screen flashes and the gem jumps to a new random spot.

{% example ruby examples/subsystems/music_triggers/music_triggers.rb 88-101 %}

### 6. Bounce the patrollers and fire the bump

Each patroller advances by its velocity and reverses on the walls; overlapping the player plays the bump sound, knocks the player back and flashes. Both effects can overlap because SFML mixes concurrent `Sound`s.

{% example ruby examples/subsystems/music_triggers/music_triggers.rb 103-117 %}

### 7. Fire the timed chime

A `Clock` restarted every five seconds triggers the chime — a "moment" event with no input involved, showing time-based triggers beside the spatial ones.

{% example ruby examples/subsystems/music_triggers/music_triggers.rb 119-125 %}

### 8. Draw the frame and the HUD

`window.clear!` erases the frame, an optional translucent `RectangleShape` paints the flash, the entities draw, and a `Text` HUD reports score, music state and status before `window.display!` presents it.

{% example ruby examples/subsystems/music_triggers/music_triggers.rb 127-144 %}

## Ingredients

| Class | Subsystem | Role | Key methods |
| --- | --- | --- | --- |
| `Music` | Audio | Streamed, looping music bed | `from_file`, `looping=`, `volume=`, `play!`, `pause!`, `status` |
| `Sound` / `SoundBuffer` | Audio | Short effects fired by triggers | `SoundBuffer.from_file`, `Sound.new`, `play!`, `volume=` |
| `Clock` | System | Drives the timed chime | `new`, `restart!`, `elapsed_time` |
| `CircleShape` / `RectangleShape` | Graphics | Player, gem and patrollers | `new`, `position=`, `move`, `fill_color=` |
| `Struct` (Ruby) | — | Holds each patroller's shape and velocity | `new` |
| `Keyboard` / `Event` | Window | Movement and music controls | `key_pressed?`, `poll_events!` |

See the [Audio API]({% link api/audio.md %}) and the [Playing Audio]({% link learn/playing-audio.md %}) tutorial for the full surface.

## The complete script

The complete program, ready to copy into `examples/subsystems/music_triggers/music_triggers.rb` and run.

{% example ruby examples/subsystems/music_triggers/music_triggers.rb %}

{: .note }
> `Sound` is for effects and `Music` for long tracks; both share the `SoundSource` controls (`volume`, `pitch`, `pan`, `position`). For spatialized sound, set an entity's `position` and enable `Listener` — see [Recording & Spatial Audio]({% link learn/audio-recording.md %}).
