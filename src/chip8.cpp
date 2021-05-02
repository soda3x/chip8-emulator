#include "chip8.h"
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

unsigned char chip8_fontset[80] =
    {
        0xF0, 0x90, 0x90, 0x90, 0xF0, //0
        0x20, 0x60, 0x20, 0x20, 0x70, //1
        0xF0, 0x10, 0xF0, 0x80, 0xF0, //2
        0xF0, 0x10, 0xF0, 0x10, 0xF0, //3
        0x90, 0x90, 0xF0, 0x10, 0x10, //4
        0xF0, 0x80, 0xF0, 0x10, 0xF0, //5
        0xF0, 0x80, 0xF0, 0x90, 0xF0, //6
        0xF0, 0x10, 0x20, 0x40, 0x40, //7
        0xF0, 0x90, 0xF0, 0x90, 0xF0, //8
        0xF0, 0x90, 0xF0, 0x10, 0xF0, //9
        0xF0, 0x90, 0xF0, 0x90, 0x90, //A
        0xE0, 0x90, 0xE0, 0x90, 0xE0, //B
        0xF0, 0x80, 0x80, 0x80, 0xF0, //C
        0xE0, 0x90, 0x90, 0x90, 0xE0, //D
        0xF0, 0x80, 0xF0, 0x80, 0xF0, //E
        0xF0, 0x80, 0xF0, 0x80, 0x80  //F
};

const unsigned short PROGRAM_START_ADDRESS = 512;

/// Initialize registers and memory
void chip8::init()
{
    // Reset opcode
    _opcode = 0;

    // Clear memory
    for (int i = 0; i < sizeof(_memory) / sizeof(unsigned char); ++i)
    {
        _memory[i] = 0;
    }

    // Clear registers
    for (int i = 0; i < sizeof(_v) / sizeof(unsigned char); ++i)
    {
        _v[i] = 0;
    }

    // Clear index register
    _indexRegister = 0;

    // Set program counter to program ROM 0x200
    // refer to chip8.h header comment for memory map
    _programCounter = 0x200;

    // Clear stack
    for (int i = 0; i < sizeof(_stack) / sizeof(unsigned short); ++i)
    {
        _stack[i] = 0;
    }

    // Reset stack pointer
    _stackPointer = 0;

    // Reset GFX
    for (int i = 0; i < sizeof(_gfx) / sizeof(unsigned char); ++i)
    {
        _gfx[i] = 0;
    }

    // Reset keys
    for (int i = 0; i < sizeof(_key) / sizeof(unsigned char); ++i)
    {
        _key[i] = 0;
    }

    // Reset timers
    _delayTimer = 0;
    _soundTimer = 0;

    // Load font set into memory
    for (int i = 0; i < sizeof(chip8_fontset) / sizeof(unsigned char); ++i)
    {
        _memory[i] = chip8_fontset[i];
    }

    // Clear screen
    setDrawFlag(true);

    // one or more opcodes require RNG
    srand(time(NULL));
}

void chip8::cycle()
{
    // Fetch opcode from the memory at location specified by program counter
    // Opcode is stored in two successive bytes and will need to be merged
    // First half of opcode is shifted 8 bits left, adding 8 zeroes
    // Bitwise OR operation is used to merge the two halves
    _opcode = _memory[_programCounter] << 8 | _memory[_programCounter + 1];

    // Decode Opcode and check opcode table to see what it means
    switch (_opcode & 0xF000)
    {
        // Multiple 0x0 opcodes so switch again and compare last four bits
    case 0x0000:
        switch (_opcode & 0x000F)
        {
            // 00E0 Clears the screen
        case 0x0000:
            // TODO: Implement 00E0
            break;
            // 00EE Returns from a subroutine
        case 0x000E:
            // TODO: Implement 00EE
            break;
        default:
            printf("Bad opcode: 0x%X\n", _opcode);
        }
        break;

        // 1NNN (0x1NNN): Jumps to address NNN
    case 0x1000:
        // Set program counter to address NNN
        _programCounter = _opcode & 0x0FFF;
        break;

        // 2NNN (0x2NNN): Calls subroutine at NNN
    case 0x2000:
        // Store current address in stack
        _stack[_stackPointer] = _programCounter;
        // Increment Stack Pointer
        ++_stackPointer;
        // Set the program counter to the address of NNN
        _programCounter = _opcode & 0x0FFF;
        break;

        // 3XNN (0x3XNN): Skips the next instruction if VX equals NN (Usually the next instruction is a jump to skip a code block)
    case 0x3000:
        if (_v[(_opcode & 0x0F00) >> 8] == _opcode & 0x00FF)
        {
            // Skip the next instruction
            _programCounter += 4;
        }
        else
        {
            // Move to next instruction
            _programCounter += 2;
        }
        break;

        // 4XNN (0x4XNN): Skips the next instruction if VX does not equal NN (Usually the next instruction is a jump to skip a code block)
    case 0x4000:
        if (_v[(_opcode & 0x0F00) >> 8] != _opcode & 0x00FF)
        {
            // Skip next instruction
            _programCounter += 4;
        }
        else
        {
            // Move to next instruction
            _programCounter += 2;
        }
        break;

        // 5XY0 (0x5XY0): Skips the next instruction if VX equals VY (Usually the next instruction is a jump to skip a code block)
    case 0x5000:
        if (_v[(_opcode & 0x0F00) >> 8] == _v[(_opcode & 0x00F0) >> 4])
        {
            // Skip next instruction
            _programCounter += 4;
        }
        else
        {
            // Move to next instruction
            _programCounter += 2;
        }
        break;

        // 6XNN (0x6XNN): Sets VX to NN
    case 0x6000:
        _v[(_opcode & 0x0F00) >> 8] = _opcode & 0x00FF;

        // Move to next instruction
        _programCounter += 2;
        break;

        // 7XNN (0x7XNN): Adds NN to VX (Carry flag is not changed)
    case 0x7000:
        _v[(_opcode & 0x0F00) >> 8] += _opcode & 0x00FF;

        // Move to next instruction
        _programCounter += 2;
        break;

        // Multiple 0x8 opcodes so switch again and compare last four bits
    case 0x8000:
        switch (_opcode & 0x000F)
        {
        // 8XY0 (0x8XY0): Sets VX to the value of VY
        case 0x0000:
            _v[(_opcode & 0x0F00) >> 8] = _v[(_opcode & 0x00F0) >> 4];

            // Move to next instruction
            _programCounter += 2;
            break;

        // 8XY1 (0x8XY1): Sets VX to VX or VY (Bitwise OR operation)
        case 0x0001:
            _v[(_opcode & 0x0F00) >> 8] |= _v[(_opcode & 0x00F0) >> 4];

            // Move to next instruction
            _programCounter += 2;
            break;

        // 8XY2 (0x8XY2): Sets VX to VX and VY (Bitwise AND operation)
        case 0x0002:
            _v[(_opcode & 0x0F00) >> 8] &= _v[(_opcode & 0x00F0) >> 4];

            // Move to next instruction
            _programCounter += 2;
            break;

        // 8XY3 (0x8XY3): Sets VX to VX xor VY.
        case 0x0003:
            _v[(_opcode & 0x0F00) >> 8] ^= _v[(_opcode & 0x00F0) >> 4];

            // Move to next instruction
            _programCounter += 2;
            break;

        // 8XY4 (0x8XY4): Adds VY to VX. VF is set to 1 when there's a carry, and to 0 when there is not
        case 0x0004:
            // Check if VY is larger than VX
            if (_v[(_opcode & 0x00F0) >> 4] > _v[(_opcode & 0x0F00) >> 8])
            {
                // VY is larger so set VF to 1 because there is a carry
                _v[0xF] = 1;
            }
            else
            {
                // VY is not larger so set VF to 0 as there is no carry
                _v[0xF] = 0;
            }

            // Add VY to VX
            _v[(_opcode & 0x0F00) >> 8] += _v[(_opcode & 0x00F0) >> 4];

            // Move to next instruction
            _programCounter += 2;
            break;

        // 8XY5 (0x8XY5): VY is subtracted from VX. VF is set to 0 when there's a borrow, and 1 when there is not
        case 0x0005:
            // Check if VY is larger than VX
            if (_v[(_opcode & 0x00F0) >> 4] > _v[(_opcode & 0x0F00) >> 8])
            {
                // VY is larger so set VF to 0 because there is a borrow
                _v[0xF] = 0;
            }
            else
            {
                // VY is not larger so set VF to 1 as there is no borrow
                _v[0xF] = 1;
            }

            // Subtract VY from VX
            _v[(_opcode & 0x0F00) >> 8] -= _v[(_opcode & 0x00F0) >> 4];

            // Move to next instruction
            _programCounter += 2;
            break;

        // 8XY6 (0x8XY6): Stores the least significant bit of VX in VF and then shifts VX to the right by 1
        case 0x0006:
            // Store least significant bit (...& 0x1) in VF
            _v[0xF] = _v[(_opcode & 0x0F00) >> 8] & 0x1;

            // Shift VX to the right by 1
            _v[(_opcode & 0x0F00) >> 8] >>= 1;

            // Move to next instruction
            _programCounter += 2;
            break;

        // 8XY7 (0x8XY7): Sets VX to VY minus VX. VF is set to 0 when there's a borrow, and 1 when there is not
        case 0x0007:
            // Check if VX is larger than VY
            if (_v[(_opcode & 0x0F00) >> 8] > _v[(_opcode & 0x00F0) >> 4])
            {
                // VX is larger so set VF to 0 because there is a borrow
                _v[0xF] = 0;
            }
            else
            {
                // VX is not larger so set VF to 1 because there is no borrow
                _v[0xF] = 1;
            }

            // VX = VY - VX
            _v[(_opcode & 0x0F00) >> 8] = _v[(_opcode & 0x00F0) >> 4] - _v[(_opcode & 0x0F00) >> 8];

            // Move to next instruction
            _programCounter += 2;
            break;

        // 8XYE (0x8XYE): Stores the most significant bit of VX in VF and then shifts VX to the left by 1
        case 0x000E:
            // Store most significant bit (...& 0x0) in VF
            _v[0xF] = _v[(_opcode & 0x0F00) >> 8] >> 7;

            // Shift VX to the left by 1
            _v[(_opcode & 0x0F00) >> 8] <<= 1;

            // Move to next instruction
            _programCounter += 2;
            break;

        default:
            printf("Bad opcode: 0x%X\n", _opcode);
        }
        break;

        // 9XY0 (0x9XY0): Skips the next instruction if VX does not equal VY (Usually the next instruction is a jump to skip a code block)
    case 0x9000:
        if (_v[(_opcode & 0x0F00) >> 8] != _v[(_opcode & 0x00F0) >> 8])
        {
            // Skip next instruction
            _programCounter += 4;
        }
        else
        {
            // Move to next instruction
            _programCounter += 2;
        }
        break;

        // ANNN (0xANNN): Sets index register to the address NNN
    case 0xA000:
        _indexRegister = _opcode & 0x0FFF;

        // Move to next instruction
        _programCounter += 2;
        break;

        // BNNN (0xBNNN): Jumps to the address NNN plus V0
    case 0xB000:
        _programCounter = (_opcode & 0x0FFF) + _v[0];
        break;

        // CXNN (0xCXNN): Sets VX to the result of a bitwise and operation on a random number (Typically: 0 to 255) and NN
    case 0xC000:
        // "rand() & 0xFF" gives a random number between 0 and 255 (0xFF)
        _v[(_opcode & 0x0F00) >> 8] = (_opcode & 0x00FF) & (rand() & 0xFF);

        // Move to next instruction
        _programCounter += 2;
        break;

        // DXYN (0xDXYN): Draws a sprite at coordinate (VX, VY) that has a width of 8 pixels and a height of N+1 pixels.
        // Each row of 8 pixels is read as bit-coded starting from memory location I; I value does not change after the execution of this instruction.
        // As described above, VF is set to 1 if any screen pixels are flipped from set to unset when the sprite is drawn, and to 0 if that does not happen
    case 0xD000:
        // TODO: Implement DXYN
        break;

        // Multiple 0xE opcodes so switch again and compare last eight bits
    case 0xE000:
        switch (_opcode & 0x00FF)
        {
        // EX9E (0xEX9E): Skips the next instruction if the key stored in VX is pressed (Usually the next instruction is a jump to skip a code block)
        case 0x009E:
            // TODO: Implement EX9E
            break;

        // EXA1 (0xEXA1): Skips the next instruction if the key stored in VX is not pressed (Usually the next instruction is a jump to skip a code block)
        case 0x00A1:
            // TODO: Implement EXA1
            break;
        default:
            printf("Bad opcode: 0x%X\n", _opcode);
        }
        break;

        // Multiple 0xF opcodes so switch again and compare last eight bits
    case 0xF000:
        switch (_opcode & 0x00FF)
        {
            // FX07 (0xFX07): Sets VX to the value of the delay timer
        case 0x0007:
            _v[(_opcode & 0x0F00) >> 8] = _delayTimer;

            // Move to next instruction
            _programCounter += 2;
            break;

            // FX0A (0xFX0A): A key press is awaited, and then stored in VX (Blocking Operation. All instruction halted until next key event)
        case 0x000A:
            // TODO: Implement FX0A
            break;

            // FX15 (0xFX15): Sets the delay timer to VX
        case 0x0015:
            _delayTimer = _v[(_opcode & 0x0F00) >> 8];

            // Move to next instruction
            _programCounter += 2;
            break;

            // FX18 (0xFX18): Sets the sound timer to VX
        case 0x0018:
            _soundTimer = _v[(_opcode & 0x0F00) >> 8];

            // Move to next instruction
            _programCounter += 2;
            break;

            // FX1E (0xFX1E): Adds VX to I. VF is not affected
        case 0x001E:
            // VF is set to 1 if range overflows (index register + VX > 0xFFF)
            if (_indexRegister + _v[(_opcode & 0x0F00) >> 8] > 0xFFF)
            {
                _v[0xF] = 1;
            }
            else
            {
                _v[0xF] = 0;
            }
            _indexRegister += _v[(_opcode & 0x0F00) >> 8];

            // Move to next instruction
            _programCounter += 2;
            break;

            // FX29 (0xFX29): Sets I to the location of the sprite for the character in VX. Characters 0-F (in hexadecimal) are represented by a 4x5 font
        case 0x0029:
            // TODO: Implement FX29
            break;

            // FX33 (0xFX33): Stores the binary-coded decimal representation of VX, with the most significant of three digits at the address in I,
            // the middle digit at I plus 1, and the least significant digit at I plus 2
            // (In other words, take the decimal representation of VX, place the hundreds digit in memory at location in I, the tens digit at location I+1, and the ones digit at location I+2)
        case 0x0033:
            // TODO: Implement FX33
            break;

            // FX55 (0xFX55): Stores V0 to VX (including VX) in memory starting at address I. The offset from I is increased by 1 for each value written, but I itself is left unmodified
        case 0x0055:
            // TODO: Implement FX55
            break;
            // FX65 (0xFX65): Fills V0 to VX (including VX) with values from memory starting at address I. The offset from I is increased by 1 for each value written, but I itself is left unmodified
        case 0x0065:
            // TODO: Implement FX65
            break;
        default:
            printf("Bad opcode: 0x%X\n", _opcode);
        }
        break;

    default:
        printf("Bad opcode: 0x%X\n", _opcode);
    }

    // Execute Opcode

    // Update Timers
    if (_delayTimer > 0)
    {
        --_delayTimer;
    }

    if (_soundTimer > 0)
    {
        if (_soundTimer == 1)
        {
            printf("BEEP\n");
            --_soundTimer;
        }
    }
}

bool chip8::drawFlag()
{
    return _drawFlag;
}

void chip8::setDrawFlag(const bool flag)
{
    _drawFlag = flag;
}

bool chip8::load(const char *path)
{
    FILE *program = fopen(path, "rb");
    if (program == nullptr)
    {
        fputs("Could not open program", stderr);
        return false;
    }

    // Check size of program
    fseek(program, 0, SEEK_END);
    long programSize = ftell(program);
    rewind(program);
    fputs("Program size is " + (int)programSize, stdout);

    // Allocate memory for program
    char *buffer = (char *)malloc(sizeof(char) * programSize);

    // Copy the program into the buffer
    size_t result = fread(buffer, sizeof(char), programSize, program);

    // Check if we can fit the program into our memory
    // and if we can, load the buffer into memory
    if (programSize > sizeof(_memory) - PROGRAM_START_ADDRESS)
    {
        fputs("Program file size is too big", stderr);
        return false;
    }
    else
    {
        for (int i = 0; i < programSize; ++i)
        {
            _memory[i + PROGRAM_START_ADDRESS] = buffer[i];
        }
    }

    // Close file and free buffer
    fclose(program);
    free(buffer);

    return true;
}