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

#include <QtDBus/QtDBus>

#include <common/X10DbusInterface.h>

class IDBus : public QObject
{
    Q_OBJECT

public:
    virtual ~IDBus();

    static IDBus* Get()
    {
        static IDBus* _instance = 0;
        if (!_instance) _instance = new IDBus();
        return _instance;
    }

    static CaCyberdineX10Interface* X10()
    {
        return IDBus::Get()->iX10();
    }

    CaCyberdineX10Interface* iX10()
    {
        if (!m_dbusInterface) Connect();
        return m_dbusInterface;
    }

    QString DBusLocation() {
        return m_dbusLocation;
    }

    bool Connect();
    bool Reconnect(); // restart dbus if not connected
    bool isConnected();

signals:
    void SourceUpdated(QString source, int value);
    void PropertyUpdated(QString property, QString value);
    void CommandCompleted(QString channel, uint unit, QString command, int value);

protected:
    explicit IDBus(QObject* parent = 0);

    bool Disconnect();

private:
    QString                     m_dbusName; // "x10bus" + m_dbusNameIndex
    int                         m_dbusNameIndex;
    QDBusConnection*            m_dbusConnection;
    CaCyberdineX10Interface*    m_dbusInterface;
    QString                     m_dbusLocation;

    QMutex                      m_mutexConnnection;

    time_t                      m_lastConnect;  // put a threshold on connections

private slots:
    void onSourceUpdated(QString source, int value);
    void onPropertyUpdated(QString property, QString value);
    void onCommandCompleted(QString channel, uint unit, QString command, int value);
};
