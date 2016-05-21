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

#include <iostream>
#include <string.h>
#include <stdlib.h>
#include <dirent.h>

#include <common/interfacehandler.h>
#include <common/command.h>
#include <common/operations.h>
#include <common/rule.h>

#include <common/x10config.h>   // for current version

CaCyberdineX10Interface* interface;

void printUsage()
{
    std::cout << "x10 (" << X10_FULLVERSION << ") - send commands to X10 controller through DBUS" << std::endl;

    std::cout << "   help | version" << std::endl;
    std::cout << "   status | start | stop | restart | reload" << std::endl;
    std::cout << "   address               -> get value" << std::endl;
    std::cout << "   address [+|-|s|b]     -> on|off|soften|brighten" << std::endl;
    std::cout << "   address {-1..7}       -> set to value -1(off), 0..7" << std::endl;
    std::cout << "   all on/+ | off/- | allon | alloff" << std::endl;
    std::cout << "   properties | p | prop -> list all properties" << std::endl;
    std::cout << "   propertyName          -> get value for property" << std::endl;
    std::cout << "   propertyName value    -> set value if allowed (e.g. SmartQueue)" << std::endl;
    std::cout << std::endl;
    std::cout << " sample   :  x10 e2; x10 e2 + ; x10 e2 3;" << std::endl;
}

void printVersion()
{
    std::cout << X10_FULLVERSION << std::endl;
}

QString getProperty(const QString& property)
{
    QDBusPendingReply<QString> bret = interface->GetProperty(property);
    bret.waitForFinished();

    if (bret.isValid())
    {
        return bret.value();
    }
    return "NOREPLY";
}

bool setProperty(const QString& property, const QString& value)
{
    QDBusPendingReply<QString> bret = interface->SetProperty(property, value);
    bret.waitForFinished();

    if (bret.isValid())
    {
        return true;
    }
    return false;
}

QString getFormattedRules()
{
    QString rules = getProperty(PROPERTY_RULES);
    Rule::List list;

    Rule::deserializeRules(list, rules);

    QString formattedRules = "";
    for (uint i=0; i<list.size(); i++)
    {
        formattedRules += "\n     " + list[i].getDisplayText();
    }

    return formattedRules;
}

pid_t proc_find(const char* name)
{
    DIR* dir;
    struct dirent* ent;
    char buf[512];

    long  pid;
    char pname[100] = {0,};
    char state;
    FILE *fp=NULL;

    if (!(dir = opendir("/proc"))) {
        perror("can't open /proc");
        return -1;
    }

    while((ent = readdir(dir)) != NULL) {
        long lpid = atol(ent->d_name);
        if(lpid < 0)
            continue;
        snprintf(buf, sizeof(buf), "/proc/%ld/stat", lpid);
        fp = fopen(buf, "r");

        if (fp) {
            if ( (fscanf(fp, "%ld (%[^)]) %c", &pid, pname, &state)) != 3 ) {
                printf("fscanf failed \n");
                fclose(fp);
                closedir(dir);
                return -1;
            }
            if (!strcmp(pname, name)) {
                fclose(fp);
                closedir(dir);
                return (pid_t)lpid;
            }
            fclose(fp);
        }
    }


    closedir(dir);
    return -1;
}

///////////////////////////////////////////////////////////////////////////////

int main(int argc, char** argv)
{
    QDBusPendingReply<void> bret;
    char channel;
    int  unit;

    if (argc > 1 && strcasecmp(argv[1], "quit") == 0) {
        pid_t runningXs = proc_find("x10_service");
        if (runningXs == -1) {
            printf("Not running\n");
        } else {
            printf("Quitting instance %d\n", runningXs);
            IDBus::X10()->Quit();
        }
        return 0;
    }

    // this will open x10_service
    interface = IDBus::X10();

    QString status = getProperty(PROPERTY_STATUS);
    if (status == "NOREPLY")
    {
        printf("Couldn't connect to specified DBUS : %s\n", qPrintable(IDBus::Get()->DBusLocation()));
        return 1;
    }

    // PROPERTIES /////////////////////////////////////////////////////////////
    if (argc == 1 || !strcasecmp(argv[1], PROPERTY_STATUS))
    {
        printf("DBUS       : %s\n", qPrintable(IDBus::Get()->DBusLocation()));
        printf("Status     : %s\n", qPrintable(getProperty(PROPERTY_STATUS)));
        printf("Nodes      : %s\n", qPrintable(getProperty(PROPERTY_NODES)));
        printf("Rules      : %s\n", qPrintable(getFormattedRules()));
    }
    else if (!strcasecmp(argv[1], "properties") ||
             !strcasecmp(argv[1], "p") ||
             !strcasecmp(argv[1], "prop"))
    {
        printf("DBUS       : %s\n", qPrintable(IDBus::Get()->DBusLocation()));
        printf("Status     : %s\n", qPrintable(getProperty(PROPERTY_STATUS)));
        printf("Nodes      : %s\n", qPrintable(getProperty(PROPERTY_NODES)));
        printf("SmartQueue : %s\n", qPrintable(getProperty(PROPERTY_SMARTQUEUE)));
        printf("Rules      : %s\n", qPrintable(getFormattedRules()));
    }
    else if (!strcasecmp(argv[1], PROPERTY_NODES))
    {
        printf("Nodes      : %s\n", qPrintable(getProperty(PROPERTY_NODES)));
    }
    else if (!strcasecmp(argv[1], PROPERTY_RULES))
    {
        printf("Rules      : %s\n", qPrintable(getFormattedRules()) );
    }
    else if (!strcasecmp(argv[1], PROPERTY_SMARTQUEUE))
    {
        bool value;
        if (argc == 2)
            printf("SmartQueue : %s\n", qPrintable(getProperty(PROPERTY_SMARTQUEUE)));
        else if (argc == 3 && X10::isValidBoolean(argv[2], &value))
            setProperty(PROPERTY_SMARTQUEUE, QString::number(value));
        else
            std::cout << " Usage: x10 " << PROPERTY_SMARTQUEUE << " {true|false|1|0}" << std::endl;
    }

    // HELP
    else if (!strcasecmp(argv[1], "help") ||
             !strcasecmp(argv[1], "--help") ||
             !strcasecmp(argv[1], "-h"))
    {
        printUsage();
    }
    else if (!strcasecmp(argv[1], "version"))
    {
        printVersion();
    }
    else if (X10::sourceToAddress(argv[1], channel, unit))
    {
        if (argc > 2)
        {
            // x10 e2 ...
            if (!strcasecmp(argv[2], "off"))  // x10 e2 off
            {
                bret = interface->Send(QString(channel).toUpper(), unit, "-");
            }
            else if (!strcasecmp(argv[2], "on"))  // x10 e2 on
            {
                bret = interface->Send(QString(channel).toUpper(), unit, "+");
            }
            else if (X10::isValidValue(argv[2]))
            {
                // x10 e2 5   SET =
                bret = interface->Set(QString(channel).toUpper(), unit, atoi(argv[2]));
            }
            else if (X10::isValidCommand(argv[2]) && argv[2][0] != '=')
            {
                // x10 e2 +
                bret = interface->Send(QString(channel).toUpper(), unit, argv[2]);

                if (bret.isError())
                    std::cout << " Send Error : "
                              << qPrintable(bret.error().message())
                              << std::endl;
            }
            else
            {
                // invalid
                std::cout << "   address [-1..7|+|-|b|s]" << std::endl;
            }
        }
        else // argc == 2
        {
            // x10 e2 -> get value
            QDBusPendingReply<int> bret = interface->GetValue(QString(argv[1]).toUpper());
            bret.waitForFinished();

            if (bret.isValid())
            {
                int value = bret.value();

                std::cout << "Current value for " << argv[1] << " : ";
                if (value == -1)
                    std::cout << "Off";
                else
                    std::cout << value;

                std::cout << std::endl;
            }

        }
    }

    // ALL ON/OFF
    else if (strcasecmp(argv[1], "all") == 0 || strcmp(argv[1], "a") == 0) // * doesn't work?
    {
        if (argc > 2)
        {
            if (strcasecmp(argv[2], "on") == 0 || strcmp(argv[2], "+") == 0)
                interface->AllOn();
            else if (strcasecmp(argv[2], "off") == 0 || strcmp(argv[2], "-") == 0)
                interface->AllOff();
            else
                std::cout << "   all on|off" << std::endl;
        }
    }
    else if (strcasecmp(argv[1], "allon") == 0)
        interface->AllOn();
    else if (strcasecmp(argv[1], "alloff") == 0)
        interface->AllOff();

    // SERVICE
    else if (strcasecmp(argv[1], "start") == 0)
        interface->Start();
    else if (strcasecmp(argv[1], "restart") == 0)
        interface->Restart();
    else if (strcasecmp(argv[1], "stop") == 0)
        interface->Stop();
    else if (strcasecmp(argv[1], "reload") == 0)
        interface->Reload();

    else
        printUsage();

    return 0;
}
