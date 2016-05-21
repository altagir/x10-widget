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

#ifndef COMMAND_H
#define COMMAND_H

#include <stdio.h>
#include <ctime>

#include <QMetaType>
#include <QString>
#include <QVariant>

typedef char Channel;   // A-P
typedef int  Unit;      // 1-16  0:all

const int MAX_VALUE = 7;

enum COMMAND_TYPE
{
    CMD_SET      = '=',
    CMD_ON       = '+',
    CMD_OFF      = '-',
    CMD_DIM      = 's',
    CMD_BRIGHT   = 'b',
    CMD_INIT     = 'i',
    CMD_MVUP     = 'u',
    CMD_MVDOWN   = 'd',
    CMD_MVLEFT   = 'l',
    CMD_MVRIGHT  = 'r'
};

struct COMMAND
{
public:
    COMMAND() :
        channel('A'), unit(1), command(CMD_OFF), value(-2)
        , created(time(0)), notified(false) {}

    COMMAND(Channel _channel, Unit _unit, COMMAND_TYPE _cmd, int _value) :
        channel(_channel), unit(_unit), command(_cmd), value(_value)
        , created(time(0)), notified(false) {}

    COMMAND(const COMMAND& copy)
    {
        channel     = copy.channel;
        unit        = copy.unit;
        command     = copy.command;
        value       = copy.value;
        created     = copy.created;
        notified    = copy.notified;
    }

    virtual ~COMMAND() {}

    bool operator == (const COMMAND& cmd)
    {
        return ((channel == cmd.channel) &&
                (unit == cmd.unit) &&
                (command == cmd.command) &&
                (value == cmd.value) &&
                (created == cmd.created)) ;
    }

    Channel      channel;
    Unit         unit;
    COMMAND_TYPE command;
    int          value;	// target value for SET
    time_t       created;
    bool		 notified;

    QString      commandStr() const
    {
        return QString("%1%2%3")
               .arg(address())
               .arg((char)command).arg(command == CMD_SET ? QString::number(value) : "");
    }

    QString      address() const {
        QString addressStr(channel);
        if (channel != '*' && unit != 0)
            addressStr += unit?QString::number(unit):"*";
        return addressStr;
    }
};

Q_DECLARE_METATYPE(COMMAND)

struct X10Unit
{
    X10Unit() : channel('a'), unit(1), value(-1), valueBeforeOff(-1)   {}

    X10Unit(Channel _channel, Unit _unit, int _value)
        : channel(_channel), unit(_unit), value(_value)
        , valueBeforeOff(-1)
    {
        _address[0] = 0;
    }

    inline bool IsOn()      {
        return (value != -1);
    }

    char* address()
    {
        sprintf(_address, "%c%d", (char)channel, (int)unit);
        return _address;
    }

    Channel channel;
    Unit    unit;
    int     value;
    int     valueBeforeOff;

    char    _address[4];
};


class X10
{
public:
    static bool sourceToAddress(const QString& source, char& channel, int& unit)
    {
        if (source == "*")
        {
            channel = '*';
            unit    = 0;
            return true;
        }

        if (source.length() < 2 || source.length() > 3)
            return false;

        channel = source[0].toUpper().toLatin1();

        bool ok = true;
        if (source.mid(1) == "*")
            unit = 0;
        else
            unit = source.mid(1).toInt(&ok, 10);

        return (ok && (unit >= 0) && (unit < 17) &&
                (channel >= 'A') && (channel <= 'P') );
    }

    static bool isValidAddress(const QString& source)
    {
        char channel;
        int unit;
        return sourceToAddress(source, channel, unit);
    }

    static bool isValidChannel(QVariant channel)
    {
        return (channel.convert(QVariant::String) &&
                channel.toString().length() == 1 &&
                (channel.toString().at(0).toLatin1() == '*' ||
                 ((channel.toString().toUpper().at(0).toLatin1() >= 'A') &&
                  (channel.toString().toUpper().at(0).toLatin1() <= 'P')) ) );
    }

    static bool isValidUnit(QVariant unit)
    {
        bool ok = false;
        return (unit.convert(QVariant::Int) &&
                (unit.toInt(&ok) >= 0) && ok &&
                (unit.toInt() < 17));
    }

    static bool isValidValue(QVariant value)
    {
        bool ok = false;
        return (value.convert(QVariant::Int) &&
                (value.toInt(&ok) >= -1) && ok &&
                (value.toInt() <= MAX_VALUE));
    }

    static bool isValidBoolean(QVariant value, bool* out)
    {
        if (!value.convert(QVariant::String))
            return false;
        QString valuestr = value.toString();
        if (valuestr == "true" || valuestr == "1")
            *out = true;
        else if (valuestr == "false" || valuestr == "0")
            *out = false;
        else
            return false;
        return true;
    }

    static bool isValidCommand(QVariant command)
    {
        if (!command.convert(QVariant::String) ||
                command.toString().length() != 1)
            return false;

        char cmd = command.toString().at(0).toLatin1();
        return (cmd == '=' || cmd == '+' || cmd == '-' ||
                cmd == 's' || cmd == 'b' ||
                cmd == 'i' || cmd == 'u' || cmd == 'd' || cmd == 'l' ||
                cmd == 'r');
    }
};

#endif // COMMAND_H
