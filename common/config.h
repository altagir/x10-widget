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

#ifndef CONFIG_H
#define CONFIG_H

// #include <stdio.h>
#include <fstream>
#include <sstream>

#include <QSettings>

// config params;
const char configLoc[]          = "/etc/x10/x10.conf";
const char addressLoc[]         = "/etc/x10/local_dbusdaemon_address.conf";

const char defaultRulesLoc[]    = "/var/spool/x10/rules.conf";
const char defaultDataConfLoc[] = "/var/spool/x10/data.conf";

class Config
{
public:

    /// Clients
    bool            USE_LOCAL_SYSTEM_BUS;
    QString         DBUS_ADDRESS;
    QString         CONTROLLER_HOST;
    uint            CONTROLLER_PORT;

    /// CONFIG
    unsigned int 	MAX_ERRORS; 		// MAX ERRORS before exiting
    bool 			RELOAD_ON_ERROR;	// Reload or Quit on Error

    QString			RULES_LOCATION;		// where to load / save rules
    QString         DATA_LOCATION;      // where to load / save data config

    /// DATA
    bool			SMART_QUEUE;		// with on, latest cmd overrides

    inline bool operator==(const Config& rhs) {
        return (USE_LOCAL_SYSTEM_BUS == rhs.USE_LOCAL_SYSTEM_BUS &&
                CONTROLLER_HOST == rhs.CONTROLLER_HOST &&
                CONTROLLER_PORT == rhs.CONTROLLER_PORT &&
                MAX_ERRORS == rhs.MAX_ERRORS &&
                RELOAD_ON_ERROR == rhs.RELOAD_ON_ERROR &&
                RULES_LOCATION == rhs.RULES_LOCATION &&
                DATA_LOCATION == rhs.DATA_LOCATION
               );
    }
    inline bool operator!=(const Config& rhs) {
        return !(*this == rhs);
    }

    Config() :
        m_settingsData(0)
    {
        Load();
    }

    void Load()
    {
        QSettings settingsEtc(configLoc, QSettings::IniFormat);

        /// Controller
        USE_LOCAL_SYSTEM_BUS = settingsEtc.value("Clients/USE_LOCAL_SYSTEM_BUS", true).toBool();
        CONTROLLER_HOST      = settingsEtc.value("Clients/CONTROLLER_HOST", "127.0.0.1").toString();
        CONTROLLER_PORT      = settingsEtc.value("Clients/CONTROLLER_PORT", 14500).toInt();
        DBUS_ADDRESS = QString("tcp:host=%1,bind=*,port=%2").arg(CONTROLLER_HOST).arg(CONTROLLER_PORT);

        /// Service
        MAX_ERRORS   	= settingsEtc.value("Service/MAX_ERRORS",      20).toInt();
        RELOAD_ON_ERROR	= settingsEtc.value("Service/RELOAD_ON_ERROR", true).toBool();
        RULES_LOCATION	= settingsEtc.value("Service/RULES_LOCATION",  defaultRulesLoc).toString();

        DATA_LOCATION   = settingsEtc.value("Service/DATA_LOCATION", defaultDataConfLoc).toString();

        ///////////////////settingsEtc./////////////////////////////////////////////////////
        /// Settings Data
        QSettings settingsData(DATA_LOCATION, QSettings::IniFormat);
        SMART_QUEUE		= settingsData.value("Data/SMART_QUEUE", "true").toBool();
    }

    void Save()
    {
        QSettings settingsData(DATA_LOCATION, QSettings::IniFormat);
        if (settingsData.isWritable())
        {
            settingsData.setValue("Data/SMART_QUEUE", SMART_QUEUE);
            settingsData.sync();
        }
    }

protected:
    QSettings*  m_settings;      // config
    QSettings*  m_settingsData;  // saveable

    bool readAddressFromConfig()
    {
        std::ifstream infile(addressLoc);

        std::string line;
        while (std::getline(infile, line))
        {
            size_t index = line.find("<listen>");
            if (index != std::string::npos)
            {
                index += strlen("<listen>");
                size_t index2 = line.find("</listen>");
                DBUS_ADDRESS = QString(line.substr(index, index2-index).c_str());
                return true;
            }
        }

        return false;
    }
};

#endif // CONFIG_H
