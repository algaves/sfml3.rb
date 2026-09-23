---
layout: default
title: Audio
parent: API Reference
nav_order: 2
---

# Audio API

Sound playback, music streaming, recording, and listener spatialization.

## Key classes

| Class | Purpose |
|---|---|
| `SF::Audio::Sound` | Short sound effect playback |
| `SF::Audio::Music` | Streamed music playback |
| `SF::Audio::SoundBuffer` | Audio data buffer |
| `SF::Audio::SoundRecorder` | Audio capture device interface |
| `SF::Audio::SoundBufferRecorder` | Captures audio into a buffer |
| `SF::Audio::SoundStream` | Custom audio stream base |
| `SF::Audio::SoundSource` | Base for spatialized sources |
| `SF::Audio::Listener` | Global listener position/orientation/volume |

See `sig/audio/` and `ext/audio/` for full method signatures and documentation.
