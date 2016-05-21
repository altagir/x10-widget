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

#include "engine.h"
#include "container.h"
#include "service.h"

#include <common/logger.h>
#include <common/operations.h>

using namespace std;

X10Engine::X10Engine(QObject* parent, const QVariantList& args) :
    DataEngine(parent), //, args)
    m_service(0)
{
    // We ignore any arguments - data engines do not have much use for them
    Q_UNUSED(args)

    setName("x10"); // X-KDE-PluginInfo-Name in .desktop

    connect(this, SIGNAL(sourceRemoved(QString)), SLOT(sourceWasRemoved(QString)));

    m_dbus = IDBus::Get();

    QObject::connect(m_dbus, SIGNAL(SourceUpdated(QString, int)),
                     this, SLOT(onSourceUpdated(QString, int)));

    QObject::connect(m_dbus, SIGNAL(PropertyUpdated(QString, QString)),
                     this, SLOT(onPropertyUpdated(QString, QString)));

    QObject::connect(m_dbus, SIGNAL(CommandCompleted(QString, uint, QString, int)),
                     this, SLOT(onCommandCompleted(QString, uint, QString, int)));

    // connect later to receive above events when connecting.
    m_dbus->Connect();

    // This prevents applets from setting an unnecessarily high
    // update interval and using too much CPU.
    // In the case of a clock that only has second precision,
    // a third of a second should be more than enough.
    setMinimumPollingInterval(500);
}

X10Engine::~X10Engine()
{
    delete m_dbus;
}

void X10Engine::init()
{
    Plasma::DataEngine::init();
    updateSourceEvent(SOURCE_PROPERTIES);
}

bool X10Engine::sourceRequestEvent(const QString& source)
{
    Logger::Log(DEBUG, "Engine: new Source Request Event: %s", qPrintable(source));

    // We do not have any special code to execute the
    // first time a source is requested, so we just call
    // updateSourceEvent().
    return updateSourceEvent(source);
}

bool X10Engine::updateSourceEvent(const QString& source)
{
    Logger::Log(DEBUG, "Engine: updateSourceEvent: %s", qPrintable(source));

    if (source == SOURCE_HELP)
    {
        setData(SOURCE_HELP, "Use an address 'e5' to get information for that address.\n"
                "Use 'properties' to get a list of all properties that may be returned."
               );
        return true;
    }
    else if (source == SOURCE_PROPERTIES)
    {
        QString value;

        // STATUS
        if (FetchProperty(PROPERTY_STATUS, value))
            setData(SOURCE_PROPERTIES, PROPERTY_STATUS, value);

        // SMART QUEUE
        if (FetchProperty(PROPERTY_SMARTQUEUE, value))
            setData(SOURCE_PROPERTIES, PROPERTY_SMARTQUEUE, value);

        // RULES
        if (FetchProperty(PROPERTY_RULES, value))
            setData(SOURCE_PROPERTIES, PROPERTY_RULES, value);

        // NODE LIST
        setData(SOURCE_PROPERTIES, PROPERTY_NODES, NodesList());

        // CONTROLLER
        setData(SOURCE_PROPERTIES, PROPERTY_CONTROLLER_STR,    m_dbus->DBusLocation());
        setData(SOURCE_PROPERTIES, PROPERTY_CONTROLLER_STATUS, m_dbus->isConnected());

        return true;
    }
    else if (X10::isValidAddress(source))
    {
        // create source if required, and update.
        if (Containers.find(source) == Containers.end())
            addX10Source(source);

        Logger::Log(DEBUG, "Engine: update value %d", Containers[source]->Value());

        setData(source, "Value", Containers[source]->Value());
        return true;
    }

    Logger::Log(ERROR, "Engine: rejected source: %s", qPrintable(source));

    return false;
}

void X10Engine::addX10Source(const QString& source)
{
    Logger::Log(DEBUG, "Engine::adding new X10 Source %s", qPrintable(source));

    const ContainersIterator it = Containers.find(source);
    if (it == Containers.end())
    {
        Container* domContainer = new Container(this, source, this);
        Containers[source] = domContainer;

        // retrieve current value is known
        int value;
        if (FetchValue(source, value))
            domContainer->SetValue(value);

        addSource(domContainer);

        setData(SOURCE_PROPERTIES, PROPERTY_NODES, NodesList());
    }
    else
    {
        Logger::Log(ERROR, "Duplicate Container found - source refused");
        addSource(it->second);
    }
}

QString X10Engine::NodesList()
{
    QString list;
    for (map<QString, Container*>::const_iterator itr = Containers.begin();
            itr != Containers.end(); ++itr)
    {
        list.append((*itr).first);
        list.append(' ');   // separator
    }
    return list;
}

bool X10Engine::FetchValue(const QString& source, int& value)
{
    QDBusPendingReply<int> bret = m_dbus->iX10()->GetValue(source);
    bret.waitForFinished();

    if (bret.isValid())
    {
        value = bret.value();
        return true;
    }
    IDBus::Get()->Reconnect();
    return false;
}

bool X10Engine::FetchProperty(const QString& name, QString& value)
{
    QDBusPendingReply<QString> bret = m_dbus->iX10()->GetProperty(name);
    bret.waitForFinished();

    if (bret.isValid() && !bret.value().isEmpty())
    {
        value = bret.value();
        return true;
    }
    IDBus::Get()->Reconnect();
    return false;
}

//------------------------------------------------------------------------------

void X10Engine::onSourceUpdated(QString source, int value)
{
    Logger::Log(DEBUG, "Engine: Source Updated %s %d", qPrintable(source), value);
    if (Containers.find(source) != Containers.end())
    {
        Containers[source]->SetValue(value);
    }
}

void X10Engine::onPropertyUpdated(QString property, QString value)
{
    Logger::Log(INFO, "Engine : Property Updated %s = %s", qPrintable(property), qPrintable(value));
    setData(SOURCE_PROPERTIES, property, value);
}

void X10Engine::onCommandCompleted(QString channel, uint unit, QString command, int value)
{
    Logger::Log(DEBUG, "Engine: Command Completed %s%d %s val=%d", qPrintable(channel), unit, qPrintable(command), value);
    QString source = QString("%1%2").arg(qPrintable(channel)).arg(unit);
    if (Containers.find(source) != Containers.end())
    {
        setData(source, "LatestCmd", QString("%1%2").arg(qPrintable(command)).arg(value));
// 		updateSourceEvent(source);
    }
}

//------------------------------------------------------------------------------

void X10Engine::sourceWasRemoved(const QString& source)
{
    Logger::Log(INFO, "Engine::sourceWasRemoved %s", qPrintable(source));

    if (X10::isValidAddress(source))
    {
        if (Containers.find(source) != Containers.end())
        {
            Containers.erase(Containers.find(source));
            setData(SOURCE_PROPERTIES, PROPERTY_NODES, NodesList());
        }
    }
}

Plasma::Service* X10Engine::serviceForSource(const QString& source)
{
//  Logger::Log(DEBUG, "Engine::serviceForSource %s", qPrintable(source));

    if (X10::isValidAddress(source))
    {
        if (Containers.find(source) == Containers.end())
            addX10Source(source);

        return Containers[source]->service(this);
    }
    else if (source.isEmpty())
    {
//      Logger::Log(INFO, "serviceForSource: returning main service");
        // return main service
        return new X10Service(this, "");
    }

    // if source does not exist, return null service
    Logger::Log(ERROR, "serviceForSource: Invalid source %s", qPrintable(source));
    return Plasma::DataEngine::serviceForSource(source);
}

void X10Engine::updateAllSources()
{
    for (map<QString, Container*>::const_iterator itr = Containers.begin(); itr != Containers.end(); ++itr)
    {
        updateSourceEvent((*itr).first);
    }

//  Plasma::DataEngine::updateAllSources();
//  forceImmediateUpdateOfAllVisualizations();
}

// This does the magic that allows Plasma to load
// this plugin.  The first argument must match
// the X-Plasma-EngineName in the .desktop file.
// The second argument is the name of the class in
// your plugin that derives from Plasma::DataEngine
K_EXPORT_PLASMA_DATAENGINE(x10, X10Engine)

// this is needed since X10Engine is a QObject
#include "engine.moc"
