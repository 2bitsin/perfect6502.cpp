#include <memory>

struct netlist_6502
{
	enum bits
	{
		reg_a,
		reg_x,
		reg_y,
		reg_s,
		reg_p,
		reg_pc,
		reg_pcl,
		reg_pch,
		reg_ir,
		bus_addr,
		bus_data,
		bus_irq,
		bus_nmi,
		bus_res,
		bus_rw
	};

	netlist_6502();
 ~netlist_6502();

	void step();

	auto get (bits) const -> uint16_t;
	void set (bits, uint16_t);

	std::unique_ptr<struct state_t> state;
};

void chipStatus(struct state_t *state);
unsigned short readPC(struct state_t *state);
unsigned char readA(struct state_t *state);
unsigned char readX(struct state_t *state);
unsigned char readY(struct state_t *state);
unsigned char readSP(struct state_t *state);
unsigned char readP(struct state_t *state);
unsigned int readRW(struct state_t *state);
unsigned char readIR(struct state_t *state);

