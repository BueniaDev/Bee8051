/*
    This file is part of Bee8051.
    Copyright (C) 2022 BueniaDev.

    Bee8051 is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    Bee8051 is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with Bee8051.  If not, see <https://www.gnu.org/licenses/>.
*/

#ifndef BEE8051_H
#define BEE8051_H

#include <iostream>
#include <sstream>
#include <cstdint>
#include <vector>
#include <array>
#include <cassert>
using namespace std;

namespace bee8051
{
    class Bee8051Interface
    {
	public:
	    Bee8051Interface();
	    ~Bee8051Interface();

	    virtual uint8_t readROM(uint16_t addr)
	    {
		cout << "Reading ROM from address of " << hex << int(addr) << endl;
		exit(0);
		return 0;
	    }
    };

    class BeeMCS51
    {
	public:
	    BeeMCS51(int prog_width, int data_width);
	    ~BeeMCS51();

	    virtual void init();
	    virtual void shutdown();

	    int runinstruction();

	    void debugoutput(bool print_disassembly = false);

	    void setInterface(Bee8051Interface *cb);

	private:
	    template<typename T>
	    bool testbit(T reg, int bit)
	    {
		return ((reg >> bit) & 1) ? true : false;
	    }

	    template<typename T>
	    T setbit(T reg, int bit)
	    {
		return (reg | (1 << bit));
	    }

	    template<typename T>
	    T resetbit(T reg, int bit)
	    {
		return (reg & ~(1 << bit));
	    }

	    template<typename T>
	    T changebit(T reg, int bit, bool is_set)
	    {
		if (is_set)
		{
		    return setbit(reg, bit);
		}
		else
		{
		    return resetbit(reg, bit);
		}
	    }

	    Bee8051Interface *inter = NULL;

	    int program_width = 0;
	    int data_bus_width = 0;

	    uint8_t readROM(uint16_t addr);

	    int executeinstr(uint8_t instr);

	    void unrecognizedinstr(uint8_t instr);

	    vector<uint8_t> internal_ram;
	    array<uint8_t, 0x100> sfr_ram;

	    uint8_t getReg(int reg)
	    {
		reg &= 7;
		return readRAM(reg);
	    }

	    void setReg(int reg, uint8_t data)
	    {
		reg &= 7;
		writeRAM(reg, data);
	    }

	    uint8_t getAccum()
	    {
		return readRAM(0x1E0);
	    }

	    void setAccum(uint8_t data)
	    {
		writeRAM(0x1E0, data);
	    }

	    uint8_t getPSW()
	    {
		return readRAM(0x1D0);
	    }

	    void setPSW(uint8_t data)
	    {
		writeRAM(0x1D0, data);
	    }

	    void changePSWBit(int bit, bool is_set)
	    {
		setPSW(changebit(getPSW(), bit, is_set));
	    }

	    void setParity(bool is_set)
	    {
		changePSWBit(0, is_set);
	    }

	    void setCarry(bool is_set)
	    {
		changePSWBit(7, is_set);
	    }

	    void setHalf(bool is_set)
	    {
		changePSWBit(6, is_set);
	    }

	    void setOverflow(bool is_set)
	    {
		changePSWBit(2, is_set);
	    }

	    void calcParity()
	    {
		uint8_t parity = 0;
		uint8_t accum = getAccum();

		for (int i = 0; i < 8; i++)
		{
		    parity ^= testbit(accum, 0);
		    accum >>= 1;
		}

		setParity(testbit(parity, 0));
	    }

	    uint8_t readRAM(uint16_t addr)
	    {
		uint8_t data = 0;

		int ram_addr = (1 << data_bus_width);

		if (addr < ram_addr)
		{
		    data = internal_ram.at(addr);
		}
		else if ((addr >= 0x100) && (addr < 0x200))
		{
		    data = sfr_ram.at(addr & 0xFF);
		}

		return data;
	    }

	    void writeRAM(uint16_t addr, uint8_t data)
	    {
		int ram_addr = (1 << data_bus_width);

		if (addr < ram_addr)
		{
		    internal_ram.at(addr) = data;
		}
		else if ((addr >= 0x100) && (addr < 0x200))
		{
		    sfr_ram.at(addr & 0xFF) = data;
		}
	    }

	    uint8_t readIRAM(uint8_t addr)
	    {
		uint8_t data = 0;
		if (addr < 0x80)
		{
		    data = readRAM(addr);
		}
		else
		{
		    data = readRAM(0x100 | addr);
		}

		return data;
	    }

	    uint8_t add_internal(uint8_t accum, uint8_t data, bool is_carry = false)
	    {
		uint16_t result = (accum + data + is_carry);
		uint16_t carry_reg = (accum ^ data ^ result);

		bool is_cf = testbit(carry_reg, 8);
		bool is_ov = (testbit(carry_reg, 7) != is_cf);
		bool is_ac = testbit(carry_reg, 4);

		setCarry(is_cf);
		setHalf(is_ac);
		setOverflow(is_ov);
		return uint8_t(result);
	    }

	    uint8_t add_accum(uint8_t data)
	    {
		return add_internal(getAccum(), data);
	    }

	    uint16_t pc = 0;
    };

    class Bee8051 : public BeeMCS51
    {
	public:
	    Bee8051() : BeeMCS51(12, 7)
	    {

	    }

	    void init()
	    {
		BeeMCS51::init();
		cout << "Bee8051::Initialized" << endl;
	    }

	    void shutdown()
	    {
		BeeMCS51::shutdown();
		cout << "Bee8051::Shutting down..." << endl;
	    }
    };

    class Bee8751 : public BeeMCS51
    {
	public:
	    Bee8751() : BeeMCS51(12, 7)
	    {

	    }

	    void init()
	    {
		BeeMCS51::init();
		cout << "Bee8751::Initialized" << endl;
	    }

	    void shutdown()
	    {
		BeeMCS51::shutdown();
		cout << "Bee8751::Shutting down..." << endl;
	    }
    };
};


#endif // BEE8051_H