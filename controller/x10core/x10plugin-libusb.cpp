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

#include "x10plugin-libusb.h"
#include "x10codes.h"
#include "logger.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>


X10Plugin_libusb::X10Plugin_libusb()
{
    for (int i = 0; i < MAX_CONTROLLER; i++)
        m_handles[i] = 0;
}

bool X10Plugin_libusb::Open()
{
    // Support 1 for now
    if (!m_handles[0])
    {
        m_handles[0] = Init();
    }

    Status = (m_handles[0] != 0);
    return Status;
}


void X10Plugin_libusb::Close()
{
    for (int i = 0; i < MAX_CONTROLLER; i++)
    {
        if (m_handles[i] != 0)
        {
            Close(m_handles[i]);
            m_handles[i] = 0;
        }
    }
}

int X10Plugin_libusb::recv(x10_ha_command* cmd)
{
    if (m_handles[0] == 0)
        return -1;

    int res;

    /// Read input.
//  do
//  {
    res = x10_recv(m_handles[0], cmd);

//      if (res > 0)
//          return res;
//          Logger::Log(INFO, "X10 command received on house %c unit %d, cmd:%s", house_code_to_char(cmd->house), unit_code_to_int(cmd->unit), cmd_code_to_str(cmd->cmd));
//  }
//  while (res > 0);

    return res;
}

int X10Plugin_libusb::send(x10_ha_command* cmd)
{
    if (m_handles[0] != 0)
        return  x10_transmit_cmd(m_handles[0], cmd);
    else
        return -1;
}

//////////////////////////////////////////////////////////

#define CM19AVendor     0x0bc7
#define CM19AProdID     2

#define CM15AVendor     0x0bc7
#define CM15AProdID     1


void X10Plugin_libusb::Close(usb_dev_handle* portnum)
{
    LibUSButils::closeUsbDev(portnum);
}

usb_dev_handle* X10Plugin_libusb::Init()
{
    usb_dev_handle* portnum;
    int open_status = -1;

    portnum = LibUSButils::GetFirstDevice(CM19AVendor, CM19AProdID);
    if (!portnum)
    {
        Logger::Log(ERROR, "Error getting X10 controller device: %s", strerror(errno));
        return portnum;
    }

    /// Set which configuration (it only has 1)
    open_status = usb_set_configuration(portnum, 1);

    if (open_status < 0)
    {
        Logger::Log(ERROR, "Error claiming the interface: %s", strerror(errno));
        LibUSButils::closeUsbDev(portnum);
        portnum = 0;
        return portnum;
    }

    // Claim the first (and only) interface
    open_status = usb_claim_interface(portnum, 0);

    if (open_status < 0)
    {
        Logger::Log(ERROR, "Error claiming the interface: %s", strerror(errno));
        LibUSButils::closeUsbDev(portnum);
        portnum = 0;
        return portnum;
    }

    LibUSButils::ahpInit(portnum); // Init CM19A so it regonizes CR14A remote

    Logger::ClearLastError();
    return portnum;
}


/// /**********************/
/// /*** X10 OPERATIONS ***/
/// /**********************/

#define IS_CAM_CODE(code) ((code) & ~0xFF)

/** Normal command length */
#define NORM_X10CMD_LEN 5
/** Pan'n'Tilt command length */
#define CAM_X10CMD_LEN 4
/** Larger of the two lengths, used for allocating buffers */
#define MAX_X10CMD_LEN NORM_X10CMD_LEN

/** Prefix for all normal commands */
#define NORM_X10CMD_PFX 0x20
/** Prefix for all Pan'n'Tilt commands */
#define CAM_X10CMD_PFX 0x14

#define ACK_LEN 1
#define ACK 0xFF


int X10Plugin_libusb::x10_transmit_cmd(usb_dev_handle* portnum, struct x10_ha_command* cmd)
{
    int writeRes, readRes;
    char cmdBuf[MAX_X10CMD_LEN];
    int cmdLen;

    memset(cmdBuf, 0, MAX_X10CMD_LEN);

    if (IS_CAM_CODE(cmd->cmd))
    {
        cmdLen = CAM_X10CMD_LEN;

        cmdBuf[0] = CAM_X10CMD_PFX;
        cmdBuf[1] = (cmd->cmd >> 8) | house_code_to_cam_code(cmd->house);
        cmdBuf[2] = cmd->cmd & 0xFF;
        cmdBuf[3] = cmd->house;
    }
    else
    {
        cmdLen = NORM_X10CMD_LEN;

        cmdBuf[0] = NORM_X10CMD_PFX;
        cmdBuf[1] = (cmd->unit >> 8) | cmd->house;
        cmdBuf[2] = ~cmdBuf[1];
        cmdBuf[3] = (cmd->unit & 0xFF) | cmd->cmd;
        cmdBuf[4] = ~cmdBuf[3];
    }

//  Logger::Log( DEBUG, "Transmitting %d byte command: %02X %02X %02X %02X %02X", cmdLen
//  , cmdBuf[0] & 0xff, cmdBuf[1] & 0xff, cmdBuf[2] & 0xff, cmdBuf[3] & 0xff, cmdBuf[4] & 0xff
//  );

    writeRes = LibUSButils::write_intr_data(portnum, cmdBuf, cmdLen);
    if (writeRes != cmdLen)
    {
        Logger::Log(ERROR, "%s: Error occurred writing command", __FUNCTION__);
        return -1;
    }

    char readbuf[8 + 1];

    /* Receive checksum from USB transceiver: */
    readRes = LibUSButils::read_intr_data(portnum, readbuf, ACK_LEN);
    if (readRes < 0)
    {
        Logger::Log(ERROR, "Error occurred reading checksum");
        return readRes;
    }
    else if ((unsigned char)readbuf[0] != ACK)
    {
        Logger::Log(ERROR, "Wrong ack received: %02X", readbuf[0]);
    }

//  Logger::Log( VERBOSE, "Read back interrupt data");
    Logger::ClearLastError();

    return 0;
}

int X10Plugin_libusb::x10_recv(usb_dev_handle* portnum, struct x10_ha_command* cmd)
{
    int i, red, red2, cmdLen = 0;
    unsigned char* buf;

    static const int X10CMD_REP = 5; /* commands are repeated up to six times by remote */
    int reading = 0;

    char readbuf[8 + 1];
    char addbuf[8 + 1];

    /* read command prefix: */
    red = LibUSButils::read_intr_data(portnum, readbuf, 1);

//  if (red != -110)
//      Logger::Log(DEBUG, "first read  %d", red);

    if (red < 0)
    {
        return red;
    }
    else if (red == 1)
    {
        switch (readbuf[0])
        {
        case CAM_X10CMD_PFX:
            cmdLen = CAM_X10CMD_LEN;
            Logger::Log(DEBUG, "INCOMING CAM");// "Read camera command prefix");
            break;
        case NORM_X10CMD_PFX:
            cmdLen = NORM_X10CMD_LEN;
            Logger::Log(DEBUG, "INCOMING CMD");//  "Read on/off/brighten/dim command prefix");
            break;
        default:
            Logger::Log(ERROR, "%s: Read invalid command prefix: %02X", __FUNCTION__, readbuf[0]);
            return 0;
        }
    }

    /* read remainder of command, plus ack */
    red = LibUSButils::read_intr_data(portnum, readbuf, cmdLen - 1);

    if (red < 0)
    {
        Logger::Log(ERROR, "body read first result: %d", red);
        return red;
    }

    for (i = 0; (i < X10CMD_REP - 1) && (reading < 2); i++)
    {
//      Logger::Log(DEBUG, "loop: %d", i);
        red2 = LibUSButils::read_intr_data(portnum, addbuf, cmdLen);

//      if (red2 == -110)
//      {
//          Logger::Log(ERROR, "body read loop  result: ERROR: %d, cmdLen: %d", red2, cmdLen);
//          break;
//      }
        /*      else */if (red2 < 0)
        {
            /* Error occurred reading data */
            if (red2 != -110)
                Logger::Log(ERROR, "body read loop  result: ERROR: %d, cmdLen: %d", red2, cmdLen);

            if (i < 3)
                return red2;
            break;
        }

        if (red2 == 0)
        {
            --i;
            if (reading == 1)
            {
                reading  = 2;
                if (red2 != red ||  memcmp(readbuf, addbuf, red) == 0)
                    break; // new cmd
            }
        }
        else
        {
            reading = 1;
        }
    }

    buf = (unsigned char*) readbuf;

//  Logger::Log(CAT, "Read data: cmdLen %d red %d", cmdLen, red);
//  for (i = 0; i < cmdLen; i++)
//  {
//      Logger::Log(CAT, "buf[%d] = %02X", i, readbuf[i]);
//  }
//  Logger::Log(CAT, "\n");

    if (cmdLen == CAM_X10CMD_LEN)
    {
        cmd->cmd = (enum CmdCode)(((buf[1] & 0xF) << 8) | buf[2]);
        cmd->house = (enum HouseCode)buf[3];
        cmd->unit = UNIT_1;
    }
    else
    {
        cmd->unit = (enum UnitCode)(((buf[1] & 0xF) << 8) | (buf[3] & ~X10CMD_OFF));
        cmd->house = (enum HouseCode)(buf[1] & 0xF0);
        cmd->cmd = (enum CmdCode)(buf[3] & X10CMD_OFF);
    }

    Logger::ClearLastError();
    return 1;
}
