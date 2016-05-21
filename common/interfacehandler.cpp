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

#include <common/logger.h>
#include <common/operations.h>
#include <common/config.h>

#include "interfacehandler.h"

const char DBUS_Service[]      = "ca.cyberdine.x10";
const char DBUS_sessionPath[]  = "/";
const char DBUS_sessionIface[] = "ca.cyberdine.x10";


IDBus::IDBus(QObject* parent):
    QObject(parent),
    m_dbusNameIndex(0),
    m_dbusConnection(0),
    m_dbusInterface(0)
{
//     Connect();
}

IDBus::~IDBus()
{
    Disconnect();
}


bool IDBus::Reconnect()
{
    if (isConnected()) return true;
    // TODO not every call -> put timer

    Disconnect();
    return Connect();
}

bool IDBus::Connect()
{
    QMutexLocker locker(&m_mutexConnnection);

    time(&m_lastConnect);
    Config config;

    QString newDbusLocation = (config.USE_LOCAL_SYSTEM_BUS)?
                              PROPVAL_CONTROLLER_SYSTEMDBUS:
                              QString("%1:%2").arg(config.CONTROLLER_HOST).arg(config.CONTROLLER_PORT);

    const bool locationChanged = (m_dbusLocation != newDbusLocation);
    m_dbusLocation  = newDbusLocation;

    Logger::Log(DEBUG, "Connecting to %s", qPrintable(m_dbusLocation));

    m_dbusName = DBUS_name + QString::number(m_dbusNameIndex++);

    if (config.USE_LOCAL_SYSTEM_BUS)
        m_dbusConnection = new QDBusConnection ( QDBusConnection::connectToBus (QDBusConnection::SystemBus, m_dbusName));
    else
        m_dbusConnection = new QDBusConnection ( QDBusConnection::connectToBus (config.DBUS_ADDRESS, m_dbusName));

    if (!m_dbusConnection->isConnected())
        Logger::Log(ERROR, "QDBusConnection Failed : %s",
                    qPrintable(m_dbusConnection->lastError().errorString(m_dbusConnection->lastError().type())) );

    m_dbusInterface = new CaCyberdineX10Interface("ca.cyberdine.x10", "/", *m_dbusConnection);

    QObject::connect(m_dbusInterface, SIGNAL(SourceUpdated(QString,int)),
                     this, SLOT(onSourceUpdated(QString,int)));

    QObject::connect(m_dbusInterface, SIGNAL(PropertyUpdated(QString,QString)),
                     this, SLOT(onPropertyUpdated(QString,QString)));

    QObject::connect(m_dbusInterface, SIGNAL(CommandCompleted(QString, uint, QString, int)),
                     this, SLOT(onCommandCompleted(QString, uint, QString, int)));

    Logger::Log(DEBUG, "Connect dbus=%s iface=%s connected=%s",
                m_dbusConnection->isConnected()?"TRUE":"FALSE",
                m_dbusInterface->connection().isConnected()?"TRUE":"FALSE",
                isConnected()?"TRUE":"FALSE" );

    if (locationChanged)
        emit PropertyUpdated(PROPERTY_CONTROLLER_STR, m_dbusLocation);

    emit PropertyUpdated(PROPERTY_CONTROLLER_STATUS, QString(isConnected()) );

    return true;
}

bool IDBus::Disconnect()
{
    QMutexLocker locker(&m_mutexConnnection);
    Logger::Log(DEBUG, "Disconnect from %s", qPrintable(m_dbusName) );

    if (m_dbusInterface)
    {
        m_dbusInterface->disconnect(SIGNAL(SourceUpdated(QString, int)), this);
        m_dbusInterface->disconnect(SIGNAL(PropertyUpdated(QString, QString)), this);
        m_dbusInterface->disconnect(SIGNAL(CommandCompleted(QString, uint, QString, int)), this);
        m_dbusInterface->disconnect(SIGNAL(destroyed(QObject*)), this);

        delete m_dbusInterface;
        m_dbusInterface = 0;
    }
    if (m_dbusConnection)
    {
        m_dbusConnection->disconnectFromBus(m_dbusName);
        delete m_dbusConnection;
        m_dbusConnection = 0;
    }
    QDBusConnection::disconnectFromBus(m_dbusName);
    return true;
}

bool IDBus::isConnected()
{
    QDBusPendingReply<QString> reply = m_dbusInterface->GetProperty(PROPERTY_STATUS);
    reply.waitForFinished();
    return reply.isValid();
}

void IDBus::onCommandCompleted(QString channel, uint unit, QString command, int value)
{
    Logger::Log(DEBUG, "Command Sent %s%d %s, %d", qPrintable(channel), unit, qPrintable(command), value);
    emit CommandCompleted(channel, unit, command, value);
}

void IDBus::onPropertyUpdated(QString property, QString value)
{
    Logger::Log(DEBUG, "Property Updated %s = %s", qPrintable(property), qPrintable(value));
    emit PropertyUpdated(property, value);
}

void IDBus::onSourceUpdated(QString source, int value)
{
    Logger::Log(DEBUG, "Source Updated %s = %s", qPrintable(source), qPrintable(value));
    emit SourceUpdated(source, value);
}
