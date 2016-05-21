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

#include "rules-manager.h"

#include <common/logger.h>

#include <algorithm>
#include <QFile>
#include <QTextStream>

using namespace std;

#define PERIOD_MS 1800000  // 30mn * 60s * 1000

RuleManager::RuleManager() :
    QObject(),
    m_lowestTime(MAX_TIMET)
{
    m_timerRuleNeedExec.setSingleShot(true);
    connect(&m_timerRuleNeedExec, SIGNAL(timeout()), this, SLOT(onEndTimerRuleNeedExec()) );
}

RuleManager::~RuleManager()
{
}

/*
void RuleManager::AddRule(Rule& rule)
{
    if (!Exists(rule))
    {
        if (!rule.isValid())
        {
            Logger::Log(ERROR, "AddRule: Rule loaded, but not valid, disabling (%s)", qPrintable(rule.getDisplayText()) );
            rule.enabled = false;
        }
        m_listRules.push_back(rule);
        updateNextExecution();
    }
    else
        Logger::Log(ERROR, "AddRule: duplicated rule, skipping");
}

void RuleManager::RemoveRule(const Rule& rule)
{
    Rule::ListIterator it = std::find(m_listRules.begin(), m_listRules.end(), rule);
    if(it != m_listRules.end())
    {
        m_listRules.erase(it);
        updateNextExecution();
    }
    else
        Logger::Log(ERROR, "RemoveRule: didn't find rule");
}*/

Rule::ListIterator RuleManager::Find(const Rule& rule)
{
    Rule::ListIterator it = std::find(m_listRules.begin(), m_listRules.end(), rule);
    return it;
}

bool RuleManager::Exists(const Rule& rule)
{
    return (Find(rule) != m_listRules.end());
}

void RuleManager::Clear()
{
    m_listRules.clear();
    m_nextRuleSet.clear();
}

bool RuleManager::Load(const QString& rulesConfigFile)
{
    Clear();

    QFile file(rulesConfigFile);
    if(!file.open(QIODevice::ReadOnly)) {
        Logger::Log(ERROR, "RuleManager:Load: couldn't read file %s", qPrintable(rulesConfigFile));
        return false;
    }

    QTextStream in(&file);

    while(!in.atEnd()) {
        QString ruleStr = in.readLine();
        Rule rule;
        if (rule.deserialize(ruleStr))
        {
            m_listRules.push_back(rule);
        }
        else
            Logger::Log(ERROR, "Couldn't deserialize rule %s", qPrintable(ruleStr) );
    }

    file.close();

    updateNextExecution();

    return true;
}

QString RuleManager::GetRules()
{
    return Rule::serializedRules(m_listRules);
}

void RuleManager::Deserialize(const QString& rulesConfig)
{
    Clear();
    Rule::deserializeRules(m_listRules, rulesConfig);
    updateNextExecution();
}

bool RuleManager::Save(const QString& rulesConfigFile)
{
    QFile file(rulesConfigFile);
    if(!file.open(QIODevice::WriteOnly)) {
        Logger::Log(ERROR, "RuleManager:Save: couldn't write file %s", qPrintable(rulesConfigFile));
        return false;
    }

    QTextStream out(&file);

    for (Rule::ListIterator it = m_listRules.begin(); it!= m_listRules.end(); it++ )
    {
        QString rule = it->serialize();
        out << rule << endl;
    }

    file.close();

    return true;
}

QString RuleManager::SecondsToString(time_t period)
{
    int secs, min, hours, days;
    time_t period2 = period;

    days = period2 / (3600*24);
    period2 -= days * (3600*24);
    hours = period2 / 3600;
    period2 -= hours * 3600;
    min = period2 / 60;
    secs = period2 - min*60;

    QString periodStr;
    if (days)
        periodStr += QString::number(days) + "d";
    if (days || hours)
        periodStr += QString::number(hours) + "h";
    if (days || hours || min)
        periodStr += QString::number(min) + "mn";
    periodStr += QString::number(secs) + "s";
    return periodStr;
}


void RuleManager::onEndTimerRuleNeedExec()
{
    time_t now;
    time(&now);

    const int nbSecsNow = m_lowestTime - now;
    if (nbSecsNow > 1) // close enough
    {
        m_timerRuleNeedExec.start(min(nbSecsNow*1000, PERIOD_MS));
        return;
    }

    Logger::Log(DEBUG, "onEndTimerRuleNeedExec rule need execute");

    for (uint i=0; i<m_nextRuleSet.size(); i++)
    {
        if (m_nextRuleSet[i]->enabled) // should all be at this point
            emit RuleExecute(*m_nextRuleSet[i]);
    }

    updateNextExecution();
}

void RuleManager::updateNextExecution()
{
    m_nextRuleSet.clear();
    m_timerRuleNeedExec.stop();

    m_lowestTime = MAX_TIMET;
    time_t ruleTime;

    time_t now;
    time(&now);

    for(uint i=0; i<m_listRules.size(); i++)
    {
        if (!m_listRules[i].enabled) continue;
        ruleTime = m_listRules[i].GetExecutionTime();
        if (ruleTime > m_lowestTime) continue;

        if (ruleTime < m_lowestTime) {
            m_nextRuleSet.clear();
            m_lowestTime = ruleTime;
        }
        m_nextRuleSet.push_back(&m_listRules[i]);
    }

    if (m_lowestTime != MAX_TIMET)
    {
        int nbSecsNow = m_lowestTime - now;
        if (nbSecsNow < 0)
            Logger::Log(ERROR, "PERIOD Ms < 0, shouldn't happen %d %d", m_lowestTime, now);
        else if (nbSecsNow > 0)
        {
            QString rules;
            for (uint i = 0; i<m_nextRuleSet.size(); i++)
                rules += m_nextRuleSet[i]->getDisplayText() + " , ";

            Logger::Log(INFO, "%d Rules Set for Execution in %s (%s) ",
                        m_nextRuleSet.size(), qPrintable(SecondsToString(nbSecsNow)), qPrintable(rules));

            // do a short period to realign evetual clock drift.
            m_timerRuleNeedExec.start( min(nbSecsNow*1000, PERIOD_MS) );
        } else // ==
            Logger::Log(DEBUG, "rule execution is now in updateNextExecution, skipping");
    } else
        Logger::Log(INFO, "No rules set to execute");
}
