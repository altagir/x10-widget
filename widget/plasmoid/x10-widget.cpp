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

#include <KConfigDialog>
#include <Plasma/ServiceJob>

#include <plasma/widgets/lineedit.h>
#include <icons/resources.h>

#include <unistd.h>
#include <QtGui>

#include <common/logger.h>
#include <common/operations.h>

// /// ////////////////////////////////////////////////////////////////////////////

X10Widget::X10Widget(QObject* parent, const QVariantList& args) :
    Plasma::Applet(parent, args),
    m_layout(0),
    m_actionAllOn(0),
    m_actionAllOff(0),
    m_engine(0),
    m_service(0),
    m_serviceStatus("Unknown"),
    m_serviceIsRunning(false)
{
    defaultConfig();

    setBackgroundHints(DefaultBackground);
//  setBackgroundHints(TranslucentBackground);
    setHasConfigurationInterface(true);
}

bool X10Widget::SetProperty(const QString& property, const QVariant& value, ...)
{
    Logger::Log(INFO, "X10Widget: SetProperty %s = %s", qPrintable(property), value.canConvert(QVariant::String) ? qPrintable(value.toString()) : "");

    if (!m_engine || !m_engine->isValid())
    {
        Logger::Log(ERROR, "SetProperty: no valid engine");
        return false;
    }

    m_service = (m_engine && m_engine->isValid()) ? m_engine->serviceForSource("") : 0;
    if (!m_service || !m_service->isOperationEnabled("SetProperty"))
    {
        Logger::Log(ERROR, "SetProperty: operation not enabled");
        return false;
    }

    KConfigGroup op = m_service->operationDescription("SetProperty");
    op.writeEntry("Property", property);
    op.writeEntry("Value", value);

    KJob* job = m_service->startOperationCall(op);
    connect(job, SIGNAL(finished(KJob*)), m_service, SLOT(deleteLater()));

    return true;
}

bool X10Widget::SendToService(const QString& operation)
{
    Logger::Log(INFO, "X10Widget: SendToService %s", qPrintable(operation));

    if (operation.isEmpty() || !m_engine || !m_engine->isValid())
    {
        Logger::Log(ERROR, "SendToService: no valid engine");
        return false;
    }

    // else crashing...
    m_service = (m_engine && m_engine->isValid()) ? m_engine->serviceForSource("") : 0;
    if (!m_service || !m_service->isOperationEnabled(operation))
    {
        Logger::Log(ERROR, "SendToService: operation %s not enabled", qPrintable(operation));
        return false;
    }

    KConfigGroup op = m_service->operationDescription(operation);
    KJob* job = m_service->startOperationCall(op);
    connect(job, SIGNAL(finished(KJob*)), m_service, SLOT(deleteLater()));

    return true;
}

void X10Widget::settingsReloadDBUS()
{
    Logger::Log(INFO, "X10Widget: reloading engine DBUS");

    if (!m_engine || !m_engine->isValid())
    {
        Logger::Log(ERROR, "settingsReloadDBUS: no valid engine");
        return;
    }

    m_service = (m_engine && m_engine->isValid()) ? m_engine->serviceForSource("") : 0;
    if (!m_service || !m_service->isOperationEnabled("Reload"))
    {
        Logger::Log(ERROR, "settingsReloadDBUS: operation Reload not enabled");
        return;
    }

    KConfigGroup op = m_service->operationDescription("Reload");

    KJob* job = m_service->startOperationCall(op);
    connect(job, SIGNAL(finished(KJob*)), m_service, SLOT(deleteLater()));
}

X10Widget::~X10Widget()
{
    if (hasFailedToLaunch())
    {
        // Do some cleanup here
    }
    else
    {
        // Save settings
    }
}

//

void X10Widget::AllOff()
{
    for (int i = 0; i < m_nodes.rowCount(); i++)
        m_nodes.Nodes()->at(i)->GetWidget()->Off();
}

void X10Widget::AllOn()
{
    for (int i = 0; i < m_nodes.rowCount(); i++)
        m_nodes.Nodes()->at(i)->GetWidget()->On();
}


void X10Widget::createConfigurationInterface(KConfigDialog* parent)
{
    Plasma::Applet::createConfigurationInterface(parent);

    QWidget* cfg_settings = new QWidget();
    m_cfgSettings.setupUi(cfg_settings);

    QWidget* cfg_nodes = new QWidget();
    m_cfgNodes.setupUi(cfg_nodes);

    QWidget* cfg_rules = new QWidget();
    m_cfgRules.setupUi(cfg_rules);


    // when rules come back from server. dup may be removed
    connect(this, SIGNAL(RulesUpdated()), this, SLOT(ruleSettingsInvalidate()));

    connect(parent, SIGNAL(cancelClicked()), this, SLOT(configCancelled()));
    connect(parent, SIGNAL(applyClicked()),  this, SLOT(configAccepted()));
    connect(parent, SIGNAL(okClicked()),     this, SLOT(configAccepted()));

    plasmoidToCfg();

    connect(this,   SIGNAL(configUpdated()), parent, SLOT(settingsModified()));

    ///////////////////////////////////////////////////////////////
    /// m_cfgSettings

//     connect(this, SIGNAL(serviceStatusChanged(QString)), m_cfgSettings.pushButtonServicetatus, SLOT(setText(QString)));
    connect(m_cfgSettings.spinBoxDefaultLighting,           SIGNAL(valueChanged(int)),  parent, SLOT(settingsModified()));
    connect(m_cfgSettings.spinBoxDefaultSwitchOffPeriod,    SIGNAL(valueChanged(int)),  parent, SLOT(settingsModified()));

    connect(m_cfgSettings.checkBoxSwitchOffWhenDimmedTo0,   SIGNAL(clicked(bool)),      parent, SLOT(settingsModified()));
    connect(m_cfgSettings.spinBoxSwitchOffWhenDimmedTo0,    SIGNAL(valueChanged(int)),  parent, SLOT(settingsModified()));
    connect(m_cfgSettings.checkBoxRememberLastIntensity,    SIGNAL(clicked(bool)),      parent, SLOT(settingsModified()));
    connect(m_cfgSettings.spinBoxRememberLastIntensity,     SIGNAL(valueChanged(int)),  parent, SLOT(settingsModified()));

    connect(m_cfgSettings.checkBoxUseSmartQueue,            SIGNAL(clicked(bool)),      parent, SLOT(settingsModified()));

    connect(m_cfgSettings.pushButtonReload,                 SIGNAL(clicked(bool)),      this, SLOT(settingsReloadDBUS()));
    connect(m_cfgSettings.pushButtonDefaults,               SIGNAL(clicked(bool)),      this, SLOT(settingsRestoreDefaults()));

    ///////////////////////////////////////////////////////////////
    /// cfg_nodes
    connect(m_cfgNodes.pushButtonAdd,       SIGNAL(clicked(bool)), this, SLOT(nodeButtonAdd()));
    connect(m_cfgNodes.pushButtonRemove,    SIGNAL(clicked(bool)), this, SLOT(nodeButtonRem()));
    connect(&m_nodes,                       SIGNAL(editCompleted(QString)), parent, SLOT(settingsModified()));

    QSortFilterProxyModel* proxyModel = new QSortFilterProxyModel(this);
    proxyModel->setSourceModel(&m_nodes);
    proxyModel->setDynamicSortFilter(true);

    m_cfgNodes.tableView->setModel(proxyModel);
    m_cfgNodes.tableView->setSortingEnabled(true);

    // regex to filter
//  QString newStr = QString("^[%1].*").arg(str);
//  proxyModel->setFilterRegExp(QRegExp(newStr, Qt::CaseInsensitive));
//  proxyModel->setFilterKeyColumn(0);

    proxyModel->sort(COL_COL, Qt::AscendingOrder);
    proxyModel->sort(COL_ROW, Qt::AscendingOrder);

    m_cfgNodes.tableView->resizeColumnToContents(COL_NAME);

    m_cfgNodes.tableView->setColumnWidth(COL_CHAN, 40);
    m_cfgNodes.tableView->setColumnWidth(COL_UNIT, 40);

    m_cfgNodes.tableView->setColumnWidth(COL_HORZ, 50);

    m_cfgNodes.tableView->setColumnWidth(COL_OFTI, 35);
    m_cfgNodes.tableView->setColumnWidth(COL_ONDE, 35);

    m_cfgNodes.tableView->setColumnWidth(COL_BSLI, 25);
    m_cfgNodes.tableView->setColumnWidth(COL_BTOF, 25);
    m_cfgNodes.tableView->setColumnWidth(COL_BTUD, 25);

    m_cfgNodes.tableView->setColumnWidth(COL_ROW, 30);
    m_cfgNodes.tableView->setColumnWidth(COL_COL, 30);

//     m_cfgNodes.tableView->resizeColumnToContents(COL_RULE);

    m_cfgNodes.tableView->show();

    ///////////////////////////////////////////////////////////////
    /// cfg_rules

    connect(m_cfgRules.checkBoxEnabled,      SIGNAL(clicked(bool)),  this, SLOT(ruleSettingsModified()));

    connect(m_cfgRules.checkBox_Mon,         SIGNAL(clicked(bool)),  this, SLOT(ruleSettingsModified()));
    connect(m_cfgRules.checkBox_Tue,         SIGNAL(clicked(bool)),  this, SLOT(ruleSettingsModified()));
    connect(m_cfgRules.checkBox_Wed,         SIGNAL(clicked(bool)),  this, SLOT(ruleSettingsModified()));
    connect(m_cfgRules.checkBox_Thu,         SIGNAL(clicked(bool)),  this, SLOT(ruleSettingsModified()));
    connect(m_cfgRules.checkBox_Fri,         SIGNAL(clicked(bool)),  this, SLOT(ruleSettingsModified()));
    connect(m_cfgRules.checkBox_Sat,         SIGNAL(clicked(bool)),  this, SLOT(ruleSettingsModified()));
    connect(m_cfgRules.checkBox_Sun,         SIGNAL(clicked(bool)),  this, SLOT(ruleSettingsModified()));

    connect(m_cfgRules.comboBoxAction,       SIGNAL(activated(QString)), this, SLOT(ruleSettingsModified()));

    connect(m_cfgRules.timeExecTime,         SIGNAL(timeChanged(QTime)), this, SLOT(ruleTimeExecTimeChanged(QTime)));
    connect(m_cfgRules.spinBox_repeatTimes,  SIGNAL(valueChanged(int)),  this, SLOT(ruleSpinBoxRepeatTimesChanged(int)));
    connect(m_cfgRules.spinBox_repeatPeriod, SIGNAL(valueChanged(int)),  this, SLOT(ruleSpinBoxRepeatPeriodChanged(int)));

    connect(m_cfgRules.pushButtonAdd,        SIGNAL(clicked(bool)), this, SLOT(ruleButtonAdd()));
    connect(m_cfgRules.pushButtonRemove,     SIGNAL(clicked(bool)), this, SLOT(ruleButtonRem()));
    connect(m_cfgRules.pushButtonClear,      SIGNAL(clicked(bool)), this, SLOT(ruleButtonClear()));

    // listWidgetRules
    connect(m_cfgRules.listWidgetRules, SIGNAL(currentItemChanged(QListWidgetItem*,QListWidgetItem*)),
            this, SLOT(ruleCurrentItemChanged(QListWidgetItem*,QListWidgetItem*)));

    connect(m_cfgRules.lineEditAddress,  SIGNAL(textEdited(QString)),
            this, SLOT(ruleLineEditAddressTextChanged(QString)));

    m_cfgRules.lineEditAction->setVisible(false);

    connect(m_cfgRules.comboBoxAction,  SIGNAL(currentIndexChanged(QString)),   this, SLOT(ruleComboBoxActionIndexChanged(QString)));
    connect(m_cfgRules.lineEditAction,  SIGNAL(textEdited(QString)), this, SLOT(ruleLineEditActionTextChanged(QString)));


    ///////////////////////////////////////////////////////////////
    /// PAGES
    parent->addPage(cfg_nodes, i18n("Nodes"),  ResManager::Get()->IconName(ICON_ON));
    parent->addPage(cfg_settings, i18n("X10"), ResManager::Get()->IconName(ICON_APPPOWERING));
    parent->addPage(cfg_rules, i18n("Rules"),  ResManager::Get()->IconName(ICON_RULES));
}

void X10Widget::layoutControlWidgets()
{
    /// Remove all subitems
    while (m_layout->count())
        m_layout->removeAt(0);

    QSortFilterProxyModel* proxyModel = new QSortFilterProxyModel(this);
    proxyModel->setSourceModel(&m_nodes);
    proxyModel->sort(COL_COL, Qt::AscendingOrder);
    proxyModel->sort(COL_ROW, Qt::AscendingOrder);

    QList<ControlWidgetInfo*>::const_iterator ctrlListIt;
    ControlWidgetInfo* ctrlIt;

    QGraphicsLinearLayout* layout;
    bool firstRow = true;
    int m_lastRow = -1;

    for (int i = 0; i < proxyModel->rowCount(); i++)
    {
        int row = i;
        QModelIndex idx   = proxyModel->index(row, 0);

        int row2 = proxyModel->mapToSource(idx).row();

        ctrlIt = m_nodes.Nodes()->at(row2);//   *ctrlListIt;
        if (firstRow || ctrlIt->row != m_lastRow)
        {
            layout = new QGraphicsLinearLayout(Qt::Horizontal, m_layout);
            layout->setSpacing(0);
            m_layout->addItem(layout);
            m_lastRow  = ctrlIt->row;
            firstRow = false;
        }

        ControlWidget* ctrlWidget = ctrlIt->GetWidget();
        if (!ctrlWidget)
        {
            ctrlWidget = ctrlIt->CreateWidget(this);
        }
        ctrlIt->UpdateWidget();

        layout->addItem(ctrlWidget);
    }

    m_layout->invalidate();
    prepareGeometryChange();
    updateGeometry();
    update();
    activate();

    emit update(boundingRect());
}

void X10Widget::init()
{
    Logger::Log(INFO, "init");

    createMenu();

    setAspectRatioMode(Plasma::IgnoreAspectRatio);

    if (ResManager::Get()->Icon(ICON_ON).isNull()) {
        setFailedToLaunch(true, i18n("Icons not found"));
    }

    QGraphicsLinearLayout* layout = new QGraphicsLinearLayout(Qt::Vertical, this);
    layout->setSpacing(0);

    m_layout = new QGraphicsLinearLayout(Qt::Vertical, layout);
    layout->addItem(m_layout);

    m_engine = dataEngine("x10");

    if (m_engine && m_engine->isValid())
        m_engine->connectSource(SOURCE_PROPERTIES, this, 0);
    else
        Logger::Log(ERROR, "Invalid x10 Engine.");

    //  m_service = (m_engine && m_engine->isValid()) ? m_engine->serviceForSource("") : 0;

    // load all config, if not present
    // plasma desktop do not read, plasmoidviewer does !!
    readConfig();

    prepareGeometryChange();
    updateGeometry();
    update();
}

void X10Widget::createMenu()
{
#ifdef Q_WS_X11

    m_actionAllOff = new QAction(ResManager::Get()->Icon(ICON_OFF), i18n("All &Off"), this);
    m_actions.append(m_actionAllOff);
    connect(m_actionAllOff, SIGNAL(triggered(bool)), this , SLOT(AllOff()));

    m_actionAllOn = new QAction(ResManager::Get()->Icon(ICON_ON), i18n("&All On"), this);
    m_actions.append(m_actionAllOn);
    connect(m_actionAllOn, SIGNAL(triggered(bool)), this , SLOT(AllOn()));

//      m_actionAllOn->setEnabled(true);

#endif
}

QList<QAction*> X10Widget::contextualActions()
{
    return m_actions;
}

void X10Widget::configChanged()
{
    Logger::Log(DEBUG, "configChanged");
    readConfig();
}

void X10Widget::dataUpdated(const QString& name, const Plasma::DataEngine::Data& data)
{
    Logger::Log(DEBUG, "X10Widget: dataUpdated: name %s", qPrintable(name));

    if (name == SOURCE_PROPERTIES)
    {
        // STATUS
        if (m_serviceStatus != data.value(PROPERTY_STATUS).toString())
        {
            m_serviceStatus        = data.value(PROPERTY_STATUS).toString();
            m_serviceIsRunning     = (m_serviceStatus != "Stopped");
            emit(serviceStatusChanged(m_serviceStatus));
        }

        // SMART QUEUE
        m_configUseSmartQueue  = data.value(PROPERTY_SMARTQUEUE).toBool();

        // RULES
        QString newRules = data.value(PROPERTY_RULES).toString();
        if (newRules != Rule::serializedRules(m_rules))
        {
            Rule::deserializeRules(m_rules, newRules);
            emit RulesUpdated();
        }

        // NODES not used

        // CONTROLLER
        m_dbusControllerStr       = data.value(PROPERTY_CONTROLLER_STR).toString();
        m_dbusControllerConnected = data.value(PROPERTY_CONTROLLER_STATUS).toBool();
    }
}

// This is the command that links your applet to the .desktop file
K_EXPORT_PLASMA_APPLET(X10Widget, X10Widget)

#include "x10-widget.moc"

