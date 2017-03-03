#include <gtest/gtest.h>
#include "common.h"
#include <cpu.h>
#include <memory.h>

using namespace Frankenstein;

TEST_F(CPUTest, DefaultState) {
    EXPECT_EQ(0, cpu.registers.A);
    EXPECT_EQ(0, cpu.registers.X);
    EXPECT_EQ(0, cpu.registers.Y);
    EXPECT_EQ(0b00100000, cpu.registers.P);
    EXPECT_EQ(0, cpu.registers.PC);
    EXPECT_EQ(0xFF, cpu.registers.SP);
    EXPECT_EQ(0, cpu.Get<Cpu::Flags::Z>());
    EXPECT_EQ(0, cpu.Get<Cpu::Flags::S>());
    EXPECT_EQ(0, cpu.Get<Cpu::Flags::V>());
    EXPECT_EQ(0, cpu.Get<Cpu::Flags::C>());
}

////////////////////////////////////////////////////////////////////////////////
// AND Tests
////////////////////////////////////////////////////////////////////////////////

TEST_F(CPUTest, AND_ZeroAndOne) {
    cpu.registers.A = 0;
    u8 value = 1;
    cpu.AND(value);
    EXPECT_EQ(0, cpu.registers.A);
    EXPECT_EQ(true,  cpu.Get<Cpu::Flags::Z>());
    EXPECT_EQ(false, cpu.Get<Cpu::Flags::S>());
}

TEST_F(CPUTest, AND_OneAndZero) {
    cpu.registers.A = 1;
    u8 value = 0;
    cpu.AND(value);
    EXPECT_EQ(0, cpu.registers.A);
    EXPECT_EQ(true,  cpu.Get<Cpu::Flags::Z>());
    EXPECT_EQ(false, cpu.Get<Cpu::Flags::S>());
}

TEST_F(CPUTest, AND_OneAndOne) {
    cpu.registers.A = 1;
    u8 value = 1;
    cpu.AND(value);    
    EXPECT_EQ(1, cpu.registers.A);
    EXPECT_EQ(false, cpu.Get<Cpu::Flags::Z>());
    EXPECT_EQ(false, cpu.Get<Cpu::Flags::S>());
}

TEST_F(CPUTest, AND_NegativeResult) {
    cpu.registers.A = 0xFF;
    u8 value = 0xF0;
    cpu.AND(value);    
    EXPECT_EQ(value, cpu.registers.A);
    EXPECT_EQ(false, cpu.Get<Cpu::Flags::Z>());
    EXPECT_EQ(true,  cpu.Get<Cpu::Flags::S>());
}

////////////////////////////////////////////////////////////////////////////////
// ADC Tests
////////////////////////////////////////////////////////////////////////////////

TEST_F(CPUTest, ADC_ZeroAndZero)
{
    cpu.registers.A = 0;
    u8 value = 0;
    cpu.ADC(value);
    EXPECT_EQ(0,     cpu.registers.A);
    EXPECT_EQ(true,  cpu.Get<Cpu::Flags::Z>());
    EXPECT_EQ(false, cpu.Get<Cpu::Flags::S>());
    EXPECT_EQ(false, cpu.Get<Cpu::Flags::V>());
    EXPECT_EQ(false, cpu.Get<Cpu::Flags::C>());
}

TEST_F(CPUTest, ADC_ZeroAndOne)
{
    cpu.registers.A = 0;
    u8 value = 1;
    cpu.ADC(value);
    EXPECT_EQ(1, cpu.registers.A);
    EXPECT_EQ(false, cpu.Get<Cpu::Flags::Z>());
    EXPECT_EQ(false, cpu.Get<Cpu::Flags::S>());
    EXPECT_EQ(false, cpu.Get<Cpu::Flags::V>());
    EXPECT_EQ(false, cpu.Get<Cpu::Flags::C>());
}

TEST_F(CPUTest, ADC_ZeroAndOne)
{
    cpu.registers.A = 0;
    u8 value = 1;
    cpu.ADC(value);
    EXPECT_EQ(1, cpu.registers.A);
    EXPECT_EQ(false, cpu.Get<Cpu::Flags::Z>());
    EXPECT_EQ(false, cpu.Get<Cpu::Flags::S>());
    EXPECT_EQ(false, cpu.Get<Cpu::Flags::V>());
    EXPECT_EQ(false, cpu.Get<Cpu::Flags::C>());
}

////////////////////////////////////////////////////////////////////////////////
// ASL Tests
////////////////////////////////////////////////////////////////////////////////