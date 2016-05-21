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

// Here we avoid loading the header multiple times
#ifndef X10_WIDGET_HEADER
#define X10_WIDGET_HEADER

#include <QAction>
#include <QIcon>
#include <QImage>

#include <Plasma/Applet>
#include <Plasma/DataEngine>

#include "ui_config.h"
#include "ui_nodes.h"
#include "ui_rules.h"

#include "control-widget.h"
#include "control-widgetinfo.h"
#include "nodes-model.h"

#include <common/rule.h>

class X10Widget : public Plasma::Applet
{
    Q_OBJECT

public:
    X10Widget(QObject* parent, const QVariantList& args);
    ~X10Widget();

    virtual QList<QAction*> contextualActions();

    Plasma::DataEngine* getDataEngine() {
        return m_engine;
    }

Q_SIGNALS:
    void settingsChanged(bool modified);
    void serviceStatusChanged(QString status);
    void configUpdated();

    void RulesUpdated();

public slots:
    void configChanged();
    void dataUpdated(const QString& name, const Plasma::DataEngine::Data& data);
    //  void sourceChanged(const QString &name);
    //  void sourcesChanged();
    void layoutControlWidgets();

protected slots:
    void createConfigurationInterface(KConfigDialog* parent);
    void configCancelled();
    void configAccepted();

    void settingsReloadDBUS();
    void settingsRestoreDefaults();

    void AllOn();
    void AllOff();

    // config node ui callbacks
    void nodeButtonAdd();
    void nodeButtonRem();

    // rules node ui callbacks
    void ruleSettingsInvalidate();
    void ruleSettingsModified();
    void ruleButtonAdd();
    void ruleButtonRem();
    void ruleButtonSave();
    void ruleButtonClear();
    void ruleCurrentItemChanged(QListWidgetItem* current, QListWidgetItem* previous);
    void ruleComboBoxActionIndexChanged(QString action);
    void ruleLineEditActionTextChanged(QString actions);
    void ruleLineEditAddressTextChanged(QString addresses);

    // these callbacks required since TimeWIdget & SpinBox don't differenciate between program and user changed
    void ruleTimeExecTimeChanged(QTime newtime);
    void ruleSpinBoxRepeatTimesChanged (int newValue);
    void ruleSpinBoxRepeatPeriodChanged(int newValue);

protected:

    void init();
    void createMenu();

    void defaultConfig();
    void readConfig();
    void writeConfig();
    void defaultsToCfg();
    void plasmoidToCfg();
    void rulesToCfg();

    void loadRule(const int index);
    void saveRule(const int index);
    bool validateRuleAction();
    bool validateRuleAddress();

    bool SetProperty(const QString& property, const QVariant& value, ...);
    bool SendToService(const QString& operation);
    bool IsServiceRunning() const {
        return m_serviceIsRunning;
    }

public:
    int                 m_configDefaultLighting;
    int                 m_configDefaultSwitchOffPeriodMn;
    bool                m_configSwitchOffWhenDimmedTo0;
    int                 m_configSwitchOffWhenDimmedTo0PeriodSecs;
    bool                m_configRememberLastIntensity;
    int                 m_configRememberLastIntensityThreshold;
    bool                m_configUseSmartQueue;
    int                 m_configTimeoutInc;
    QString             m_dbusControllerStr;
    bool                m_dbusControllerConnected;

private:
    QGraphicsLinearLayout* m_layout;
    QList<QAction*>     m_actions;
    QAction*            m_actionAllOn;
    QAction*            m_actionAllOff;

    // Config Screens  (defined in #include "ui_nameui.h")
    Ui_ConfigSettings 	m_cfgSettings;
    Ui_ConfigNodes    	m_cfgNodes;
    Ui_ConfigRules		m_cfgRules;

    NodesModel          m_nodes;
    Rule::List			m_rules;
    QString             m_rulesSaved;

    Plasma::DataEngine* m_engine;
    Plasma::Service*    m_service;
    QString             m_serviceStatus;    // queue is running
    bool                m_serviceIsRunning;
};

#endif // X10_WIDGET_HEADER
