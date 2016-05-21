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

#include "control-widgetinfo.h"

/// ////////////////////////////////////////////////////////////////////////////

ControlWidgetInfo::ControlWidgetInfo() :
    name("Untitled"),
    channel('A'),
    unit(1),
    horizontal(true),
    style(ControlWidget::ONOFF | ControlWidget::SLIDER),
    preferredLighting(80),
    switchOffPeriodMn(20),
    row(1),
    col(1),
    m_widget(0)
{
}

ControlWidgetInfo::~ControlWidgetInfo()
{
    DeleteWidget();
}

ControlWidget* ControlWidgetInfo::CreateWidget(X10Widget* parent)
{
    if (!m_widget)
    {
        m_widget = new ControlWidget(
            qPrintable(name),
            (Channel)qPrintable(channel)[0],
            (Unit)unit,
            (ControlWidget::EControlWidgetStyle)style,
            horizontal ? Qt::Horizontal : Qt::Vertical,
            MAX_VALUE, parent);
    }

    return m_widget;
}

void ControlWidgetInfo::DeleteWidget()
{
    if (m_widget)
    {
        delete m_widget;
        m_widget = 0;
    }
}

void ControlWidgetInfo::UpdateWidget()
{
    if (m_widget)
    {
        m_widget->setName(name);
        m_widget->setUnit((Channel)qPrintable(channel)[0], (Unit)unit);
        m_widget->setOrientation(horizontal ? Qt::Horizontal : Qt::Vertical);
        m_widget->setPreferredLighting(preferredLighting);
        m_widget->setSwitchOffPeriodMn(switchOffPeriodMn);
        m_widget->setStyle((ControlWidget::EControlWidgetStyle)style);
    }
}

void ControlWidgetInfo::writeConfig(int key, KConfigGroup cg)
{
    QString params = name + '/' +
                     channel  + '/' +
                     QString::number(unit) + '/' +
                     QString::number(style) + '/' +
                     QString::number(horizontal) + '/' +
                     QString::number(preferredLighting)  + '/' +
                     QString::number(switchOffPeriodMn) + '/' +
                     QString::number(row) + '/' +
                     QString::number(col);

    cg.writeEntry(QString::number(key), params);
}

void ControlWidgetInfo::readConfig(int key, KConfigGroup cg)
{
    QString params = cg.readEntry(QString::number(key));
    QStringList paramsList = params.split('/');

    if (paramsList.count() < 9)
    {
        name = "";  // invalid
        return;
    }

    name                = paramsList[0];
    channel             = paramsList[1][0]; // char
    unit                = paramsList[2].toInt();
    style               = paramsList[3].toInt();
    horizontal          = paramsList[4].toInt();
    preferredLighting   = paramsList[5].toInt();
    switchOffPeriodMn   = paramsList[6].toInt();
    row                 = paramsList[7].toInt();
    col                 = paramsList[8].toInt();

//     if (paramsList.count() ==  10) // since added since release
//         rules               = paramsList[9];
}
