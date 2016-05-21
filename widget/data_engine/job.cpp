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

#include "job.h"
#include "engine.h"
#include "service.h"

X10Job::X10Job(
    const QString& operation,
    QMap<QString, QVariant>& parameters,
    X10Service* service)
    : ServiceJob(service->destination(), operation, parameters, service /*parent*/)
    , m_service(service)
{
//  Logger::Log(DEBUG, "X10Job: operation %s in service %s", qPrintable(operation), qPrintable(service->name()));
}

bool X10Job::CheckResult(QDBusPendingReply<void> bret)
{
    bret.waitForFinished();
    if (bret.isValid())
        return true;

    setError(Failed);
    setResult("DBUS Down");
    IDBus::Get()->Reconnect();
    return false;
}

void X10Job::start()
{
    const QString operation(operationName());

    kDebug() << "Trying to perform the action" << operationName();
    if (!m_service->isOperationEnabled(operation))
    {
        setError(Denied);
        setResult("Denied");
        emitResult();
        return;
    }

    if (operation == "Start")
    {
        if (CheckResult(m_service->engine()->GetDbusInterface()->Start()))
            setResult("Starting then...");
        return;
    }
    else if (operation == "Stop")
    {
        if (CheckResult(m_service->engine()->GetDbusInterface()->Stop()))
            setResult("Stopped");
    }
    else if (operation == "Reload")
    {
        if (m_service->engine()->GetIDbus()->Reconnect())
            setResult("Config Reloaded");
    }
    else if (operation == "AllOn")
    {
        if (CheckResult(m_service->engine()->GetDbusInterface()->AllOn()))
            setResult("And There was the Light");
    }
    else if (operation == "AllOff")
    {
        if (CheckResult(m_service->engine()->GetDbusInterface()->AllOff()))
            setResult("Doom is Coming");
    }
    else if (operation == "SendCommand")
    {
        if (parameters().count() != 3 ||
                !X10::isValidChannel(parameters().value("Channel")) ||
                !X10::isValidUnit(parameters().value("Unit")) ||
                !X10::isValidCommand(parameters().value("Command")))
        {
            setErrorText("on");
            setError(MissingArgument);
        }
        else
        {
            if (CheckResult(m_service->engine()->GetDbusInterface()->Send(
                                parameters().value("Channel").toString(),
                                parameters().value("Unit").toInt(),
                                parameters().value("Command").toString()
                            )))
                setResult("Queued");
        }
    }
    else if (operation == "SetCommand")
    {
        if (parameters().count() != 3)
        {
            setErrorText("on");
            setError(MissingArgument);
        }
        else if (!X10::isValidChannel(parameters().value("Channel")) ||
                 !X10::isValidUnit(parameters().value("Unit")) ||
                 !X10::isValidValue(parameters().value("Value")))
        {
            setErrorText("on");
            setError(InvalidArgument);
        }
        else
        {
            if (CheckResult(m_service->engine()->GetDbusInterface()->Set(
                                parameters().value("Channel").toString(),
                                parameters().value("Unit").toInt(),
                                parameters().value("Value").toInt()
                            )))
                setResult("Queued");
        }
    }
    else if (operation == "Send")
    {
        char channel;
        int unit;

        if (destination().isEmpty() ||
                parameters().count() != 1 ||
                parameters().value("Command").type() != QVariant::String ||
                !X10::isValidCommand(parameters().value("Command").toString())
           )
        {
            setErrorText("MissingArgument");
            setError(MissingArgument);
        }
        else if (!X10::sourceToAddress(destination(), channel, unit))
        {
            setErrorText(QString("Invalid Source: %1").arg(destination()));
            setError(MissingArgument);
        }
        else
        {
            Channel channel = destination()[0].toLatin1();
            Unit unit = (Unit) destination().mid(1).toInt();

            if (CheckResult(m_service->engine()->GetDbusInterface()->Send(
                                QString(channel),
                                unit,
                                parameters().value("Command").toString()
                            )))
                setResult("Sent");
        }
    }
    else if (operation == "Set")
    {
        char channel;
        int unit;

        if (destination().isEmpty() || parameters().count() != 1)
        {
            setErrorText("on");
            setError(MissingArgument);
        }
        else if (!X10::isValidValue(parameters().value("Value")))
        {
            setErrorText(QString("Invalid value: %1=%2 [1-%3]")
                         .arg(destination())
                         .arg(parameters().value("Value").toString())
                         .arg(MAX_VALUE));
            setError(MissingArgument);
        }
        else if (!X10::sourceToAddress(destination(), channel, unit))
        {
            setErrorText(QString("Invalid Source: %1").arg(destination()));
            setError(MissingArgument);
        }
        else
        {
            if (CheckResult(m_service->engine()->GetDbusInterface()->Set(
                                QString(channel),
                                unit,
                                parameters().value("Value").toInt() )))
                setResult("Set");
        }
    }
    else if (operation == "SetProperty")
    {
        if (parameters().count() != 2 ||
                parameters().value("Property").toString().isEmpty() ||
                parameters().value("Value").toString().isEmpty()
           )
        {
            setErrorText("on");
            setError(MissingArgument);
        }
        else
        {
            if (CheckResult(m_service->engine()->GetDbusInterface()->SetProperty(
                                parameters().value("Property").toString(),
                                parameters().value("Value").toString()
                            )))
                setResult("Property set");
        }
    }
    else
    {
        setErrorText("on");
        setError(UnknownOperation);
    }

    emitResult();
}

#include "job.moc"
