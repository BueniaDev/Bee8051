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
#include <unordered_map>
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

	    virtual uint8_t portIn(int port)
	    {
		port &= 3;
		cout << "Reading value from port of " << dec << int(port) << endl;
		exit(0);
		return 0;
	    }

	    virtual void portOut(int port, uint8_t data)
	    {
		port &= 3;
		cout << "Writing value of " << hex << int(data) << " to port of " << dec << int(port) << endl;
		exit(0);
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

	    void debugoutput(bool print_disassembly = true);
	    size_t disassembleinstr(ostream &stream, uint32_t pc);

	    void setInterface(Bee8051Interface *cb);

	protected:
	    virtual string get_sfr_names(uint16_t addr)
	    {
		stringstream ss;
		auto i = mem_names.find(addr);

		if (i == mem_names.end())
		{
		    ss << "$" << hex << int(addr);
		}
		else
		{
		    ss << i->second;
		}

		return ss.str();
	    }

	    string get_bit_addr(uint8_t addr)
	    {
		stringstream ss;
		if (addr < 0x80)
		{
		    ss << "$" << hex << int((addr >> 3) | 0x20) << "." << dec << int(addr & 0x7);
		}
		else
		{
		    auto i = mem_names.find((addr | 0x100));

		    if (i == mem_names.end())
		    {
			i = mem_names.find((addr & 0xF8));

			if (i == mem_names.end())
			{
			    ss << "$" << hex << int((addr & 0xF8)) << "." << dec << int(addr & 0x7);
			}
			else
			{
			    ss << i->second << "." << dec << int((addr & 0x7));
			}
		    }
		    else
		    {
			ss << i->second;
		    }
		}

		return ss.str();
	    }

	    struct meminfo
	    {
		int addr = 0;
		string name = "";
	    };

	    void add_names(vector<meminfo> info)
	    {
		for (size_t i = 0; info[i].addr >= 0; i++)
		{
		    mem_names[info[i].addr] = info[i].name;
		}
	    }

	    template<typename ...Names>
	    void add_names(vector<meminfo> info, Names &&... names)
	    {
		add_names(names...);
		add_names(info);
	    }

	    vector<meminfo> default_names = 
	    {
		{0x81, "sp"},
		{0x1B6, "wr"},
		{0x1B7, "rd"},
		{-1, ""},
	    };

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
	    uint8_t portIn(int port);
	    void portOut(int port, uint8_t data);

	    int executeinstr(uint8_t instr);

	    void unrecognizedinstr(uint8_t instr);

	    vector<uint8_t> internal_ram;
	    array<uint8_t, 0x100> sfr_ram;

	    uint8_t getReg(int reg)
	    {
		reg &= 7;
		int addr = ((getPSW() & 0x18) | reg);
		return readRAM(addr);
	    }

	    void setReg(int reg, uint8_t data)
	    {
		reg &= 7;
		int addr = ((getPSW() & 0x18) | reg);
		writeRAM(addr, data);
	    }

	    uint8_t getAccum()
	    {
		return readSFR(0xE0);
	    }

	    void setAccum(uint8_t data)
	    {
		writeSFR(0xE0, data);
	    }

	    uint8_t getPSW()
	    {
		return readSFR(0xD0);
	    }

	    void setPSW(uint8_t data)
	    {
		writeSFR(0xD0, data);
	    }

	    uint8_t getSP()
	    {
		return readSFR(0x81);
	    }

	    void setSP(uint8_t data)
	    {
		writeSFR(0x81, data);
	    }

	    uint8_t getP0()
	    {
		return readRAM(0x180);
	    }

	    void setP0(uint8_t data)
	    {
		writeSFR(0x80, data);
	    }

	    uint8_t getP1()
	    {
		return readRAM(0x190);
	    }

	    void setP1(uint8_t data)
	    {
		writeSFR(0x90, data);
	    }

	    uint8_t getP2()
	    {
		return readRAM(0x1A0);
	    }

	    void setP2(uint8_t data)
	    {
		writeSFR(0xA0, data);
	    }

	    uint8_t getP3()
	    {
		return readRAM(0x1B0);
	    }

	    void setP3(uint8_t data)
	    {
		writeSFR(0xB0, data);
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
		    data = readSFR(addr);
		}

		return data;
	    }

	    void writeIRAM(uint8_t addr, uint8_t data)
	    {
		if (addr < 0x80)
		{
		    writeRAM(addr, data);
		}
		else
		{
		    writeSFR(addr, data);
		}
	    }

	    uint8_t readIRAMIndirect(uint8_t addr)
	    {
		uint8_t data = 0xFF;
		int ram_addr = (1 << data_bus_width);

		if (addr < ram_addr)
		{
		    data = readRAM(addr);
		}

		return data;
	    }

	    void writeIRAMIndirect(uint8_t addr, uint8_t data)
	    {
		int ram_addr = (1 << data_bus_width);

		if (addr < ram_addr)
		{
		    writeRAM(addr, data);
		}
	    }

	    uint8_t readSFR(uint8_t addr)
	    {
		uint8_t data = 0xFF;

		switch (addr)
		{
		    case 0xB0:
		    {
			uint8_t port3_val = readRAM(0x1B0);

			if (is_rwm)
			{
			    data = port3_val;
			}
			else
			{
			    data = (port3_val & portIn(3));
			}
		    }
		    break;
		    case 0x81:
		    case 0x88:
		    case 0xD0:
		    case 0xE0:
		    {
			data = readRAM(addr | 0x100);
		    }
		    break;
		    default:
		    {
			cout << "Invalid/unimplemented read from SFR address of " << hex << int(addr) << endl;
		    }
		    break;
		}

		return data;
	    }

	    void writeSFR(uint8_t addr, uint8_t data)
	    {
		if (addr < 0x80)
		{
		    return;
		}

		switch (addr)
		{
		    case 0xB0: portOut(3, data); break;
		    case 0x81:
		    case 0x88: break;
		    case 0xD0:
		    case 0xE0: break;
		    default:
		    {
			cout << "Invalid/unimplemented write to SFR address of " << hex << int(addr) << endl;
		    }
		    break;
		}

		writeRAM((addr | 0x100), data);
	    }

	    void writeBit(uint8_t addr, bool is_set)
	    {
		bool is_sfr = (addr >= 0x80);

		int distance = (is_sfr) ? 8 : 1;
		int offs = (is_sfr) ? 0x80 : 0x20;
		int word = ((addr & 0x78) >> 3) * distance + offs;
		int bit_pos = (addr & 0x7);

		cout << "Reading bit at address of " << hex << int(word) << endl;
		uint8_t result = readIRAM(word);
		result = changebit(result, bit_pos, is_set);
		writeIRAM(word, result);
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
	    bool is_rwm = false;

	    unordered_map<uint16_t, string> mem_names;
    };

    class Bee8051 : public BeeMCS51
    {
	public:
	    Bee8051() : BeeMCS51(12, 7)
	    {

	    }

	    virtual void init()
	    {
		BeeMCS51::init();
		add_names(default_names);
		cout << "Bee8051::Initialized" << endl;
	    }

	    virtual void shutdown()
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
		add_names(default_names);
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