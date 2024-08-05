#include "cpu.h"
#include "helper.h"

SM83::SM83(Memory &mem, bool test_mode) : mem(mem), regs{0, 0, 0, 0, 0, 0, 0, 0}, test_mode(test_mode)
{
    IR = 0;
    IE = 0;
    PC = 0;
    SP_reg = 0;
    flags = {
        0,
        0,
        0,
        0};
    IME = false;
    prevIME = false;
}

SM83::~SM83()
{
}

// Sets half registers, unless it's register 0b110, then sets [hl]
void SM83::setHalfRegister(int reg, byte value)
{
    if (reg == 0b110)
    {
        mem.setByte(getWordRegister(HL), value);
    }
    regs[reg] = value;
}

// TODO: test
//! The endianess might be wrong
void SM83::setWordRegister(int reg, word value)
{
    switch (reg)
    {
    case 0:
        // BC
        regs[B] = (byte) (value >> 8);
        regs[C] = (byte) (value & 0xFF);
        break;
    case 1:
        // DE
        regs[D] = (byte) (value >> 8);
        regs[E] = (byte) (value & 0xFF);
        break;
    case 2:
        // HL
        regs[H] = (byte) (value >> 8);
        regs[L] = (byte) (value & 0xFF);
        break;
    case 3:
        // SP
        SP_reg = value;
        break;
    }
}

byte SM83::getHalfRegister(int reg)
{
    if (reg == 0b110)
    {
        mem.getByte(getWordRegister(HL));
    }
    return regs[reg];
}

word SM83::getWordRegister(int reg)
{
    switch (reg)
    {
    case 0:
        // BC
        return combineBytes(regs[B], regs[C]);
    case 1:
        // DE
        return combineBytes(regs[D], regs[E]);
    case 2:
        // HL
        return combineBytes(regs[H], regs[L]);
    case 3:
        // SP
        return SP_reg;
    }
    return 0;
}

// TODO Test
void SM83::incrementWordRegister(int reg)
{
    setWordRegister(reg, getWordRegister(reg) + 1);
}

// TODO Test
void SM83::decrementWordRegister(int reg)
{
    setWordRegister(reg, getWordRegister(reg) + 1);
}

void SM83::pushStack(word data)
{
    mem.setWord(SP_reg - 1, data);
    SP_reg -= 2;
}

word SM83::popStack()
{
    word temp = mem.getWord(SP_reg);
    SP_reg += 2;
    return temp;
}

// TODO Test
void SM83::DAA()
{
    byte offset = 0;
    if (flags.H || ((regs[A] & 0b1111) > 0x9 && !flags.N))
    {
        offset |= 0x6;
    }
    if (flags.C || (regs[A] > 0x99 && !flags.N))
    {
        offset |= 0x60;
        flags.C = 1;
    }

    if (flags.N)
    {
        regs[A] -= offset;
    }
    else
    {
        regs[A] += offset;
    }
    flags.Z = regs[A] == 0;
    flags.H = 0;
}

// Executes one opcode and returns the number of m-cycles it took
int SM83::execute()
{
    if (IME && prevIME)
    {
        //! HANDLE EXCEPTION
    }
    prevIME = IME;
    // Get the Current Instruction and increment PC
    byte cur_instruction = mem.getByte(PC);
    PC++;

    int cycles;

    // Find the Opcode and execute the needed code

    // Matches first 2 bits
    switch (cur_instruction >> 6)
    {
    case 0b00:
        // If in testing mode and the instruction is 00000000, then it should return the test machine-cycle count, 0
        if (cur_instruction == 0b00000000 && test_mode)
        {
            return 0;
        }
        switch (cur_instruction & 0b111)
        {
        case 0b000:
        {
            switch ((cur_instruction & 0b111000) >> 3)
            {
            case 0b000:
                // NOP
                return 1;
                break;
            case 0b001:
                // LD (u16), SP
                mem.setWord(mem.getWord(PC), getWordRegister(SP));
                return 5;
                break;
            case 0b010:
                // STOP
                // TODO DO WHATEVER THIS NEEDS
                // TODO Until I see this instruction used, it's not doing anything
                // Should stop processor and screen until an input
                return 1;
                break;
            case 0b011:
                // JR e: Relative Jump
                //! TEST PLEASE, the signed is kinda weird
                PC += mem.getByte(PC) + 1;
                return 3;
                break;
            case 0b100:
                // JR NZ e
                if (!flags.Z)
                {
                    PC += mem.getByte(PC) + 1;
                    return 3;
                }
                else
                {
                    PC++;
                    return 2;
                }
                break;
            case 0b101:
                // JR Z e
                if (flags.Z)
                {
                    PC += mem.getByte(PC) + 1;
                    return 3;
                }
                else
                {
                    PC++;
                    return 2;
                }
                break;
            case 0b110:
                // JR NC e
                if (!flags.C)
                {
                    PC += mem.getByte(PC) + 1;
                    return 3;
                }
                else
                {
                    PC++;
                    return 2;
                }
                break;
            case 0b111:
                // JR C e
                if (flags.C)
                {
                    PC += mem.getByte(PC) + 1;
                    return 3;
                }
                else
                {
                    PC++;
                    return 2;
                }
                break;
            }
            break;
        }
        case 0b001:
            if ((cur_instruction & 0b1000) == 0)
            {
                // LD word r, u16
                // TODO Test
                setWordRegister(getAltDestination(cur_instruction), mem.getWord(PC));
                PC += 2;
                return 3;
            }
            else
            {
                // ADD HL, word r
                // TODO Test
                int dest = getWordRegister(HL);
                int src = getWordRegister(getAltDestination(cur_instruction));
                int result = dest + src;
                flags.N = false;
                flags.C = result > 0xFF;
                flags.H = (dest & 0b1111) + (dest & 0b1111) > 0b1111;
                setWordRegister(HL, (byte)result & 0b11111111);
                return 2;
            }
            break;
        case 0b010:
            // LD (word r), A
            //! Incrementing and Decrementing of HL could be wrong
            // TODO Test
            if (getAltDestination(cur_instruction) == 0b10)
            {
                mem.setByte(getWordRegister(HL), regs[A]);
                incrementWordRegister(HL);
            }
            else if (getAltDestination(cur_instruction) == 0b11)
            {
                mem.setByte(getWordRegister(HL), regs[A]);
                decrementWordRegister(HL);
            }
            else
            {
                mem.setByte(getWordRegister(getAltDestination(cur_instruction)), regs[A]);
            }
            return 2;
            break;
        case 0b011:
            // TODO Test
            if (cur_instruction & 0b1000 == 0)
            {
                // INC word r
                incrementWordRegister(getAltDestination(cur_instruction));
                return 2;
            }
            else
            {
                // DEC word r
                decrementWordRegister(getAltDestination(cur_instruction));
            }
            if (getDestination(cur_instruction) == 7)
            {
                return 3;
            }
            else
            {
                return 1;
            }
            break;
        case 0b100:
        {
            // INC byte r
            // TODO Test, ESP CARRY
            int temp = getHalfRegister(getDestination(cur_instruction));
            setHalfRegister(getDestination(cur_instruction), temp + 1);
            flags.Z = temp + 1 == 0;
            flags.N = false;
            flags.C = temp == 0b1111;
            if (getDestination(cur_instruction) == 7)
            {
                return 3;
            }
            else
            {
                return 1;
            }
            break;
        }
        case 0b101:
        {
            // DEC byte r
            // TODO Test
            int temp = getHalfRegister(getDestination(cur_instruction));
            setHalfRegister(getDestination(cur_instruction), temp - 1);
            flags.Z = temp - 1 == 0;
            flags.N = true;
            flags.C = temp == 0;
            if (getDestination(cur_instruction) == 7)
            {
                return 3;
            }
            else
            {
                return 1;
            }
            break;
        }
        case 0b110:
            // LD byte r, n
            setHalfRegister(getDestination(cur_instruction), mem.getByte(PC));
            PC++;
            // If register is HL, then it's an indirect Load and takes 2 m-cycles
            if (getDestination(cur_instruction) == 7)
            {
                return 3;
            }
            else
            {
                return 2;
            }
            break;
        // TODO Test
        case 0b111:
            switch ((cur_instruction & 0b111000) >> 3)
            {
            case 0b000:
            {
                // RLCA
                byte temp = regs[A];
                flags.Z = false;
                flags.N = false;
                flags.H = false;
                flags.C = temp & 0b10000000;
                regs[A] = (temp << 1) | (byte)flags.C;
                break;
            }
            case 0b001:
            {
                // RRCA
                byte temp = regs[A];
                flags.Z = false;
                flags.N = false;
                flags.H = false;
                flags.C = temp & 0b00000001;
                regs[A] = (temp >> 1) | ((byte)flags.C << 7);
            }
            break;
            case 0b010:
            {
                // RLA
                byte temp = regs[A];
                regs[A] = (temp << 1) | (byte) flags.C;
                flags.Z = false;
                flags.N = false;
                flags.H = false;
                flags.C = temp & 0b10000000 == 1;
                break;
            }
            case 0b011:
            { // RRA
                byte temp = regs[A];
                regs[A] = (temp >> 1) | (((byte) flags.C) << 7);
                flags.Z = false;
                flags.N = false;
                flags.H = false;
                flags.C = temp & 0b00000001;
                break;
            }
            case 0b100:
                // DAA
                DAA();
                break;
            case 0b101:
                // CPL: Complement Accumulator
                regs[A] = ~regs[A];
                flags.N = true;
                flags.H = true;
                break;
            case 0b110:
                // SCF: Set carry flag
                flags.C = true;
                flags.H = false;
                flags.N = false;
                break;
            case 0b111:
                // CCF: Complement Carry Flag
                flags.N = false;
                flags.H = false;
                flags.C = !flags.C;
                break;
            }
            return 1;
        }
        break;
    case 0b01:
        // LD byte r, byte r'
        if (cur_instruction == 0x76)
        {
            // TODO Halt
        }
        else
        {
            setHalfRegister(getDestination(cur_instruction), getHalfRegister(getSource(cur_instruction)));
            // If either register is HL, then it takes two m-cycles
            if (getSource(cur_instruction) == 7 || getSource(cur_instruction) == 7)
            {
                return 2;
            }
            else
            {
                return 1;
            }
        }
        break;

    // TODO UNTESTED
    case 0b10:
        switch ((cur_instruction & 0b00110000) >> 4)
        {
        case 0b00:
        {
            // ADD/ADC A, byte r
            int sum = (int)getHalfRegister(A) + (int)getHalfRegister(getSource(cur_instruction));
            // check if with carry
            if ((cur_instruction & 0b1000) != 0)
            {
                sum += flags.C;
            }
            flags.N = 0;
            flags.Z = (sum == 0);
            flags.C = (sum > 0b11111111);
            flags.H = ((getHalfRegister(A) & 0b1000 != 0) && ((getHalfRegister(getSource(cur_instruction)) & 0b1000) != 0));
            setHalfRegister(A, (byte)(sum & 0b11111111));
            if (getSource(cur_instruction) == 7)
            {
                return 2;
            }
            else
            {
                return 1;
            }
            break;
        }
        case 0b01:
        {
            // SUB/SBC A, byte r
            byte sub1 = getHalfRegister(A);
            byte sub2 = getHalfRegister(getSource(cur_instruction));
            byte extra = 0;
            // check if with carry
            if ((cur_instruction & 0b1000) != 0)
            {
                extra = (byte)flags.C;
            }
            flags.Z = (sub1 == sub2 + extra);
            flags.N = 1;
            flags.C = (sub1 < sub2 + extra);
            flags.H = ((sub1 & 0b1111) < ((sub2 & 0b1111) + extra));
            // TODO Check if it actually puts it in binary if i subtract a big number from a small number
            setHalfRegister(A, sub1 - sub2 - extra);
            if (getSource(cur_instruction) == 7)
            {
                return 2;
            }
            else
            {
                return 1;
            }
            break;
        }
        case 0b10:
        {
            if (cur_instruction & 0b1000 == 0)
            {
                // AND A, byte r
                byte res = getHalfRegister(A) & getHalfRegister(getSource(cur_instruction));
                flags.Z = res == 0;
                flags.N = 0;
                flags.H = 1;
                flags.C = 0;
                setHalfRegister(A, res);
                if (getSource(cur_instruction) == 7)
                {
                    return 2;
                }
                else
                {
                    return 1;
                }
            }
            else
            {
                // XOR A, byte r
                byte res = getHalfRegister(A) ^ getHalfRegister(getSource(cur_instruction));
                flags.Z = res == 0;
                flags.N = 0;
                flags.H = 0;
                flags.C = 0;
                setHalfRegister(A, res);
                if (getSource(cur_instruction) == 7)
                {
                    return 2;
                }
                else
                {
                    return 1;
                }
            }
            break;
        }
        case 0b11:
            if (cur_instruction & 0b1000 == 0)
            {
                // OR A, byte r
                byte res = getHalfRegister(A) | getHalfRegister(getSource(cur_instruction));
                flags.Z = res == 0;
                flags.N = 0;
                flags.H = 0;
                flags.C = 0;
                setHalfRegister(A, res);
                if (getSource(cur_instruction) == 7)
                {
                    return 2;
                }
                else
                {
                    return 1;
                }
            }
            else
            {
                // CP A, byte r
                byte sub1 = getHalfRegister(A);
                byte sub2 = getHalfRegister(getSource(cur_instruction));
                byte extra = 0;
                // check if with carry
                if ((cur_instruction & 0b1000) != 0)
                {
                    extra = (byte)flags.C;
                }
                flags.Z = (sub1 == sub2 + extra);
                flags.N = 1;
                flags.C = (sub1 < sub2 + extra);
                flags.H = ((sub1 & 0b1111) < ((sub2 & 0b1111) + extra));
                if (getSource(cur_instruction) == 7)
                {
                    return 2;
                }
                else
                {
                    return 1;
                }
            }
            break;
        }
        break;
    case 0b11:
        switch (cur_instruction & 0b111)
        {
        case 0b000:
        {
            switch ((cur_instruction & 0b111000) >> 3)
            {
            case 0b000:
            {
                // RET NZ
                if (!flags.Z)
                {
                    PC = mem.getWord(SP_reg);
                    SP_reg += 2;
                    return 5;
                }
                else
                {
                    return 2;
                }
                break;
            }
            case 0b001:
            {
                // RET Z
                if (flags.Z)
                {
                    PC = mem.getWord(SP_reg);
                    SP_reg += 2;
                    return 5;
                }
                else
                {
                    return 2;
                }
                break;
            }
            case 0b010:
            {
                // RET NC
                if (!flags.C)
                {
                    PC = mem.getWord(SP_reg);
                    SP_reg += 2;
                    return 5;
                }
                else
                {
                    return 2;
                }
                break;
            }
            case 0b011:
            {
                // RET C
                if (flags.C)
                {
                    PC = mem.getWord(SP_reg);
                    SP_reg += 2;
                    return 5;
                }
                else
                {
                    return 2;
                }
                break;
            }
            case 0b100:
            {
                // LD (n), A
                mem.setByte(combineBytes(0xFF, mem.getByte(PC)), regs[A]);
                PC++;
                return 3;
                break;
            }
            case 0b101:
            {
                // ADD SP, s8
                //! AGAIN CHECK IF SIGNED ADDITION IS CORRECT also the flag setting
                byte temp = mem.getByte(PC);
                byte sum = SP_reg + temp;
                PC++;
                flags.Z = false;
                flags.N = false;
                flags.H = (temp & 0b1111) + (SP_reg & 0b1111) > 0b1111;
                flags.C = (sum < SP_reg);
                SP_reg = sum;
                return 4;
                break;
            }
            case 0b110:
            {
                // LDH A, (n)
                regs[A] = combineBytes(0xFF, mem.getByte(PC));
                PC++;
                return 3;
                break;
            }
            case 0b111:
            {
                // LD HL, SP+s8
                //! AGAIN CHECK IF SIGNED ADDITION IS CORRECT also the flag setting
                byte temp = mem.getByte(PC);
                byte sum = SP_reg + temp;
                PC++;
                flags.Z = false;
                flags.N = false;
                flags.H = (temp & 0b1111) + (SP_reg & 0b1111) > 0b1111;
                flags.C = (sum < SP_reg);
                mem.setByte(getWordRegister(HL), sum);
                return 4;
                break;
            }
            }
        }
        case 0b001:
        {
            if ((cur_instruction & 0b1000) == 0)
            {
                // POP word reg
                setWordRegister(getAltDestination(cur_instruction), popStack());
                return 3;
            }
            else
            {
                switch ((cur_instruction & 0b110000) >> 4)
                {
                case 0b00:
                {
                    // RET
                    PC = mem.getWord(SP_reg);
                    SP_reg += 2;
                    return 4;
                    break;
                }
                case 0b01:
                {
                    // RETI
                    PC = mem.getWord(SP_reg);
                    SP_reg += 2;
                    IME = true;
                    return 4;
                    break;
                }
                case 0b10:
                {
                    // JP HL
                    PC = getWordRegister(HL);
                    return 1;
                    break;
                }
                case 0b11:
                {
                    // LD SP, HL
                    SP_reg = getWordRegister(HL);
                    return 1;
                    break;
                }
                }
            }
            break;
        }
        case 0b010:
        {
            switch ((cur_instruction & 0b111000) >> 3)
            {
            case 0b000:
            {
                // JP NZ, u16
                word temp = mem.getWord(PC);
                PC += 2;
                if (!flags.Z)
                {
                    PC = temp;
                    return 4;
                }
                else
                {
                    return 3;
                }
                break;
            }
            case 0b001:
            {
                // JP Z, u16
                word temp = mem.getWord(PC);
                PC += 2;
                if (flags.Z)
                {
                    PC = temp;
                    return 4;
                }
                else
                {
                    return 3;
                }
                break;
            }
            case 0b010:
            {
                // JP NC, u16
                word temp = mem.getWord(PC);
                PC += 2;
                if (!flags.C)
                {
                    PC = temp;
                    return 4;
                }
                else
                {
                    return 3;
                }
                break;
            }
            case 0b011:
            {
                // JP C, u16
                word temp = mem.getWord(PC);
                PC += 2;
                if (flags.C)
                {
                    PC = temp;
                    return 4;
                }
                else
                {
                    return 3;
                }
                break;
            }
            case 0b100:
            {
                // LD (n + C), A
                mem.setByte(combineBytes(0xFF, regs[C]), regs[A]);
                return 2;
                break;
            }
            case 0b101:
            {
                // LD (u16), A
                mem.setByte(mem.getWord(PC), regs[A]);
                PC += 2;
                return 4;
                break;
            }
            case 0b110:
            {
                // LD A, (n + C)
                regs[A] = mem.getByte(combineBytes(0xFF, regs[C]));
                return 2;
                break;
            }
            case 0b111:
            {
                // LD A, (u16)
                regs[A] = mem.getByte(mem.getWord(PC));
                PC += 2;
                return 4;
                break;
            }
            }
            break;
        }
        case 0b011:
        {
            switch ((cur_instruction & 0b111000) >> 3)
            {
            case 0b000:
            {
                // JP u16
                PC = mem.getWord(PC);
                return 4;
                break;
            }
            case 0b001:
            {
                // PREFIX CB
                return executeCB();
                break;
            }
            case 0b110:
            {
                // DI
                IME = false;
                return 1;
                break;
            }
            case 0b111:
            {
                // EI
                IME = true;
                return 1;
                break;
            }
            }
            break;
        }
        case 0b100:
        {
            switch ((cur_instruction & 0b111000) >> 3)
            {
            case 0b000:
            {
                // CALL NZ, u16
                word address = mem.getWord(PC);
                PC += 2;
                if (!flags.Z)
                {
                    SP_reg--;
                    mem.setByte(SP, (byte)(PC >> 8));
                    SP_reg--;
                    mem.setByte(SP, (byte)(PC & 0b11111111));
                    PC = mem.getWord(PC);
                    return 6;
                }
                else
                {
                    return 3;
                }
                break;
            }
            case 0b001:
            {
                // CALL Z, u16
                word address = mem.getWord(PC);
                PC += 2;
                if (flags.Z)
                {
                    SP_reg--;
                    mem.setByte(SP, (byte)(PC >> 8));
                    SP_reg--;
                    mem.setByte(SP, (byte)(PC & 0b11111111));
                    PC = mem.getWord(PC);
                    return 6;
                }
                else
                {
                    return 3;
                }
                break;
            }
            case 0b010:
            {
                // CALL NC, u16
                word address = mem.getWord(PC);
                PC += 2;
                if (!flags.C)
                {
                    SP_reg--;
                    mem.setByte(SP, (byte)(PC >> 8));
                    SP_reg--;
                    mem.setByte(SP, (byte)(PC & 0b11111111));
                    PC = mem.getWord(PC);
                    return 6;
                }
                else
                {
                    return 3;
                }
                break;
            }
            case 0b011:
            {
                // CALL C, u16
                word address = mem.getWord(PC);
                PC += 2;
                if (flags.C)
                {
                    SP_reg--;
                    mem.setByte(SP, (byte)(PC >> 8));
                    SP_reg--;
                    mem.setByte(SP, (byte)(PC & 0b11111111));
                    PC = mem.getWord(PC);
                    return 6;
                }
                else
                {
                    return 3;
                }
                break;
            }
            }
            break;
        }
        case 0b101:
        {
            if (cur_instruction & 0b10000)
            {
                // CALL u16
                word address = mem.getWord(PC);
                PC += 2;
                SP_reg--;
                mem.setByte(SP, (byte)(PC >> 8));
                SP_reg--;
                mem.setByte(SP, (byte)(PC & 0b11111111));
                PC = mem.getWord(PC);
                return 6;
                break;
            }
            else
            {
                // PUSH word r
                pushStack(getWordRegister(getAltDestination(cur_instruction)));
                return 4;
            }
            break;
        }
        case 0b110:
        {
            switch ((cur_instruction & 0b111000) >> 3)
            {
            case 0b000:
            {
                // ADD A, u8
                byte temp = mem.getByte(PC);
                PC++;
                int sum = (int)regs[A] + (int)temp;
                flags.N = 0;
                flags.Z = (sum == 0);
                flags.C = (sum > 0b11111111);
                flags.H = ((regs[A] & 0b1000 != 0) && ((temp & 0b1000) != 0));
                setHalfRegister(A, (byte)(sum & 0b11111111));
                return 2;
                break;
            }
            case 0b001:
            {
                // ADC A, u8
                byte temp = mem.getByte(PC);
                PC++;
                int sum = (int)regs[A] + (int)temp;
                sum += (int)flags.C;
                flags.N = 0;
                flags.Z = (sum == 0);
                flags.C = (sum > 0b11111111);
                flags.H = ((regs[A] & 0b1000 != 0) && ((temp & 0b1000) != 0));
                setHalfRegister(A, (byte)(sum & 0b11111111));
                return 2;
                break;
            }
            case 0b010:
            {
                // SUB A, u8
                byte sub1 = getHalfRegister(A);
                byte sub2 = mem.getByte(PC);
                PC++;
                flags.Z = (sub1 == sub2);
                flags.N = 1;
                flags.C = (sub1 < sub2);
                flags.H = ((sub1 & 0b1111) < ((sub2 & 0b1111)));
                // TODO Check if it actually puts it in binary if i subtract a big number from a small number
                setHalfRegister(A, sub1 - sub2);
                return 2;
                break;
            }
            case 0b011:
            {
                // SBC A, u8
                byte sub1 = getHalfRegister(A);
                byte sub2 = mem.getByte(PC);
                PC++;
                flags.Z = (sub1 == sub2 + 1);
                flags.N = 1;
                flags.C = (sub1 < sub2 + 1);
                flags.H = ((sub1 & 0b1111) < ((sub2 & 0b1111) + 1));
                // TODO Check if it actually puts it in binary if i subtract a big number from a small number
                setHalfRegister(A, sub1 - sub2 - 1);
                return 2;
                break;
            }
            case 0b100:
            {
                // AND A, u8
                byte res = getHalfRegister(A) & mem.getByte(PC);
                PC++;
                flags.Z = res == 0;
                flags.N = 0;
                flags.H = 1;
                flags.C = 0;
                setHalfRegister(A, res);
                return 2;
                break;
            }
            case 0b101:
            {
                // XOR A, u8
                regs[A] = regs[A] ^ mem.getByte(PC);
                PC++;
                flags.Z = regs[A] == 0;
                flags.N = 0;
                flags.C = 0;
                flags.H = 0;
                return 2;
                break;
            }
            case 0b110:
            {
                // OR A, u8
                regs[A] = regs[A] | mem.getByte(PC);
                PC++;
                flags.Z = regs[A] == 0;
                flags.N = 0;
                flags.C = 0;
                flags.H = 0;
                return 2;
                break;
            }
            case 0b111:
            {
                // CP A, u8
                byte sub1 = getHalfRegister(A);
                byte sub2 = mem.getByte(PC);
                PC++;
                flags.Z = (sub1 == sub2);
                flags.N = 1;
                flags.C = (sub1 < sub2);
                flags.H = ((sub1 & 0b1111) < ((sub2 & 0b1111)));
                return 2;
                break;
            }
            }
            break;
        }
        case 0b111:
        {
            // RST n:  Restart/Call function
            pushStack(PC);
            PC = 0x8 * ((cur_instruction & 0b111000) >> 3);
            return 4;
            break;
        }
        break;
        }
        break;
    }
    return cycles;
}

int SM83::executeCB()
{
    byte cur_instruction = mem.getByte(PC);
    PC++;
    switch ((cur_instruction & 0b11000000) >> 6)
    {
    case 0b00:
    {
        switch ((cur_instruction & 0b111000) >> 3)
        {
        case 0b00000:
        {
            // RLC r8
            byte temp = getHalfRegister(getSource(cur_instruction));
            byte carry = (temp & 0b10000000) >> 7;
            flags.C = carry;
            flags.Z = temp == 0;
            flags.H = false;
            flags.N = false;
            //! check that this is just rotation not circular for <<
            setHalfRegister(getSource(cur_instruction), (temp << 1) | carry);
            break;
        }
        case 0b00001:
        {
            // RRC r8
            byte temp = getHalfRegister(getSource(cur_instruction));
            byte carry = temp & 0b00000001;
            flags.C = carry;
            flags.Z = temp == 0;
            flags.H = false;
            flags.N = false;
            setHalfRegister(getSource(cur_instruction), (temp >> 1) | (carry << 7));
            break;
        }
        case 0b00010:
        {
            // RL r8
            byte temp = getHalfRegister(getSource(cur_instruction));
            byte carry = (temp & 0b10000000) >> 7;
            setHalfRegister(getSource(cur_instruction), temp << 1 | ((byte)flags.C));
            flags.C = carry;
            flags.Z = temp == 0;
            flags.H = false;
            flags.N = false;
            break;
        }
        case 0b00011:
        {
            // RR r8
            byte temp = getHalfRegister(getSource(cur_instruction));
            byte carry = temp & 0b00000001;
            setHalfRegister(getSource(cur_instruction), temp >> 1 | (((byte)flags.C) << 7));
            flags.C = carry;
            flags.Z = temp == 0;
            flags.H = false;
            flags.N = false;
            break;
        }
        case 0b00100:
        {
            // SLA r8
            byte temp = getHalfRegister(getSource(cur_instruction));
            byte carry = (temp & 0b10000000) >> 7;
            flags.C = carry;
            flags.Z = temp == 0;
            flags.H = false;
            flags.N = false;
            setHalfRegister(getSource(cur_instruction), temp << 1);
            break;
        }
        case 0b00101:
        {
            // SRA r8
            byte temp = getHalfRegister(getSource(cur_instruction));
            byte sign = temp & 0b10000000;
            flags.C = (temp & 0b00000001);
            flags.Z = temp == 0;
            flags.H = false;
            flags.N = false;
            setHalfRegister(getSource(cur_instruction), (temp >> 1) | sign);
            break;
        }
        case 0b00110:
        {
            // SWAP r8
            byte temp = getHalfRegister(getSource(cur_instruction));
            flags.Z = temp == 0;
            flags.H = 0;
            flags.N = 0;
            flags.C = 0;
            setHalfRegister(getSource(cur_instruction), (temp << 4) | (temp >> 4));
        }
        case 0b00111:
        {
            // SRL r8
            byte temp = getHalfRegister(getSource(cur_instruction));
            flags.C = (temp & 0b00000001);
            flags.Z = temp == 0;
            flags.H = false;
            flags.N = false;
            setHalfRegister(getSource(cur_instruction), temp >> 1);
            break;
        }
        
        }
        break;
    }
    case 0b01:
    {
        // Bit 0-7, r8
        bool bit = getBit(getHalfRegister(getSource(cur_instruction)), getDestination(cur_instruction));
        flags.Z = bit;
        flags.H = 1;
        flags.N = 0;
        break;
    }
    case 0b10:
    {
        // RES 0-7, r8
        byte temp = getHalfRegister(getSource(cur_instruction));
        int bit = getDestination(cur_instruction);
        //! CHECK TO SEE IF THIS WORKS
        temp &= (0b11111110 << bit) | (0b01111111 >> 7-bit);
        setHalfRegister(getSource(cur_instruction), temp);
        break;
    }
    case 0b11:
    {
        // SET 0-7, r8
        // RES 0-7, r8
        byte temp = getHalfRegister(getSource(cur_instruction));
        int bit = getDestination(cur_instruction);
        //! CHECK TO SEE IF THIS WORKS
        temp |= 0b00000001 << bit;
        setHalfRegister(getSource(cur_instruction), temp);
        break;
    }
        }
    if (getSource(cur_instruction) == 7)
    {
        return 4;
    }
    return 2;
}
