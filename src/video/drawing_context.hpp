//  SuperTux
//  Copyright (C) 2006 Matthias Braun <matze@braunis.de>
//
//  This program is free software: you can redistribute it and/or modify
//  it under the terms of the GNU General Public License as published by
//  the Free Software Foundation, either version 3 of the License, or
//  (at your option) any later version.
//
//  This program is distributed in the hope that it will be useful,
//  but WITHOUT ANY WARRANTY; without even the implied warranty of
//  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//  GNU General Public License for more details.
//
//  You should have received a copy of the GNU General Public License
//  along with this program.  If not, see <http://www.gnu.org/licenses/>.

#ifndef HEADER_SUPERTUX_VIDEO_DRAWING_CONTEXT_HPP
#define HEADER_SUPERTUX_VIDEO_DRAWING_CONTEXT_HPP

#include <string>
#include <vector>
#include <obstack.h>
#include <boost/optional.hpp>

#include "math/rect.hpp"
#include "math/rectf.hpp"
#include "math/vector.hpp"
#include "video/canvas.hpp"
#include "video/color.hpp"
#include "video/drawing_context.hpp"
#include "video/drawing_transform.hpp"
#include "video/font.hpp"
#include "video/font_ptr.hpp"

class VideoSystem;
struct DrawingRequest;
struct obstack;

/** This class provides functions for drawing things on screen. It
    also maintains a stack of transforms that are applied to
    graphics. */
class DrawingContext final
{
public:
  DrawingContext(VideoSystem& video_system, obstack& obst, bool overlay);
  ~DrawingContext();

  /** Returns the visible area in world coordinates */
  Rectf get_cliprect() const;

  /** on next update, set color to lightmap's color at position */
  void get_light(const Vector& position, Color* color_out);

  Canvas& color() { return m_colormap_canvas; }
  Canvas& light() { assert(!m_overlay); return m_lightmap_canvas; }
  Canvas& get_canvas(DrawingTarget target) {
    switch(target)
    {
      case DrawingTarget::LIGHTMAP:
        return light();

      default:
        return color();
    }
  }

  void set_ambient_color(Color ambient_color);
  Color get_ambient_color() const { return m_ambient_color; }

  void push_transform();
  void pop_transform();
  DrawingTransform& transform();
  const DrawingTransform& transform() const;

  const Vector& get_translation() const
  {  return transform().translation;  }

  void set_translation(const Vector& newtranslation)
  {  transform().translation = newtranslation;  }

  /** Apply that effect in the next draws (effects are listed on surface.h). */
  void set_drawing_effect(DrawingEffect effect);
  DrawingEffect get_drawing_effect() const;

  /** apply that alpha in the next draws (1.0 means fully opaque) */
  void set_alpha(float alpha);
  float get_alpha() const;

  void clear()
  {
    m_lightmap_canvas.clear();
    m_colormap_canvas.clear();
  }

  void set_viewport(const Rect& viewport)
  {
    m_viewport = viewport;
  }

  const Rect& get_viewport() const { return m_viewport; }

  int get_width() const { return m_viewport.get_width(); }
  int get_height() const { return m_viewport.get_height(); }

  bool use_lightmap() const
  {
    return !m_overlay && m_ambient_color != Color::WHITE;
  }

  bool is_overlay() const { return m_overlay; }

private:
  VideoSystem& m_video_system;

  /** obstack holds the memory of all the drawing requests, it is
      shared with the Canvas */
  obstack& m_obst;

  /** A context marked as overlay will not have it's light section
      rendered. */
  bool m_overlay;

  Rect m_viewport;
  Color m_ambient_color;
  std::vector<DrawingTransform> m_transform_stack;

  Canvas m_colormap_canvas;
  Canvas m_lightmap_canvas;

private:
  DrawingContext(const DrawingContext&);
  DrawingContext& operator=(const DrawingContext&);
};

#endif

/* EOF */
