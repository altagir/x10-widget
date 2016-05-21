/*
 *  Copyright (C) 2013 Sébastien Sénéchal <altagir@gmail.com>
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 3 of the License, or
 *  any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program. If not, see <http://www.gnu.org/licenses/>.
 *
 */

#include "container.h"
#include "queue-manager.h"

#include <common/command.h>

Container::Container(const QString& _source, QueueManager* _manager):
    QObject(),
    source(_source),
    value(-1),
    manager(_manager)
{
}

void Container::SetValue(int _value)
{
    value = _value;
    emit manager->UpdateSource(source, value);
}

void Container::SetOn(bool on)
{
    if (!on)
        SetValue(-1);
    else if (value == -1)
        SetValue(MAX_VALUE);
}

void Container::PushDirection(bool up)
{
    if (value == -1) // off?
    {
        SetValue(MAX_VALUE - (up ? 0 : 1));
    }
    else if (up && value != MAX_VALUE)
    {
        SetValue(value + 1);    // brighgen
    }
    else if (!up && value != 0)
    {
        SetValue(value - 1);    // soften
    }
}
