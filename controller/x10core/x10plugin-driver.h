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

#ifndef X10DRIVER_H
#define X10DRIVER_H


#include "x10codes.h"
#include "x10plugin.h"

class X10Plugin_driver : public X10plugin
{
public:
    static const int MAX_CONTROLLER = 5;

    X10Plugin_driver();

    X10_PLUGIN_TYPE Type() {
        return CM19A_DRV;
    }

    static bool HasController();

    bool Open();
    void Close();

    int send(struct x10_ha_command* cmd);
    int recv(struct x10_ha_command* cmd);

private:
    int Init();

    std::string  m_handles[MAX_CONTROLLER];
};

#endif // X10DRIVER_H
