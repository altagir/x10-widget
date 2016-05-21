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

#ifndef X10_JOB_H
#define X10_JOB_H

#include <Plasma/ServiceJob>
#include <QDBusPendingReply>

class X10Service;

class X10Job : public Plasma::ServiceJob
{
    Q_OBJECT

public:
    X10Job(
        const QString& operation,
        QMap<QString, QVariant>& parameters,
        X10Service* service = 0);

    enum
    {
        /**
         * The media player reports that the operation is not possible
         */
        Denied = UserDefinedError,
        /**
         * Calling the media player resulted in an error
         */
        Failed,
        /**
         * An argument is missing or of wrong type
         * errorText is argument name
         */
        MissingArgument,
        /**
         * An argument is invalid
         * errorText is argument name
         */
        InvalidArgument,
        /**
         * The operation name is unknown
         */
        UnknownOperation
    };

    void start();
//      virtual QString errorString() const;

private:
    bool CheckResult(QDBusPendingReply<void> bret);
    //  void listenToCall(const QDBusPendingCall& call);

    X10Service* m_service;
};

#endif // X10_JOB_H

