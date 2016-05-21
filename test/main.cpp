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

#include <iostream>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>

#include <QtDBus/QtDBus>

#include <common/interfacehandler.h>
#include <common/command.h>
#include <common/operations.h>

QString getProperty(const QString& property)
{

    QDBusPendingReply<QString> bret = IDBus::X10()->GetProperty(property);
    bret.waitForFinished();

    if (bret.isError() || !bret.isValid())
    {
        // TODO move that inside class...
        IDBus::Get()->Reconnect();
        return "NOREPLY";
    }

    printf("%s\n", qPrintable( bret.value()));
    return bret.value();
}

bool setProperty(const QString& property, const QString& value)
{
    QDBusPendingReply<QString> bret = IDBus::Get()->X10()->SetProperty(property, value);
    bret.waitForFinished();

    if (bret.isValid())
    {
        return true;
    }
    IDBus::Get()->Reconnect();
    return false;
}


///////////////////////////////////////////////////////////////////////////////

int main(int /*argc*/, char** /*argv*/)
{
    getProperty(PROPERTY_STATUS);

    while (1)
    {
        sleep(2);
        getProperty(PROPERTY_STATUS);
    }

    return 0;
}
