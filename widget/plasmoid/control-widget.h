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

#ifndef CONTROLWIDGET_H
#define CONTROLWIDGET_H

#include <QGraphicsWidget>
#include <QGraphicsLinearLayout>
#include <QTimer>

#include <Plasma/DataEngine>

#include <common/command.h>

namespace Plasma
{
class IconWidget;
class Slider;
}

class X10Widget;

class ControlWidget : public QGraphicsWidget
{
    Q_OBJECT

public:

    enum EControlWidgetStyle
    {
        ONOFF   = 1,
        DIM     = 2,
        BRIGHT  = 4,
        DIMBRIGHT = BRIGHT + DIM,
        SLIDER  = 8,
        ALL     = ONOFF | DIMBRIGHT | SLIDER
    };

    ControlWidget(const char* name, Channel channel, Unit unit, EControlWidgetStyle style, X10Widget* parent);
    ControlWidget(const char* name, Channel channel, Unit unit, EControlWidgetStyle style, Qt::Orientation orientation, int maximum, X10Widget* parent);

    virtual ~ControlWidget();

    void setName(QString name);
    void setUnit(Channel channel, Unit  unit);
    void setOrientation(Qt::Orientation orientation);
    void setMaximum(int maximum);

    void updateTooltip();

    void setStyle(EControlWidgetStyle style);
    bool hasStyle(EControlWidgetStyle style) {
        return (m_style & style);
    }

    void setSwitchOffPeriodMn(int mn);
    void setPreferredLighting(int percent);

    bool isAppliance() {
        return !(hasStyle(SLIDER) || hasStyle(DIM) || hasStyle(DIM));
    }

    ////////////

    void On()   {
        if (!m_unit.IsOn()) on_pushOnOff(false);
    }
    void Off()  {
        if (m_unit.IsOn())  on_pushOnOff(false);
    }
    void setUnitTo(int value);

    ////////////

    void ConnectSource();
    void DisconnectSource();

public slots:

    void dataUpdated(const QString& sourceName, const Plasma::DataEngine::Data& data);

    void on_pushOnOff(bool uiInitiated = true);
    void on_pushDim()    {
        on_pushDirection(false);
    }
    void on_pushBright() {
        on_pushDirection(true);
    }

    void sliderMoved(int value);

    void on_timerSwitchOff();
    void on_timerEndSuspendUpdate();

protected:
    void redraw();

private:
    // refresh
    void refreshOnOff();
    void refreshSlider();

    // update control with new value
    void updateValue(int newValue); // -1 off, 0-8

    // to update various states of timer with new value
    void checkTimer(int value);
    void SwitchOffTimer();

    void on_pushUnit();
    void on_pushDirection(bool up);

    // this send & queue the command to the Data Engine
    void send_command(COMMAND_TYPE command);

private:
    X10Widget*              m_parent;
    X10Unit                 m_unit;

    QString                 m_name;
    EControlWidgetStyle     m_style;
    Qt::Orientation         m_orientation;
    int                     m_maximum;

    int                     m_configSwitchOffPeriodMn;
    int                     m_configPreferredLighting;

    QGraphicsLinearLayout*  m_layout;
    Plasma::Slider*         m_slider;
    Plasma::IconWidget*     m_pushButtonOnOff;
    Plasma::IconWidget*     m_pushButtonDim;
    Plasma::IconWidget*     m_pushButtonBright;

    Plasma::Service*        m_serviceUnit;

    QTimer                  m_timerSwitchOff;
    QTimer                  m_timerSuspendUpdate;

    int                     m_lastValueUpdateRecv;  // used when update freeze timeout
};


class X10Appliance : public ControlWidget
{
    Q_OBJECT
public:
    X10Appliance(const char* name, Channel channel, Unit unit, EControlWidgetStyle style, X10Widget* parent)
        : ControlWidget(name, channel, unit, (EControlWidgetStyle)(style & (!SLIDER) & (!DIMBRIGHT)), parent)
    {
    }
};

#endif // CONTROLWIDGET_H
