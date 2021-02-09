#include <memory>

struct netlist_6502
{
	netlist_6502();
	~netlist_6502();

	std::unique_ptr<struct state_t> state;
};


void step(struct state_t *state);
void chipStatus(struct state_t *state);
unsigned short readPC(struct state_t *state);
unsigned char readA(struct state_t *state);
unsigned char readX(struct state_t *state);
unsigned char readY(struct state_t *state);
unsigned char readSP(struct state_t *state);
unsigned char readP(struct state_t *state);
unsigned int readRW(struct state_t *state);
unsigned short readAddressBus(struct state_t *state);
void writeDataBus(struct state_t *state, unsigned char);
unsigned char readDataBus(struct state_t *state);
unsigned char readIR(struct state_t *state);

extern unsigned char memory[65536];
