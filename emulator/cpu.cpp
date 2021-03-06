#include "cpu.h"
#include "nes.h"

using namespace Frankenstein;

using Mode = Frankenstein::Addressing;

Cpu::Cpu(Nes& pNes)
    : nes(pNes)
{
    this->LoadRom(nes.rom);
    this->Reset();
}

void Cpu::LoadRom(const Rom& rom)
{
    const iNesHeader header = rom.GetHeader();
    int prgRomBanks = header.prgRomBanks;
    int trainerOffset = rom.GetTrainerOffset();
    int prgRomBanksLocation = Rom::HeaderSize + trainerOffset;

    switch (prgRomBanks) {
    case 1:
        nes.ram.Copy(rom.GetRaw() + prgRomBanksLocation, Frankenstein::ADDR_PRG_ROM_LOWER_BANK, PRGROM_BANK_SIZE);
        nes.ram.Copy(rom.GetRaw() + prgRomBanksLocation, Frankenstein::ADDR_PRG_ROM_UPPER_BANK, PRGROM_BANK_SIZE);
        break;
    case 2:
        nes.ram.Copy(rom.GetRaw() + prgRomBanksLocation, Frankenstein::ADDR_PRG_ROM_LOWER_BANK, PRGROM_BANK_SIZE);
        nes.ram.Copy(rom.GetRaw() + prgRomBanksLocation + PRGROM_BANK_SIZE, Frankenstein::ADDR_PRG_ROM_UPPER_BANK, PRGROM_BANK_SIZE);
        break;
    default: //TODO: implement multiple PRG-ROM banks
        break;
    }
    this->Reset();
}

void Cpu::Reset()
{
    this->registers.PC = (nes.ram[0xFFFC] | nes.ram[0xFFFD] << 8);
    this->registers.P = 0b00100100;
    this->registers.SP = 0xFF;
    this->stall = 0;
}

void Cpu::Step()
{
    if (this->stall > 0) {
        this->stall--;
        this->cycles = 1;
        return;
    }
    if (nmiOccurred) {
        this->cycles = NMI();
        this->nmiOccurred = false;
    } else {
        this->currentOpcode = OpCode();
        this->previousPC = this->registers.PC;
        auto instruction = this->instructions[this->currentOpcode];
        this->cycles = (this->*(instruction.fct))();
        this->registers.PC += instruction.size;
        this->nextOpcode = OpCode();
    }
}

void Cpu::PushOnStack(u8 value)
{
    this->nes.ram[Frankenstein::ADDR_STACK + this->registers.SP] = value;
    this->registers.SP -= 1;
}

u8 Cpu::PopFromStack()
{
    this->registers.SP += 1;
    u8 value = this->nes.ram[Frankenstein::ADDR_STACK + this->registers.SP];
    return value;
}

/**
* Fetch the opcode at memory[PC]
*/
u8 Cpu::OpCode()
{
    return nes.ram[this->registers.PC];
}

/**
* Fetch the operand at memory[PC + number]
*/
u8 Cpu::Operand(int number)
{
    return nes.ram[this->registers.PC + number];
}

////////////////////////////////////////////////////////////////////////////////
/// Binary Operations Definition
////////////////////////////////////////////////////////////////////////////////

void Cpu::AND(const u8 value)
{
    this->registers.A &= value;
    Set<Flags::Z>(CheckZero(this->registers.A));
    Set<Flags::S>(CheckSign(this->registers.A));
}

void Cpu::ASL(u8& value)
{
    // 0 is shifted into bit 0 and the original bit 7 is shifted into the Carry.
    Set<Flags::C>(CheckBit<8>(value));
    value <<= 1;
    Set<Flags::Z>(CheckZero(value));
    Set<Flags::S>(CheckSign(value));
}

void Cpu::ASL(const Memory value)
{
    u8 val = value;
    ASL(val);
    value = val;
}

void Cpu::BIT(const u8 value)
{
    auto result = value & this->registers.A;
    Set<Flags::Z>(CheckZero(result));
    Set<Flags::V>(CheckBit<7>(value));
    Set<Flags::S>(CheckBit<8>(value));
}

void Cpu::EOR(const u8 value)
{
    this->registers.A ^= value;
    Set<Flags::Z>(CheckZero(this->registers.A));
    Set<Flags::S>(CheckSign(this->registers.A));
}

void Cpu::LSR(u8& value)
{
    // 0 is shifted into bit 7 and the original bit 0 is shifted into the Carry.
    Set<Flags::C>(CheckBit<1>(value));
    value >>= 1;
    Set<Flags::Z>(CheckZero(value));
    Set<Flags::S>(CheckSign(value));
}

void Cpu::LSR(const Memory value)
{
    u8 val = value;
    LSR(val);
    value = val;
}

void Cpu::ORA(const u8 value)
{
    this->registers.A |= value;
    Set<Flags::Z>(CheckZero(this->registers.A));
    Set<Flags::S>(CheckSign(this->registers.A));
}

void Cpu::ROL(u8& value)
{
    bool carry = Get<Flags::C>();
    Set<Flags::C>(CheckBit<8>(value));
    value <<= 1;
    AssignBit<1>(value, carry);
    Set<Flags::Z>(CheckZero(value));
    Set<Flags::S>(CheckSign(value));
}

void Cpu::ROL(const Memory value)
{
    u8 val = value;
    ROL(val);
    value = val;
}

void Cpu::ROR(u8& value)
{
    auto carry = Get<Flags::C>();
    Set<Flags::C>(CheckBit<1>(value));
    value >>= 1;
    AssignBit<8>(value, carry);
    Set<Flags::Z>(CheckZero(value));
    Set<Flags::S>(CheckSign(value));
}

void Cpu::ROR(const Memory value)
{
    u8 val = value;
    ROR(val);
    value = val;
}

////////////////////////////////////////////////////////////////////////////////
/// Arithmetic Operations Definition
////////////////////////////////////////////////////////////////////////////////

void Cpu::ADC(const u8 value)
{
    u16 result = value + this->registers.A + Get<Flags::C>();
    u8 truncResult = static_cast<u8>(result);

    Set<Flags::Z>(CheckZero(truncResult));
    Set<Flags::S>(CheckSign(truncResult));
    Set<Flags::C>(CheckBit<9, u16>(result));
    Set<Flags::V>(CheckOverflow<>(this->registers.A, value, truncResult, true));

    this->registers.A = truncResult;
}

void Cpu::DEC(u8& value)
{
    value -= 1;
    Set<Flags::Z>(CheckZero(value));
    Set<Flags::S>(CheckSign(value));
}

void Cpu::DEC(const Memory value)
{
    u8 val = value;
    DEC(val);
    value = val;
}

void Cpu::INC(u8& value)
{
    value += 1;
    Set<Flags::Z>(CheckZero(value));
    Set<Flags::S>(CheckSign(value));
}

void Cpu::INC(const Memory value)
{
    u8 val = value;
    INC(val);
    value = val;
}

void Cpu::SBC(const u8 value)
{
    s16 result = this->registers.A - value - (1 - Get<Flags::C>());
    u8 truncResult = static_cast<u8>(result);

    Set<Flags::Z>(CheckZero(truncResult));
    Set<Flags::S>(CheckSign(truncResult));
    Set<Flags::C>(result >= 0);
    Set<Flags::V>(CheckOverflow<>(this->registers.A, value, truncResult, false));
    this->registers.A = truncResult;
}

////////////////////////////////////////////////////////////////////////////////
/// Memory Operations Definition
////////////////////////////////////////////////////////////////////////////////

void Cpu::LDA(const u8 value)
{
    this->registers.A = value;
    Set<Flags::Z>(CheckZero(this->registers.A));
    Set<Flags::S>(CheckSign(this->registers.A));
}

void Cpu::LDX(const u8 value)
{
    this->registers.X = value;
    Set<Flags::Z>(CheckZero(this->registers.X));
    Set<Flags::S>(CheckSign(this->registers.X));
}

void Cpu::LDY(const u8 value)
{
    this->registers.Y = value;
    Set<Flags::Z>(CheckZero(this->registers.Y));
    Set<Flags::S>(CheckSign(this->registers.Y));
}

void Cpu::STA(const Memory value)
{
    value = this->registers.A;
}

void Cpu::STX(const Memory value)
{
    value = this->registers.X;
}

void Cpu::STY(const Memory value)
{
    value = this->registers.Y;
}

////////////////////////////////////////////////////////////////////////////////
/// Branch Operations
////////////////////////////////////////////////////////////////////////////////

//Branch on plus
u8 Cpu::BPL()
{
    if (!Get<Flags::S>()) {
        s8 offset = Operand(1);
        auto pageCrossed = NesMemory::IsPageCrossed(this->registers.PC + 2, this->registers.PC + offset);
        this->registers.PC += offset;
        return 3 + pageCrossed;
    }
    return 2;
}

//Branch on minus
u8 Cpu::BMI()
{
    if (Get<Flags::S>()) {
        s8 offset = Operand(1);
        auto pageCrossed = NesMemory::IsPageCrossed(this->registers.PC + 2, this->registers.PC + offset);
        this->registers.PC += offset;
        return 3 + pageCrossed;
    }
    return 2;
}

//Branch on overflow clear
u8 Cpu::BVC()
{
    if (!Get<Flags::V>()) {
        s8 offset = Operand(1);
        auto pageCrossed = NesMemory::IsPageCrossed(this->registers.PC + 2, this->registers.PC + offset);
        this->registers.PC += offset;
        return 3 + pageCrossed;
    }
    return 2;
}

//Branch on overflow set
u8 Cpu::BVS()
{
    if (Get<Flags::V>()) {
        s8 offset = Operand(1);
        auto pageCrossed = NesMemory::IsPageCrossed(this->registers.PC + 2, this->registers.PC + offset);
        this->registers.PC += offset;
        return 3 + pageCrossed;
    }
    return 2;
}

//Branch on carry clear
u8 Cpu::BCC()
{
    if (!Get<Flags::C>()) {
        s8 offset = Operand(1);
        auto pageCrossed = NesMemory::IsPageCrossed(this->registers.PC + 2, this->registers.PC + offset);
        this->registers.PC += offset;
        return 3 + pageCrossed;
    }
    return 2;
}

//Branch on carry set
u8 Cpu::BCS()
{
    if (Get<Flags::C>()) {
        s8 offset = Operand(1);
        auto pageCrossed = NesMemory::IsPageCrossed(this->registers.PC + 2, this->registers.PC + offset);
        this->registers.PC += offset;
        return 3 + pageCrossed;
    }
    return 2;
}

//Branch on not equal
u8 Cpu::BNE()
{
    if (!Get<Flags::Z>()) {
        s8 offset = Operand(1);
        auto pageCrossed = NesMemory::IsPageCrossed(this->registers.PC + 2, this->registers.PC + offset);
        this->registers.PC += offset;
        return 3 + pageCrossed;
    }
    return 2;
}

//Branch on equal
u8 Cpu::BEQ()
{
    if (Get<Flags::Z>()) {
        s8 offset = Operand(1);
        auto pageCrossed = NesMemory::IsPageCrossed(this->registers.PC + 2, this->registers.PC + offset);
        this->registers.PC += offset;
        return 3 + pageCrossed;
    }
    return 2;
}

////////////////////////////////////////////////////////////////////////////////
/// Register Operations Definition
/// T[1][2] -> Transfert register 1 to register 2
/// DE[1] -> DEC register 1
/// IN[1] -> INC register 1
////////////////////////////////////////////////////////////////////////////////

u8 Cpu::TAX()
{
    auto& value = this->registers.A;
    this->registers.X = value;
    Set<Flags::Z>(CheckZero(this->registers.X));
    Set<Flags::S>(CheckSign(this->registers.X));
    return 2;
}

u8 Cpu::TXA()
{
    this->registers.A = this->registers.X;
    Set<Flags::Z>(CheckZero(this->registers.A));
    Set<Flags::S>(CheckSign(this->registers.A));
    return 2;
}

u8 Cpu::DEX()
{
    this->registers.X -= 1;
    Set<Flags::Z>(CheckZero(this->registers.X));
    Set<Flags::S>(CheckSign(this->registers.X));
    return 2;
}

u8 Cpu::INX()
{
    this->registers.X += 1;
    Set<Flags::Z>(CheckZero(this->registers.X));
    Set<Flags::S>(CheckSign(this->registers.X));
    return 2;
}

u8 Cpu::TAY()
{
    this->registers.Y = this->registers.A;
    Set<Flags::Z>(CheckZero(this->registers.Y));
    Set<Flags::S>(CheckSign(this->registers.Y));
    return 2;
}

u8 Cpu::TYA()
{
    this->registers.A = this->registers.Y;
    Set<Flags::Z>(CheckZero(this->registers.A));
    Set<Flags::S>(CheckSign(this->registers.A));
    return 2;
}

u8 Cpu::DEY()
{
    this->registers.Y -= 1;
    Set<Flags::Z>(CheckZero(this->registers.Y));
    Set<Flags::S>(CheckSign(this->registers.Y));
    return 2;
}

u8 Cpu::INY()
{
    this->registers.Y += 1;
    Set<Flags::Z>(CheckZero(this->registers.Y));
    Set<Flags::S>(CheckSign(this->registers.Y));
    return 2;
}

////////////////////////////////////////////////////////////////////////////////
/// Stack Operations Definition
////////////////////////////////////////////////////////////////////////////////

// Transfert X to Stack ptr
u8 Cpu::TXS()
{
    this->registers.SP = this->registers.X;
    return 2;
}

u8 Cpu::TSX()
{
    this->registers.X = this->registers.SP;
    Set<Flags::Z>(CheckZero(this->registers.X));
    Set<Flags::S>(CheckSign(this->registers.X));
    return 2;
}

// Push A
u8 Cpu::PHA()
{
    PushOnStack(this->registers.A);
    return 3;
}

// Pop A
u8 Cpu::PLA()
{
    auto value = PopFromStack();
    this->registers.A = value;
    Set<Flags::Z>(CheckZero(value));
    Set<Flags::S>(CheckSign(value));
    return 4;
}

// Push Processor Status
u8 Cpu::PHP()
{
    u8 copy = this->registers.P;
    SetBit<5>(copy);
    SetBit<6>(copy);
    PushOnStack(copy);
    return 3;
}

// Pull Processor Status
u8 Cpu::PLP()
{
    u8 temp = PopFromStack();
    ClearBit<5>(temp);
    ClearBit<6>(temp);
    this->registers.P = temp;
    return 4;
}

////////////////////////////////////////////////////////////////////////////////
/// PC Operations Definition
////////////////////////////////////////////////////////////////////////////////

void Cpu::Interrupt()
{
    PushOnStack((this->registers.PC >> 8) & 0xFF); /* Push return address onto the stack. */
    PushOnStack(this->registers.PC & 0xFF);
    PHP();
    Set<Flags::I>(true);
}

u8 Cpu::NMI()
{
    Interrupt();
    this->registers.PC = (nes.ram[0xFFFA] | nes.ram[0xFFFB] << 8);
    return 7;
}

u8 Cpu::BRK()
{
    this->registers.PC += 2;
    Interrupt();
    this->registers.PC = (nes.ram[0xFFFE] | nes.ram[0xFFFF] << 8);
    return 7;
}

void Cpu::JMP(const u16 value)
{
    this->registers.PC = value;
}

u8 Cpu::JSR()
{
    auto address = NesMemory::Absolute(Operand(1), Operand(2));
    this->registers.PC += 2;
    PushOnStack((this->registers.PC >> 8) & 0xFF); /* Push return address onto the stack. */
    PushOnStack(this->registers.PC & 0xFF);
    this->registers.PC = address;
    return 6;
}

u8 Cpu::RTI()
{
    this->registers.P = (PopFromStack() & 0xEF) | 0x20;
    u8 low = PopFromStack();
    u8 high = PopFromStack();
    u16 address = u16(low) | (u16(high) << 8);
    this->registers.PC = address;
    return 6;
}

u8 Cpu::RTS()
{
    u8 low = PopFromStack();
    u8 high = PopFromStack();
    u16 address = u16(low) | (u16(high) << 8);
    this->registers.PC = address;
    return 6;
}

///////////////////////////////////////////////////////////////////////////////
/// Flag Instructions
///////////////////////////////////////////////////////////////////////////////

void Cpu::CMP(const u8 value)
{
    u16 result = this->registers.A - value;
    Set<Flags::C>(this->registers.A >= value);
    Set<Flags::S>(CheckSign(result));
    Set<Flags::Z>(this->registers.A == value);
}

void Cpu::CPX(const u8 value)
{
    u16 result = this->registers.X - value;
    Set<Flags::C>(this->registers.X >= value);
    Set<Flags::S>(CheckSign(result));
    Set<Flags::Z>(this->registers.X == value);
}

void Cpu::CPY(const u8 value)
{
    u16 result = this->registers.Y - value;
    Set<Flags::C>(this->registers.Y >= value);
    Set<Flags::S>(CheckSign(result));
    Set<Flags::Z>(this->registers.Y == value);
}

u8 Cpu::CLC()
{
    Set<Flags::C>(false);
    return 2;
}

u8 Cpu::SEC()
{
    Set<Flags::C>(true);
    return 2;
}

u8 Cpu::CLI()
{
    Set<Flags::I>(false);
    return 2;
}

u8 Cpu::SEI()
{
    Set<Flags::I>(true);
    return 2;
}

u8 Cpu::CLV()
{
    Set<Flags::V>(false);
    return 2;
}

u8 Cpu::CLD()
{
    Set<Flags::D>(false);
    return 2;
}

u8 Cpu::SED()
{
    Set<Flags::D>(true);
    return 2;
}

////////////////////////////////////////////////////////////////////////////////
/// Other operations
////////////////////////////////////////////////////////////////////////////////

u8 Cpu::NOP()
{
    return 2;
}

u8 Cpu::UNIMP()
{
    return 2;
}

////////////////////////////////////////////////////////////////////////////////
/// Operations variant with addressing
////////////////////////////////////////////////////////////////////////////////

/// Logical OR
/// 2 bytes; 6 cycles
u8 Cpu::ORA_IND_X()
{
    ORA(nes.ram.Get<Mode::PreIndexedIndirect>(Operand(1), this->registers.X));
    return 6;
}

/// 2 bytes; 3 cycles
u8 Cpu::ORA_ZP()
{
    ORA(nes.ram.Get<Mode::ZeroPage>(Operand(1)));
    return 3;
}

u8 Cpu::ASL_ZP()
{
    ASL(nes.ram.Get<Mode::ZeroPage>(Operand(1)));
    return 5;
}

u8 Cpu::ORA_IMM()
{
    ORA(Operand(1));
    return 2;
}

u8 Cpu::ASL_ACC()
{
    ASL(this->registers.A);
    return 2;
}

u8 Cpu::ORA_ABS()
{
    ORA(nes.ram.Get<Mode::Absolute>(Operand(1), Operand(2)));
    return 4;
}

u8 Cpu::ASL_ABS()
{
    ASL(nes.ram.Get<Mode::Absolute>(Operand(1), Operand(2)));
    return 6;
}

u8 Cpu::ORA_IND_Y()
{
    ORA(nes.ram.Get<Mode::PostIndexedIndirect>(Operand(1), this->registers.Y));
    auto address = nes.ram.Indirect(Operand(1), 0);
    return 5 + NesMemory::IsPageCrossed(address, address + this->registers.Y);
}

u8 Cpu::ORA_ZP_X()
{
    ORA(nes.ram.Get<Mode::ZeroPageIndexed>(Operand(1), this->registers.X));
    return 4;
}

u8 Cpu::ASL_ZP_X()
{
    ASL(nes.ram.Get<Mode::ZeroPageIndexed>(Operand(1), this->registers.X));
    return 6;
}

u8 Cpu::ORA_ABS_Y()
{
    ORA(nes.ram.Get<Mode::Indexed>(Operand(1), Operand(2), this->registers.Y));
    auto address = NesMemory::FromValues(Operand(1), Operand(2));
    return 4 + NesMemory::IsPageCrossed(address, address + this->registers.Y);
}

u8 Cpu::ORA_ABS_X()
{
    ORA(nes.ram.Get<Mode::Indexed>(Operand(1), Operand(2), this->registers.X));
    auto address = NesMemory::FromValues(Operand(1), Operand(2));
    return 4 + NesMemory::IsPageCrossed(address, address + this->registers.X);
}

u8 Cpu::ASL_ABS_X()
{
    ASL(nes.ram.Get<Mode::Indexed>(Operand(1), Operand(2), this->registers.X));
    return 7;
}

u8 Cpu::AND_IND_X()
{
    AND(nes.ram.Get<Mode::PreIndexedIndirect>(Operand(1), this->registers.X));
    return 6;
}

u8 Cpu::BIT_ZP()
{
    BIT(nes.ram.Get<Mode::ZeroPage>(Operand(1)));
    return 3;
}

u8 Cpu::AND_ZP()
{
    AND(nes.ram.Get<Mode::ZeroPage>(Operand(1)));
    return 3;
}

u8 Cpu::ROL_ZP()
{
    ROL(nes.ram.Get<Mode::ZeroPage>(Operand(1)));
    return 5;
}

u8 Cpu::AND_IMM()
{
    AND(Operand(1));
    return 2;
}

u8 Cpu::ROL_ACC()
{
    ROL(this->registers.A);
    return 2;
}

u8 Cpu::BIT_ABS()
{
    BIT(nes.ram.Get<Mode::Absolute>(Operand(1), Operand(2)));
    return 4;
}

u8 Cpu::AND_ABS()
{
    AND(nes.ram.Get<Mode::Absolute>(Operand(1), Operand(2)));
    return 4;
}

u8 Cpu::ROL_ABS()
{
    ROL(nes.ram.Get<Mode::Absolute>(Operand(1), Operand(2)));
    return 6;
}

u8 Cpu::AND_IND_Y()
{
    AND(nes.ram.Get<Mode::PostIndexedIndirect>(Operand(1), this->registers.Y));
    auto address = nes.ram.Indirect(Operand(1), 0);
    return 5 + NesMemory::IsPageCrossed(address, address + this->registers.Y);
}

u8 Cpu::AND_ZP_X()
{
    AND(nes.ram.Get<Mode::ZeroPageIndexed>(Operand(1), this->registers.X));
    return 4;
}

u8 Cpu::ROL_ZP_X()
{
    ROL(nes.ram.Get<Mode::ZeroPageIndexed>(Operand(1), this->registers.X));
    return 6;
}

u8 Cpu::AND_ABS_Y()
{
    AND(nes.ram.Get<Mode::Indexed>(Operand(1), Operand(2), this->registers.Y));
    auto address = NesMemory::FromValues(Operand(1), Operand(2));
    return 4 + NesMemory::IsPageCrossed(address, address + this->registers.Y);
}

u8 Cpu::AND_ABS_X()
{
    AND(nes.ram.Get<Mode::Indexed>(Operand(1), Operand(2), this->registers.X));
    auto address = NesMemory::FromValues(Operand(1), Operand(2));
    return 4 + NesMemory::IsPageCrossed(address, address + this->registers.X);
}

u8 Cpu::ROL_ABS_X()
{
    ROL(nes.ram.Get<Mode::Indexed>(Operand(1), Operand(2), this->registers.X));
    return 7;
}

u8 Cpu::EOR_IND_X()
{
    EOR(nes.ram.Get<Mode::PreIndexedIndirect>(Operand(1), this->registers.X));
    return 6;
}

u8 Cpu::EOR_ZP()
{
    EOR(nes.ram.Get<Mode::ZeroPage>(Operand(1)));
    return 3;
}

u8 Cpu::LSR_ZP()
{
    LSR(nes.ram.Get<Mode::ZeroPage>(Operand(1)));
    return 5;
}

u8 Cpu::EOR_IMM()
{
    EOR(Operand(1));
    return 2;
}

u8 Cpu::LSR_ACC()
{
    LSR(this->registers.A);
    return 2;
}

u8 Cpu::JMP_ABS()
{
    JMP(nes.ram.Absolute(Operand(1), Operand(2)));
    return 3;
}

u8 Cpu::EOR_ABS()
{
    EOR(nes.ram.Get<Mode::Absolute>(Operand(1), Operand(2)));
    return 4;
}

u8 Cpu::LSR_ABS()
{
    LSR(nes.ram.Get<Mode::Absolute>(Operand(1), Operand(2)));
    return 6;
}

u8 Cpu::EOR_IND_Y()
{
    EOR(nes.ram.Get<Mode::PostIndexedIndirect>(Operand(1), this->registers.Y));
    auto address = nes.ram.Indirect(Operand(1), 0);
    return 5 + NesMemory::IsPageCrossed(address, address + this->registers.Y);
}

u8 Cpu::EOR_ZP_X()
{
    EOR(nes.ram.Get<Mode::ZeroPageIndexed>(Operand(1), this->registers.X));
    return 4;
}

u8 Cpu::LSR_ZP_X()
{
    LSR(nes.ram.Get<Mode::ZeroPageIndexed>(Operand(1), this->registers.X));
    return 6;
}

u8 Cpu::EOR_ABS_Y()
{
    EOR(nes.ram.Get<Mode::Indexed>(Operand(1), Operand(2), this->registers.Y));
    auto address = NesMemory::FromValues(Operand(1), Operand(2));
    return 4 + NesMemory::IsPageCrossed(address, address + this->registers.Y);
}

u8 Cpu::EOR_ABS_X()
{
    EOR(nes.ram.Get<Mode::Indexed>(Operand(1), Operand(2), this->registers.X));
    auto address = NesMemory::FromValues(Operand(1), Operand(2));
    return 4 + NesMemory::IsPageCrossed(address, address + this->registers.X);
}

u8 Cpu::LSR_ABS_X()
{
    LSR(nes.ram.Get<Mode::Indexed>(Operand(1), Operand(2), this->registers.X));
    return 7;
}

u8 Cpu::ADC_IND_X()
{
    ADC(nes.ram.Get<Mode::PreIndexedIndirect>(Operand(1), this->registers.X));
    return 6;
}

u8 Cpu::ADC_ZP()
{
    ADC(nes.ram.Get<Mode::ZeroPage>(Operand(1)));
    return 3;
}

u8 Cpu::ROR_ZP()
{
    ROR(nes.ram.Get<Mode::ZeroPage>(Operand(1)));
    return 5;
}

u8 Cpu::ADC_IMM()
{
    ADC(Operand(1));
    return 2;
}

u8 Cpu::ROR_ACC()
{
    ROR(this->registers.A);
    return 2;
}

u8 Cpu::JMP_IND()
{
    if (NesMemory::IsPageCrossed(this->registers.PC + 1, this->registers.PC + 2))
        JMP(nes.ram.Indirect(Operand(1), Operand(-0xFE))); //wrap around
    else
        JMP(nes.ram.Indirect(Operand(1), Operand(2)));
    return 5;
}

u8 Cpu::ADC_ABS()
{
    ADC(nes.ram.Get<Mode::Absolute>(Operand(1), Operand(2)));
    return 4;
}

u8 Cpu::ROR_ABS()
{
    ROR(nes.ram.Get<Mode::Absolute>(Operand(1), Operand(2)));
    return 6;
}

u8 Cpu::ADC_IND_Y()
{
    ADC(nes.ram.Get<Mode::PostIndexedIndirect>(Operand(1), this->registers.Y));
    auto address = nes.ram.Indirect(Operand(1), 0);
    return 5 + NesMemory::IsPageCrossed(address, address + this->registers.Y);
}

u8 Cpu::ADC_ZP_X()
{
    ADC(nes.ram.Get<Mode::ZeroPageIndexed>(Operand(1), this->registers.X));
    return 4;
}

u8 Cpu::ROR_ZP_X()
{
    ROR(nes.ram.Get<Mode::ZeroPageIndexed>(Operand(1), this->registers.X));
    return 6;
}

u8 Cpu::ADC_ABS_Y()
{
    ADC(nes.ram.Get<Mode::Indexed>(Operand(1), Operand(2), this->registers.Y));
    auto address = NesMemory::FromValues(Operand(1), Operand(2));
    return 4 + NesMemory::IsPageCrossed(address, address + this->registers.Y);
}

u8 Cpu::ADC_ABS_X()
{
    ADC(nes.ram.Get<Mode::Indexed>(Operand(1), Operand(2), this->registers.X));
    auto address = NesMemory::FromValues(Operand(1), Operand(2));
    return 4 + NesMemory::IsPageCrossed(address, address + this->registers.X);
}

u8 Cpu::ROR_ABS_X()
{
    ROR(nes.ram.Get<Mode::Indexed>(Operand(1), Operand(2), this->registers.X));
    return 7;
}

u8 Cpu::STA_IND_X()
{
    STA(nes.ram.Get<Mode::PreIndexedIndirect>(Operand(1), this->registers.X));
    return 6;
}

u8 Cpu::STY_ZP()
{
    STY(nes.ram.Get<Mode::ZeroPage>(Operand(1)));
    return 3;
}

u8 Cpu::STA_ZP()
{
    STA(nes.ram.Get<Mode::ZeroPage>(Operand(1)));
    return 3;
}

u8 Cpu::STX_ZP()
{
    STX(nes.ram.Get<Mode::ZeroPage>(Operand(1)));
    return 3;
}

u8 Cpu::STY_ABS()
{
    STY(nes.ram.Get<Mode::Absolute>(Operand(1), Operand(2)));
    return 4;
}

u8 Cpu::STA_ABS()
{
    STA(nes.ram.Get<Mode::Absolute>(Operand(1), Operand(2)));
    return 4;
}

u8 Cpu::STX_ABS()
{
    STX(nes.ram.Get<Mode::Absolute>(Operand(1), Operand(2)));
    return 4;
}

u8 Cpu::STA_IND_Y()
{
    STA(nes.ram.Get<Mode::PostIndexedIndirect>(Operand(1), this->registers.Y));
    return 6;
}

u8 Cpu::STY_ZP_X()
{
    STY(nes.ram.Get<Mode::ZeroPageIndexed>(Operand(1), this->registers.X));
    return 4;
}

u8 Cpu::STA_ZP_X()
{
    STA(nes.ram.Get<Mode::ZeroPageIndexed>(Operand(1), this->registers.X));
    return 4;
}

u8 Cpu::STX_ZP_Y()
{
    STX(nes.ram.Get<Mode::ZeroPageIndexed>(Operand(1), this->registers.Y));
    return 4;
}

u8 Cpu::STA_ABS_Y()
{
    STA(nes.ram.Get<Mode::Indexed>(Operand(1), Operand(2), this->registers.Y));
    return 5;
}

u8 Cpu::STA_ABS_X()
{
    STA(nes.ram.Get<Mode::Indexed>(Operand(1), Operand(2), this->registers.X));
    return 5;
}

u8 Cpu::LDY_IMM()
{
    LDY(Operand(1));
    return 2;
}

u8 Cpu::LDA_IND_X()
{
    LDA(nes.ram.Get<Mode::PreIndexedIndirect>(Operand(1), this->registers.X));
    return 6;
}

u8 Cpu::LDX_IMM()
{
    LDX(Operand(1));
    return 2;
}

u8 Cpu::LDY_ZP()
{
    LDY(nes.ram.Get<Mode::ZeroPage>(Operand(1)));
    return 3;
}

u8 Cpu::LDA_ZP()
{
    LDA(nes.ram.Get<Mode::ZeroPage>(Operand(1)));
    return 3;
}

u8 Cpu::LDX_ZP()
{
    LDX(nes.ram.Get<Mode::ZeroPage>(Operand(1)));
    return 3;
}

u8 Cpu::LDA_IMM()
{
    LDA(Operand(1));
    return 2;
}

u8 Cpu::LDY_ABS()
{
    LDY(nes.ram.Get<Mode::Absolute>(Operand(1), Operand(2)));
    return 4;
}

u8 Cpu::LDA_ABS()
{
    LDA(nes.ram.Get<Mode::Absolute>(Operand(1), Operand(2)));
    return 4;
}

u8 Cpu::LDX_ABS()
{
    LDX(nes.ram.Get<Mode::Absolute>(Operand(1), Operand(2)));
    return 4;
}

u8 Cpu::LDA_IND_Y()
{
    LDA(nes.ram.Get<Mode::PostIndexedIndirect>(Operand(1), this->registers.Y));
    auto address = nes.ram.Indirect(Operand(1), 0);
    return 5 + NesMemory::IsPageCrossed(address, address + this->registers.Y);
}

u8 Cpu::LDY_ZP_X()
{
    LDY(nes.ram.Get<Mode::ZeroPageIndexed>(Operand(1), this->registers.X));
    return 4;
}

u8 Cpu::LDA_ZP_X()
{
    LDA(nes.ram.Get<Mode::ZeroPageIndexed>(Operand(1), this->registers.X));
    return 4;
}

u8 Cpu::LDX_ZP_Y()
{
    LDX(nes.ram.Get<Mode::ZeroPageIndexed>(Operand(1), this->registers.Y));
    return 4;
}

u8 Cpu::LDA_ABS_Y()
{
    LDA(nes.ram.Get<Mode::Indexed>(Operand(1), Operand(2), this->registers.Y));
    auto address = NesMemory::FromValues(Operand(1), Operand(2));
    return 4 + NesMemory::IsPageCrossed(address, address + this->registers.Y);
}

u8 Cpu::LDY_ABS_X()
{
    LDY(nes.ram.Get<Mode::Indexed>(Operand(1), Operand(2), this->registers.X));
    auto address = NesMemory::FromValues(Operand(1), Operand(2));
    return 4 + NesMemory::IsPageCrossed(address, address + this->registers.X);
}

u8 Cpu::LDA_ABS_X()
{
    LDA(nes.ram.Get<Mode::Indexed>(Operand(1), Operand(2), this->registers.X));
    auto address = NesMemory::FromValues(Operand(1), Operand(2));
    return 4 + NesMemory::IsPageCrossed(address, address + this->registers.X);
}

u8 Cpu::LDX_ABS_Y()
{
    LDX(nes.ram.Get<Mode::Indexed>(Operand(1), Operand(2), this->registers.Y));
    auto address = NesMemory::FromValues(Operand(1), Operand(2));
    return 4 + NesMemory::IsPageCrossed(address, address + this->registers.Y);
}

u8 Cpu::CPY_IMM()
{
    CPY(Operand(1));
    return 2;
}

u8 Cpu::CMP_IND_X()
{
    CMP(nes.ram.Get<Mode::PreIndexedIndirect>(Operand(1), this->registers.X));
    return 6;
}

u8 Cpu::CPY_ZP()
{
    CPY(nes.ram.Get<Mode::ZeroPage>(Operand(1)));
    return 3;
}

u8 Cpu::CMP_ZP()
{
    CMP(nes.ram.Get<Mode::ZeroPage>(Operand(1)));
    return 3;
}

u8 Cpu::DEC_ZP()
{
    DEC(nes.ram.Get<Mode::ZeroPage>(Operand(1)));
    return 5;
}

u8 Cpu::CMP_IMM()
{
    CMP(Operand(1));
    return 2;
}

u8 Cpu::CPY_ABS()
{
    CPY(nes.ram.Get<Mode::Absolute>(Operand(1), Operand(2)));
    return 4;
}

u8 Cpu::CMP_ABS()
{
    CMP(nes.ram.Get<Mode::Absolute>(Operand(1), Operand(2)));
    return 4;
}

u8 Cpu::DEC_ABS()
{
    DEC(nes.ram.Get<Mode::Absolute>(Operand(1), Operand(2)));
    return 6;
}

u8 Cpu::CMP_IND_Y()
{
    CMP(nes.ram.Get<Mode::PostIndexedIndirect>(Operand(1), this->registers.Y));
    auto address = nes.ram.Indirect(Operand(1), 0);
    return 5 + NesMemory::IsPageCrossed(address, address + this->registers.Y);
}

u8 Cpu::CMP_ZP_X()
{
    CMP(nes.ram.Get<Mode::ZeroPageIndexed>(Operand(1), this->registers.X));
    return 4;
}

u8 Cpu::DEC_ZP_X()
{
    DEC(nes.ram.Get<Mode::ZeroPageIndexed>(Operand(1), this->registers.X));
    return 6;
}

u8 Cpu::CMP_ABS_Y()
{
    CMP(nes.ram.Get<Mode::Indexed>(Operand(1), Operand(2), this->registers.Y));
    auto address = NesMemory::FromValues(Operand(1), Operand(2));
    return 4 + NesMemory::IsPageCrossed(address, address + this->registers.Y);
}

u8 Cpu::CMP_ABS_X()
{
    CMP(nes.ram.Get<Mode::Indexed>(Operand(1), Operand(2), this->registers.X));
    auto address = NesMemory::FromValues(Operand(1), Operand(2));
    return 4 + NesMemory::IsPageCrossed(address, address + this->registers.X);
}

u8 Cpu::DEC_ABS_X()
{
    DEC(nes.ram.Get<Mode::Indexed>(Operand(1), Operand(2), this->registers.X));
    return 7;
}

u8 Cpu::CPX_IMM()
{
    CPX(Operand(1));
    return 2;
}

u8 Cpu::SBC_IND_X()
{
    SBC(nes.ram.Get<Mode::PreIndexedIndirect>(Operand(1), this->registers.X));
    return 6;
}

u8 Cpu::CPX_ZP()
{
    CPX(nes.ram.Get<Mode::ZeroPage>(Operand(1)));
    return 3;
}

u8 Cpu::SBC_ZP()
{
    SBC(nes.ram.Get<Mode::ZeroPage>(Operand(1)));
    return 3;
}

u8 Cpu::INC_ZP()
{
    INC(nes.ram.Get<Mode::ZeroPage>(Operand(1)));
    return 5;
}

u8 Cpu::SBC_IMM()
{
    SBC(Operand(1));
    return 2;
}

u8 Cpu::CPX_ABS()
{
    CPX(nes.ram.Get<Mode::Absolute>(Operand(1), Operand(2)));
    return 4;
}

u8 Cpu::SBC_ABS()
{
    SBC(nes.ram.Get<Mode::Absolute>(Operand(1), Operand(2)));
    return 4;
}

u8 Cpu::INC_ABS()
{
    INC(nes.ram.Get<Mode::Absolute>(Operand(1), Operand(2)));
    return 6;
}

u8 Cpu::SBC_IND_Y()
{
    SBC(nes.ram.Get<Mode::PostIndexedIndirect>(Operand(1), this->registers.Y));
    auto address = nes.ram.Indirect(Operand(1), 0);
    return 5 + NesMemory::IsPageCrossed(address, address + this->registers.Y);
}

u8 Cpu::SBC_ZP_X()
{
    SBC(nes.ram.Get<Mode::ZeroPageIndexed>(Operand(1), this->registers.X));
    return 4;
}

u8 Cpu::INC_ZP_X()
{
    INC(nes.ram.Get<Mode::ZeroPageIndexed>(Operand(1), this->registers.X));
    return 6;
}

u8 Cpu::SBC_ABS_Y()
{
    SBC(nes.ram.Get<Mode::Indexed>(Operand(1), Operand(2), this->registers.Y));
    auto address = NesMemory::FromValues(Operand(1), Operand(2));
    return 4 + NesMemory::IsPageCrossed(address, address + this->registers.Y);
}

u8 Cpu::SBC_ABS_X()
{
    SBC(nes.ram.Get<Mode::Indexed>(Operand(1), Operand(2), this->registers.X));
    auto address = NesMemory::FromValues(Operand(1), Operand(2));
    return 4 + NesMemory::IsPageCrossed(address, address + this->registers.X);
}

u8 Cpu::INC_ABS_X()
{
    INC(nes.ram.Get<Mode::Indexed>(Operand(1), Operand(2), this->registers.X));
    return 7;
}
