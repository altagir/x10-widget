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

#include "service.h"
#include "job.h"
#include "container.h"
#include "engine.h"

#include <common/logger.h>

X10Service::X10Service(X10Engine* engine, const QString& source)
    : Plasma::Service(engine)
    , m_engine(engine)
{
    if (source.isEmpty())
        setName("service"); // -> service.operations
    else
        setName("unit");    // -> unit.operations

    setDestination(source);

    updateEnabledOperations();
}

Plasma::ServiceJob* X10Service::createJob(const QString& operation,
        QMap <QString, QVariant>& parameters)
{
    if (!m_engine)
        return 0;

    return new X10Job(operation, parameters, this);
}

void X10Service::updateEnabledOperations()
{
    // we have two services, one for empty source "" (i.e. general cmd)
    // the others being for a source (set/send)
    const bool forService = destination().isEmpty();

    setOperationEnabled("Send",         !forService);
    setOperationEnabled("Set",          !forService);

    setOperationEnabled("Start",        forService);
    setOperationEnabled("Stop",         forService);
    setOperationEnabled("Reload",       forService);

    setOperationEnabled("AllOn",        forService);
    setOperationEnabled("AllOff",       forService);

    setOperationEnabled("SendCommand",  forService);
    setOperationEnabled("SetCommand",   forService);

    setOperationEnabled("SetProperty",  forService);

}

void X10Service::containerDestroyed()
{
    Logger::Log(INFO, "Service: Container Destroyed");
    m_engine = 0;
}

#include "service.moc"
