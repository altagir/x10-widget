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

#include <Plasma/DataContainer>
#include <QFlags>

class X10Engine;
class QDBusInterface;
class QDBusPendingCallWatcher;

class Container : public Plasma::DataContainer
{
//      Q_OBJECT

public:

    explicit Container(X10Engine* engine, const QString& source, QObject* parent = 0);

    Plasma::Service* service(QObject* parent = 0);

    int     Value() const   {
        return value;
    }
    // set to specific value & update clients
    void    SetValue(int _value);

public slots:
    void    updateInfo();

private:
    int     value;

    X10Engine* m_engine;
};

#endif // CONTAINER_H
