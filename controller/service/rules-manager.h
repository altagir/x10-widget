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

#ifndef RULE_MANAGER_H
#define RULE_MANAGER_H

#include <QTimer>

#include <vector>

#include <common/rule.h>

class RuleManager : public QObject
{
    Q_OBJECT

public:
    /*explicit*/
    RuleManager();
    ~RuleManager();

//     void AddRule(Rule& rule);
//     void RemoveRule(const Rule& rule);
    Rule::ListIterator Find(const Rule& rule);
    bool Exists(const Rule& rule);

    void updateNextExecution();

    Rule::List	m_listRules;
    bool Load(const QString& rulesConfigFile);
    bool Save(const QString& rulesConfigFile);

    void Deserialize(const QString& rulesConfig);

    QString GetRules();

public slots:
    void onEndTimerRuleNeedExec();

signals:
    // Signal émis quand la commande est a ete execute
    void RuleExecute(Rule rule);

protected:
    void    Clear();
    QString SecondsToString(time_t period);

private:
    QTimer          	m_timerRuleNeedExec;
    time_t              m_lowestTime;
    std::vector<Rule*>	m_nextRuleSet;
};

#endif // RULE_MANAGER_H
