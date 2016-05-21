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

#include "x10controller.h"

#include "x10plugin-driver.h"
#include "x10plugin-libusb.h"

#include "logger.h"

X10Controller::X10Controller() :
    m_driver(0)
{
}

X10Controller::~X10Controller()
{
    Unload();
}


bool X10Controller::Load()
{
    Unload();

    // first try driver
    switch (detectPluginType())
    {
    case X10plugin::LIBUSB:
        Logger::Log(INFO, "Using LIBUSB Plugin");
        m_driver = new X10Plugin_libusb();
        break;
    case X10plugin::CM19A_DRV:
        Logger::Log(INFO, "Using X10 Driver Plugin");
        m_driver = new X10Plugin_driver();
        break;
    case X10plugin::NONE:
        return 0;
    }

    if (m_driver)
    {
        Logger::Log(DEBUG, "Loading Plugin");
        m_driver->Open();
    }
    else
        Logger::Log(DEBUG, "No Plugin available");

    return (IsOperational = (m_driver && m_driver->Status));
}

void X10Controller::Unload()
{
    if (m_driver)
    {
        m_driver->Close();
        delete m_driver;
        m_driver = 0;
    }

    IsOperational = false;
}


int X10Controller::x10_recv(x10_ha_command* cmd)
{
    if (!IsOperational)
        return -1;

    return m_driver->recv(cmd);
}

int X10Controller::x10_send(x10_ha_command* cmd)
{
    if (!IsOperational || !cmd)
        return -1;

    int result = 0;

    if (((cmd->house != m_lastHouse) || (cmd->unit != m_lastUnit)) && (cmd->cmd == X10CMD_DIM || cmd->cmd == X10CMD_BRIGHT))
    {
        x10_ha_command* cmdOn = new x10_ha_command(X10CMD_ON, cmd->house, cmd->unit);
        x10_send(cmdOn);
        del_x10_ha_command(cmdOn);
    }

    bool hack = (cmd->cmd == X10CMD_DIM || cmd->cmd == X10CMD_BRIGHT);
    UnitCode oldunit = cmd->unit;
    cmd->unit = int_to_unit_code(unit_code_to_int(cmd->unit) - (hack ? 1 : 0));
    m_driver->send(cmd);
    cmd->unit = oldunit;

    m_lastHouse = cmd->house;
    m_lastUnit  = cmd->unit;

    char cmdStr[256];
    cmdStr[0] = 0;
    cmd->print(cmdStr);
    Logger::Log(INFO, "EXEC-> %s", cmdStr);

    if (result >= 0)
        usleep(x10_waitTime(cmd->cmd));

    return result;
}

X10plugin::X10_PLUGIN_TYPE X10Controller::Type() const
{
    if (!m_driver)
        return X10plugin::NONE;
    return m_driver->Type();
}


X10plugin::X10_PLUGIN_TYPE X10Controller::detectPluginType()
{
    if (X10Plugin_driver::HasController())
        return X10plugin::CM19A_DRV;
    else
        return X10plugin::LIBUSB;
}

int X10Controller::x10_waitTime(const CmdCode code) const
{
    // TODO FOR NOW check cam
    return c_waitTimeStandard;

    switch (code)
    {
        /* Standard 5-byte commands: */
    case X10CMD_ON:
    case X10CMD_OFF:
    case X10CMD_DIM:
    case X10CMD_BRIGHT:
        return c_waitTimeStandard;
        /* Pan'n'Tilt 4-byte commands: */
    case X10CMD_UP:
    case X10CMD_RIGHT:
    case X10CMD_DOWN:
    case X10CMD_LEFT:
        return c_waitTimeStandard; // TODO TOTEST shorter?
        /* Error flag */
    case X10CMD_INVALID:
    default:
        return 0;
    }
}
