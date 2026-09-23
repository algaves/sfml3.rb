---
layout: default
title: Installation
nav_order: 3
has_children: true
permalink: /installation
---

# Installation

Install the gem:

```sh
gem install sfml3-rb
```

On platforms with a precompiled gem, FreeType, SFML 3 and CSFML 3 are already linked in — no toolchain required.

To link against a system CSFML 3 instead (no download/build):

```sh
gem install sfml3-rb -- --enable-system-libraries
```
