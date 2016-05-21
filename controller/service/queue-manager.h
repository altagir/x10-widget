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

#ifndef QUEUE_MANAGER_H
#define QUEUE_MANAGER_H

#include <QThread>
#include <QMutex>
#include <QWaitCondition>
#include <QStringList>

#include <string>
#include <list>
#include <map>

using namespace std;

#include <common/command.h>
#include <common/config.h>

#include "rules-manager.h"

class X10Controller;
class Container;

typedef enum
{
    QM_STOPPING,
    QM_STOPPED,
    QM_STARTING,
    QM_LOADING,
    QM_STARTED,
    QM_NOCONTROLLER
} QueueManagerStatus;


class QueueManagerReader : public QThread
{
    Q_OBJECT

public:
    QueueManagerReader(X10Controller* _X10Controller, QObject* parent = 0);
    ~QueueManagerReader();

    void Start();
    void Stop();

protected:
    void run();

private:
    X10Controller* m_X10Controller;
    bool m_isRunning;
};

class QueueManager : public QThread
{
    Q_OBJECT

public:
    QueueManager(QObject* parent = 0);
    ~QueueManager();

    /// The refs
    map<QString, Container*>   Containers;
    QString NodesList();
    QString RulesList();

    /// STATUS
    void Start();
    void Stop();
    void Reload();

    QueueManagerStatus  Status();
    QString             StatusStr();

    Config          	config;

    // COMMANDS
    void QueueCommand(COMMAND_TYPE command, Channel channel, Unit unit, int value);
    void QueueCommand(const COMMAND& command);

    void SetAllTo(bool on);

    bool SetProperty(const QString& property, const QVariant& value);

    int  GetValue    (const QString& source);
    void UpdateSource(const QString& source, const int value);

signals:
    void SourceUpdated(const QString& address,
                       const int value);

    // Signal émis quand la commande est a ete execute
    void CommandReceived(/*COMMAND command*/);
    void CommandCompleted(COMMAND command);

    void StatusUpdate(QString status);

    void RestartDBUS();

protected slots:
    void on_RuleExecute(Rule rule);

protected:
    void run();
    void SetStatus(QueueManagerStatus status);
    void CheckService();

    bool ControllerLoad();
    void ControllerUnload();

private:
    static QString StatusToStr(const QueueManagerStatus& status);
    int     GetNextCommand(const COMMAND& setCommand, COMMAND** outputCmd);
    void    CheckContainerExist(const QString& address);

    bool                m_isRunning;
    QueueManagerStatus  m_status;
    bool                m_controllerLoaded;

    X10Controller*  m_X10Controller;
    RuleManager	    m_ruleManager;

    list<COMMAND*>  m_queueCommands;

    QWaitCondition  queueNotEmpty;
    QMutex          m_mutex;
    QMutex          m_mutexService;
    QMutex          m_mutexStatus;
};

#endif // QUEUE_MANAGER_H
