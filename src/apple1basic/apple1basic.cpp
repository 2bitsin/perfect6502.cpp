#include <cassert>
#include <cstdio>
//#ifndef _WIN32
#include <sys/stat.h>
#include <io.h>

#include "../netlist_6502.hpp"

/************************************************************
 *
 * Interface to OS Library Code / Monitor
 *
 ************************************************************/

/* imported by runtime.c */
unsigned char A, X, Y, S, P;
unsigned short PC;
int N, Z, C;

void
init_monitor (netlist_6502& nlsym)
{
	auto memory = nlsym.memory();

	FILE* f;
	f = fopen ("apple1basic.bin", "r");
	fread (&memory[0xE000], 1, 4096, f);
	fclose (f);

	memory [0xfffc] = 0x00;
	memory [0xfffd] = 0xE0;

}


void
charout (netlist_6502& nlsym, char ch)
{
	auto memory = nlsym.memory();

	unsigned char S = (uint8_t)nlsym.get(nlsym.reg_s);
	unsigned short a = 1 + memory [0x0100 + S + 1] | memory [0x0100 + ((S + 2) & 0xFF)] << 8;

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
	struct state_t* state = nlsym.state.get ();
	if(nlsym.get (nlsym.bus_rw))
	{
		unsigned short a = nlsym.get (nlsym.bus_addr);
		if ((a & 0xFF1F) == 0xD010)
		{
			unsigned char c = getchar ();
			if (c == 10)
				c = 13;
			c |= 0x80;
			nlsym.set (nlsym.bus_data, c);
		}
		if ((a & 0xFF1F) == 0xD011)
		{
			if (nlsym.get (nlsym.reg_pc) == 0xE006)
				/* if the code is reading a character, we have one ready */
				nlsym.set (nlsym.bus_data, 0x80);
			else
				/* if the code checks for a STOP condition, nothing is pressed */
				nlsym.set (nlsym.bus_data, 0);
		}
		if ((a & 0xFF1F) == 0xD012)
		{
/* 0x80 would mean we're not yet ready to receive a character */
			nlsym.set (nlsym.bus_data, 0);
		}
	}
	else
	{
		auto a = nlsym.get (nlsym.bus_addr);
		auto d = (uint8_t)nlsym.get (nlsym.bus_data);
		if ((a & 0xFF1F) == 0xD012)
		{
			unsigned char temp8 = d & 0x7F;
			if (temp8 == 13)
				temp8 = 10;
			charout (nlsym, temp8);
		}
	}
}

int
main ()
{
	int clk = 0;

	netlist_6502 nlsym;

	/* set up memory for user program */
	init_monitor (nlsym);

	/* emulate the 6502! */
	for (;;)
	{
		nlsym.step ();
		clk = !clk;
		if (!clk)
			handle_monitor (nlsym);

	};
}
