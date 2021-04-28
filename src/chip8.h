
/// Chip 8 Emulator class
/// The bulk of the emulation happens in this class
///
/// Chip 8 memory map
/// 0x000 - 0x1FF - Chip 8 interpreter (contains font set in emu)
/// 0x050 - 0x0A0 - Used fpr the built in 4x5 pixel font set (0-F)
/// 0x200 - 0xFFF - Program ROM and work RAM
///
#ifndef CHIP8_H
#define CHIP8_H

class chip8
{
public:
    chip8();
    ~chip8();

    void init();
    void cycle();
    bool load(const char *path);
    bool drawFlag();
    void setDrawFlag(const bool flag);

private:
    /// The Chip 8 has 35 opcodes which are all two bytes long.
    /// To store the current opcode, an unsigned short has length of two bytes fitting our needs
    unsigned short _opcode;

    /// The Chip 8 has 4KB of memory
    unsigned char _memory[4096];

    /// The Chip 8 has 15 8-bit general purpose registers named V0, V1...VE
    /// The 16th register is used for the 'carry flag'
    /// Unsigned chars can be used as they are 8 bits long
    unsigned char _v[16];

    /// Index register I which can have a value from 0x000 to 0xFFF
    unsigned short _indexRegister;

    /// Program Counter which can have a value from 0x000 to 0xFFF
    unsigned short _programCounter;

    /// The graphics of the Chip 8 are black and white
    /// and the screen has a total of 2048 pixels (64 x 32 resolution)
    /// This is implemented as an array that holds the pixel state of either 0 or 1
    unsigned char _gfx[64 * 32];

    /// Timer registers that count at 60hz

    /// When set above zero, they will count down to zero
    unsigned char _delayTimer;

    /// The system's buzzer sounds whenever the sound timer reaches zero
    unsigned char _soundTimer;

    unsigned short _stack[16];
    unsigned short _stackPointer;

    /// Chip 8 has a HEX based keypad (0x0 - 0xF)
    unsigned char _key[16];

    bool _drawFlag;
};

#endif