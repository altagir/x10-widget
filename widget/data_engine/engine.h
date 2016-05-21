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

#ifndef X10_ENGINE_H
#define X10_ENGINE_H

#include <Plasma/DataEngine>
#include <QWaitCondition>
#include <map>

#include <common/command.h>
#include <common/interfacehandler.h>

class X10Service;
class Container;
class IDBus;

class X10Engine : public Plasma::DataEngine
{
    Q_OBJECT

    friend class Container;

public:
    X10Engine(QObject* parent, const QVariantList& args);
    ~X10Engine();

    CaCyberdineX10Interface* GetDbusInterface() {
        return m_dbus->iX10();
    }
    IDBus* GetIDbus() {
        return m_dbus;
    }

    Plasma::Service* serviceForSource(const QString& source);

    bool updateSourceEvent(const QString& source);
    void updateAllSources();

protected:
    void init();

    bool sourceRequestEvent(const QString& source);

    void addX10Source(const QString& address);
    QString NodesList();

    bool FetchValue   (const QString& source, int&     value);
    bool FetchProperty(const QString& name,   QString& value);

    bool ConnectToDBUS();

private slots:
    void onSourceUpdated(QString source, int value);
    void onPropertyUpdated(QString property, QString value);
    void onCommandCompleted(QString channel, uint unit, QString command, int value);

    void sourceWasRemoved(const QString& source);

private:
    std::map<QString, Container*>   Containers;
    typedef std::map<QString, Container*>::iterator	ContainersIterator;

    IDBus*                      m_dbus;

    // this one is Plasma service.operations (start/stop/allon/...)
    X10Service*                 m_service;
};


#endif // X10_ENGINE_H
