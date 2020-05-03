//
// header file for enums
//
// GUI commands
//  most of these are supported in globals.h with enums
//
public enum CONSOLECOMMANDS
{
    PING = 0x10,    			        // ping back to show slave is alive
    ACK = 0x11,					        // this receives and sends a pattern to the GUI
    AUTOSAVE = 0x12,                    // save after each write, or wait for save command 0 = no, 1 = save, 2 = save now

    SLIDERCH1 = 0x20,
    SLIDERCH2 = 0x21,
    SLIDERCH3 = 0x22,
    SLIDERCH4 = 0x23,
    SLIDERCH5 = 0x24,
    SLIDERCH6 = 0x25,

    TEMP = 0x2A,
    VOLTS = 0x2B,
    READDRVVERSION = 0x2C,
    SETI2CADDRESS = 0x2D,

    READCHUBVERSION = 0x30,
    SENDTEMPERATURE = 0x31,

    SETAUDITCODE = 0xAA,                    // 16-bit audit value for readback by host

    SET_UPGRADE_FIRMWARE = 0xAB,

    RESET = 0xBB,
};

public enum SWITCHINDEX
{
    PING,
    ACK,
    AUTOSAVE,
    TEMP,
    VOLTS,
    READDRVVERSION,
    SETI2CADDRESS,
    READCHUBVERSION,
    SENDTEMPERATURE,
    SETAUDITCODE,
    RESET,

    SET_UPGRADE_FIRMWARE,
};

// packet format for a single byte value
enum PACKETA
{
    COMMAND,
    LENGTH,
    AUDIT,
    READWRITE,
    ACK,
    DATA,
    CHECKSUM
};
enum PACKETB
{
    COMMAND,
    LENGTH,
    AUDIT,
    READWRITE,
    ACK,
    UNIVERSE,
    DATA,
    CHECKSUM
};
enum PACKETC
{
    COMMAND,
    LENGTH,
    AUDIT,
    READWRITE,
    ACK,
    UNIVERSE,
    DATAH,
    DATAL,
    CHECKSUM
};
enum PACKETD
{
    COMMAND,
    LENGTH,
    AUDIT,
    READWRITE,
    ACK,
    DATAH,
    DATAL,
    CHECKSUM
};

// packet format for 4 data values, no universe (RDM version)
enum PACKETE
{
    E_COMMAND,
    E_LENGTH,
    E_AUDIT,
    E_READWRITE,
    E_ACK,
    E_DATA1,
    E_DATA2,
    E_DATA3,
    E_DATA4,
    E_CHECKSUM
};
// packet format for strings, no universe (RDM version)
enum PACKETF
{
    F_COMMAND,
    F_LENGTH,
    F_AUDIT,
    F_READWRITE,
    F_ACK,
    F_DATA,
    F_CHECKSUM      // this location can be variable
};
enum READWRITE
{
    WRITE = 0,
    READ = 1
};
enum ACKSTATUS
{
    ACKOFF,
    ACKON
};
