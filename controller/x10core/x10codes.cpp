#include "x10codes.h"

#include <stdio.h>
#include <stdlib.h>

#include "logger.h"

/// /**********************/
/// /*** X10 OPERATIONS ***/
/// /**********************/

enum HouseCode HouseCodeLut[(HOUSE_MAX - HOUSE_MIN) + 1] =
    {
        HOUSE_A, HOUSE_B, HOUSE_C, HOUSE_D, HOUSE_E, HOUSE_F, HOUSE_G, HOUSE_H,
        HOUSE_I, HOUSE_J, HOUSE_K, HOUSE_L, HOUSE_M, HOUSE_N, HOUSE_O, HOUSE_P
    };


enum UnitCode UnitCodeLut[(UNIT_MAX - UNIT_MIN) + 1] =
    {
        UNIT_1, UNIT_2,  UNIT_3,  UNIT_4,  UNIT_5,  UNIT_6,  UNIT_7,  UNIT_8,
        UNIT_9, UNIT_10, UNIT_11, UNIT_12, UNIT_13, UNIT_14, UNIT_15, UNIT_16
    };

#define TO_LOWER(ch) (((ch) < 'a')? (((ch) - 'A') + 'a') : (ch))
#define OUT_OF_BOUNDS(val, low, high) (((val) < (low)) || ((high) < (val)))



/**
 * Construct HA command from parameters
 */
struct x10_ha_command* new_x10_ha_command(enum CmdCode cmd, char house, int unit)
{
    struct x10_ha_command* res;
    house = TO_LOWER(house);
    if (OUT_OF_BOUNDS(house, HOUSE_MIN, HOUSE_MAX))
    {
        Logger::Log(ERROR, "%s: House code out of bounds: %c", __FUNCTION__, house);
        return 0;
    }

    if (OUT_OF_BOUNDS(unit, UNIT_MIN, UNIT_MAX))
    {
        Logger::Log(ERROR, "%s: Unit code out of bounds: %d", __FUNCTION__, unit);
        return 0;
    }

    res = (struct x10_ha_command*)malloc(sizeof(struct x10_ha_command));
    if (res == 0)
    {
        Logger::Log(ERROR, "%s: malloc of HA command failed!", __FUNCTION__);
        return NULL;
    }
    res->cmd = cmd;
    res->house = HouseCodeLut[house - HOUSE_MIN];
    res->unit = UnitCodeLut[unit - UNIT_MIN];

//  Logger::Log(DEBUG, "Created HA command: %s %c %d", cmd_code_to_str(cmd), house, unit);
    Logger::ClearLastError();

    return res;
}

/**
 * Destroy command structure
 */
void del_x10_ha_command(struct x10_ha_command* haCmd)
{
    free(haCmd);
}


/// //////////////////////////////////////////////////////
/// //////////////////////////////////////////////////////
/// //////////////////////////////////////////////////////


char cmd_code_to_char(enum CmdCode code)
{
    switch (code)
    {
    case X10CMD_ON:
            return '+';
    case X10CMD_OFF:
        return '-';
    case X10CMD_DIM:
        return 's';
    case X10CMD_BRIGHT:
        return 'b';
    case X10CMD_UP:
        return 'u';
    case X10CMD_RIGHT:
        return 'r';
    case X10CMD_DOWN:
        return 'd';
    case X10CMD_LEFT:
        return 'l';
    default:
        return '?';
    }
}

const char* cmd_code_to_str(enum CmdCode code)
{
    switch (code)
    {
    case X10CMD_ON:
            return "on";
    case X10CMD_OFF:
        return "off";
    case X10CMD_DIM:
        return "dim";
    case X10CMD_BRIGHT:
        return "brighten";
    case X10CMD_UP:
        return "up";
    case X10CMD_RIGHT:
        return "right";
    case X10CMD_DOWN:
        return "down";
    case X10CMD_LEFT:
        return "left";
    default:
        return "invalid";
    }
}


char house_code_to_char(enum HouseCode code)
{
    switch (code)
    {
    case HOUSE_A:
            return 'a';
    case HOUSE_B:
        return 'b';
    case HOUSE_C:
        return 'c';
    case HOUSE_D:
        return 'd';
    case HOUSE_E:
        return 'e';
    case HOUSE_F:
        return 'f';
    case HOUSE_G:
        return 'g';
    case HOUSE_H:
        return 'h';
    case HOUSE_I:
        return 'i';
    case HOUSE_J:
        return 'j';
    case HOUSE_K:
        return 'k';
    case HOUSE_L:
        return 'l';
    case HOUSE_M:
        return 'm';
    case HOUSE_N:
        return 'n';
    case HOUSE_O:
        return 'o';
    case HOUSE_P:
        return 'p';
    default:
        return '?';
    }
}


/**
 * Translate house code to octet necessary for 2nd byte in Pan'n'Tilt commands
 */
unsigned char house_code_to_cam_code(enum HouseCode code)
{
    switch (code)
    {
    case HOUSE_A:
            return 0x90;
    case HOUSE_B:
        return 0xA0;
    case HOUSE_C:
        return 0x70;
    case HOUSE_D:
        return 0x80;
    case HOUSE_E:
        return 0xB0;
    case HOUSE_F:
        return 0xC0;
    case HOUSE_G:
        return 0xD0;
    case HOUSE_H:
        return 0xE0;
    case HOUSE_I:
        return 0x10;
    case HOUSE_J:
        return 0x20;
    case HOUSE_K:
        return 0xF0;
    case HOUSE_L:
        return 0x00;
    case HOUSE_M:
        return 0x30;
    case HOUSE_N:
        return 0x40;
    case HOUSE_O:
        return 0x50;
    case HOUSE_P:
        return 0x60;
    default:
        return 0x90; /* default to A if bad housecode */
    }
}


int unit_code_to_int(enum UnitCode code)
{
    int unit = ((code << 1) & 0x8);
    unit |= ((code >> 4) & 0x4);
    unit |= ((code >> 2) & 0x2);
    unit |= ((code >> 4) & 0x1);

    return ++unit;
}

UnitCode int_to_unit_code(int unit)
{
    return UnitCodeLut[unit - UNIT_MIN];
}


enum CmdCode parse_cmd_code(char c)
{
    switch (c)
    {
    case '+':
            return X10CMD_ON;
    case '-':
        return X10CMD_OFF;
    case 'u':
        return X10CMD_UP;
    case 'd':
        return X10CMD_DOWN;
    case 'l':
        return X10CMD_LEFT;
    case 'r':
        return X10CMD_RIGHT;
    case 'b':
        return X10CMD_BRIGHT;
    case 's':
        return X10CMD_DIM;
    default:
        Logger::Log(ERROR, "%s: Invalid command code: %c", __FUNCTION__, c);
        return X10CMD_INVALID;
    }
}

void x10_ha_command::print(char* buffer)
{
    buffer[0] = 0;
    sprintf(buffer, "%s%c%d %s", buffer,
            toupper(house_code_to_char(house)),
            unit_code_to_int(unit),
            cmd_code_to_str(cmd));
}
