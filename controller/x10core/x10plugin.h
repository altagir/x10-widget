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

#ifndef X10PLUGIN_H
#define X10PLUGIN_H

#include <string>


class X10plugin
{
public:
    typedef enum
    {
        LIBUSB,
        CM19A_DRV,
        NONE
    } X10_PLUGIN_TYPE;

    X10plugin() {}
    virtual ~X10plugin() {}

    bool Status;

    virtual bool Open() = 0;
    virtual void Close() = 0;

    virtual X10_PLUGIN_TYPE Type() = 0;

    virtual int send(struct x10_ha_command* cmd) = 0;
    virtual int recv(struct x10_ha_command* cmd) = 0;

};

#endif // X10PLUGIN_H
