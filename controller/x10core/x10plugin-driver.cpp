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

#include "x10plugin-driver.h"

#include <errno.h>
#include <stdio.h>  // sprintf
#include <stdlib.h> // system
#include <unistd.h> // usleep

#include "logger.h"

#include <fstream>
using namespace std;


X10Plugin_driver::X10Plugin_driver()
{
    for (int i = 0; i < MAX_CONTROLLER; i++)
        m_handles[i] = "";
}

bool X10Plugin_driver::HasController()
{
    bool found = false;
    char devicePath[13];
    devicePath[0] = 0;

    for (int i = 0; i < 10; i++)
    {
        sprintf(devicePath, "/dev/cm19a%d", i);

        int rval = access(devicePath, F_OK);

        if (rval)
        {
            // Check file existence. (errno == ENOENT) -> file don't exist
            /* Check file access. */
            if (errno == EACCES)
                Logger::Log(ERROR, "Interface %s is not accessible", devicePath);

            continue;
        } // file exist

        /* Check read access. */
        rval = access(devicePath, R_OK);
        if (rval != 0)
        {
            Logger::Log(ERROR, "Interface %s is not readable (access denied)", devicePath);
            continue;
        }

        /* Check write access. */
        rval = access(devicePath, W_OK);
        if (rval == 0)
        {
            found = true;
            Logger::Log(INFO, "Interface FOUND %s - OK", devicePath);
        }
        else if (errno == EACCES)
            Logger::Log(ERROR, "Interface %s is not writable (access denied)", devicePath);
        else if (errno == EROFS)
            Logger::Log(ERROR, "Interface %s is not writable (read-only filesystem)", devicePath);
    }

    return found;
}

bool X10Plugin_driver::Open()
{
    Status = false;
    char deviceName[13];
    deviceName[0] = 0;

    int j = 0;
    // will crash after <101 ?????
    for (int i = 0; i < 10; i++)
    {
        sprintf(deviceName, "/dev/cm19a%d", i);
        if (ifstream(deviceName))
        {
            m_handles[j++] = deviceName;
            Logger::Log(INFO, "Interface found %s", deviceName);
            Status = true;
        }
    }

    for (; j < MAX_CONTROLLER; j++)
        m_handles[j] = "";
    return Status;
}

void X10Plugin_driver::Close()
{
    for (int j = 0; j < MAX_CONTROLLER; j++)
        m_handles[j] = "";
}

int X10Plugin_driver::recv(struct x10_ha_command* /*cmd*/)
{
    Logger::Log(DEBUG, "bytes reading");

    return 0;

    if (m_handles[0] == "")
        return -1;

    long nbytes;
    FILE* fd = fopen(m_handles[0].c_str(), "r");

    char* buffer;

    if (fd != 0)
    {
        // obtain file size:
        fseek(fd , 0 , SEEK_END);
        long lSize = ftell(fd);
        rewind(fd);

        if (lSize == -1)
            return 0;

        // allocate memory to contain the whole file:
        buffer = (char*) malloc(sizeof(char) * lSize);
        if (buffer == NULL) {
            Logger::Log(ERROR, "Memory error %ld\n", lSize);
            return -1;
        }

        // copy the file into the buffer:
        nbytes = fread(buffer, 1, lSize, fd);
        if (nbytes != lSize) {
            fputs("Reading error\n", stderr);
            return -1;
        }

//      nbytes = sizeof(buf);
//      bytes_read = fread(buf, 1, nbytes, fd);
        Logger::Log(DEBUG, "bytes read %ld\n", nbytes);
        fclose(fd);
        free(buffer);
    }

    return 0;
}

int X10Plugin_driver::send(struct x10_ha_command* command)
{
    if (!command || m_handles[0] == "")
        return -1;

    char fullCommand[512];
    fullCommand[0] = 0;

    sprintf(fullCommand, "echo \"%c%c%d\" > %s",
            cmd_code_to_char(command->cmd),
            house_code_to_char(command->house),
            unit_code_to_int(command->unit),
            m_handles[0].c_str()
           );

    Logger::Log(INFO, "%s", fullCommand);

    return system(fullCommand);
}
