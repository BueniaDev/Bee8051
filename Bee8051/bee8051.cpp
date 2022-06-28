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

#include "bee8051.h"
using namespace bee8051;

namespace bee8051
{
    Bee8051Interface::Bee8051Interface()
    {

    }

    Bee8051Interface::~Bee8051Interface()
    {

    }

    BeeMCS51::BeeMCS51(int prog_width, int data_width) : program_width(prog_width), data_bus_width(data_width)
    {
    }

    BeeMCS51::~BeeMCS51()
    {

    }

    void BeeMCS51::init()
    {
	pc = 0;
	setPSW(0);
	setAccum(0);
	int ram_size = (1 << data_bus_width);
	internal_ram.resize(ram_size, 0);
    }

    void BeeMCS51::shutdown()
    {
	internal_ram.clear();
    }

    int BeeMCS51::runinstruction()
    {
	// TODO: Implement other components (i.e. IRQs, serial, timers, etc.)
	uint8_t instr = readROM(pc++);

	int inst_cycles = executeinstr(instr);

	int cycles = (inst_cycles * 12);
	return cycles;
    }

    void BeeMCS51::debugoutput(bool print_disassembly)
    {
	cout << "PC: " << hex << int(pc) << endl;
	// cout << "SP: " << hex << int(sp) << endl;
	cout << "PSW: " << hex << int(getPSW()) << endl;
	cout << "A: " << hex << int(getAccum()) << endl;
	// cout << "B: " << hex << int(regb) << endl;
	// cout << "DPTR: " << hex << int(regdptr) << endl;
	// cout << "IE: " << hex << int(regie) << endl;
	// cout << "IP: " << hex << int(regip) << endl;
	/*
	for (int i = 0; i < 4; i++)
	{
	    cout << "P" << dec << int(i) << ": " << hex << int(getPReg(i)) << endl;
	}
	*/

	for (int i = 0; i < 8; i++)
	{
	    cout << "R" << dec << int(i) << ": " << hex << int(getReg(i)) << endl;
	}

	// cout << "RB: " << hex << int(regrb) << endl;
	// cout << "TCON: " << hex << int(regtcon) << endl;
	// cout << "TMOD: " << hex << int(tmod) << endl;
	// cout << "TL0: " << hex << int(regtl0) << endl;
	// cout << "TH0: " << hex << int(regth0) << endl;
	// cout << "TL1: " << hex << int(regtl1) << endl;
	// cout << "TH1: " << hex << int(regth1) << endl;
	cout << endl;
    }

    int BeeMCS51::executeinstr(uint8_t instr)
    {
	int cycles = 1;

	switch (instr)
	{
	    case 0x02:
	    {
		uint8_t high = readROM(pc++);
		uint8_t low = readROM(pc++);
		uint16_t addr = ((high << 8) | low);
		pc = addr;
		cycles = 2;
	    }
	    break; // ljmp code addr
	    case 0x25:
	    {
		uint8_t addr = readROM(pc++);
		uint8_t data = readIRAM(addr);

		setAccum(add_accum(data));
	    }
	    break; // add a, data addr
	    case 0x74:
	    {
		uint8_t data = readROM(pc++);
		setAccum(data);
	    }
	    break; // mov a, #data
	    case 0x78:
	    case 0x79:
	    case 0x7A:
	    case 0x7B:
	    case 0x7C:
	    case 0x7D:
	    case 0x7E:
	    case 0x7F:
	    {
		uint8_t data = readROM(pc++);
		int reg = (instr & 0x7);
		setReg(reg, data);
	    }
	    break; // mov r0-r7, #data
	    default: unrecognizedinstr(instr); break;
	}

	calcParity();

	return cycles;
    }

    void BeeMCS51::unrecognizedinstr(uint8_t instr)
    {
	cout << "Unrecognized instruction of " << hex << int(instr) << endl;
	exit(1);
    }

    void BeeMCS51::setInterface(Bee8051Interface *cb)
    {
	inter = cb;
    }

    uint8_t BeeMCS51::readROM(uint16_t addr)
    {
	if (program_width != 0)
	{
	    int program_mask = ((1 << program_width) - 1);
	    addr &= program_mask;
	}

	if (inter == NULL)
	{
	    return 0x00;
	}

	return inter->readROM(addr);
    }
};