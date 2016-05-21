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

#ifndef RESMANAGER_H
#define RESMANAGER_H

#include <QIcon>

enum EICON
{
    ICON_ON,
    ICON_ON_TIMER,
    ICON_OFF,
    ICON_DOWN,
    ICON_UP,
    ICON_APPON,
    ICON_APPOFF,
    ICON_APPPOWERING,
    ICON_RULES
};

class ResManager
{
public:
    ResManager();

    QIcon   Icon(EICON icon);
    QString IconName(EICON icon);
    static ResManager* Get()
    {
        if (m_instance == 0)
            m_instance = new ResManager();
        return m_instance;
    }


private:
    QIcon   m_iconOn, m_iconOnTimer, m_iconOff, m_iconDown, m_iconUp;
    QIcon   m_iconAppOn, m_iconAppOff, m_iconAppPowering, m_iconRules;

    static  ResManager* m_instance;
};

#endif // RESMANAGER_H
