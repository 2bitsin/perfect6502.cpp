#include <stdio.h>
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
init_monitor(state_t* state)
{
	auto* memory = (uint8_t*)state;

	FILE *f;
	f = fopen("apple1basic.bin", "r");
	fread(memory + 0xE000, 1, 4096, f);
	fclose(f);

	memory[0xfffc] = 0x00;
	memory[0xfffd] = 0xE0;

}


void
charout(struct state_t* state, char ch) {
	auto* memory = (uint8_t*)state;

	unsigned char S = readSP(state);
	unsigned short a = 1 + memory[0x0100+S+1] | memory[0x0100+((S+2) & 0xFF)] << 8;

	/*
	 * Apple I BASIC prints every character received
	 * from the terminal. UNIX terminals do this
	 * anyway, so we have to avoid printing every
	 * line again
	 */
	if (a==0xe2a6)	/* character echo */
		return;
	if (a==0xe2b6)	/* CR echo */
		return;

	/*
	 * Apple I BASIC prints a line break and 6 spaces
	 * after every 37 characters. UNIX terminals do
	 * line breaks themselves, so ignore these
	 * characters
	 */
	if (a==0xe025 && (ch==10 || ch==' '))
		return;

	/* INPUT */
	if (a==0xe182) {
#if _WIN32
		if (!isatty(0))
			return;
#else
		struct stat st;
		fstat(0, &st);
		if (S_ISFIFO (st.st_mode))
			return;
#endif
	}
//#endif

	putc(ch, stdout);
	fflush(stdout);
}

void
handle_monitor(netlist_6502& nlsfo2)
{
	 struct state_t* state = nlsfo2.state.get();

	if (readRW(state)) {
		unsigned short a = nlsfo2.get(nlsfo2.bus_addr);
		if ((a & 0xFF1F) == 0xD010) {
			unsigned char c = getchar();
			if (c == 10)
				c = 13;
			c |= 0x80;
			nlsfo2.set(nlsfo2.bus_data, c);
		}
		if ((a & 0xFF1F) == 0xD011) {
			if (readPC(state) == 0xE006)
				/* if the code is reading a character, we have one ready */
				nlsfo2.set(nlsfo2.bus_data, 0x80);
			else
				/* if the code checks for a STOP condition, nothing is pressed */
				nlsfo2.set(nlsfo2.bus_data, 0);
		}
		if ((a & 0xFF1F) == 0xD012) {
			/* 0x80 would mean we're not yet ready to receive a character */
			nlsfo2.set(nlsfo2.bus_data, 0);
		}
	} else {
		unsigned short a = nlsfo2.get(nlsfo2.bus_addr);
		unsigned char d = nlsfo2.get(nlsfo2.bus_data);
		if ((a & 0xFF1F) == 0xD012) {
			unsigned char temp8 = d & 0x7F;
			if (temp8 == 13)
				temp8 = 10;
			charout(state, temp8);
		}
	}
}

int
main()
{
	int clk = 0;

	netlist_6502 nlso2;

	/* set up memory for user program */
	init_monitor(nlso2.state.get());

	/* emulate the 6502! */
	for (;;) {
		nlso2.step();
		clk = !clk;
		if (!clk)
			handle_monitor(nlso2);

		//chipStatus(state);
		//if (!(cycle % 1000)) printf("%d\n", cycle);
	};
}
