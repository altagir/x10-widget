#ifndef X10CODES_H
#define X10CODES_H

const int X10_DIM_STEPS = 7;

/* Bounds of X10 house codes */
#define HOUSE_MIN 'a'
#define HOUSE_MAX 'p'

/* Bounds of X10 unit codes */
#define UNIT_MIN 1
#define UNIT_MAX 16

/// House codes
enum HouseCode
{
    HOUSE_A = 0x60, HOUSE_B = 0x70, HOUSE_C = 0x40, HOUSE_D = 0x50,
    HOUSE_E = 0x80, HOUSE_F = 0x90, HOUSE_G = 0xA0, HOUSE_H = 0xB0,
    HOUSE_I = 0xE0, HOUSE_J = 0xF0, HOUSE_K = 0xC0, HOUSE_L = 0xD0,
    HOUSE_M = 0x00, HOUSE_N = 0x10, HOUSE_O = 0x20, HOUSE_P = 0x30
};

/// Unit codes
enum UnitCode
{
    UNIT_1  = 0x000, UNIT_2  = 0x010, UNIT_3  = 0x008, UNIT_4  = 0x018,
    UNIT_5  = 0x040, UNIT_6  = 0x050, UNIT_7  = 0x048, UNIT_8  = 0x058,
    UNIT_9  = 0x400, UNIT_10 = 0x410, UNIT_11 = 0x408, UNIT_12 = 0x418,
    UNIT_13 = 0x440, UNIT_14 = 0x450, UNIT_15 = 0x448, UNIT_16 = 0x458
};

/// Command codes
enum CmdCode
{
    /* Standard 5-byte commands: */
    X10CMD_ON      = 0x00,  /* Turn on unit */
    X10CMD_OFF     = 0x20,  /* Turn off unit */
    X10CMD_DIM     = 0x98,  /* Dim lamp */
    X10CMD_BRIGHT  = 0x88,  /* Brighten lamp */
    /* Pan'n'Tilt 4-byte commands: */
    X10CMD_UP      = 0x762,
    X10CMD_RIGHT   = 0x661,
    X10CMD_DOWN    = 0x863,
    X10CMD_LEFT    = 0x560,
    /* Error flag */
    X10CMD_INVALID = 0xFF
};


char cmd_code_to_char(enum CmdCode code);
const char* cmd_code_to_str(enum CmdCode code);
enum CmdCode parse_cmd_code(char c);

char house_code_to_char(enum HouseCode code);
unsigned char house_code_to_cam_code(enum HouseCode code);

int unit_code_to_int(enum UnitCode code);
enum UnitCode int_to_unit_code(int unit);


/// X10 home automation command structure
struct x10_ha_command
{
    x10_ha_command() {}

    x10_ha_command(CmdCode _cmd, HouseCode _house, UnitCode _unit)
        : cmd(_cmd), house(_house), unit(_unit) {}

    void print(char* buffer);

    enum CmdCode   cmd;     // Command code
    enum HouseCode house;   // House code
    enum UnitCode  unit;    // Unit code, 1-16 (not used for dim/bright or camera commands)
};


struct x10_ha_command* new_x10_ha_command(enum CmdCode cmd, char house, int unit);
void del_x10_ha_command(struct x10_ha_command* haCmd);


#endif // X10CODES_H
