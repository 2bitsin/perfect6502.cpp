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

#pragma once

namespace node_names
{
	inline static constexpr const auto a0 = 0;
	inline static constexpr const auto a1 = 1;
	inline static constexpr const auto a2 = 2;
	inline static constexpr const auto a3 = 3;
	inline static constexpr const auto a4 = 4;
	inline static constexpr const auto a5 = 5;
	inline static constexpr const auto a6 = 6;
	inline static constexpr const auto a7 = 7;
	inline static constexpr const auto x0 = 8;
	inline static constexpr const auto x1 = 9;
	inline static constexpr const auto x2 = 10;
	inline static constexpr const auto x3 = 11;
	inline static constexpr const auto x4 = 12;
	inline static constexpr const auto x5 = 13;
	inline static constexpr const auto x6 = 14;
	inline static constexpr const auto x7 = 15;
	inline static constexpr const auto y0 = 16;
	inline static constexpr const auto y1 = 17;
	inline static constexpr const auto y2 = 18;
	inline static constexpr const auto y3 = 19;
	inline static constexpr const auto y4 = 20;
	inline static constexpr const auto y5 = 21;
	inline static constexpr const auto y6 = 22;
	inline static constexpr const auto y7 = 23;
	inline static constexpr const auto p0 = 24;
	inline static constexpr const auto p1 = 25;
	inline static constexpr const auto p2 = 26;
	inline static constexpr const auto p3 = 27;
	inline static constexpr const auto p4 = 28;  // brk ?
	inline static constexpr const auto p5 = 183; // vcc
	inline static constexpr const auto p6 = 30;
	inline static constexpr const auto p7 = 31;
	inline static constexpr const auto s0 = 32;
	inline static constexpr const auto s1 = 33;
	inline static constexpr const auto s2 = 34;
	inline static constexpr const auto s3 = 35;
	inline static constexpr const auto s4 = 36;
	inline static constexpr const auto s5 = 37;
	inline static constexpr const auto s6 = 38;
	inline static constexpr const auto s7 = 39;
	inline static constexpr const auto nots0 = 40;
	inline static constexpr const auto nots1 = 41;
	inline static constexpr const auto nots2 = 42;
	inline static constexpr const auto nots3 = 43;
	inline static constexpr const auto nots4 = 44;
	inline static constexpr const auto nots5 = 45;
	inline static constexpr const auto nots6 = 46;
	inline static constexpr const auto nots7 = 47;
	inline static constexpr const auto pch0 = 48;
	inline static constexpr const auto pch1 = 49;
	inline static constexpr const auto pch2 = 50;
	inline static constexpr const auto pch3 = 51;
	inline static constexpr const auto pch4 = 52;
	inline static constexpr const auto pch5 = 53;
	inline static constexpr const auto pch6 = 54;
	inline static constexpr const auto pch7 = 55;
	inline static constexpr const auto pcl0 = 56;
	inline static constexpr const auto pcl1 = 57;
	inline static constexpr const auto pcl2 = 58;
	inline static constexpr const auto pcl3 = 59;
	inline static constexpr const auto pcl4 = 60;
	inline static constexpr const auto pcl5 = 61;
	inline static constexpr const auto pcl6 = 62;
	inline static constexpr const auto pcl7 = 63;
	inline static constexpr const auto notir0 = 64;
	inline static constexpr const auto notir1 = 65;
	inline static constexpr const auto notir2 = 66;
	inline static constexpr const auto notir3 = 67;
	inline static constexpr const auto notir4 = 68;
	inline static constexpr const auto notir5 = 69;
	inline static constexpr const auto notir6 = 70;
	inline static constexpr const auto notir7 = 71;
	inline static constexpr const auto ab0 = 72;
	inline static constexpr const auto ab1 = 73;
	inline static constexpr const auto ab2 = 74;
	inline static constexpr const auto ab3 = 75;
	inline static constexpr const auto ab4 = 76;
	inline static constexpr const auto ab5 = 77;
	inline static constexpr const auto ab6 = 78;
	inline static constexpr const auto ab7 = 79;
	inline static constexpr const auto ab8 = 80;
	inline static constexpr const auto ab9 = 81;
	inline static constexpr const auto ab10 = 82;
	inline static constexpr const auto ab11 = 83;
	inline static constexpr const auto ab12 = 84;
	inline static constexpr const auto ab13 = 85;
	inline static constexpr const auto ab14 = 86;
	inline static constexpr const auto ab15 = 87;
	inline static constexpr const auto db0 = 88;
	inline static constexpr const auto db1 = 89;
	inline static constexpr const auto db2 = 90;
	inline static constexpr const auto db3 = 91;
	inline static constexpr const auto db4 = 92;
	inline static constexpr const auto db5 = 93;
	inline static constexpr const auto db6 = 94;
	inline static constexpr const auto db7 = 95;
	inline static constexpr const auto adh0 = 96;
	inline static constexpr const auto adh1 = 97;
	inline static constexpr const auto adh2 = 98;
	inline static constexpr const auto adh3 = 99;
	inline static constexpr const auto adh4 = 100;
	inline static constexpr const auto adh5 = 101;
	inline static constexpr const auto adh6 = 102;
	inline static constexpr const auto adh7 = 103;
	inline static constexpr const auto adl0 = 104;
	inline static constexpr const auto adl1 = 105;
	inline static constexpr const auto adl2 = 106;
	inline static constexpr const auto adl3 = 107;
	inline static constexpr const auto adl4 = 108;
	inline static constexpr const auto adl5 = 109;
	inline static constexpr const auto adl6 = 110;
	inline static constexpr const auto adl7 = 111;
	inline static constexpr const auto alu0 = 112;
	inline static constexpr const auto alu1 = 113;
	inline static constexpr const auto alu2 = 114;
	inline static constexpr const auto alu3 = 115;
	inline static constexpr const auto alu4 = 116;
	inline static constexpr const auto alu5 = 117;
	inline static constexpr const auto alu6 = 118;
	inline static constexpr const auto alu7 = 119;
	inline static constexpr const auto pd0 = 120;
	inline static constexpr const auto pd1 = 121;
	inline static constexpr const auto pd2 = 122;
	inline static constexpr const auto pd3 = 123;
	inline static constexpr const auto pd4 = 124;
	inline static constexpr const auto pd5 = 125;
	inline static constexpr const auto pd6 = 126;
	inline static constexpr const auto pd7 = 127;
	inline static constexpr const auto sb0 = 128;
	inline static constexpr const auto sb1 = 129;
	inline static constexpr const auto sb2 = 130;
	inline static constexpr const auto sb3 = 131;
	inline static constexpr const auto sb4 = 132;
	inline static constexpr const auto sb5 = 133;
	inline static constexpr const auto sb6 = 134;
	inline static constexpr const auto sb7 = 135;
	inline static constexpr const auto idb0 = 136;
	inline static constexpr const auto idb1 = 137;
	inline static constexpr const auto idb2 = 138;
	inline static constexpr const auto idb3 = 139;
	inline static constexpr const auto idb4 = 140;
	inline static constexpr const auto idb5 = 141;
	inline static constexpr const auto idb6 = 142;
	inline static constexpr const auto idb7 = 143;
	inline static constexpr const auto idl0 = 144;
	inline static constexpr const auto idl1 = 145;
	inline static constexpr const auto idl2 = 146;
	inline static constexpr const auto idl3 = 147;
	inline static constexpr const auto idl4 = 148;
	inline static constexpr const auto idl5 = 149;
	inline static constexpr const auto idl6 = 150;
	inline static constexpr const auto idl7 = 151;
	inline static constexpr const auto dor0 = 152;
	inline static constexpr const auto dor1 = 153;
	inline static constexpr const auto dor2 = 154;
	inline static constexpr const auto dor3 = 155;
	inline static constexpr const auto dor4 = 156;
	inline static constexpr const auto dor5 = 157;
	inline static constexpr const auto dor6 = 158;
	inline static constexpr const auto dor7 = 159;
	inline static constexpr const auto nmi = 160;
	inline static constexpr const auto irq = 161;
	inline static constexpr const auto res = 162;
	inline static constexpr const auto rdy = 163;
	inline static constexpr const auto notRdy0 = 164;
	inline static constexpr const auto so = 165;
	inline static constexpr const auto rw = 166;
	inline static constexpr const auto sync_ = 167;
	inline static constexpr const auto clk0 = 168;
	inline static constexpr const auto clk1out = 169;
	inline static constexpr const auto clk2out = 170;
	inline static constexpr const auto clock1 = 171;
	inline static constexpr const auto clock2 = 172;
	inline static constexpr const auto cclk = 173;
	inline static constexpr const auto cp1 = 174;
	inline static constexpr const auto clearIR = 175;
	inline static constexpr const auto D1x1 = 176;
	inline static constexpr const auto h1x1 = 177;
	inline static constexpr const auto fetch = 178;
	inline static constexpr const auto t2 = 179;
	inline static constexpr const auto t3 = 180;
	inline static constexpr const auto t4 = 181;
	inline static constexpr const auto t5 = 182;
	inline static constexpr const auto vcc = 183;
	inline static constexpr const auto vss = 184;
	
	inline static constexpr const auto P0 = 238 ;
	inline static constexpr const auto P1 = 1326;
	inline static constexpr const auto P2 = 906	;
	inline static constexpr const auto P3 = 208	;
	inline static constexpr const auto P4 = p4 ;
	inline static constexpr const auto P5 = p5 ;
	inline static constexpr const auto P6 = 670	;
	inline static constexpr const auto P7 = 939	;

}

