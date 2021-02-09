#include <memory>
#include <span>

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
		bus_reset,
		bus_rw,
		bus_sync,
		bus_ready,
		bus_clock
	};

	netlist_6502();
 ~netlist_6502();

	void step();

	auto get (bits) const -> uint16_t;
	void set (bits, uint16_t);
	auto memory() -> std::span<std::uint8_t>;


	std::unique_ptr<struct state_type> state;
};


