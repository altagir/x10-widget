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

#include "control-widget.h"

#include <QAction>
#include <QObject>
#include <QSlider>
#include <Plasma/ServiceJob>

#include <plasma/widgets/iconwidget.h>
#include <plasma/widgets/slider.h>

#include <icons/resources.h>
#include <common/logger.h>

#include "x10-widget.h"

ControlWidget::ControlWidget(const char* name, char channel, int unit, EControlWidgetStyle style, X10Widget* parent) :
    QGraphicsWidget(parent)
{
    ControlWidget(name, channel, unit, style, Qt::Horizontal, 7, parent);
}

ControlWidget::ControlWidget(const char* name, char channel, int unit, EControlWidgetStyle style, Qt::Orientation orientation, int maximum, X10Widget* parent) :
    QGraphicsWidget(parent),
    m_parent(parent),
    m_unit(channel, unit, -1),
    m_name(name),
    m_style(style),
    m_orientation(orientation),
    m_maximum(maximum),
    m_configSwitchOffPeriodMn(1),
    m_configPreferredLighting(maximum),
    m_layout(0),
    m_slider(0),
    m_pushButtonOnOff(0),
    m_pushButtonDim(0),
    m_pushButtonBright(0),
    m_serviceUnit(0),
    m_timerSwitchOff(),
    m_timerSuspendUpdate(),
    m_lastValueUpdateRecv(-1)
{
    redraw();

    m_timerSwitchOff.setSingleShot(true);
    connect(&m_timerSwitchOff, SIGNAL(timeout()), this, SLOT(on_timerSwitchOff()));

    m_timerSuspendUpdate.setSingleShot(true);
    connect(&m_timerSuspendUpdate, SIGNAL(timeout()), this, SLOT(on_timerEndSuspendUpdate()));

    ConnectSource();
}

ControlWidget::~ControlWidget()
{
    SwitchOffTimer();
}

void ControlWidget::ConnectSource()
{
    Plasma::DataEngine* engine = m_parent->getDataEngine();
    if (engine && engine->isValid())
    {
        Logger::Log(DEBUG, "Connecting source %s", m_unit.address());
        engine->connectSource(m_unit.address(), this, 0);

        m_serviceUnit = engine->serviceForSource(m_unit.address());
        if (!m_serviceUnit)
            Logger::Log(ERROR, "Connecting source %s: Null Service", m_unit.address());
    }
    else
    {
        Logger::Log(ERROR, "Connecting source %s: Null Data Engine", m_unit.address());
        m_serviceUnit = 0;
    }
}

void ControlWidget::DisconnectSource()
{
    Plasma::DataEngine* engine = m_parent->getDataEngine();
    if (engine && engine->isValid())
    {
        Logger::Log(DEBUG, "Disconnecting source %s", m_unit.address());
        engine->disconnectSource(m_unit.address(), this);
    }
}

/// //////////

void ControlWidget::setName(QString name)
{
    if (m_name != name)
    {
        m_name = name;
        updateTooltip();
    }
}

void ControlWidget::setUnit(Channel channel, Unit unit)
{
    if (channel != m_unit.channel || unit != m_unit.unit)
    {
        DisconnectSource();
        m_unit.channel  = channel;
        m_unit.unit     = unit;
        ConnectSource();
    }
}

void ControlWidget::setOrientation(Qt::Orientation orientation)
{
    m_orientation = orientation;

    if (m_layout)	m_layout->setOrientation(orientation);
    if (m_slider)	m_slider->setOrientation(orientation);
}

void ControlWidget::setMaximum(int maximum)
{
    if (m_maximum != maximum)
    {
        m_configPreferredLighting = (maximum * m_configPreferredLighting) / m_maximum;
        m_maximum = maximum;

        if (m_slider)	m_slider->setMaximum(m_maximum);
    }
}

void ControlWidget::updateTooltip()
{
    if (hasStyle(SLIDER) && m_slider)
        m_slider->setToolTip(m_name);
    if (hasStyle(ONOFF) && m_pushButtonOnOff)
    {
        m_pushButtonOnOff->setToolTip(m_name + QString(" (timeoff %1mn)").arg(m_configSwitchOffPeriodMn));
    }
}

void ControlWidget::setStyle(ControlWidget::EControlWidgetStyle style)
{
    bool different = (m_style != style);
    m_style = style;

    if (different)
        redraw();
}

void ControlWidget::SwitchOffTimer()
{
    m_timerSwitchOff.stop();
// 	updateTooltip(); // to display timeoff
}

void ControlWidget::setSwitchOffPeriodMn(int mn)
{
    if (!mn)
    {
        SwitchOffTimer();
        refreshOnOff();  // icon on
    }
    else if (mn != m_configSwitchOffPeriodMn && m_timerSwitchOff.isActive())
        m_timerSwitchOff.start(mn * 60000);

    m_configSwitchOffPeriodMn = mn;

    updateTooltip(); // to display timeoff
}

void ControlWidget::setPreferredLighting(int percent)
{
    m_configPreferredLighting = (m_maximum * percent) / 100;
}

/// //////////

void ControlWidget::on_pushUnit()
{
//     if (QApplication::keyboardModifiers() & Qt::ControlModifier)    // Ctrl
//         if (QApplication::keyboardModifiers() & Qt::ShiftModifier)
    setUnitTo(m_unit.IsOn() ? -1 : m_maximum);
}

void ControlWidget::on_pushOnOff(bool uiInitiated)
{
    if (!m_unit.IsOn())
    {
        if (isAppliance())
        {
            on_pushUnit();
        }
        else if (m_parent->m_configRememberLastIntensity && (m_unit.valueBeforeOff * 100 / m_maximum) > m_parent->m_configRememberLastIntensityThreshold)
        {
            setUnitTo(m_unit.valueBeforeOff);
        }
        else
        {
            setUnitTo(m_configPreferredLighting);
        }
    }
    else
    {
        if (uiInitiated)
            m_unit.valueBeforeOff = m_unit.value;

        on_pushUnit();
    }
}

void ControlWidget::on_timerSwitchOff()
{
    if (m_unit.IsOn())
        on_pushUnit();
}

void ControlWidget::on_timerEndSuspendUpdate()
{
    if (m_lastValueUpdateRecv != m_unit.value)
    {
        SwitchOffTimer();
        updateValue(m_lastValueUpdateRecv);
    }
}

void ControlWidget::on_pushDirection(bool up)
{
    if (!m_unit.IsOn())
        m_unit.value = m_maximum;

    if (m_unit.value != (up ? m_maximum : 0))
        m_unit.value += (up ? 1 : -1);

    send_command(up ? CMD_BRIGHT : CMD_DIM); // , cancellable);
}

void ControlWidget::sliderMoved(int value)
{
    setUnitTo(value);
}

void ControlWidget::checkTimer(int value)
{
    if (value == -1)
    {
        SwitchOffTimer();
    }
    else if (value == 0) // dimmed to 0
    {
        if (m_parent->m_configSwitchOffWhenDimmedTo0)
            m_timerSwitchOff.start(m_parent->m_configSwitchOffWhenDimmedTo0PeriodSecs * 1000);
    }
    else if (m_configSwitchOffPeriodMn)
    {
        m_timerSwitchOff.start(m_configSwitchOffPeriodMn * 60000);
    }
    else
    {
        // we raised from 0 (value!=0), end timer @0
        if (m_timerSwitchOff.isActive() && m_parent->m_configSwitchOffWhenDimmedTo0)
            SwitchOffTimer();
    }
}

void ControlWidget::setUnitTo(int value)
{
    if (value == m_unit.value)
        return;

    m_timerSuspendUpdate.start(6000);

    m_unit.value = value;

    m_serviceUnit = m_parent->getDataEngine()->serviceForSource(m_unit.address());
    if (!m_serviceUnit || !m_serviceUnit->isOperationEnabled("Set"))
    {
        Logger::Log(ERROR, "No %s available in %s",
                    m_serviceUnit ? "SET Operation" : "Service",
                    m_unit.address());
        return;
    }

    KConfigGroup op = m_serviceUnit->operationDescription("Set");
    op.writeEntry("Value", value);

    KJob* job = m_serviceUnit->startOperationCall(op);
    connect(job, SIGNAL(finished(KJob*)), m_serviceUnit, SLOT(deleteLater()));

    checkTimer(value);

    refreshSlider();
    refreshOnOff();
}

void ControlWidget::send_command(COMMAND_TYPE command)
{
    char fullCommand[512];
    sprintf(fullCommand, "%c%s", command, m_unit.address());

    // not updating this crash after some reuse???
// TODO investigate.
    m_serviceUnit = m_parent->getDataEngine()->serviceForSource(m_unit.address());
    if (!m_serviceUnit->isOperationEnabled("Send"))
    {
        Logger::Log(INFO, "No Send Operation available");
        return;
    }

    KConfigGroup op = m_serviceUnit->operationDescription("Send");
    op.writeEntry("Command", QString(command));

    KJob* job = m_serviceUnit->startOperationCall(op);
    connect(job, SIGNAL(finished(KJob*)), m_serviceUnit, SLOT(deleteLater()));

    checkTimer((command == CMD_OFF) ? -1 : m_unit.value);

    refreshSlider();
    refreshOnOff();
}

void ControlWidget::dataUpdated(const QString& sourceName, const Plasma::DataEngine::Data& data)
{
    if (sourceName == m_unit.address())
    {
        QString latestExecCmd = data.value("LatestCmd").toString();
        m_lastValueUpdateRecv = data.value("Value").toInt();
// 		printf ( "latestExecCmd %s val=%d\n", qPrintable(latestExecCmd), m_lastValueUpdateRecv);

        if (!m_timerSuspendUpdate.isActive())
        {
            // outside update
            if (m_timerSwitchOff.isActive())
                SwitchOffTimer();

            updateValue(m_lastValueUpdateRecv);
            Logger::Log(DEBUG, "ControlWidget %s::dataUpdated %s = %d", qPrintable(m_name), qPrintable(sourceName), m_unit.value);
        }
        else if (m_lastValueUpdateRecv == m_unit.value)
        {
            // we reached desired value
//          Logger::Log(DEBUG, "ControlWidget %s::dataUpdated desired Value %s = %d", qPrintable(m_name), qPrintable(sourceName), m_unit.value);
            m_timerSuspendUpdate.stop();
        }
//      else
//          Logger::Log(DEBUG, "ControlWidget %s::dataUpdated: freeze active %s = %d", qPrintable(m_name), qPrintable(sourceName), m_unit.value);

    }
    else
        Logger::Log(ERROR, "ControlWidget %s::dataUpdated unknown name %s in source %s", qPrintable(m_name), qPrintable(sourceName), m_unit.address());
}

//////////

void ControlWidget::redraw()
{
    if (!m_layout)
    {
        m_layout = new QGraphicsLinearLayout(this);
// 		m_layout->setContentsMargins(0.8, 0.8, 0.8, 0.8);
// 		m_layout->setSpacing(0.7);
        m_layout->setOrientation(m_orientation);
    }

    int missing = 0;

    if (hasStyle(BRIGHT))
    {
        if (!m_pushButtonBright)
        {
            m_pushButtonBright = new Plasma::IconWidget(ResManager::Get()->Icon(ICON_UP), "", m_parent);
            m_pushButtonBright->setToolTip("Brighten");
            m_pushButtonBright->setMaximumSize(22, 22);
            QObject::connect(m_pushButtonBright, SIGNAL(clicked()), this, SLOT(on_pushBright()));
        }
        m_pushButtonBright->setVisible(true);
        m_layout->insertItem(0 - missing, m_pushButtonBright);
    }
    else
    {
        if (m_pushButtonBright)
        {
            m_layout->removeItem(m_pushButtonBright);
            m_pushButtonBright->setVisible(false);
        }
        missing++;
    }

    if (hasStyle(DIM))
    {
        if (!m_pushButtonDim)
        {
            m_pushButtonDim = new Plasma::IconWidget(ResManager::Get()->Icon(ICON_DOWN), "", m_parent);
            m_pushButtonDim->setToolTip("Dim");
            m_pushButtonDim->setMaximumSize(22, 22);
            QObject::connect(m_pushButtonDim, SIGNAL(clicked()), this, SLOT(on_pushDim()));
        }
        m_pushButtonDim->setVisible(true);
        m_layout->insertItem(1 - missing, m_pushButtonDim);
    }
    else
    {
        if (m_pushButtonDim)
        {
            m_layout->removeItem(m_pushButtonDim);
            m_pushButtonDim->setVisible(false);
        }
        missing++;
    }

    if (hasStyle(SLIDER))
    {
        if (!m_slider)
        {
            m_slider = new Plasma::Slider();
            m_slider->setRange(0, m_maximum);
            m_slider->nativeWidget()->setTickInterval(1);
            m_slider->nativeWidget()->setPageStep(1);
            m_slider->setOrientation(m_orientation);

            QObject::connect(m_slider, SIGNAL(valueChanged(int)), this, SLOT(sliderMoved(int)));
        }
        m_slider->setVisible(true);
        m_layout->insertItem(2 - missing, m_slider);
    }
    else
    {
        if (m_slider)
        {
            m_layout->removeItem(m_slider);
            m_slider->setVisible(false);
        }
        missing++;
    }

    if (hasStyle(ONOFF))
    {
        if (!m_pushButtonOnOff)
        {
            m_pushButtonOnOff = new Plasma::IconWidget(ResManager::Get()->Icon(isAppliance() ? ICON_APPOFF : ICON_OFF), "", m_parent);
            m_pushButtonOnOff->setMaximumSize(22, 22);
            m_pushButtonOnOff->setMinimumSize(22, 22);
            QObject::connect(m_pushButtonOnOff, SIGNAL(clicked()), this, SLOT(on_pushOnOff()));
        }
        m_pushButtonOnOff->setVisible(true);
        m_layout->insertItem(3 - missing, m_pushButtonOnOff);
    }
    else
    {
        if (m_pushButtonOnOff)
        {
            m_layout->removeItem(m_pushButtonOnOff);
            m_pushButtonOnOff->setVisible(false);
        }
        missing++;
    }

    updateTooltip();
}

void ControlWidget::refreshOnOff()
{
    if (m_pushButtonOnOff)
    {
        m_pushButtonOnOff->setIcon(m_unit.IsOn() ?
                                   ResManager::Get()->Icon(isAppliance() ? ICON_APPON :
                                           m_timerSwitchOff.isActive() ? ICON_ON_TIMER : ICON_ON) :
                                       ResManager::Get()->Icon(isAppliance() ? ICON_APPOFF : ICON_OFF));
    }
}

void ControlWidget::refreshSlider()
{
    if (m_slider)
    {
        bool oldState = m_slider->blockSignals(true);

        if (m_unit.value <= 0)
            m_slider->setValue(0);
        else if (m_unit.value > m_slider->maximum())
            m_slider->setValue(m_maximum);
        else
            m_slider->setValue(m_unit.value);

        m_slider->blockSignals(oldState);
    }
}

void ControlWidget::updateValue(int newValue)
{
    if (m_unit.value == newValue)
        return;

    m_unit.value = newValue;

    if (newValue == -1 && m_timerSwitchOff.isActive())
        SwitchOffTimer();

    refreshSlider();
    refreshOnOff();
}
