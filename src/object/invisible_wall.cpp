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

#include "object/invisible_wall.hpp"

#include "supertux/object_factory.hpp"
#include "util/gettext.hpp"
#include "util/reader_mapping.hpp"

InvisibleWall::InvisibleWall(const ReaderMapping& lisp):
  width(),
  height()
{
  if (!lisp.get("name" , name)) name = "";
  if (!lisp.get("x", bbox.p1.x)) bbox.p1.x = 0;
  if (!lisp.get("y", bbox.p1.y)) bbox.p1.y = 0;
  if (!lisp.get("width", width)) width = 32;
  if (!lisp.get("height", height)) height = 32;

  bbox.set_size(width, height);
}

void
InvisibleWall::save(Writer& writer) {
  MovingSprite::save(writer);
  writer.write("width", width);
  writer.write("height", height);
}

ObjectSettings
InvisibleWall::get_settings() {
  ObjectSettings result(_("Invisible wall"));
  result.options.push_back( ObjectOption(MN_TEXTFIELD, _("Name"), &name));
  result.options.push_back( ObjectOption(MN_NUMFIELD, _("Width"), &width));
  result.options.push_back( ObjectOption(MN_NUMFIELD, _("Height"), &height));

  return result;
}

HitResponse
InvisibleWall::collision(GameObject& , const CollisionHit& )
{
  return FORCE_MOVE;
}

void
InvisibleWall::draw(DrawingContext& )
{
}

void
InvisibleWall::update(float )
{
}

/* EOF */
