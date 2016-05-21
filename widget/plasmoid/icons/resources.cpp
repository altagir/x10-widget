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

#include <common/x10config.h>

#include "resources.h"

ResManager* ResManager::m_instance = 0;

ResManager::ResManager() :
    m_iconOn(IconName(ICON_ON)),
    m_iconOnTimer(IconName(ICON_ON_TIMER)),
    m_iconOff(IconName(ICON_OFF)),
    m_iconDown(IconName(ICON_DOWN)),
    m_iconUp(IconName(ICON_UP)),
    m_iconAppOn(IconName(ICON_APPON)),
    m_iconAppOff(IconName(ICON_APPOFF)),
    m_iconAppPowering(IconName(ICON_APPPOWERING)),
    m_iconRules(IconName(ICON_RULES))
{
}

QIcon ResManager::Icon(EICON icon)
{
    switch (icon)
    {
    case ICON_ON:
        return m_iconOn;
    case ICON_ON_TIMER:
        return m_iconOnTimer;
    case ICON_OFF:
        return m_iconOff;
    case ICON_DOWN:
        return m_iconDown;
    case ICON_UP:
        return m_iconUp;
    case ICON_APPON:
        return m_iconAppOn;
    case ICON_APPOFF:
        return m_iconAppOff;
    case ICON_APPPOWERING:
        return m_iconAppPowering;
    case ICON_RULES:
        return m_iconRules;
    default:
        return QIcon("dialog-cancel");
    }
}

QString ResManager::IconName(EICON icon)
{
    switch (icon)
    {
    case ICON_ON:
        return QString(CMAKE_INSTALL_PREFIX) + "/share/x10-widget/light_on.png";
    case ICON_ON_TIMER:
        return QString(CMAKE_INSTALL_PREFIX) + "/share/x10-widget/light_on_timer.png";
    case ICON_OFF:
        return QString(CMAKE_INSTALL_PREFIX) + "/share/x10-widget/light_off.png";
    case ICON_DOWN:
        return QString(CMAKE_INSTALL_PREFIX) + "/share/x10-widget/arrowdown.png";
    case ICON_UP:
        return QString(CMAKE_INSTALL_PREFIX) + "/share/x10-widget/arrowup.png";
    case ICON_APPON:
        return QString(CMAKE_INSTALL_PREFIX) + "/share/x10-widget/connect_established.png";
    case ICON_APPOFF:
        return QString(CMAKE_INSTALL_PREFIX) + "/share/x10-widget/connect_no.png";
    case ICON_APPPOWERING:
        return QString(CMAKE_INSTALL_PREFIX) + "/share/x10-widget/connect_creating.png";
    case ICON_RULES:
        return QString(CMAKE_INSTALL_PREFIX) + "/share/x10-widget/clock.png";
    default:
        return "";
    }
}
