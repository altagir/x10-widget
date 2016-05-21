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

#include "container.h"
#include "service.h"
#include "engine.h"

Container::Container(X10Engine* engine, const QString& source, QObject* parent) :
    DataContainer(parent)
    , value(-1)
    , m_engine(engine)
{
//  Logger::Log(DEBUG, "Container: ctor %s", qPrintable(source));
    setObjectName(source);
//  setData("Value", -1);
}

Plasma::Service* Container::service(QObject* /*parent*/)
{
    Plasma::Service* service = new X10Service(m_engine, objectName());
    connect(this, SIGNAL(updateRequested(DataContainer*)), service, SLOT(updateEnabledOperations()));
    return service;
}

void Container::updateInfo()
{
// propagate changes, all of these didn't work
//  setData("Value",    value);
//  checkForUpdate();
//  forceImmediateUpdate();
    m_engine->updateSourceEvent(objectName());
}

void Container::SetValue(int _value)
{
    value = _value;
    updateInfo();
}
