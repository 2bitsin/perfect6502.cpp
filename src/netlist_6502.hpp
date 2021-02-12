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

#include <memory>
#include <span>

struct netlist_6502
{
	netlist_6502();
 ~netlist_6502();
	
	netlist_6502 (const netlist_6502&) = delete;
	netlist_6502& operator = (const netlist_6502&) = delete;
	netlist_6502 (netlist_6502&&) = default;
	netlist_6502& operator = (netlist_6502&&) = default;

	void eval();

	auto address	() const -> std::uint16_t;
	auto data			() const -> std::uint8_t;
	auto clock		() const -> bool;
	auto ready		() const -> bool;
	auto nmi			() const -> bool;
	auto irq			() const -> bool;
	auto reset		() const -> bool;
	auto read			() const -> bool;
	auto sync			() const -> bool;
	auto so				() const -> bool;
	auto a				() const -> std::uint8_t;
	auto x				() const -> std::uint8_t;
	auto y				() const -> std::uint8_t;
	auto s				() const -> std::uint8_t;
	auto p				() const -> std::uint8_t;
	auto pc				() const -> std::uint16_t;							 
	auto pch			() const -> std::uint8_t;
	auto pcl			() const -> std::uint8_t;
	auto ir				() const -> std::uint8_t;

	void address	(std::uint16_t);
	void data			(std::uint8_t);
	void clock		(bool);
	void ready		(bool);
	void nmi			(bool);
	void irq			(bool);
	void reset		(bool);
	void read			(bool);
	void sync			(bool);
	void so				(bool);
	void a				(std::uint8_t);
	void x				(std::uint8_t);
	void y				(std::uint8_t); 
	void s				(std::uint8_t);
	void p				(std::uint8_t);
	void pc				(std::uint16_t);							 
	void pch			(std::uint8_t);
	void pcl			(std::uint8_t);
	void ir				(std::uint8_t);

private:

	std::unique_ptr<struct state_type> state;
};



