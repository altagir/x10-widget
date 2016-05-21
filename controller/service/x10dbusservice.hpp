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

#ifndef X10DBUSSERVICE_HPP
#define X10DBUSSERVICE_HPP

#include <QObject>
#include <common/command.h>

class QueueManager;

class X10DbusService : public QObject
{
    Q_OBJECT

    Q_CLASSINFO("D-Bus Interface", "ca.cyberdine.x10")

public Q_SLOTS:

    void Send(const QString& channel, const unsigned int unit, const QString& command);
    void Set (const QString& channel, const unsigned int unit, const int value);

    void AllOn();
    void AllOff();

    int     GetValue    (const QString& source);

    QString GetProperty (const QString& name);
    void    SetProperty (const QString& name, const QString& value);

    // Queue Manager
    void Start();
    void Stop();
    void Restart();
    void Reload();  // force reloading config

    // Service
    void Quit();

Q_SIGNALS:
    void CommandCompleted(const QString& channel,
                          const unsigned int unit,
                          const QString& command,
                          const int value);

    void PropertyUpdated(const QString& name,
                         const QString& value);

    void SourceUpdated(const QString& address,
                       const int value);

public:
    explicit X10DbusService(QObject* parent = 0);
    virtual ~X10DbusService();

private Q_SLOTS:
    void OnQM_Exit();
    void OnQM_CommandCompleted(const COMMAND& cmd);
    void OnQM_StatusUpdate(const QString& status);
    void OnQM_RestartDBUS();

// private:
// 	bool RegisterDBusService();
// 	void DeregisterDBusService();
// 	bool ReregisterDBusService();

private:
    QueueManager*    queueManager;
};

#endif // X10DBUSSERVICE_HPP
