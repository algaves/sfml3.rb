---
layout: default
title: Mobile (Android, iOS)
parent: Installation
nav_order: 2
---

# Mobile platforms (roadmap)

{: .note }
> Mobile targets (Android precompiled gems / iOS XCFramework) are **not yet available** for `sfml3-rb`.

## Current status

The platform table in the desktop guide reflects what's actively built and tested. Mobile support will be revisited when upstream CSFML/SFML packaging and Ruby ABI considerations allow it.

## What's needed

- Android: precompiled `.so` per ABI (arm64-v8a, armeabi-v7a, x86, x86_64) packaged in a gem or vendored with proper `require` paths.
- iOS: XCFramework distribution and integration with Ruby on iOS/mac Catalyst or cross-compilation strategy.

If this is important to you, please open an issue or discussion on [GitHub](https://github.com/algaves/sfml3.rb/issues).
