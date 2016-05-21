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

#include "x10-widget.h"

#include <common/rule.h>
#include <common/logger.h>
#include <common/operations.h>

#include <KTemporaryFile>
#include <QSortFilterProxyModel>
#include <QFileDialog>

#include <string>
using namespace std;

void X10Widget::defaultConfig()
{
    m_configDefaultLighting                  = 80;
    m_configDefaultSwitchOffPeriodMn         = 5;
    m_configSwitchOffWhenDimmedTo0           = true;
    m_configSwitchOffWhenDimmedTo0PeriodSecs = 20;
    m_configRememberLastIntensity            = true;
    m_configRememberLastIntensityThreshold   = 20;
    m_configUseSmartQueue                    = true;
    m_configTimeoutInc                       = 1;
}

void X10Widget::defaultsToCfg()
{
    defaultConfig();
    plasmoidToCfg();
}

void X10Widget::plasmoidToCfg()
{
    m_cfgSettings.spinBoxDefaultLighting->setValue(m_configDefaultLighting);
    m_cfgSettings.spinBoxDefaultSwitchOffPeriod->setValue(m_configDefaultSwitchOffPeriodMn);

    m_cfgSettings.checkBoxSwitchOffWhenDimmedTo0->setCheckState(m_configSwitchOffWhenDimmedTo0 ? Qt::Checked : Qt::Unchecked);
    m_cfgSettings.spinBoxSwitchOffWhenDimmedTo0->setValue(m_configSwitchOffWhenDimmedTo0PeriodSecs);
    m_cfgSettings.checkBoxRememberLastIntensity->setCheckState(m_configRememberLastIntensity ? Qt::Checked : Qt::Unchecked);
    m_cfgSettings.spinBoxRememberLastIntensity->setValue(m_configRememberLastIntensityThreshold);

    // Service
    m_cfgSettings.checkBoxUseSmartQueue->setCheckState(m_configUseSmartQueue ? Qt::Checked : Qt::Unchecked);

    // Controller
    if (m_dbusControllerStr == PROPVAL_CONTROLLER_SYSTEMDBUS)
    {
        m_cfgSettings.lineEditControllerIP->setText(PROPVAL_CONTROLLER_SYSTEMDBUS);
        m_cfgSettings.lineEditControllerPort->setText(0);
    }
    else
    {
        QStringList addressesSplit = m_dbusControllerStr.split(":", QString::SkipEmptyParts);
        if (addressesSplit.count() != 2)
        {
            Logger::Log(ERROR, "Invalid controller address received from DataEngine (%s)", qPrintable(m_dbusControllerStr));
            m_cfgSettings.lineEditControllerIP->setText("ERROR");
            m_cfgSettings.lineEditControllerPort->setText(0);
        }
        else
        {
            m_cfgSettings.lineEditControllerIP->setText(addressesSplit[0]);
            m_cfgSettings.lineEditControllerPort->setText(addressesSplit[1]);
        }
    }
    m_cfgSettings.label_ControllerStatus->setText(m_dbusControllerConnected?"Connected":"Disconnected");

    // rules
    rulesToCfg();
}

void X10Widget::rulesToCfg()
{
    m_rulesSaved = Rule::serializedRules(m_rules);

    m_cfgRules.listWidgetRules->clear();
    for (Rule::ListIterator it = m_rules.begin(); it != m_rules.end(); it++)
    {
        m_cfgRules.listWidgetRules->addItem(it->getDisplayText());
    }

    m_cfgRules.pushButtonRemove->setEnabled(m_cfgRules.listWidgetRules->count());
    m_cfgRules.listWidgetRules->reset();
}

void X10Widget::settingsRestoreDefaults()
{
    defaultsToCfg();
}

void X10Widget::configCancelled()
{
    disconnect(this, SIGNAL(RulesUpdated()));
    Rule::deserializeRules(m_rules, m_rulesSaved);
    layoutControlWidgets();
}

void X10Widget::configAccepted()
{
    disconnect(this, SIGNAL(RulesUpdated()));
    Logger::Log(DEBUG, "configAccepted");

    m_configDefaultLighting = m_cfgSettings.spinBoxDefaultLighting->value();
    m_configDefaultSwitchOffPeriodMn = m_cfgSettings.spinBoxDefaultSwitchOffPeriod->value();

    m_configSwitchOffWhenDimmedTo0 = (m_cfgSettings.checkBoxSwitchOffWhenDimmedTo0->checkState() == Qt::Checked);
    m_configSwitchOffWhenDimmedTo0PeriodSecs = m_cfgSettings.spinBoxSwitchOffWhenDimmedTo0->value();
    m_configRememberLastIntensity = (m_cfgSettings.checkBoxRememberLastIntensity->checkState() == Qt::Checked);
    m_configRememberLastIntensityThreshold = m_cfgSettings.spinBoxRememberLastIntensity->value();

    // service
    ruleButtonSave();	// if pending change

    if (m_rulesSaved != Rule::serializedRules(m_rules))
        SetProperty(PROPERTY_RULES, Rule::serializedRules(m_rules));

    const bool useSmartQueuingIsChecked = (m_cfgSettings.checkBoxUseSmartQueue->checkState() == Qt::Checked);
    if (m_configUseSmartQueue != useSmartQueuingIsChecked)
    {
        m_configUseSmartQueue = useSmartQueuingIsChecked;
        SetProperty(PROPERTY_SMARTQUEUE, m_configUseSmartQueue);
    }

    writeConfig();
}


void X10Widget::writeConfig()
{
    Logger::Log(DEBUG, "writeConfig");

    KConfigGroup cg = config();

    cg.deleteGroup();

    cg.writeEntry("DefaultLighting",            m_configDefaultLighting);
    cg.writeEntry("DefaultSwitchOffPeriodMn",   m_configDefaultSwitchOffPeriodMn);

    cg.writeEntry("SwitchOffWhenDimmedTo0",             m_configSwitchOffWhenDimmedTo0);
    cg.writeEntry("SwitchOffWhenDimmedTo0PeriodSecs",   m_configSwitchOffWhenDimmedTo0PeriodSecs);
    cg.writeEntry("RememberLastIntensity",              m_configRememberLastIntensity);
    cg.writeEntry("RememberLastIntensityThreshold",     m_configRememberLastIntensityThreshold);

    cg.writeEntry("TimeoutInc",         m_configTimeoutInc);

    cg.group("Nodes").deleteGroup();
    m_nodes.writeConfig(cg.group("Nodes"));

    emit configNeedsSaving();
}

void X10Widget::readConfig()
{
    Logger::Log(DEBUG, "readConfig");

    KConfigGroup cg = config();
    if (cg.hasKey("DefaultLighting"))
    {
        m_configDefaultLighting = cg.readEntry("DefaultLighting", m_configDefaultLighting);
    }

    if (cg.hasKey("DefaultDefaultSwitchOffPeriodMn"))
    {
        m_configDefaultSwitchOffPeriodMn = cg.readEntry("DefaultSwitchOffPeriodMn", m_configDefaultSwitchOffPeriodMn);
    }

    if (cg.hasKey("SwitchOffWhenDimmedTo0"))
    {
        m_configSwitchOffWhenDimmedTo0 = cg.readEntry("SwitchOffWhenDimmedTo0", m_configSwitchOffWhenDimmedTo0);
    }
    if (cg.hasKey("SwitchOffWhenDimmedTo0PeriodSecs"))
    {
        m_configSwitchOffWhenDimmedTo0PeriodSecs = cg.readEntry("SwitchOffWhenDimmedTo0PeriodSecs", m_configSwitchOffWhenDimmedTo0PeriodSecs);
    }
    if (cg.hasKey("RememberLastIntensity"))
    {
        m_configRememberLastIntensity = cg.readEntry("RememberLastIntensity", m_configRememberLastIntensity);
    }
    if (cg.hasKey("RememberLastIntensityThreshold"))
    {
        m_configRememberLastIntensityThreshold = cg.readEntry("RememberLastIntensityThreshold", m_configRememberLastIntensityThreshold);
    }

    ////////////////////////////////////////////////////////

    if (cg.hasKey("TimeoutInc"))
    {
        m_configTimeoutInc = cg.readEntry("TimeoutInc", m_configTimeoutInc);
    }

    ////////////////////////////////////////////////////////

    if (cg.hasGroup("Nodes"))
    {
        if (!m_nodes.rowCount())    // don't replace
        {
            m_nodes.readConfig(cg.group("Nodes"));  // reload file
        }
        layoutControlWidgets();     // redraw add (TODO for now) + relayout widgets
    }
}


/// NODES ////////////////////////////////////////////////////////////////////////////

void X10Widget::nodeButtonAdd()
{
    QSortFilterProxyModel* proxy = static_cast<QSortFilterProxyModel*>(m_cfgNodes.tableView->model());
    proxy->setDynamicSortFilter(false);

    if (!m_nodes.insertRow(0, QModelIndex()))
        return;

    m_nodes.setData(m_nodes.index(0, COL_ONDE, QModelIndex()) , m_configDefaultLighting, Qt::EditRole);
    m_nodes.setData(m_nodes.index(0, COL_OFTI, QModelIndex()) , m_configDefaultSwitchOffPeriodMn, Qt::EditRole);

    m_cfgNodes.tableView->selectRow(0);

    proxy->setDynamicSortFilter(true);
    layoutControlWidgets();

    QItemSelectionModel* selectionModel = m_cfgNodes.tableView->selectionModel();

    m_cfgNodes.tableView->scrollTo(selectionModel->selectedRows().first(), QAbstractItemView::EnsureVisible);

}

void X10Widget::nodeButtonRem()
{
    QSortFilterProxyModel* proxy = static_cast<QSortFilterProxyModel*>(m_cfgNodes.tableView->model());
    QItemSelectionModel* selectionModel = m_cfgNodes.tableView->selectionModel();

    QModelIndexList indexes = selectionModel->selectedRows();
    QModelIndex index;

    foreach (index, indexes)
    {
        int row = proxy->mapToSource(index).row();
        ControlWidgetInfo* widget = m_nodes.Nodes()->at(row);

        if (widget && widget->GetWidget())
        {
            m_layout->removeItem(widget->GetWidget());
            widget->DeleteWidget();
        }

        m_nodes.removeRows(row, 1, QModelIndex());
    }
//  resize(1,1);
}


/// RULES ////////////////////////////////////////////////////////////////////////////

void X10Widget::ruleSettingsInvalidate()
{
    rulesToCfg();
}

void X10Widget::ruleSettingsModified()
{
    saveRule(m_cfgRules.listWidgetRules->currentRow());

    QListWidgetItem* selected = m_cfgRules.listWidgetRules->currentItem();
    if (selected)
    {
        selected->setText( m_rules[m_cfgRules.listWidgetRules->row(selected)].getDisplayText() );
    }

// 	emit configUpdated();
}

void X10Widget::ruleButtonAdd()
{
    Rule newRule;
    m_rules.push_back(newRule);
    m_cfgRules.listWidgetRules->addItem(newRule.getDisplayText());
    m_cfgRules.listWidgetRules->setCurrentRow(m_cfgRules.listWidgetRules->count()-1);
//     m_cfgRules.listWidgetRules->setCurrentItem(m_cfgRules.listWidgetRules->item(m_cfgRules.listWidgetRules->count()-1));
    m_cfgRules.pushButtonRemove->setEnabled(true);
    emit configUpdated();
}

void X10Widget::ruleButtonRem()
{
    int index = m_cfgRules.listWidgetRules->currentRow();
    if (index == -1) return;

    delete m_cfgRules.listWidgetRules->takeItem(index);
    m_rules.erase( m_rules.begin() + index);

    if (!m_cfgRules.listWidgetRules->count())
        m_cfgRules.pushButtonRemove->setEnabled(false);

    emit configUpdated();
}

void X10Widget::ruleButtonSave()
{
    saveRule(m_cfgRules.listWidgetRules->currentRow());
}

void X10Widget::ruleButtonClear()
{
    m_cfgRules.checkBox_Mon->setCheckState(Qt::Unchecked);
    m_cfgRules.checkBox_Tue->setCheckState(Qt::Unchecked);
    m_cfgRules.checkBox_Wed->setCheckState(Qt::Unchecked);
    m_cfgRules.checkBox_Thu->setCheckState(Qt::Unchecked);
    m_cfgRules.checkBox_Fri->setCheckState(Qt::Unchecked);
    m_cfgRules.checkBox_Sat->setCheckState(Qt::Unchecked);
    m_cfgRules.checkBox_Sun->setCheckState(Qt::Unchecked);

    ruleSettingsModified();
}

void X10Widget::ruleCurrentItemChanged(QListWidgetItem* /*current*/, QListWidgetItem* /*previous*/)
{
    loadRule(m_cfgRules.listWidgetRules->currentRow());
}

bool X10Widget::validateRuleAction()
{
    QFont font = m_cfgRules.lineEditAction->font();
    if (m_cfgRules.comboBoxAction->currentText() == "Send")
        font.setStrikeOut(!Rule::validateAction(m_cfgRules.lineEditAction->text()));
    else if (m_cfgRules.comboBoxAction->currentText() == "Set")
        font.setStrikeOut(!X10::isValidValue(m_cfgRules.lineEditAction->text()));
    else
        font.setStrikeOut(false);

    m_cfgRules.lineEditAction->setFont(font);

    return !font.strikeOut();
}

void X10Widget::ruleComboBoxActionIndexChanged(QString action)
{
    m_cfgRules.lineEditAction->setVisible(action == "Send" || action == "Set");

    // validate current actionSpecific for selected action
    validateRuleAction();

    if (action == "Send")
        m_cfgRules.lineEditAction->setToolTip("Enter any combination of { + - b s }");
    else if (action == "Set")
        m_cfgRules.lineEditAction->setToolTip("Enter a valid value to set to : [-1 .. 7]");
    else
        m_cfgRules.lineEditAction->setToolTip("");
}

void X10Widget::ruleLineEditActionTextChanged(QString /*actions*/)
{
    if (validateRuleAction())
        ruleSettingsModified();
}

bool X10Widget::validateRuleAddress()
{
    QFont font = m_cfgRules.lineEditAddress->font();
    const QString address = m_cfgRules.lineEditAddress->text();
    if (address.toUpper() != address)
        m_cfgRules.lineEditAddress->setText(address.toUpper());
    const bool valid = Rule::validateAddress(address);
    font.setStrikeOut(!valid);
    m_cfgRules.lineEditAddress->setFont(font);
    return valid;
}

void X10Widget::ruleLineEditAddressTextChanged(QString /*addresses*/)
{
    if (validateRuleAddress())
        ruleSettingsModified();
}

void X10Widget::loadRule(const int index)
{
    QFont font;
    if (index == -1)
    {
        // clear
        m_cfgRules.groupBox_EDIT->setEnabled(false);

        m_cfgRules.checkBoxEnabled->setCheckState(Qt::Unchecked);

        font = m_cfgRules.lineEditAddress->font();
        font.setStrikeOut(false);
        m_cfgRules.lineEditAddress->setFont(font);
        m_cfgRules.lineEditAddress->setText("");

        m_cfgRules.timeExecTime->setTime(QTime(0, 0, 0, 0));

        ruleButtonClear();

        m_cfgRules.comboBoxAction->setCurrentIndex(0);

        font = m_cfgRules.lineEditAction->font();
        font.setStrikeOut(false);
        m_cfgRules.lineEditAction->setFont(font);
        m_cfgRules.lineEditAction->setText("");

        m_cfgRules.spinBox_repeatTimes->setValue(0);
        m_cfgRules.spinBox_repeatPeriod->setValue(0);
        m_cfgRules.spinBox_repeatPeriod->setEnabled(false);
    }
    else
    {
        m_cfgRules.groupBox_EDIT->setEnabled(true);

        Rule rule = m_rules[index];

        m_cfgRules.checkBoxEnabled->setCheckState(rule.enabled?Qt::Checked:Qt::Unchecked);

        m_cfgRules.lineEditAddress->setText(rule.address);

        const uint hour = rule.exec_time/3600;
        m_cfgRules.timeExecTime->setTime(QTime(hour, (rule.exec_time%3600) / 60, 0, 0));

        m_cfgRules.checkBox_Mon->setCheckState(rule.period & Rule::MONDAY?Qt::Checked:Qt::Unchecked);
        m_cfgRules.checkBox_Tue->setCheckState(rule.period & Rule::TUESDAY?Qt::Checked:Qt::Unchecked);
        m_cfgRules.checkBox_Wed->setCheckState(rule.period & Rule::WEDNESDAY?Qt::Checked:Qt::Unchecked);
        m_cfgRules.checkBox_Thu->setCheckState(rule.period & Rule::THURSDAY?Qt::Checked:Qt::Unchecked);
        m_cfgRules.checkBox_Fri->setCheckState(rule.period & Rule::FRIDAY?Qt::Checked:Qt::Unchecked);
        m_cfgRules.checkBox_Sat->setCheckState(rule.period & Rule::SATURDAY?Qt::Checked:Qt::Unchecked);
        m_cfgRules.checkBox_Sun->setCheckState(rule.period & Rule::SUNDAY?Qt::Checked:Qt::Unchecked);

        m_cfgRules.comboBoxAction->setCurrentIndex(rule.actionType);

        font = m_cfgRules.lineEditAction->font();
        font.setStrikeOut(false);
        m_cfgRules.lineEditAction->setFont(font);
        m_cfgRules.lineEditAction->setText(rule.action);

        m_cfgRules.spinBox_repeatTimes->setValue(rule.repeatTimes);
        m_cfgRules.spinBox_repeatPeriod->setValue(rule.repeatPeriodMn);
        m_cfgRules.spinBox_repeatPeriod->setEnabled((rule.repeatTimes != 0));
    }
}

void X10Widget::saveRule(const int index)
{
    if (index == -1 || (uint)index >= m_rules.size()) return;

    Rule* rule = &m_rules[index];

    rule->enabled = m_cfgRules.checkBoxEnabled->isChecked();

    if (!m_cfgRules.lineEditAddress->font().strikeOut())
        rule->address = m_cfgRules.lineEditAddress->text();
    else
    {
        QFont font = m_cfgRules.lineEditAddress->font();
        font.setStrikeOut(false);
        m_cfgRules.lineEditAddress->setFont(font);
        m_cfgRules.lineEditAddress->setText(rule->address);
    }

    QTime timeExec = m_cfgRules.timeExecTime->time();
    rule->exec_time = timeExec.hour()*3600 + timeExec.minute()*60;

    rule->period = (Rule::DAYS) (
                       (int)m_cfgRules.checkBox_Mon->isChecked() * Rule::MONDAY +
                       (int)m_cfgRules.checkBox_Tue->isChecked() * Rule::TUESDAY +
                       (int)m_cfgRules.checkBox_Wed->isChecked() * Rule::WEDNESDAY +
                       (int)m_cfgRules.checkBox_Thu->isChecked() * Rule::THURSDAY +
                       (int)m_cfgRules.checkBox_Fri->isChecked() * Rule::FRIDAY +
                       (int)m_cfgRules.checkBox_Sat->isChecked() * Rule::SATURDAY +
                       (int)m_cfgRules.checkBox_Sun->isChecked() * Rule::SUNDAY );

    rule->actionType = (Rule::ACTION_TYPE) m_cfgRules.comboBoxAction->currentIndex();

    if (!m_cfgRules.lineEditAction->font().strikeOut())
        rule->action = m_cfgRules.lineEditAction->text();
    else
    {
// 		QFont font = m_cfgRules.lineEditAction->font();
// 		font.setStrikeOut(false);
// 		m_cfgRules.lineEditAction->setFont(font);
// 		m_cfgRules.lineEditAction->setText(rule->action);
    }


    rule->repeatTimes  = m_cfgRules.spinBox_repeatTimes->value();
    rule->repeatPeriodMn = m_cfgRules.spinBox_repeatPeriod->value();

    // update name of rules in list
    m_cfgRules.listWidgetRules->item(index)->setText(rule->getDisplayText());

    emit configUpdated();
}


void X10Widget::ruleTimeExecTimeChanged(QTime newtime)
{
    time_t newTimeI = newtime.hour()*3600 + newtime.minute()*60;

    if (m_rules[m_cfgRules.listWidgetRules->currentRow()].exec_time != newTimeI)
        ruleSettingsModified();
}

void X10Widget::ruleSpinBoxRepeatTimesChanged(int newValue)
{
    if ((int)m_rules[m_cfgRules.listWidgetRules->currentRow()].repeatTimes != newValue)
        ruleSettingsModified();
    m_cfgRules.spinBox_repeatPeriod->setEnabled(newValue!=0);
}

void X10Widget::ruleSpinBoxRepeatPeriodChanged(int newValue)
{
    if (m_rules[m_cfgRules.listWidgetRules->currentRow()].repeatPeriodMn != newValue)
        ruleSettingsModified();
}

