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

#ifndef X10UTILS_H
#define X10UTILS_H

#include "x10codes.h"


// direct usb access, /tmp/bus must be mounted mount --bind /dev/bus/usb /tmp/usb -o devmode=0666
// else use driver echo "+e2" > /dev/cm19a[0-9]

#include "x10plugin.h"

class X10Plugin_libusb;
class X10Plugin_driver;
class usb_dev_handle;

class X10Controller
{
public:
    X10Controller();
    virtual ~X10Controller();

    bool Load();
    void Unload();

    X10plugin::X10_PLUGIN_TYPE Type() const;

    int x10_send(struct x10_ha_command* cmd);
    int x10_recv(struct x10_ha_command* cmd);

    bool IsOperational;

private:
    X10plugin::X10_PLUGIN_TYPE detectPluginType();

    static const int    c_waitTimeStandard = 950000;
    static const int    c_waitTimePanTilt  = 950000;
    inline int  x10_waitTime(const CmdCode cmdCode) const;

    X10plugin*      m_driver;

    HouseCode       m_lastHouse;
    UnitCode        m_lastUnit;
};

#endif // X10UTILS_H


