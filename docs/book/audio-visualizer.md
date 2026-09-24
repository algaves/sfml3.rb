---
layout: default
title: Audio Visualizer
parent: "Part III — Intermediate"
grand_parent: Book
nav_order: 6
---

# Recipe: Audio Visualizer

Real-time amplitude bars from the raw PCM of a looping `SoundBuffer`, read via `SoundBuffer#samples`.

> **Intermediate**

## Goal

An 800×600 window showing 128 coloured bars whose heights track the waveform of `beep.wav` as it loops, mirrored around the vertical centre. Escape quits.

## How the code works

### 1. Setup helpers and constants

`Font.from_file` and `text` build the caption, and `BARS`, `WIDTH` and `HEIGHT` fix the layout. The bar count controls how finely the waveform is sliced.

{% example ruby examples/subsystems/audio_visualizer/audio_visualizer.rb 21-31 %}

### 2. A colour per bar

`bar_color` derives an RGB triple from the bar index with three phase-shifted sines, so the row of bars forms a smooth rainbow even though every bar is a flat `RectangleShape`.

{% example ruby examples/subsystems/audio_visualizer/audio_visualizer.rb 33-39 %}

### 3. Load and play the sound

`SoundBuffer.from_file` decodes `beep.wav`, `Sound.new` pairs it with a playback source, and `loop = true` plus `play!` start it. Playback is asynchronous, so the loop keeps rendering while it runs.

{% example ruby examples/subsystems/audio_visualizer/audio_visualizer.rb 41-44 %}

### 4. Read and slice the PCM

`buffer.samples` is the raw signed 16-bit data and `channel_count` says how many values make one frame. Dividing gives the frame count, and dividing that by `BARS` gives how many frames each bar averages.

{% example ruby examples/subsystems/audio_visualizer/audio_visualizer.rb 46-49 %}

### 5. Open the window and poll events

The window is capped at 60 fps and `poll_events!` closes it on the close button or Escape. Nothing else changes per frame because the waveform is static.

{% example ruby examples/subsystems/audio_visualizer/audio_visualizer.rb 51-62 %}

### 6. Draw each bar

For each bar the slice starts at `index * step` frames, and the value is the mean of that frame's channels. Dividing by `32_768.0` normalises to `-1.0`–`1.0`, giving a 2–250 pixel height that is centred vertically and coloured by `bar_color`.

{% example ruby examples/subsystems/audio_visualizer/audio_visualizer.rb 66-78 %}

### 7. Caption and present

A caption names the data source, then `display!` presents the finished frame before the loop re-checks `window.open?`.

{% example ruby examples/subsystems/audio_visualizer/audio_visualizer.rb 80-82 %}

## Ingredients

| Class | Subsystem | Role | Key methods |
| --- | --- | --- | --- |
| `SoundBuffer` | Audio | Decoded audio, source of the PCM | `from_file`, `samples`, `channel_count` |
| `Sound` | Audio | Playback source | `new`, `loop=`, `play!` |
| `RectangleShape` | Graphics | One bar per slice | `new`, `position=`, `fill_color=` |
| `Window` | Window | Render surface | `new`, `poll_events!`, `clear!`, `draw`, `display!` |
| `Text` / `Font` | Graphics | Caption | `Font.from_file`, `Text.new` |

See the [Audio API]({% link api/audio.md %}) for the full signature of each.

## The complete script

The complete program, ready to copy into `examples/subsystems/audio_visualizer/audio_visualizer.rb` and run.

{% example ruby examples/subsystems/audio_visualizer/audio_visualizer.rb %}

{: .note }
> The example loops the bundled `beep.wav`; swap the `from_file` path for any audio file. `samples` are 16-bit integers, so amplitude is normalized by 32768; for stereo it averages across channels because the frames are interleaved.
