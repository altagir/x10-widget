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

#include "queue-manager.h"
#include "container.h"

#include <controller/x10core/x10controller.h>

#include <common/logger.h>
#include <common/operations.h>


QueueManager::QueueManager(QObject* parent) :
    QThread(parent),
    m_isRunning(false),
    m_status(QM_STOPPED),
    m_X10Controller(0),
    m_ruleManager(),
    m_queueCommands(),
    queueNotEmpty(),
    m_mutex(),
    m_mutexStatus()
{
    qRegisterMetaType<COMMAND>("COMMAND");

    config.Load();

    connect(&m_ruleManager, SIGNAL(RuleExecute(Rule)), this, SLOT(on_RuleExecute(Rule)));
}

QueueManager::~QueueManager()
{
    if (Status() != QM_STOPPED)
        Stop();

    if (m_X10Controller)
        delete m_X10Controller;

    config.Save();	// etc/x10.conf won't be writeable by default and we only want it RO
}

void QueueManager::Reload()
{
    Config configNew;

    const bool reloadDBUS = (configNew.USE_LOCAL_SYSTEM_BUS != config.USE_LOCAL_SYSTEM_BUS ||
                             configNew.CONTROLLER_HOST != config.CONTROLLER_HOST ||
                             configNew.CONTROLLER_PORT != config.CONTROLLER_PORT );

    const bool reloadParams = (configNew.MAX_ERRORS != config.MAX_ERRORS ||
                               configNew.RELOAD_ON_ERROR != config.RELOAD_ON_ERROR);

    const bool reloadRules = (configNew.RULES_LOCATION != config.RULES_LOCATION);
    const bool reloadData  = (configNew.DATA_LOCATION != config.DATA_LOCATION);

    if (reloadDBUS || reloadParams || reloadRules || reloadData)
    {
        config.Load(); // will reload Data & Params & everything

        if (reloadRules)
            m_ruleManager.Load(config.RULES_LOCATION);

        if (reloadDBUS)
            emit RestartDBUS();
    }
}

bool QueueManager::ControllerLoad()
{
    if (!m_X10Controller)
        m_X10Controller = new X10Controller();

    m_controllerLoaded = m_X10Controller->Load();
    Logger::Log(INFO, "Controller loaded : %s", m_controllerLoaded ? "YES" : "NO");

    if (Status() != QM_STOPPING)
    {
//         m_isRunning = true; // loaded;
        SetStatus(m_controllerLoaded ? QM_STARTED : QM_NOCONTROLLER);
    }

    return m_controllerLoaded;
}

void QueueManager::ControllerUnload()
{
    if (m_X10Controller)
    {
        m_X10Controller->Unload();
        delete m_X10Controller;
        m_X10Controller = 0;
    }
    m_controllerLoaded = false;
}

////////////////////////////////////////////////////////////////////////////////

QueueManagerReader::QueueManagerReader(X10Controller* _X10Controller, QObject* parent) :
    QThread(parent),
    m_X10Controller(_X10Controller),
    m_isRunning(false)
{
}

QueueManagerReader::~QueueManagerReader()
{
    Stop();
}

void QueueManagerReader::Start()
{
//     int res = 0;
    m_isRunning = true;
    while (m_isRunning)
    {
//         res = m_X10Controller->x10_recv(&x10CmdRecv);
    }
}

void QueueManagerReader::Stop()
{
    m_isRunning = false;
    wait();
}

void QueueManagerReader::run()
{
    QThread::run();
}

////////////////////////////////////////////////////////////////////////////////

void QueueManager::run()
{
    SetStatus(QM_STARTING);
    m_isRunning = true;

    m_ruleManager.Load(config.RULES_LOCATION);

    SetStatus(QM_LOADING);
    m_controllerLoaded = ControllerLoad();

    int res = 0;

    // flush queue commented because of cmd line (will wake up queue)
//  m_queueCommands.clear();

    struct x10_ha_command x10CmdRecv;

    Logger::Log(DEBUG, "** QueueManager Entering loop", qPrintable(StatusStr()));
    while (m_isRunning)
    {
        QMutexLocker locker(&m_mutex);
        unsigned int nbErrors = 0;

        // while nothing to send
        while (m_isRunning && !m_queueCommands.size())
        {
            /// Read input.
            if (m_controllerLoaded)
            {
                res = m_X10Controller->x10_recv(&x10CmdRecv);

                if (res > 0)
                {
                    nbErrors = 0;

                    Logger::Log(INFO, "X10 command received on house %c unit %d, cmd:%s res:%d", house_code_to_char(x10CmdRecv.house), unit_code_to_int(x10CmdRecv.unit), cmd_code_to_str(x10CmdRecv.cmd), res);

                    //              emit CommandReceived(*cmd);
                    QString address = QString("%1%2")
                                      .arg(house_code_to_char(x10CmdRecv.house))
                                      .arg(unit_code_to_int(x10CmdRecv.unit))
                                      .toUpper();

                    CheckContainerExist(address);

                    switch (x10CmdRecv.cmd)
                    {
                    case X10CMD_ON:
                        Containers[address]->SetOn(true);
                        break;
                    case X10CMD_OFF:
                        Containers[address]->SetOn(false);
                        break;
                    case X10CMD_BRIGHT:
                        Containers[address]->PushDirection(true);
                        break;
                    case X10CMD_DIM:
                        Containers[address]->PushDirection(false);
                        break;
                    case X10CMD_UP:
                    case X10CMD_RIGHT:
                    case X10CMD_DOWN:
                    case X10CMD_LEFT:
                        break;
                    case X10CMD_INVALID:
                        break;
                    }
                    emit CommandReceived();
                }
                else if (res && res != -110 /*&& res != -1*/)   // first res is -1
                {
                    //              Logger::Log(ERROR, "res = -1 !!", res);
                    nbErrors++;

                    Logger::Log(DEBUG, "ERROR: (%d)", res);
                    if (nbErrors > config.MAX_ERRORS)
                        break;
                }
            }

            queueNotEmpty.wait(&m_mutex, 1000);
        }

        if (nbErrors > config.MAX_ERRORS)   // reload controller
        {
            ControllerUnload();
            if (!config.RELOAD_ON_ERROR)
            {
                Logger::Log(ERROR, "X10 Interface died, quitting QueueManager thread");
                break;
            }
            nbErrors = 0;
            ControllerLoad();
            continue;
        }

        COMMAND* cmd = 0;
        if (m_isRunning && m_queueCommands.size()) // not supposed to be empty at this point
        {
            cmd = m_queueCommands.back();

            // when receiving it, it is this engine role to figure out how
            if (cmd->command == CMD_SET)
            {
                COMMAND* intermCmd;

                int nbSteps = GetNextCommand(*cmd, &intermCmd);

                // unique step or nothing to do
                if (nbSteps <= 1)
                    m_queueCommands.remove(cmd);

                if (nbSteps)
                {
                    if (!cmd->notified)
                    {
                        emit CommandCompleted(*cmd);
                        cmd->notified = true;
                    }
                    intermCmd->notified = true;	// not to be resent by subcmd
                }

//              Logger::Log(DEBUG, "** SET to %d produced %d steps", cmd->value, nbSteps);
                cmd = intermCmd;    // null if nothing to do
            }
            else
            {
//              Logger::Log(DEBUG, "** SEND command");
                m_queueCommands.remove(cmd);    // 1 step per command
            }
        }

        locker.unlock();

        if (!cmd)
            continue; // end marker

        x10_ha_command* x10cmd = new_x10_ha_command(parse_cmd_code(cmd->command), cmd->channel, cmd->unit);

        if (x10cmd) // else params out of bound
        {
            QString address = cmd->address();

            CheckContainerExist(address);	// for read

            int currentValue = Containers[address]->Value();

            switch (cmd->command)
            {
            case CMD_SET:
                Containers[address]->SetValue(cmd->value);
                break;
            case CMD_ON:
                Containers[address]->SetValue(currentValue < 0 ? MAX_VALUE : currentValue);
                break;
            case CMD_OFF:
                Containers[address]->SetValue(-1);
                break;
            case CMD_DIM:
                if (currentValue == -1) currentValue = MAX_VALUE;
                Containers[address]->SetValue(currentValue ? currentValue - 1 : 0);
                break;
            case CMD_BRIGHT:
                if (currentValue == -1)
                    currentValue = MAX_VALUE;
                Containers[address]->SetValue(currentValue >= MAX_VALUE ? MAX_VALUE : currentValue + 1);
                break;
            default:
                break;
            }

            if (m_controllerLoaded)
            {
                // emit CommandCompleted if not SET
                if (!cmd->notified)
                    emit CommandCompleted(*cmd);
                m_X10Controller->x10_send(x10cmd);
            }
            else
            {
                Logger::Log(ERROR, "NO Controller to send Command %s", qPrintable(cmd->commandStr()));
            }

            del_x10_ha_command(x10cmd);
        }

        delete cmd;
    }

    ControllerUnload();

    SetStatus(QM_STOPPED);
}

void QueueManager::CheckContainerExist(const QString& address)
{
    if (Containers.find(address) == Containers.end())
        Containers[address] = new Container(address, this);
}

int QueueManager::GetNextCommand(const COMMAND& setCommand, COMMAND** outputCmd)
{
    *outputCmd = 0;
    QString address = setCommand.address();

    if (address.isEmpty())
        return 0;

    CheckContainerExist(address);

    int curValue = Containers[address]->Value();

    if (setCommand.command != CMD_SET)/* || (curValue == setCommand.value))*/
        return 0;

    // force repeat off
    if (setCommand.value == -1)
    {
        *outputCmd = new COMMAND(setCommand.channel, setCommand.unit, CMD_OFF, -1);
        return 1;
    }

    if (curValue == setCommand.value)
    {
        Logger::Log(ERROR, "GetNextCommand outputs 0 sub cmds");
        return 0;
    }

    if (curValue == -1)
    {
        if (setCommand.value == MAX_VALUE)
        {
            *outputCmd = new COMMAND(setCommand.channel, setCommand.unit, CMD_ON, MAX_VALUE);
            return 1;
        }
        else
        {
            *outputCmd = new COMMAND(setCommand.channel, setCommand.unit, CMD_DIM, MAX_VALUE - 1);
            return 1 + MAX_VALUE - setCommand.value;
        }
    }
    else    // already on
    {
        if (curValue > setCommand.value)
        {
            *outputCmd = new COMMAND(setCommand.channel, setCommand.unit, CMD_DIM, curValue - 1);
            return (curValue - setCommand.value);
        }
        else
        {
            // shortcut ? off - on - down(s) may be faster
            if ((setCommand.value - curValue) > (2 + MAX_VALUE - setCommand.value))
            {
                *outputCmd = new COMMAND(setCommand.channel, setCommand.unit, CMD_OFF, -1);
                return (2 + MAX_VALUE - setCommand.value);
            }
            *outputCmd = new COMMAND(setCommand.channel, setCommand.unit, CMD_BRIGHT, curValue + 1);
            return (setCommand.value - curValue);
        }
    }
//  return 0;   // nothing to do
}

void QueueManager::Start()
{
//	QMutexLocker locker(&m_mutexService);
    start();
}

void QueueManager::Stop()
{
//	QMutexLocker locker(&m_mutexService);
    QueueManagerStatus status = Status();
    if (status == QM_STOPPED || status == QM_STOPPING)
        return;

    Logger::Log(DEBUG, "** QueueManager Stopping (was %s)", qPrintable(StatusStr()));
    SetStatus(QM_STOPPING);

//  QMutexLocker locker(&m_mutex);
    m_isRunning = false;
    m_queueCommands.clear();
//  queueNotEmpty.wakeOne();

    while (Status() != QM_STOPPED)
    {
        m_isRunning = false;
        Logger::Log(DEBUG, "** QueueManager waiting: %s", qPrintable(StatusStr()));
        msleep(300);
    }

    quit();
}

///////////////////////////////////////////////////////////////
// PROPERTIES

QString QueueManager::NodesList()
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

QString QueueManager::RulesList()
{
    return m_ruleManager.GetRules();
}


///////////////////////////////////////////////////////////////
// OPERATIONS

void QueueManager::SetAllTo(bool on)
{
    Channel channel;
    Unit unit;

    for (map<QString, Container*>::const_iterator itr = Containers.begin();
            itr != Containers.end(); ++itr)
    {
        if (X10::sourceToAddress(itr->first, channel, unit))
            QueueCommand(on ? CMD_ON : CMD_OFF, channel, unit, on ? -2 : -1);
    }
}

QString QueueManager::StatusToStr(const QueueManagerStatus& status)
{
    switch (status)
    {
    case QM_STOPPING:
        return "Stopping";
    case QM_STOPPED:
        return "Stopped";
    case QM_STARTING:
        return "Starting";
    case QM_STARTED:
        return "Started";
    case QM_NOCONTROLLER:
        return "No Controller";
    case QM_LOADING:
        return "Loading Driver";
    }
    return "Unknown";
}

QueueManagerStatus QueueManager::Status()
{
    QMutexLocker locker(&m_mutexStatus);
    return m_status;
}

QString QueueManager::StatusStr()
{
    QMutexLocker locker(&m_mutexStatus);
    return StatusToStr(m_status);
}

void QueueManager::SetStatus(QueueManagerStatus status)
{
    QMutexLocker locker(&m_mutexStatus);
    if (m_status == status)
    {
        Logger::Log(DEBUG, "QueueManager SetStatus: no change");
        return;
    }

    m_status = status;
    locker.unlock();

    Logger::Log(INFO, "Queue Manager : %s", qPrintable(StatusToStr(status)));

    // notify engine & its clients of status change :
    emit StatusUpdate(StatusToStr(status));
}

void QueueManager::CheckService()
{
// TODO
//  if (Status() == QM_STOPPED)
//      start();
}

void QueueManager::QueueCommand(COMMAND_TYPE command, Channel channel, Unit unit, int value)
{
    COMMAND* cmd = new COMMAND(channel, unit, command, value);
    QueueCommand(*cmd);
}

void QueueManager::QueueCommand(const COMMAND& cmd)
{
    CheckService();

    QString listNodes = NodesList();
    QStringList nodes = listNodes.split(" ", QString::SkipEmptyParts);

    if (cmd.channel == '*' || cmd.unit == 0)
    {
        Logger::Log(INFO, "Incoming Generic Command %s", qPrintable(cmd.commandStr()));
        char nodeChannel = 'A';
        int nodeUnit = 0;
        bool bMatch = false;
        for (int i=0; i<nodes.count(); i++)
        {
            X10::sourceToAddress(nodes[i], nodeChannel, nodeUnit);
            if ( ((cmd.channel == '*' ) || (nodeChannel == cmd.channel)) &&
                    ((cmd.unit == 0 ) || (nodeUnit == cmd.unit)) )
            {
                // match desired pattern E0 * *2 among known containers
                QueueCommand(COMMAND(nodeChannel, nodeUnit, cmd.command, cmd.value));
                bMatch = true;
            }
        }
        if (!bMatch) {
            Logger::Log(INFO, "No source matching %s in nodes list (%s)",
                        qPrintable(cmd.address()), qPrintable(listNodes) );
        }
        return;
    }

    Logger::Log(INFO, "Incoming Command %s", qPrintable(cmd.commandStr()));

    const QMutexLocker locker(&m_mutex);

    //check existing commmand
    if (config.SMART_QUEUE)
    {
        for (std::list<COMMAND*>::iterator itr = m_queueCommands.begin(); itr != m_queueCommands.end();)
        {
            COMMAND* cmdit = *itr;

            // same channel / unit ?
            if ((cmd.channel == cmdit->channel) && (cmd.unit == cmdit->unit))
            {
                if (cmd.command == CMD_SET)
                {
                    // SET erase all other command
//                     Logger::Log(DEBUG, "Incoming SET %s CANCELLED queued command %s", qPrintable(cmd.commandStr()), qPrintable(cmdit->commandStr()));
                    itr = m_queueCommands.erase(itr);
                    continue;
                }
                else if (cmd.command == CMD_OFF)
                {
                    // OFF erase all other command
//                     Logger::Log(DEBUG, "Incoming Stop %s CANCELLED queued command %s", qPrintable(cmd.commandStr()), qPrintable(cmdit->commandStr()));
                    itr = m_queueCommands.erase(itr);
                    continue;
                }
                else if (cmd.command == CMD_BRIGHT || cmd.command == CMD_DIM)
                {
                    if (cmdit->command != cmd.command) // reverse direction, remove both commands
                    {
//                         Logger::Log(DEBUG, "Incoming command %s CANCELLED queued command %s", qPrintable(cmd.commandStr()), qPrintable(cmdit->commandStr()));
                        itr = m_queueCommands.erase(itr);
                        return;
                    }
                }
            }
            ++itr;
        } // end for
    } // end smart queue

    // enqueue
    m_queueCommands.push_front(new COMMAND(cmd));
    queueNotEmpty.wakeOne();
}

bool QueueManager::SetProperty(const QString& property, const QVariant& value)
{
    if (property == PROPERTY_SMARTQUEUE)
    {
        if (!value.canConvert(QVariant::Bool))
            return false;

        config.SMART_QUEUE = value.toBool();
        config.Save();
    }
    else if (property == PROPERTY_RULES)
    {
        if (!value.canConvert(QVariant::String))
            return false;

        m_ruleManager.Deserialize(value.toString());
        m_ruleManager.Save(config.RULES_LOCATION);
    }
    else
    {
        Logger::Log(INFO, "SetProperty: property %s not settable", qPrintable(property));
        return false;
    }

    return true;
}

int QueueManager::GetValue(const QString& source)
{
    if (X10::isValidAddress(source))
    {
        CheckContainerExist(source);
        return Containers[source]->Value();
    }
    return -1;
}

void QueueManager::UpdateSource(const QString& source, const int value)
{
    emit SourceUpdated(source, value);
}

void QueueManager::on_RuleExecute(Rule rule)
{
    Logger::Log(INFO, "Rule execution command %s", qPrintable(rule.getDisplayText()) );
    vector<COMMAND> listCmds = rule.getCommands();

    if (!listCmds.size()) {
        Logger::Log(ERROR, "on_RuleExecute, no commands resulted from rule %s",
                    qPrintable(rule.getDisplayText()) );
    } else {
        for (uint i=0; i<listCmds.size(); i++)
            QueueCommand(listCmds[i]);
    }
}
