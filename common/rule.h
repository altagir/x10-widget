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

#ifndef RULE_H
#define RULE_H

#include <vector>

#include <common/command.h>

#define MAX_TIMET LONG_MAX  // max value for size_t

class Rule
{
public:
    enum DAYS
    {
        NONE      = 0,
        SUNDAY    = 1,  // english way
        MONDAY    = 2,
        TUESDAY   = 4,
        WEDNESDAY = 8,
        THURSDAY  = 16,
        FRIDAY    = 32,
        SATURDAY  = 64,
        WEEKDAYS  = MONDAY + TUESDAY + WEDNESDAY + THURSDAY + FRIDAY,
        WEEK      = WEEKDAYS + SATURDAY + SUNDAY
    };

    enum ACTION_TYPE
    {
        AT_ON,
        AT_OFF,
        AT_SET,
        AT_SEND
    };

    typedef std::vector<Rule>           List;
    typedef std::vector<Rule>::iterator ListIterator;

    Rule();
    bool operator==(const Rule& other);

    QString getDisplayText();

    time_t GetExecutionTime();
    std::vector<COMMAND> getCommands();

    bool isValid();
    static bool validateAction(const QString& action);
    static bool validateAddress(const QString& addresses);

    /// Serialization for transport
    QString serialize();
    bool deserialize(const QString& params);

    static QString serializedRules(Rule::List& list);
    static void    deserializeRules(Rule::List& list, const QString& rules);

public:
    bool	enabled;
    QString	address; 		// *, E, E*, E1, whatever man
    time_t	exec_time;		// @ XX:XX  -> nb of seconds in a day
    DAYS    period;
    ACTION_TYPE actionType;
    QString	action;

    uint    repeatTimes;
    time_t  repeatPeriodMn;

protected:
    time_t  m_nextExecTime;

    QString getPeriodText();
    QString getDisplayTime();
};

#endif // RULE_H
