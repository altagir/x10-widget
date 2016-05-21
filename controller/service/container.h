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

#ifndef CONTAINER_H
#define CONTAINER_H

#include <QObject>

class QueueManager;

class Container : QObject
{
    Q_OBJECT

public:
    explicit Container(const QString& _source, QueueManager* _manager);

    int     Value() const {
        return value;
    }

    // set to specific value & update clients
    void    SetValue(int _value);

    // set On/Off & update clients
    void    SetOn(bool on);
    // Dim up / down & update clients
    void    PushDirection(bool up);

private:
    QString         source;
    int             value;
    QueueManager*   manager;
};

#endif // CONTAINER_H
