---
layout: default
title: Text & Fonts
parent: "Part I — Essential"
grand_parent: Book
nav_order: 7
---

# Recipe: Text & Fonts

`Font` loads a typeface once; `Text` is a drawable that renders a string with it. This recipe puts the whole `Text` surface on one screen: sizes, the five styles, fill and outline, letter and line spacing, centring from measured bounds, and the measuring helpers.

> **Essential**

## Goal

A 900×620 window showing the same sentence at several sizes, a row of styles (regular, bold, italic, bold+italic, underlined, strike-through), an outlined label, a multi-line block, a label centred inside a box using its measured bounds, and a live measurement read-out. `+`/`-` change the size, `S` cycles the style, `L` toggles spacing, escape quits.

## How the code works

### 1. Load SFML and the shared text helper

`Font.from_file` loads the face once into `FONT`; the `text` helper takes optional `size`, `position`, `color` and `style` keywords so labels can be built in one call.

{% example ruby examples/graphics/text_basics/text_basics.rb 13-28 %}

### 2. Open the window and build the sample labels

After opening and frame-capping the window, `SENTENCE` and `STYLES` define the content, and the size row is built by mapping sizes to labels with `with_index`.

{% example ruby examples/graphics/text_basics/text_basics.rb 30-45 %}

### 3. Outline, multiline and centring

`outline_thickness=`/`outline_color=` border the glyphs, `"one\ntwo\nthree"` shows line breaking, and a label is centred by setting `origin` to half of its `local_bounds.size`.

{% example ruby examples/graphics/text_basics/text_basics.rb 47-61 %}

### 4. Cycle style and spacing

`:s` advances `style_index` through `STYLES` and assigns `sample.style`, while `:l` toggles `sample.letter_spacing` and `multiline.line_spacing`.

{% example ruby examples/graphics/text_basics/text_basics.rb 63-77 %}

### 5. Resize and re-centre

`:Equal`/`:Hyphen` change `sample.character_size` within 8–48 and mirror it onto the centred label, re-reading `local_bounds` so the origin stays centred.

{% example ruby examples/graphics/text_basics/text_basics.rb 78-88 %}

### 6. Measure the text and font metrics

Each frame reads `sample.local_bounds`, a `Glyph` from `FONT.glyph`, `find_character_pos(4)`, and the `FONT.kerning`/`FONT.line_spacing` values for the read-out.

{% example ruby examples/graphics/text_basics/text_basics.rb 92-96 %}

### 7. Draw the labelled drawables

`window.clear!` starts the frame, then the box, size row, outlined, multiline, centred and sample labels are emitted with `window.draw`.

{% example ruby examples/graphics/text_basics/text_basics.rb 98-106 %}

### 8. Draw the measurement HUD and present

One `Text` reports style, size, spacing, measured bounds, character position, glyph size, coverage and kerning; `window.display!` presents the frame.

{% example ruby examples/graphics/text_basics/text_basics.rb 108-120 %}

## Ingredients

| Class | Subsystem | Role | Key methods |
| --- | --- | --- | --- |
| `Font` | Graphics | The loaded typeface | `from_file`, `glyph`, `has_glyph?`, `kerning`, `line_spacing` |
| `Text` | Graphics | A drawable string | `new`, `string=`, `character_size=`, `style=`, `letter_spacing=`, `line_spacing=`, `fill_color=`, `outline_thickness=`, `outline_color=`, `local_bounds`, `global_bounds`, `find_character_pos` |
| `Glyph` | Graphics | One rendered character's metrics | `advance`, `bounds`, `texture_rect` |
| `Rect` | Graphics | Bounds returned by the measurement calls | `size`, `left`, `top` |
| `Window` / `Event` | Window | Surface and keys | `draw`, `poll_events!` |

See the [Graphics API]({% link api/graphics.md %}) for the full signature of each.

## The complete script

The complete program, ready to copy into `examples/graphics/text_basics/text_basics.rb` and run.

{% example ruby examples/graphics/text_basics/text_basics.rb %}

{: .note }
> Fonts and text are both `Drawable`, so `window.draw` takes them alongside shapes. Measure with `local_bounds` for layout and `global_bounds` after transforms; the font metrics (`kerning`, `line_spacing`) are what a layout engine would use.
