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

#include <QStringList>

#include <common/logger.h>
#include <common/command.h>

#include "rule.h"

#include <math.h>
#include <time.h>

#define SECS_IN_DAY  86400 // 3600x24

Rule::Rule() :
    enabled(true),
    address("*"),
    period(WEEK),
    actionType(AT_ON),
    action(""),
    repeatTimes(0),
    repeatPeriodMn(60),
    m_nextExecTime(MAX_TIMET)
{
    time_t now;
    time(&now);
    const struct tm *tmNow = std::localtime(&now);
    exec_time = tmNow->tm_hour*3600 + tmNow->tm_min*60;
}

bool Rule::operator==(const Rule& other)
{
    return ((address 		== other.address) &&
            (exec_time 		== other.exec_time) &&
            (period    		== other.period) &&
            (actionType		== other.actionType) &&
            (action 		== other.action) &&
            (repeatTimes	== other.repeatTimes) &&
            (repeatPeriodMn	== other.repeatPeriodMn) );
}

QString Rule::getPeriodText()
{
    QString days = "";
    days += (period & MONDAY)?"M":"-";
    days += (period & TUESDAY)?"T":"-";
    days += (period & WEDNESDAY)?"W":"-";
    days += (period & THURSDAY)?"T":"-";
    days += (period & FRIDAY)?"F":"-";
    days += (period & SATURDAY)?"S":"-";
    days += (period & SUNDAY)?"S":"-";
    return days;
}

QString Rule::getDisplayTime()
{
    char bufferTime [80];
    strftime (bufferTime, 80,"%H:%M", gmtime(&exec_time));
    return QString(bufferTime);
}

QString Rule::getDisplayText()
{
    QString actionStr;
    switch (actionType)
    {
    case AT_ON:
        actionStr = "Turn on " + address;
        break;
    case AT_OFF:
        actionStr = "Turn off " + address;
        break;
    case AT_SEND:
        actionStr = "Send to " + address + " : " + action;
        break;
    case AT_SET:
        actionStr = "Set " + address + " to " + action;
        break;
    }

    QString repeatPattern;
    if (repeatTimes > 0)
        repeatPattern = QString(" [%1x @%2mn]").arg(repeatTimes).arg(repeatPeriodMn);

    return QString("%1|%2, %3: %4%5")
           .arg(enabled?"o":"x")
           .arg(getPeriodText())
           .arg(qPrintable(getDisplayTime()))
           .arg(actionStr)
           .arg(qPrintable(repeatPattern));
}

//////////////////////////////////////////////////////////////////////

// get next execution time. in real time_t
time_t Rule::GetExecutionTime()
{
    if (!enabled || period == NONE)
        return MAX_TIMET;   // no days selected

    time_t now;
    time(&now);
    const struct tm *tmNow = std::localtime(&now);
    // for some weird reason, tmNow->tm_wday was incremented at end of loop?? like for no fucking reason ? TODO investigate
    const uint wday = tmNow->tm_wday;
    const int nbSecsToday = tmNow->tm_hour*3600 + tmNow->tm_min*60 + tmNow->tm_sec;

    time_t finalExecTime = now + exec_time - nbSecsToday;

    // due today?
    if (period & (uint)pow(2, wday) )
    {
        // past ?
        if (exec_time < nbSecsToday)
        {
            // in repeat zone?
            uint nbRepeatsRequired = repeatPeriodMn?1+((nbSecsToday - exec_time) / (60*repeatPeriodMn)):(repeatTimes+1);
            if (nbRepeatsRequired <= repeatTimes) {
                return (finalExecTime + nbRepeatsRequired*repeatPeriodMn*60);
            }
            // else due another day and include in search same weekday than today
        }
        else // today future
            return finalExecTime;
    }

    // due in inNbDays?
    uint inNbDays=1; // ain't today we know that.
    for (; inNbDays<7; inNbDays++)
    {
        // looking for a valid day for that rule
        if (period & (uint)pow(2, (wday+inNbDays)%7 ))
            break;
    }

    return finalExecTime + inNbDays*SECS_IN_DAY;
}

std::vector<COMMAND> Rule::getCommands()
{
    std::vector<COMMAND> list;

    QStringList addressesSplit = address.split(" ", QString::SkipEmptyParts);

    Channel channel;
    Unit unit;
    bool ok;

    // split all destination
    QStringList addresses;
    for (int i=0; i<addressesSplit.count(); i++)
    {
        // unsure ... * is all known added sources in QM...
        // should remove next text and ensure creation of
        // individual source... * E* do not create source, other should.
        if (addressesSplit[i] == "*")
        {
            addresses.clear();
            addresses.push_back("*");
            break;
        }

        if (X10::isValidAddress(addressesSplit[i]))
            addresses.push_back(addressesSplit[i]);
        else
            Logger::Log(ERROR, "Invalid address in rule %s", qPrintable(addressesSplit[i]) );
    } // end build address list

    // for each address
    for (int ad=0; ad<addresses.count(); ad++)
    {
        if (!X10::sourceToAddress(addresses[ad], channel, unit)) {
            Logger::Log(ERROR, "Invalid address in addresses %s", qPrintable(addresses[ad]) );
            continue;
        }

        int value = -1;

        if (actionType == AT_ON) {
            list.push_back( COMMAND(channel, unit, CMD_ON, -1 ) );
        }
        else if (actionType == AT_OFF) {
            list.push_back( COMMAND(channel, unit, CMD_OFF, -1 ) );
        }
        else if (actionType == AT_SEND) {
            // for each command
            QStringList actions = action.split(" ", QString::SkipEmptyParts);

            for (int ac=0; ac<actions.count(); ac++) {
                if (actions[ac][0] != '=' &&  X10::isValidCommand(actions[ac])) {
                    COMMAND_TYPE type = (COMMAND_TYPE) actions[ac][0].toLatin1();
                    list.push_back( COMMAND(channel, unit, type, -1 ) );
                }
            }
        }
        else if (actionType == AT_SET) {
            if (X10::isValidValue(action)) {
                value = action.toInt(&ok, 10);
                list.push_back( COMMAND(channel, unit, CMD_SET, value ) );
            }
        }

    } // for each address

    return list;
}

//////////////////////////////////////////////////////////////////////

bool Rule::isValid()
{
    return (validateAddress(address)/* && validateAction(action)*/);
}

bool Rule::validateAction(const QString& action)
{
    QStringList fields = action.split(" ", QString::SkipEmptyParts);
    if (!fields.count()) {
// 		Logger::Log(ERROR, "validateAction failed, empty %s", qPrintable(action));
        return true;
    }

    bool ok = true;
    for (int i=0; i<fields.count(); i++)
    {
        ok &= (fields[i] != "=") &&
              X10::isValidCommand(fields[i]);
    }

    if (!ok)
        Logger::Log(ERROR, "validateAction failed %s", qPrintable(action));

    return ok;
}

bool Rule::validateAddress(const QString& addresses)
{
    QStringList fields = addresses.split(" ", QString::SkipEmptyParts);
    if (!fields.count()) {
        Logger::Log(ERROR, "validateAddress failed, empty %s", qPrintable(addresses));
        return false;
    }

    bool ok = true;
    for (int i=0; i<fields.count(); i++)
    {
        ok &= X10::isValidAddress(fields[i]);
    }

    if (!ok)
        Logger::Log(ERROR, "validateAddress failed %s", qPrintable(addresses));

    return ok;
}

//////////////////////////////////////////////////////////////////////
// Serialization for transport

QString Rule::serialize()
{
    QString params = enabled?"1":"0";
    params += ',' + address.toUpper() + ',';
    params += QString::number(exec_time) + ',';
    params += QString::number(period) + ',';
    params += QString::number(actionType) + ',';
    params += action + ',';
    params += QString::number(repeatTimes) + ',';
    params += QString::number(repeatPeriodMn);

    return params;
}

bool Rule::deserialize(const QString& params)
{
    QStringList fields = params.split(",");

    if (fields.count() != 8)
    {
        Logger::Log(ERROR, "Incorrect number of params %d in %s", fields.count(), qPrintable(params));
        return false;
    }

    enabled      = (fields[0]=="1");
    address      = fields[1].toUpper();
    exec_time    = fields[2].toInt();
    period       = (DAYS) fields[3].toInt();
    actionType   = (ACTION_TYPE) fields[4].toInt();
    action       = fields[5];
    repeatTimes  = fields[6].toUInt();
    repeatPeriodMn = fields[7].toInt();

    if (!isValid())
    {
        Logger::Log(ERROR, "Rule deserialized, but not valid, disabling (%s)", qPrintable(params) );
        enabled = false;
    }

    return true;
}

QString Rule::serializedRules(Rule::List& list)
{
    QString output = "";
    for (Rule::ListIterator it=list.begin(); it!=list.end(); it++)
    {
        output += it->serialize() + "|";
    }
    return output;
}

void Rule::deserializeRules(Rule::List& list, const QString& rules)
{
    list.clear();

    QStringList rulesList = rules.split("|", QString::SkipEmptyParts);

    for (int i=0; i<rulesList.count(); i++)
    {
        Rule rule;
        if (rule.deserialize(rulesList[i]))
        {
//             if (rule.enabled)
//             {
//                 Rule::ListIterator it = std::find(list.begin(), list.end(), rule);
//                 if (it != list.end())
//                 {
//                     Logger::Log(ERROR, "duplicated rule found, disabling since no point duplicating them %s", qPrintable(rulesList[i]) );
//                     rule.enabled = false;
//                 }
//             }

            list.push_back(rule);
        }
        else
            Logger::Log(ERROR, "Couldn't deserialize rule %s", qPrintable(rulesList[i]) );
    }
}
