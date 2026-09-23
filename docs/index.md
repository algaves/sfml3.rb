---
layout: default
title: Home
nav_order: 1
description: "sfml3-rb — idiomatic CSFML 3 bindings for Ruby"
permalink: /
---

# sfml3-rb

Ruby bindings for [SFML 3](https://www.sfml-dev.org/), via its C API, [CSFML](https://github.com/SFML/CSFML).

[![Ruby](https://img.shields.io/badge/ruby-3.1%2B-red?style=flat-square)](https://www.ruby-lang.org/)
[![Test](https://img.shields.io/github/actions/workflow/status/algaves/sfml3.rb/test.yaml?style=flat-square)](https://github.com/algaves/sfml3.rb/actions/workflows/test.yaml)
[![Check](https://img.shields.io/github/actions/workflow/status/algaves/sfml3.rb/check.yaml?style=flat-square)](https://github.com/algaves/sfml3.rb/actions/workflows/check.yaml)
[![Docs](https://img.shields.io/badge/docs-GitHub%20Pages-4c9a2a?style=flat-square)](https://algaves.github.io/sfml3.rb/)
[![Gem Version](https://img.shields.io/gem/v/sfml3-rb?style=flat-square)](https://rubygems.org/gems/sfml3-rb)
[![Gem Downloads](https://img.shields.io/gem/dt/sfml3-rb?style=flat-square)](https://rubygems.org/gems/sfml3-rb)
[![License](https://img.shields.io/badge/license-0BSD-green?style=flat-square)](https://github.com/algaves/sfml3.rb/blob/main/LICENSE.md)

Latest release: **0.3.1**, bound against **CSFML 3**.

## Features

- **Broad coverage of SFML 3**, bound through CSFML: windows and events, graphics, audio, network, and the system layer.
- **A Rubyesque (Matz-like) layer** over the raw binding (`?` predicates, `!` mutators, block iterators, scoped resources, positional constructors).
- **Precompiled binary gems** for common platforms; source fallback builds SFML/CSFML at install time where needed.
- **Complete API documentation and types** with RBS signatures shipped in the gem.
- **A close fit to SFML's own model**, mirroring C++ types minus C++-only constructs.

## Quick links

- [Quick Start]({% link quick-start.md %})
- [Installation]({% link installation/index.md %})
- [Learn]({% link learn/index.md %})
- [Examples]({% link examples/index.md %})
- [API Reference]({% link api/index.md %})
