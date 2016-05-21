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

#include <stdio.h>
#include <signal.h>
#include <unistd.h>
#include <sys/stat.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#include <QCoreApplication>
#include <QDBusConnection>

#include <common/operations.h>
#include <common/logger.h>

#include "x10dbusservice.hpp"
#include "X10DbusAdaptor.h"
#include "queue-manager.h"

X10DbusService x10service;

QDBusConnection* m_dbusConnection = 0;

QString m_dbusName = ""; // "x10bus" + m_dbusNameIndex
int m_dbusNameIndex = 0;

QString m_dbusLocation = "";
QMutex m_mutexConnection;

////////////////////////////////////////////////////////////////////////////////

bool CheckPortTCP(short int dwPort , const char *ipAddressStr)
{
    struct sockaddr_in client;
    int sock;

    client.sin_family = AF_INET;
    client.sin_port = htons(dwPort);
    client.sin_addr.s_addr = inet_addr(ipAddressStr);

    sock = (int) socket(AF_INET, SOCK_STREAM, 0);

    int result = connect(sock, (struct sockaddr *) &client,sizeof(client));

    if (result < 0) return false;
    else return true;
}

bool /*X10DbusService::*/RegisterDBusService()
{
    QMutexLocker locker(&m_mutexConnection);

    Config config;

    m_dbusLocation = (config.USE_LOCAL_SYSTEM_BUS)?
                     PROPVAL_CONTROLLER_SYSTEMDBUS:
                     QString("%1:%2").arg(config.CONTROLLER_HOST).arg(config.CONTROLLER_PORT);

    if (!config.USE_LOCAL_SYSTEM_BUS && !CheckPortTCP(config.CONTROLLER_PORT, qPrintable(config.CONTROLLER_HOST)))
    {
        Logger::Log(ERROR, "Connection to %s couldn't be established", qPrintable(m_dbusLocation));
        return false;
    }
    Logger::Log(INFO, "Connecting to %s", qPrintable(m_dbusLocation));

    m_dbusName = DBUS_name + QString::number(m_dbusNameIndex++);

    if (config.USE_LOCAL_SYSTEM_BUS)
        m_dbusConnection = new QDBusConnection ( QDBusConnection::connectToBus (QDBusConnection::SystemBus, m_dbusName));
    else
        m_dbusConnection = new QDBusConnection ( QDBusConnection::connectToBus (config.DBUS_ADDRESS, m_dbusName));

    if (!m_dbusConnection->isConnected())
    {
        Logger::Log(ERROR, "QDBusConnection failed to %s (%s)",
                    qPrintable(m_dbusLocation), qPrintable( m_dbusConnection->lastError().message()) );
    }
    else
    {
        Logger::Log(INFO, "QDBusConnection successful to (%s)", qPrintable(m_dbusLocation) );
        if (!m_dbusConnection->registerService("ca.cyberdine.x10"))
        {
            Logger::Log(ERROR, "Couldn't register service ca.cyberdine.x10 on that interface. already running? (%s)",
                        qPrintable(m_dbusConnection->lastError().message()) );
        }
        else if (!m_dbusConnection->registerObject("/", &x10service))
        {
            Logger::Log(ERROR, "Couldn't register object / for service ca.cyberdine.x10 (%s)",
                        qPrintable(m_dbusConnection->lastError().message()) );
        }
        else
        {
            Logger::Log(INFO, "Registration of service ca.cyberdine.x10 successful");
            return true;
        }
    }
    return false;
}

void /*X10DbusService::*/DeregisterDBusService()
{
    QMutexLocker locker(&m_mutexConnection);
    Logger::Log(DEBUG, "Disconnect from %s", qPrintable(m_dbusName) );

    if (m_dbusConnection)
    {
        m_dbusConnection->unregisterObject("/");
        m_dbusConnection->unregisterService("ca.cyberdine.x10");
        m_dbusConnection->disconnectFromBus(m_dbusName);
        delete m_dbusConnection;
        m_dbusConnection = 0;
    }
    QDBusConnection::disconnectFromBus(m_dbusName);
}

bool /*X10DbusService::*/ReregisterDBusService()
{
    DeregisterDBusService();
    return RegisterDBusService();
}


////////////////////////////////////////////////////////////////////////////////

X10DbusService::X10DbusService(QObject* parent) :
    QObject(parent)
{
    queueManager = new QueueManager(this);

    QObject::connect(queueManager, SIGNAL(CommandCompleted(COMMAND)),
                     this, SLOT(OnQM_CommandCompleted(COMMAND)));
    QObject::connect(queueManager, SIGNAL(StatusUpdate(QString)),
                     this, SLOT(OnQM_StatusUpdate(QString)));
    QObject::connect(queueManager, SIGNAL(SourceUpdated(QString, int)),
                     this, SIGNAL(SourceUpdated(QString,int)));
    QObject::connect(queueManager, SIGNAL(RestartDBUS()),
                     this, SLOT(OnQM_RestartDBUS()));

    QObject::connect(queueManager, SIGNAL(finished()),
                     this, SLOT(OnQM_Exit()));
}

X10DbusService::~X10DbusService()
{
    Stop();
    delete queueManager;
}

// EVENTS /////////////////////////////////

void X10DbusService::OnQM_CommandCompleted(const COMMAND& cmd)
{
    emit CommandCompleted(QString(cmd.channel),
                          cmd.unit,
                          QString(cmd.command),
                          cmd.value);
}

void X10DbusService::OnQM_StatusUpdate(const QString& status)
{
    emit PropertyUpdated(PROPERTY_STATUS, status);
}

void X10DbusService::OnQM_RestartDBUS()
{
    Logger::Log(INFO, "Restarting DBUS");
    ReregisterDBusService();
}

// ACTIONS /////////////////////////////////

void X10DbusService::Start()
{
    if (!queueManager->isRunning())
        queueManager->Start();
}

void X10DbusService::Stop()
{
    if (queueManager->isRunning())
        queueManager->Stop();
}

void X10DbusService::Reload()
{
    queueManager->Reload();
}

void X10DbusService::Restart()
{
    queueManager->Stop();
    queueManager->Start();
}

void X10DbusService::Quit()
{
    Stop();
    QCoreApplication::exit(0);
}

void X10DbusService::OnQM_Exit()
{
    // TODO: true but no!... disconnect-> reconnect this signal instead instead...
    // let's sleep so we don't prevent restart (stop will triger signal
    // here and exit without restart)
    sleep(1);
    if (queueManager->Status() == QM_STOPPED)
        QCoreApplication::exit(0);
}

void X10DbusService::Send(const QString& channel, const unsigned int unit, const QString& command)
{
    if (X10::isValidChannel(channel) &&
            X10::isValidUnit(unit) &&
            X10::isValidCommand(command))
    {
        queueManager->QueueCommand((COMMAND_TYPE) command.at(0).toLatin1(),
                                   channel.toUpper().at(0).toLatin1(),
                                   unit,
                                   -1);
//      Q_EMIT CommandSent(channel, unit, command, -2);
    }
    else
    {
        Logger::Log(ERROR, "ERROR SENDING");
    }
}

void X10DbusService::Set(const QString& channel, const unsigned int unit, const int value)
{
    if (X10::isValidChannel(channel) &&
            X10::isValidUnit(unit) &&
            X10::isValidValue(value))
    {
        queueManager->QueueCommand(CMD_SET,
                                   channel.toUpper().at(0).toLatin1(),
                                   unit,
                                   value);
    }
    else
    {
        Logger::Log(ERROR, "ERROR SETTING");
    }
}

void X10DbusService::AllOn()
{
    queueManager->SetAllTo(true);
}

void X10DbusService::AllOff()
{
    queueManager->SetAllTo(false);
}


int X10DbusService::GetValue(const QString& source)
{
    return queueManager->GetValue(source.toUpper());
}

QString X10DbusService::GetProperty(const QString& name)
{
    if (name == PROPERTY_STATUS)
        return queueManager->StatusStr();
    else if (name == PROPERTY_SMARTQUEUE)
        return queueManager->config.SMART_QUEUE ? "1" : "0";
    else if (name == PROPERTY_NODES)
        return queueManager->NodesList();
    else if (name == PROPERTY_RULES)
        return queueManager->RulesList();
    else
        return "";
}

void X10DbusService::SetProperty(const QString& name, const QString& value)
{
    queueManager->SetProperty(name, value);

    // udpate all widgets TODO to check
    emit PropertyUpdated(name, value);
}


// MAIN ////////////////////////////////////////////////////////////////////////

void sighandler(int signum)
{
    Logger::Log(INFO, "Process %d got signal %d\n", getpid(), signum);
    x10service.Quit();
//  QCoreApplication::exit(0);
}

int main(int argc, char* argv[])
{
    bool demonize = false;
    for (int i=1; i<argc; i++)
    {
        if (!strcasecmp(argv[i], "--help") ||
                !strcasecmp(argv[i], "-h"))
        {
            printf("Use the cmd line 'x10', this is the daemon\n");
            printf("Like all daemons it will spawn to fulfill your x10 commands.\n");
            printf("--daemon | -d           : fork as daemon\n");
            printf("--log    | -l filename  : output log to file filename\n");
            return 0;
        }
        else if (!strcasecmp(argv[i], "--log") ||
                 !strcasecmp(argv[i], "-l") )
        {
            if ((i+1) != argc)
                Logger::SetDestination(argv[i+1]);
            else
                printf("--log | -l filename  : missing filename\n");
        }
        else if (!strcasecmp(argv[i], "--daemon") ||
                 !strcasecmp(argv[i], "-d") )
        {
            demonize = true;
        }
    }

    // Attempt right away to have availability of DBUS service.
    if (RegisterDBusService())
    {
        Logger::Log(DEBUG, "Setting Signal Handlers...");
        signal(SIGTERM, sighandler);  // termination request, sent to the program
        signal(SIGINT,  sighandler);  // external interrupt, usually initiated by the user like Ctrl-C
        signal(SIGABRT, sighandler);  // abnormal termination condition, as is e.g. initiated by abort()
        signal(SIGHUP,  sighandler);  // sent to a process when its controlling terminal is closed.

        if (demonize)
        {
            Logger::Log(DEBUG, "Forking daemon...");
            /* Our process ID and Session ID */
            pid_t pid, sid;

            /* Fork off the parent process */
            pid = fork();
            if (pid < 0)
                exit(EXIT_FAILURE);

            /* If we got a good PID, then
            *		we can exit the parent process. */
            if (pid > 0)
                exit(EXIT_SUCCESS);

            /* Change the file mode mask */
            umask(0);

            /* Open any logs here */

            /* Create a new SID for the child process */
            sid = setsid();
            if (sid < 0) exit(EXIT_FAILURE);

            /* Change the current working directory */
            if ((chdir("/")) < 0) exit(EXIT_FAILURE);

            /* Close out the standard file descriptors */
            close(STDIN_FILENO);
            close(STDOUT_FILENO);
            close(STDERR_FILENO);
            /// port specified

            /* Daemon-specific initialization goes here */
        } // endif daemon

// 		if (!RegisterDBusService())
// 			return 1;

        Logger::Log(INFO, "Starting X10 Daemon...");
        X10Adaptor* x10adaptor = new X10Adaptor(&x10service);
        x10adaptor->Start();

        QCoreApplication a(argc, argv);
        return a.exec();
    } // endif ConnectService

    return 1;
}
