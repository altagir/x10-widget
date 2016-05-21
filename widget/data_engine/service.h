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

#ifndef X10_SERVICE_H
#define X10_SERVICE_H

#include <Plasma/Service>

class X10Engine;

class X10Service : public Plasma::Service
{
    Q_OBJECT

public:
    X10Service(X10Engine* engine, const QString& source);
    X10Engine* engine()    {
        return m_engine;
    }

signals:
    void enabledOperationsChanged();

private slots:
    void updateEnabledOperations();
    void containerDestroyed();

protected:
    Plasma::ServiceJob* createJob(const QString& operation,
                                  QMap<QString, QVariant>& parameters);

private:
    X10Engine* m_engine;
};

#endif // X10_SERVICE_H
