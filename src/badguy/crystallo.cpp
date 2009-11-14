//  SuperTux - Crystallo
//  Copyright (C) 2008 Wolfgang Becker <uafr@gmx.de>
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

#include <config.h>

#include "crystallo.hpp"

#include "object_factory.hpp"
#include "lisp/lisp.hpp"
#include "sprite/sprite.hpp"

Crystallo::Crystallo(const lisp::Lisp& reader)
  : WalkingBadguy(reader, "images/creatures/crystallo/crystallo.sprite", "left", "right")
{
  walk_speed = 80;
  max_drop_height = 16;
  radius = 100;
  reader.get("radius", radius);
}

Crystallo::Crystallo(const Vector& pos, Direction d)
  : WalkingBadguy(pos, d, "images/creatures/crystallo/crystallo.sprite", "left", "right")
{
  walk_speed = 80;
  max_drop_height = 16;
  radius = 100;
}

void
Crystallo::active_update(float elapsed_time)
{
  if(get_pos().x > (start_position.x + radius)){
    if(dir != LEFT){
      turn_around();
    }
  }
  if( get_pos().x < (start_position.x - radius)){
    if(dir != RIGHT){
      turn_around();
    }
  }
  BadGuy::active_update(elapsed_time);
}

bool
Crystallo::collision_squished(GameObject& object)
{
  sprite->set_action(dir == LEFT ? "shattered-left" : "shattered-right");
  kill_squished(object);
  return true;
}

IMPLEMENT_FACTORY(Crystallo, "crystallo");