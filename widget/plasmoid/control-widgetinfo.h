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

#ifndef SETTINGS_H
#define SETTINGS_H

#include "control-widget.h"

#include <QString>
#include <KConfigGroup>

class X10Widget;

class ControlWidgetInfo
{
public:
    ControlWidgetInfo();
    virtual ~ControlWidgetInfo();

    // Operations
    ControlWidget*  GetWidget() {
        return m_widget;
    }
    ControlWidget*  CreateWidget(X10Widget* parent);
    void            DeleteWidget();
    void            UpdateWidget();

    void            writeConfig(int key, KConfigGroup cg);
    void            readConfig(int key, KConfigGroup cg);

    // Getters / Setters
    void            setChannel(QChar channel);
    void            setUnit(int unit);

    // Attributes
    QString         name;
    QChar           channel;
    int             unit;
    bool            horizontal;
    int             style;

    int             preferredLighting;
    int             switchOffPeriodMn;

    int             row;
    int             col;

private:
    ControlWidget*  m_widget;
};

typedef QList<ControlWidgetInfo*> ControlWidgetInfoList;

#endif // SETTINGS_H
