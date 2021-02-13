/*
 Copyright (c) 2010,2014,2021 Michael Steil, Brian Silverman, Barry Silverman, Aleksandr Ševčenko

 Permission is hereby granted, free of charge, to any person obtaining a copy
 of this software and associated documentation files (the "Software"), to deal
 in the Software without restriction, including without limitation the rights
 to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 copies of the Software, and to permit persons to whom the Software is
 furnished to do so, subject to the following conditions:

 The above copyright notice and this permission notice shall be included in
 all copies or substantial portions of the Software.

 THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
 THE SOFTWARE.
*/

#include <cassert>
#include <cstdio>
#include <sys/stat.h>

#ifdef _WIN32
#include <io.h>
#endif

#include "../netlist_6502.hpp"
#include "apple1_basic_bin.hpp"
#include "../utils/array_list.hpp"

static std::uint8_t memory [0x10000];
static netlist_6502 nlsym;

void
charout (netlist_6502& nlsym, char ch)
{
	auto S = (uint8_t)nlsym.s();
	auto a = 1 + memory [0x0100 + S + 1] | memory [0x0100 + ((S + 2) & 0xFF)] << 8;

	/*
	 * Apple I BASIC prints every character received
	 * from the terminal. UNIX terminals do this
	 * anyway, so we have to avoid printing every
	 * line again
	 */
	if (a == 0xe2a6)	/* character echo */
		return;
	if (a == 0xe2b6)	/* CR echo */
		return;

	/*
	 * Apple I BASIC prints a line break and 6 spaces
	 * after every 37 characters. UNIX terminals do
	 * line breaks themselves, so ignore these
	 * characters
	 */
	if (a == 0xe025 && (ch == 10 || ch == ' '))
		return;

	/* INPUT */
	if (a == 0xe182)
	{
	#if _WIN32
		if (!isatty (0))
			return;
	#else
		struct stat st;
		fstat (0, &st);
		if (S_ISFIFO (st.st_mode))
			return;
	#endif
	}
//#endif

	putc (ch, stdout);
	fflush (stdout);
}

void
handle_monitor (netlist_6502& nlsym)
{
	auto a = nlsym.address();
	if (nlsym.read())
	{
		nlsym.data(memory [a]);
		if ((a & 0xFF1F) == 0xD010)
		{
			auto c = getchar ();
			if (c == 10)
				c = 13;
			c |= 0x80;
			nlsym.data(c);
		}
		if ((a & 0xFF1F) == 0xD011)
		{
			if (nlsym.pc() == 0xE006)
				/* if the code is reading a character, we have one ready */
				nlsym.data(0x80);
			else
				/* if the code checks for a STOP condition, nothing is pressed */
				nlsym.data(0);
		}
		if ((a & 0xFF1F) == 0xD012)
			/* 0x80 would mean we're not yet ready to receive a character */
			nlsym.data(0);
	}
	else
	{
		auto d = (uint8_t)nlsym.data();
		memory [a] = d;
		if ((a & 0xFF1F) == 0xD012)
		{
			auto temp8 = d & 0x7F;
			if (temp8 == 13)
				temp8 = 10;
			charout (nlsym, temp8);
		}
	}
}

void step (netlist_6502& nlsym)
{
	auto clk = nlsym.clock();
	nlsym.clock(!clk);
	nlsym.eval ();
	if (!clk)
		handle_monitor (nlsym);
}

static void
init_monitor (netlist_6502& nlsym)
{
	std::memset (memory, 0, sizeof (memory));

	std::memcpy (&memory [0xE000], apple1_basic_bin, sizeof(apple1_basic_bin));
	memory [0xfffc] = 0x00;
	memory [0xfffd] = 0xE0;

	/* hold RESET for 8 cycles */
	for (int i = 0; i < 16; i++)
		step (nlsym);
	nlsym.reset(1);
	/* release RESET */
	step (nlsym);
}

#if 1
int main ()
{
	// set up memory for user program 
	init_monitor (nlsym);

	// emulate the 6502! 
	for (;;)
		step (nlsym);
}
#endif