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

#ifndef X10LIBUSB_H
#define X10LIBUSB_H

#include "libusbutils.h"
#include "x10plugin.h"


class X10Plugin_libusb : public X10plugin
{
public:
    X10Plugin_libusb();

    X10_PLUGIN_TYPE Type() {
        return LIBUSB;
    }

    bool Open();
    void Close();

    int send(struct x10_ha_command* cmd);
    int recv(struct x10_ha_command* cmd);

    static int x10_transmit_cmd(usb_dev_handle* portnum, struct x10_ha_command* cmd);
    static int x10_recv(usb_dev_handle* portnum, struct x10_ha_command* cmd);

private:
    static const int MAX_CONTROLLER = 5;

    usb_dev_handle* Init();
    void Close(usb_dev_handle* portnum);

    usb_dev_handle* m_handles[MAX_CONTROLLER];
};

#endif // X10LIBUSB_H
