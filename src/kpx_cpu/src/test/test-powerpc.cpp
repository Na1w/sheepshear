/*
 *  test-powerpc.cpp - PowerPC regression testing
 *
 *  Kheperix (C) 2003 Gwenole Beauchesne
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 */

#include <stdlib.h>
#include "sysdeps.h"
#include "cpu/ppc/ppc-cpu.hpp"

#define TEST_ADD		0
#define TEST_SUB		0
#define TEST_MUL		0
#define TEST_DIV		0
#define TEST_SHIFT		0
#define TEST_ROTATE		0
#define TEST_MISC		0
#define TEST_LOGICAL	0
#define TEST_COMPARE	0
#define TEST_CR_LOGICAL	1

// Partial PowerPC runtime assembler from GNU lightning
#define _I(X)			((uint32)(X))
#define _UL(X)			((uint32)(X))
#define _MASK(N)		((uint32)((1<<(N)))-1)
#define _ck_s(W,I)		(_UL(I) & _MASK(W))
#define _ck_u(W,I)    	(_UL(I) & _MASK(W))
#define _ck_su(W,I)    	(_UL(I) & _MASK(W))
#define _u1(I)          _ck_u( 1,I)
#define _u5(I)          _ck_u( 5,I)
#define _u6(I)          _ck_u( 6,I)
#define _u9(I)          _ck_u( 9,I)
#define _u10(I)         _ck_u(10,I)
#define _s16(I)         _ck_s(16,I)

#define _D(   OP,RD,RA,         DD )  	_I((_u6(OP)<<26)|(_u5(RD)<<21)|(_u5(RA)<<16)|                _s16(DD)                          )
#define _X(   OP,RD,RA,RB,   XO,RC )  	_I((_u6(OP)<<26)|(_u5(RD)<<21)|(_u5(RA)<<16)|( _u5(RB)<<11)|              (_u10(XO)<<1)|_u1(RC))
#define _XO(  OP,RD,RA,RB,OE,XO,RC )  	_I((_u6(OP)<<26)|(_u5(RD)<<21)|(_u5(RA)<<16)|( _u5(RB)<<11)|(_u1(OE)<<10)|( _u9(XO)<<1)|_u1(RC))
#define _M(   OP,RS,RA,SH,MB,ME,RC )  	_I((_u6(OP)<<26)|(_u5(RS)<<21)|(_u5(RA)<<16)|( _u5(SH)<<11)|(_u5(MB)<< 6)|( _u5(ME)<<1)|_u1(RC))


struct exec_return { };

class powerpc_test_cpu
	: public powerpc_cpu
{
	uint32 emul_get_xer() const
		{ return xer().get(); }

	void emul_set_xer(uint32 value)
		{ xer().set(value); }

	uint32 emul_get_cr() const
		{ return cr().get(); }

	void emul_set_cr(uint32 value)
		{ cr().set(value); }

	uint32 native_get_xer() const
		{ uint32 xer; asm volatile ("mfxer %0" : "=r" (xer)); return xer; }

	void native_set_xer(uint32 xer) const
		{ asm volatile ("mtxer %0" : : "r" (xer)); }

	uint32 native_get_cr() const
		{ uint32 cr; asm volatile ("mfcr %0" : "=r" (cr)); return cr; }

	void native_set_cr(uint32 cr) const
		{ asm volatile ("mtcr %0" :  : "r" (cr)); }

	void init_decoder();
	void print_flags(uint32 cr, uint32 xer, int crf = 0) const;
	void execute_return(uint32 opcode);
	void execute(uint32 opcode);

public:

	powerpc_test_cpu()
		: powerpc_cpu(NULL)
		{ init_decoder(); }

	bool test(void);

private:

	static const bool verbose = false;
	uint32 tests, errors;

	// Initial CR0, XER state
	uint32 init_cr;
	uint32 init_xer;

	// Emulated registers IDs
	enum {
		R_ = -1,
		RD = 10,
		RA = 11,
		RB = 12
	};

	void test_add(void);
	void test_sub(void);
	void test_mul(void);
	void test_div(void);
	void test_shift(void);
	void test_rotate(void);
	void test_logical(void);
	void test_compare(void);
	void test_cr_logical(void);
};

void powerpc_test_cpu::execute_return(uint32 opcode)
{
	throw exec_return();
}

void powerpc_test_cpu::init_decoder()
{
#ifndef PPC_NO_STATIC_II_INDEX_TABLE
	static bool initialized = false;
	if (initialized)
		return;
	initialized = true;
#endif

	static const instr_info_t return_ii_table[] = {
		{ "return",
		  (execute_fn)&powerpc_test_cpu::execute_return,
		  NULL,
		  D_form, 6, 0, CFLOW_TRAP
		}
	};

	const int ii_count = sizeof(return_ii_table)/sizeof(return_ii_table[0]);

	for (int i = 0; i < ii_count; i++) {
		const instr_info_t * ii = &return_ii_table[i];
		init_decoder_entry(ii);
	}
}

void powerpc_test_cpu::execute(uint32 opcode)
{
	uint32 code[] = { opcode, 0x18000000 };

	try {
		invalidate_cache();
		pc() = (uintptr)code;
		powerpc_cpu::execute();
	}
	catch (exec_return const &) {
		// Nothing, simply return
	}
}

void powerpc_test_cpu::print_flags(uint32 cr, uint32 xer, int crf) const
{
	cr = cr << (4 * crf);
	printf("%s,%s,%s,%s,%s,%s",
		   (cr & CR_LT_field<0>::mask() ? "LT" : "__"),
		   (cr & CR_GT_field<0>::mask() ? "GT" : "__"),
		   (cr & CR_EQ_field<0>::mask() ? "EQ" : "__"),
		   (cr & CR_SO_field<0>::mask() ? "SO" : "__"),
		   (xer & XER_OV_field::mask()  ? "OV" : "__"),
		   (xer & XER_CA_field::mask()  ? "CA" : "__"));
}

#define TEST_ASM______(OP,D0,A0,A1,A2,A3) asm volatile (OP : : : "cc")
#define TEST_ASM_R____(OP,RD,A0,A1,A2,A3) asm volatile (OP " %0" : "=r" (RD) : : "cc")
#define TEST_ASM_RR___(OP,RD,RA,A1,A2,A3) asm volatile (OP " %0,%1" : "=r" (RD) : "r" (RA) : "cc")
#define TEST_ASM_CRR__(OP,RD,CR,RA,RB,A3) asm volatile (OP " %0,%1,%2" : : "i" (CR), "r" (RA), "r" (RB) : "cc")
#define TEST_ASM_CRI__(OP,RD,CR,RA,IM,A3) asm volatile (OP " %0,%1,%2" : : "i" (CR), "r" (RA), "I" (int16(IM)) : "cc")
#define TEST_ASM_CRK__(OP,RD,CR,RA,IM,A3) asm volatile (OP " %0,%1,%2" : : "i" (CR), "r" (RA), "K" (IM) : "cc")
#define TEST_ASM_RRS__(OP,RD,RA,IM,A2,A3) asm volatile (OP " %0,%1,%2" : "=r" (RD) : "r" (RA), "i" (IM) : "cc")
#define TEST_ASM_RRI__(OP,RD,RA,IM,A2,A3) asm volatile (OP " %0,%1,%2" : "=r" (RD) : "r" (RA), "I" (int16(IM)) : "cc")
#define TEST_ASM_RRK__(OP,RD,RA,IM,A2,A3) asm volatile (OP " %0,%1,%2" : "=r" (RD) : "r" (RA), "K" (IM) : "cc")
#define TEST_ASM_RRR__(OP,RD,RA,RB,A2,A3) asm volatile (OP " %0,%1,%2" : "=r" (RD) : "r" (RA), "r" (RB) : "cc")
#define TEST_ASM_RRIII(OP,RD,RA,SH,MB,ME) asm volatile (OP " %0,%1,%2,%3,%4" : "+r" (RD) : "r" (RA), "i" (SH), "i" (MB), "i" (ME))
#define TEST_ASM_RRRII(OP,RD,RA,RB,MB,ME) asm volatile (OP " %0,%1,%2,%3,%4" : "+r" (RD) : "r" (RA), "r" (RB), "i" (MB), "i" (ME))
#define TEST_ASM_CCC__(OP,RD,RA,RB,RC,A3) asm volatile (OP " %0,%1,%2" : : "i" (RA), "i" (RB), "i" (RC) : "cc")

#define TEST_ASM(FORMAT, OP, RD, CR, XER, A0, A1, A2, A3) do {	\
		native_set_xer(init_xer);								\
		native_set_cr(init_cr);									\
		TEST_ASM_##FORMAT(OP, RD, A0, A1, A2, A3);				\
		XER = native_get_xer();									\
		CR = native_get_cr();									\
	} while (0)

#define TEST_EMU_R0(OP,PRE,POST) do { PRE; execute(OP); POST; } while (0)
#define TEST_EMU_RD(OP,RD,VD,PRE) TEST_EMU_R0(OP,PRE,VD=gpr(RD))
#define TEST_EMU______(OP,RD,VD,R0,A0,R1,A1,R2,A2) TEST_EMU_R0(OP,/**/,/**/)
#define TEST_EMU_R____(OP,RD,VD,R0,A0,R1,A1,R2,A2) TEST_EMU_RD(OP,RD,VD,/**/)
#define TEST_EMU_RR___(OP,RD,VD,R0,A0,R1,A1,R2,A2) TEST_EMU_RD(OP,RD,VD,gpr(RD)=VD;gpr(R0)=A0)
#define TEST_EMU_RRS__(OP,RD,VD,R0,A0,R1,A1,R2,A2) TEST_EMU_RD(OP,RD,VD,gpr(RD)=VD;gpr(R0)=A0)
#define TEST_EMU_RRI__(OP,RD,VD,R0,A0,R1,A1,R2,A2) TEST_EMU_RD(OP,RD,VD,gpr(RD)=VD;gpr(R0)=A0)
#define TEST_EMU_RRK__(OP,RD,VD,R0,A0,R1,A1,R2,A2) TEST_EMU_RD(OP,RD,VD,gpr(RD)=VD;gpr(R0)=A0)
#define TEST_EMU_CRR__(OP,RD,VD,R0,A0,R1,A1,R2,A2) TEST_EMU_RD(OP,RD,VD,gpr(RD)=VD;gpr(R1)=A1;gpr(R2)=A2)
#define TEST_EMU_CRI__(OP,RD,VD,R0,A0,R1,A1,R2,A2) TEST_EMU_RD(OP,RD,VD,gpr(RD)=VD;gpr(R1)=A1)
#define TEST_EMU_CRK__(OP,RD,VD,R0,A0,R1,A1,R2,A2) TEST_EMU_RD(OP,RD,VD,gpr(RD)=VD;gpr(R1)=A1)
#define TEST_EMU_RRR__(OP,RD,VD,R0,A0,R1,A1,R2,A2) TEST_EMU_RD(OP,RD,VD,gpr(RD)=VD;gpr(R0)=A0;gpr(R1)=A1)
#define TEST_EMU_RRIII(OP,RD,VD,R0,A0,R1,A1,R2,A2) TEST_EMU_RD(OP,RD,VD,gpr(RD)=VD;gpr(R0)=A0)
#define TEST_EMU_RRRII(OP,RD,VD,R0,A0,R1,A1,R2,A2) TEST_EMU_RD(OP,RD,VD,gpr(RD)=VD;gpr(R0)=A0;gpr(R1)=A1)
#define TEST_EMU_CCC__(OP,RD,VD,R0,A0,R1,A1,R2,A2) TEST_EMU_R0(OP,/**/,/**/)

#define TEST_EMU(FORMAT, OP, RD, VD, CR, XER, R0, A0, R1, A1, R2, A2) do {	\
		emul_set_xer(init_xer);												\
		emul_set_cr(init_cr);												\
		TEST_EMU_##FORMAT(OP, RD, VD, R0, A0, R1, A1, R2, A2);				\
		XER = emul_get_xer();												\
		CR = emul_get_cr();													\
	} while (0)

#define PRINT_INPUTS_RRS__() printf("%08x, %04x", ra, im)
#define PRINT_INPUTS_RRI__() printf("%08x, %04x", ra, im)
#define PRINT_INPUTS_RRK__() printf("%08x, %04x", ra, im)
#define PRINT_INPUTS_RRR__() printf("%08x, %08x", ra, rb)
#define PRINT_INPUTS_RR___() printf("%08x", ra)
#define PRINT_INPUTS_CRR__() printf("%d, %08x, %08x", cr, ra, rb)
#define PRINT_INPUTS_CRI__() printf("%d, %08x, %04x", cr, ra, im)
#define PRINT_INPUTS_CRK__() printf("%d, %08x, %04x", cr, ra, im)
#define PRINT_INPUTS_RRIII() printf("%08x, %02d, %02d, %02d", ra, sh, mb, me)
#define PRINT_INPUTS_RRRII() printf("%08x, %02d, %02d, %02d", ra, rb, mb, me)
#define PRINT_INPUTS_CCC__() printf("%02d, %02d", crbA, crbB)

#define PRINT_OPERANDS(FORMAT, PREFIX) do {		\
	printf(" ");								\
	PRINT_INPUTS_##FORMAT();					\
	printf(" => %08x [", PREFIX##_rd);			\
	print_flags(PREFIX##_cr, PREFIX##_xer);		\
	printf("]\n");								\
} while (0)

#define TEST_ONE(FORMAT, NATIVE_OP, EMUL_OP, RD, R0, A0, R1, A1, R2, A2, A3) do {		\
	uint32 native_rd = 0, native_xer, native_cr;										\
	TEST_ASM(FORMAT, NATIVE_OP, native_rd, native_cr, native_xer, A0, A1, A2, A3);		\
	uint32 emul_rd = 0, emul_xer, emul_cr;												\
	TEST_EMU(FORMAT, EMUL_OP, RD, emul_rd, emul_cr, emul_xer, R0, A0, R1, A1, R2, A2);	\
	++tests;																			\
																						\
	bool ok = native_rd == emul_rd														\
		&& native_xer == emul_xer														\
		&& native_cr == emul_cr;														\
																						\
	if (!ok) {																			\
		printf("FAIL: " NATIVE_OP " [%08x]\n", EMUL_OP);								\
		errors++;																		\
	}																					\
	else if (verbose) {																	\
		printf("PASS: " NATIVE_OP " [%08x]\n", EMUL_OP);								\
	}																					\
																						\
	if (!ok || verbose) {																\
		PRINT_OPERANDS(FORMAT, native);													\
		PRINT_OPERANDS(FORMAT, emul);													\
	}																					\
} while (0)

#define TEST_INSTRUCTION_RRR__(FORMAT, NATIVE_OP, EMUL_OP) do {					\
	const uint32 opcode = EMUL_OP;												\
	for (uint32 i = 0; i < 8; i++) {											\
		const uint32 ra = i << 29;												\
		for (uint32 j = 0; j < 8; j++) {										\
			const uint32 rb = j << 29;											\
			TEST_ONE(FORMAT, NATIVE_OP, opcode, RD, RA, ra, RB, rb, R_, 0, 0);	\
		}																		\
	}																			\
	for (int32 i = -2; i <= 2; i++) {											\
		const uint32 ra = i;													\
		for (int32 j = -2; j <= 2; j++) {										\
			const uint32 rb = j;												\
			TEST_ONE(FORMAT, NATIVE_OP, opcode, RD, RA, ra, RB, rb, R_, 0, 0);	\
		}																		\
	}																			\
} while (0)

#define TEST_INSTRUCTION_RR___(FORMAT, NATIVE_OP, EMUL_OP) do {				\
	const uint32 opcode = EMUL_OP;											\
	for (uint32 i = 0; i < 8; i++) {										\
		const uint32 ra = i << 29;											\
		TEST_ONE(FORMAT, NATIVE_OP, opcode, RD, RA, ra, R_, 0, R_, 0, 0);	\
	}																		\
	for (int32 i = -2; i <= 2; i++) {										\
		const uint32 ra = i;												\
		TEST_ONE(FORMAT, NATIVE_OP, opcode, RD, RA, ra, R_, 0, R_, 0, 0);	\
	}																		\
} while (0)

#define TEST_ONE_IM(FORMAT, NATIVE_OP, IM, SH) do {										\
	const uint32 im = IM;																\
	TEST_ONE(FORMAT, NATIVE_OP, opcode|((IM) << (SH)), RD, RA, ra, R_, (IM), R_, 0, 0);	\
} while (0)

#define TEST_INSTRUCTION_IM(FORMAT, NATIVE_OP, EMUL_OP) do {	\
	const uint32 opcode = (EMUL_OP) & ~0xffff;					\
	for (uint32 i = 0; i < 8; i++) {							\
		const uint32 ra = i << 29;								\
		TEST_ONE_IM(FORMAT, NATIVE_OP, 0x0000, 0);				\
		TEST_ONE_IM(FORMAT, NATIVE_OP, 0x2000, 0);				\
		TEST_ONE_IM(FORMAT, NATIVE_OP, 0x4000, 0);				\
		TEST_ONE_IM(FORMAT, NATIVE_OP, 0x6000, 0);				\
		TEST_ONE_IM(FORMAT, NATIVE_OP, 0x8000, 0);				\
		TEST_ONE_IM(FORMAT, NATIVE_OP, 0xa000, 0);				\
		TEST_ONE_IM(FORMAT, NATIVE_OP, 0xc000, 0);				\
		TEST_ONE_IM(FORMAT, NATIVE_OP, 0xe000, 0);				\
		TEST_ONE_IM(FORMAT, NATIVE_OP, 0xfffe, 0);				\
		TEST_ONE_IM(FORMAT, NATIVE_OP, 0xffff, 0);				\
		TEST_ONE_IM(FORMAT, NATIVE_OP, 0x0001, 0);				\
		TEST_ONE_IM(FORMAT, NATIVE_OP, 0x0002, 0);				\
	}															\
} while (0)

#define TEST_INSTRUCTION_RRI__(FORMAT, NATIVE_OP, EMUL_OP)	\
	TEST_INSTRUCTION_IM(FORMAT, NATIVE_OP, EMUL_OP)

#define TEST_INSTRUCTION_RRK__(FORMAT, NATIVE_OP, EMUL_OP)	\
	TEST_INSTRUCTION_IM(FORMAT, NATIVE_OP, EMUL_OP)

#define TEST_INSTRUCTION_SH(FORMAT, NATIVE_OP) do {	\
	TEST_ONE_IM(FORMAT, NATIVE_OP,  0, 11);			\
	TEST_ONE_IM(FORMAT, NATIVE_OP,  1, 11);			\
	TEST_ONE_IM(FORMAT, NATIVE_OP,  2, 11);			\
	TEST_ONE_IM(FORMAT, NATIVE_OP,  3, 11);			\
	TEST_ONE_IM(FORMAT, NATIVE_OP, 28, 11);			\
	TEST_ONE_IM(FORMAT, NATIVE_OP, 29, 11);			\
	TEST_ONE_IM(FORMAT, NATIVE_OP, 30, 11);			\
	TEST_ONE_IM(FORMAT, NATIVE_OP, 31, 11);			\
} while (0)

#define TEST_INSTRUCTION_RRS__(FORMAT, NATIVE_OP, EMUL_OP) do {	\
	const uint32 opcode = (EMUL_OP) & ~SH_field::mask();		\
	for (uint32 i = 0; i < 8; i++) {							\
		const uint32 ra = i << 29;								\
		TEST_INSTRUCTION_SH(FORMAT, NATIVE_OP);					\
	}															\
} while (0)

#define TEST_INSTRUCTION_RT_3(FORMAT, NATIVE_OP, SH, MB, ME) do {			\
	const uint32 me = ME;													\
	const uint32 opcode3 = opcode2 | (me << 1);								\
	TEST_ONE(FORMAT, NATIVE_OP, opcode3, RD, RA, ra, RB, SH, R_, MB, ME);	\
} while (0)

#define TEST_INSTRUCTION_RT_2(FORMAT, NATIVE_OP, SH, MB) do {	\
	const uint32 mb = MB;										\
	const uint32 opcode2 = opcode1 | (mb << 6);					\
	TEST_INSTRUCTION_RT_3(FORMAT, NATIVE_OP, SH, MB,  0);		\
	TEST_INSTRUCTION_RT_3(FORMAT, NATIVE_OP, SH, MB,  1);		\
	TEST_INSTRUCTION_RT_3(FORMAT, NATIVE_OP, SH, MB, 30);		\
	TEST_INSTRUCTION_RT_3(FORMAT, NATIVE_OP, SH, MB, 31);		\
} while (0)

#define TEST_INSTRUCTION_RT_1(FORMAT, NATIVE_OP, SH) do {	\
	const uint32 sh = SH;									\
	const uint32 opcode1 = opcode  | (sh << 11);			\
	TEST_INSTRUCTION_RT_2(FORMAT, NATIVE_OP, SH,  0);		\
	TEST_INSTRUCTION_RT_2(FORMAT, NATIVE_OP, SH,  1);		\
	TEST_INSTRUCTION_RT_2(FORMAT, NATIVE_OP, SH, 30);		\
	TEST_INSTRUCTION_RT_2(FORMAT, NATIVE_OP, SH, 31);		\
} while (0)

#define TEST_INSTRUCTION_RRIII(FORMAT, NATIVE_OP, EMUL_OP) do {					\
	const uint32 opcode = (EMUL_OP) & (OPCD_field::mask() | rS_field::mask() |	\
									   rA_field::mask() | Rc_field::mask());	\
	for (uint32 i = 0; i < 8; i++) {											\
		const uint32 ra = i << 29;												\
		TEST_INSTRUCTION_RT_1(FORMAT, NATIVE_OP,  0);							\
		TEST_INSTRUCTION_RT_1(FORMAT, NATIVE_OP,  1);							\
		TEST_INSTRUCTION_RT_1(FORMAT, NATIVE_OP, 30);							\
		TEST_INSTRUCTION_RT_1(FORMAT, NATIVE_OP, 31);							\
	}																			\
} while (0)

#define TEST_INSTRUCTION_RRRII(FORMAT, NATIVE_OP, EMUL_OP) do {					\
	const uint32 opcode = (EMUL_OP) & ~(MB_field::mask() | ME_field::mask());	\
	for (uint32 i = 0; i < 8; i++) {											\
		const uint32 ra = i << 29;												\
		for (uint32 j = 0; j < 32; j++) {										\
			const uint32 rb = j;												\
			const uint32 opcode1 = opcode;										\
			TEST_INSTRUCTION_RT_2(FORMAT, NATIVE_OP, rb,  0);					\
			TEST_INSTRUCTION_RT_2(FORMAT, NATIVE_OP, rb,  1);					\
			TEST_INSTRUCTION_RT_2(FORMAT, NATIVE_OP, rb, 30);					\
			TEST_INSTRUCTION_RT_2(FORMAT, NATIVE_OP, rb, 31);					\
		}																		\
	}																			\
} while (0)

#define TEST_INSTRUCTION_CR(FORMAT, NATIVE_OP, CRFD) do {							\
	const int cr = CRFD;															\
	TEST_ONE(FORMAT, NATIVE_OP, opcode|(cr<<23), RD, R_, CRFD, RA, ra, RB, rb, 0);	\
} while (0)

#define TEST_INSTRUCTION_CRR__(FORMAT, NATIVE_OP, EMUL_OP) do {	\
	const uint32 opcode = (EMUL_OP) & ~crfD_field::mask();		\
	for (int32 i = -1; i <= 1; i++) {							\
		const uint32 ra = i;									\
		for (int32 j = -1; j <= 1; j++) {						\
			const uint32 rb = j;								\
			TEST_INSTRUCTION_CR(FORMAT, NATIVE_OP, 0);			\
			TEST_INSTRUCTION_CR(FORMAT, NATIVE_OP, 1);			\
			TEST_INSTRUCTION_CR(FORMAT, NATIVE_OP, 2);			\
			TEST_INSTRUCTION_CR(FORMAT, NATIVE_OP, 3);			\
			TEST_INSTRUCTION_CR(FORMAT, NATIVE_OP, 4);			\
			TEST_INSTRUCTION_CR(FORMAT, NATIVE_OP, 5);			\
			TEST_INSTRUCTION_CR(FORMAT, NATIVE_OP, 6);			\
			TEST_INSTRUCTION_CR(FORMAT, NATIVE_OP, 7);			\
		}														\
	}															\
} while (0)

#define TEST_INSTRUCTION_CR_IM_2(FORMAT, NATIVE_OP, CRFD, IMM) do {				\
	const uint32 im = (uint32)IMM;												\
	const uint32 opcode2 = opcode1 | (im & 0xffff);								\
	TEST_ONE(FORMAT, NATIVE_OP, opcode2, RD, R_, CRFD, RA, ra, R_, IMM, 0);	\
} while (0);

#define TEST_INSTRUCTION_CR_IM_1(FORMAT, NATIVE_OP, CRFD) do {	\
	const uint32 cr = CRFD;										\
	const uint32 opcode1 = opcode | (cr << 23);					\
	TEST_INSTRUCTION_CR_IM_2(FORMAT, NATIVE_OP, CRFD, 0x0000);	\
	TEST_INSTRUCTION_CR_IM_2(FORMAT, NATIVE_OP, CRFD, 0x4000);	\
	TEST_INSTRUCTION_CR_IM_2(FORMAT, NATIVE_OP, CRFD, 0x8000);	\
	TEST_INSTRUCTION_CR_IM_2(FORMAT, NATIVE_OP, CRFD, 0xc000);	\
} while (0)

#define TEST_INSTRUCTION_CRI__(FORMAT, NATIVE_OP, EMUL_OP) do {						\
	const uint32 opcode = (EMUL_OP) & ~(crfD_field::mask() | SIMM_field::mask());	\
	for (int32 i = -1; i <= 1; i++) {												\
		const uint32 ra = i;														\
		TEST_INSTRUCTION_CR_IM_1(FORMAT, NATIVE_OP, 0);								\
		TEST_INSTRUCTION_CR_IM_1(FORMAT, NATIVE_OP, 1);								\
		TEST_INSTRUCTION_CR_IM_1(FORMAT, NATIVE_OP, 2);								\
		TEST_INSTRUCTION_CR_IM_1(FORMAT, NATIVE_OP, 3);								\
		TEST_INSTRUCTION_CR_IM_1(FORMAT, NATIVE_OP, 4);								\
		TEST_INSTRUCTION_CR_IM_1(FORMAT, NATIVE_OP, 5);								\
		TEST_INSTRUCTION_CR_IM_1(FORMAT, NATIVE_OP, 6);								\
		TEST_INSTRUCTION_CR_IM_1(FORMAT, NATIVE_OP, 7);								\
	}																				\
} while (0)

#define TEST_INSTRUCTION_CRK__(FORMAT, NATIVE_OP, EMUL_OP)	\
	TEST_INSTRUCTION_CRI__(CRK__, NATIVE_OP, EMUL_OP)

#define PRINT_CR_OPERANDS(PREFIX) do {				\
	printf(" [");									\
	print_flags(PREFIX##_cr, PREFIX##_xer, crbA);	\
	printf("], [");									\
	print_flags(PREFIX##_cr, PREFIX##_xer, crbB);	\
	printf("] => [");								\
	print_flags(PREFIX##_cr, PREFIX##_xer, crbD);	\
	printf("]\n");									\
} while (0)

#define TEST_ONE_CR_OP(NATIVE_OP, EMUL_OP, RD, RA, RB, CR) do {						\
	const uint32 orig_init_cr = init_cr;											\
	uint32 native_rd = 0, native_xer, native_cr; init_cr = CR;						\
	TEST_ASM(CCC__, NATIVE_OP, native_rd, native_cr, native_xer, RD, RA, RB, 0);	\
	uint32 emul_rd = 0, emul_xer, emul_cr; init_cr = CR;							\
	TEST_EMU(CCC__, EMUL_OP, RD, emul_rd, emul_cr, emul_xer, RD, 0, RA, 0, RB, 0);	\
	init_cr = orig_init_cr;															\
	++tests;																		\
																					\
	bool ok = native_cr == emul_cr;													\
																					\
	if (!ok) {																		\
		printf("FAIL: " NATIVE_OP " [%08x]\n", EMUL_OP);							\
		errors++;																	\
	}																				\
	else if (verbose) {																\
		printf("PASS: " NATIVE_OP " [%08x]\n", EMUL_OP);							\
	}																				\
																					\
	if (!ok || verbose) {															\
		PRINT_CR_OPERANDS(native);													\
		PRINT_CR_OPERANDS(emul);													\
	}																				\
} while (0)

#define TEST_INSTRUCTION_CCC_1(FORMAT, NATIVE_OP, CRBD, CRBA, CRBB, CR) do {	\
	const uint32 crbD = (CRBD), crbA = (CRBA), crbB = (CRBB);					\
	const uint32 opcode1 = opcode | (crbD << 21) | (crbA << 16) | (crbB << 11);	\
	TEST_ONE_CR_OP(NATIVE_OP, opcode1, CRBD, CRBA, CRBB, CR);					\
} while (0)

#define TEST_INSTRUCTION_CCC__(FORMAT, NATIVE_OP, EMUL_OP) do {		\
	const uint32 opcode = (EMUL_OP) & ~(crbD_field::mask() |		\
										crbA_field::mask() |		\
										crbB_field::mask());		\
																	\
	TEST_INSTRUCTION_CCC_1(FORMAT, NATIVE_OP, 0, 1, 2, 0xdeadbeef);	\
	TEST_INSTRUCTION_CCC_1(FORMAT, NATIVE_OP, 0, 1, 2, 0x01200000);	\
	TEST_INSTRUCTION_CCC_1(FORMAT, NATIVE_OP, 0, 1, 2, 0x02100000);	\
	TEST_INSTRUCTION_CCC_1(FORMAT, NATIVE_OP, 0, 1, 2, 0x1c200000);	\
	TEST_INSTRUCTION_CCC_1(FORMAT, NATIVE_OP, 0, 1, 2, 0x81200000);	\
	TEST_INSTRUCTION_CCC_1(FORMAT, NATIVE_OP, 0, 1, 2, 0x41200000);	\
	TEST_INSTRUCTION_CCC_1(FORMAT, NATIVE_OP, 0, 1, 2, 0x21200000);	\
	TEST_INSTRUCTION_CCC_1(FORMAT, NATIVE_OP, 0, 1, 2, 0x11200000);	\
	TEST_INSTRUCTION_CCC_1(FORMAT, NATIVE_OP, 0, 1, 2, 0x81200000);	\
	TEST_INSTRUCTION_CCC_1(FORMAT, NATIVE_OP, 0, 1, 2, 0x42200000);	\
	TEST_INSTRUCTION_CCC_1(FORMAT, NATIVE_OP, 0, 1, 2, 0x24200000);	\
	TEST_INSTRUCTION_CCC_1(FORMAT, NATIVE_OP, 0, 1, 2, 0x18200000);	\
	TEST_INSTRUCTION_CCC_1(FORMAT, NATIVE_OP, 0, 1, 2, 0x83200000);	\
	TEST_INSTRUCTION_CCC_1(FORMAT, NATIVE_OP, 0, 1, 2, 0x45200000);	\
	TEST_INSTRUCTION_CCC_1(FORMAT, NATIVE_OP, 0, 1, 2, 0x27200000);	\
	TEST_INSTRUCTION_CCC_1(FORMAT, NATIVE_OP, 0, 1, 2, 0x19200000);	\
	TEST_INSTRUCTION_CCC_1(FORMAT, NATIVE_OP, 0, 1, 2, 0x1c200000);	\
	TEST_INSTRUCTION_CCC_1(FORMAT, NATIVE_OP, 0, 1, 2, 0x81c00000);	\
	TEST_INSTRUCTION_CCC_1(FORMAT, NATIVE_OP, 0, 1, 2, 0x41c00000);	\
	TEST_INSTRUCTION_CCC_1(FORMAT, NATIVE_OP, 0, 1, 2, 0x21c00000);	\
	TEST_INSTRUCTION_CCC_1(FORMAT, NATIVE_OP, 0, 1, 2, 0x11c00000);	\
	TEST_INSTRUCTION_CCC_1(FORMAT, NATIVE_OP, 0, 1, 2, 0x81c00000);	\
	TEST_INSTRUCTION_CCC_1(FORMAT, NATIVE_OP, 0, 1, 2, 0x42c00000);	\
	TEST_INSTRUCTION_CCC_1(FORMAT, NATIVE_OP, 0, 1, 2, 0x24c00000);	\
	TEST_INSTRUCTION_CCC_1(FORMAT, NATIVE_OP, 0, 1, 2, 0x18c00000);	\
	TEST_INSTRUCTION_CCC_1(FORMAT, NATIVE_OP, 0, 1, 2, 0x83c00000);	\
	TEST_INSTRUCTION_CCC_1(FORMAT, NATIVE_OP, 0, 1, 2, 0x45c00000);	\
	TEST_INSTRUCTION_CCC_1(FORMAT, NATIVE_OP, 0, 1, 2, 0x27c00000);	\
	TEST_INSTRUCTION_CCC_1(FORMAT, NATIVE_OP, 0, 1, 2, 0x19c00000);	\
} while (0);

#define TEST_INSTRUCTION(FORMAT, NATIVE_OP, EMUL_OP) do {	\
	printf("Testing " NATIVE_OP "\n");						\
	TEST_INSTRUCTION_##FORMAT(FORMAT, NATIVE_OP, EMUL_OP);	\
} while (0)

void powerpc_test_cpu::test_add(void)
{
#if TEST_ADD
	TEST_INSTRUCTION(RRR__,"add",		_XO(31,RD,RA,RB,0,266,0));
	TEST_INSTRUCTION(RRR__,"add.",		_XO(31,RD,RA,RB,0,266,1));
	TEST_INSTRUCTION(RRR__,"addo",		_XO(31,RD,RA,RB,1,266,0));
	TEST_INSTRUCTION(RRR__,"addo." ,	_XO(31,RD,RA,RB,1,266,1));
	TEST_INSTRUCTION(RRR__,"addc.",		_XO(31,RD,RA,RB,0, 10,1));
	TEST_INSTRUCTION(RRR__,"addco.",	_XO(31,RD,RA,RB,1, 10,1));
	TEST_INSTRUCTION(RRR__,"adde.",		_XO(31,RD,RA,RB,0,138,1));
	TEST_INSTRUCTION(RRR__,"addeo.",	_XO(31,RD,RA,RB,1,138,1));
	TEST_INSTRUCTION(RRI__,"addi",		_D (14,RD,RA,00));
	TEST_INSTRUCTION(RRI__,"addic",		_D (12,RD,RA,00));
	TEST_INSTRUCTION(RRI__,"addic.",	_D (13,RD,RA,00));
	TEST_INSTRUCTION(RRI__,"addis",		_D (15,RD,RA,00));
	TEST_INSTRUCTION(RR___,"addme.",	_XO(31,RD,RA,00,0,234,1));
	TEST_INSTRUCTION(RR___,"addmeo.",	_XO(31,RD,RA,00,1,234,1));
	TEST_INSTRUCTION(RR___,"addze.",	_XO(31,RD,RA,00,0,202,1));
	TEST_INSTRUCTION(RR___,"addzeo.",	_XO(31,RD,RA,00,1,202,1));
	init_xer |= XER_CA_field::mask();
	TEST_INSTRUCTION(RRR__,"adde.",		_XO(31,RD,RA,RB,0,138,1));
	TEST_INSTRUCTION(RRR__,"addeo.",	_XO(31,RD,RA,RB,1,138,1));
	TEST_INSTRUCTION(RR___,"addme.",	_XO(31,RD,RA,00,0,234,1));
	TEST_INSTRUCTION(RR___,"addmeo.",	_XO(31,RD,RA,00,1,234,1));
	TEST_INSTRUCTION(RR___,"addze.",	_XO(31,RD,RA,00,0,202,1));
	TEST_INSTRUCTION(RR___,"addzeo.",	_XO(31,RD,RA,00,1,202,1));
	init_xer &= ~XER_CA_field::mask();
#endif
}

void powerpc_test_cpu::test_sub(void)
{
#if TEST_SUB
	TEST_INSTRUCTION(RRR__,"subf.",		_XO(31,RD,RA,RB,0, 40,1));
	TEST_INSTRUCTION(RRR__,"subfo.",	_XO(31,RD,RA,RB,1, 40,1));
	TEST_INSTRUCTION(RRR__,"subfc.",	_XO(31,RD,RA,RB,0,  8,1));
	TEST_INSTRUCTION(RRR__,"subfco.",	_XO(31,RD,RA,RB,1,  8,1));
	TEST_INSTRUCTION(RRR__,"subfe.",	_XO(31,RD,RA,RB,0,136,1));
	TEST_INSTRUCTION(RRR__,"subfeo.",	_XO(31,RD,RA,RB,1,136,1));
	TEST_INSTRUCTION(RRI__,"subfic",	_D ( 8,RD,RA,00));
	TEST_INSTRUCTION(RR___,"subfme.",	_XO(31,RD,RA,00,0,232,1));
	TEST_INSTRUCTION(RR___,"subfmeo.",	_XO(31,RD,RA,00,1,232,1));
	TEST_INSTRUCTION(RR___,"subfze.",	_XO(31,RD,RA,00,0,200,1));
	TEST_INSTRUCTION(RR___,"subfzeo.",	_XO(31,RD,RA,00,1,200,1));
	init_xer |= XER_CA_field::mask();
	TEST_INSTRUCTION(RRR__,"subfe.",	_XO(31,RD,RA,RB,0,136,1));
	TEST_INSTRUCTION(RRR__,"subfeo.",	_XO(31,RD,RA,RB,1,136,1));
	TEST_INSTRUCTION(RR___,"subfme.",	_XO(31,RD,RA,00,0,232,1));
	TEST_INSTRUCTION(RR___,"subfmeo.",	_XO(31,RD,RA,00,1,232,1));
	TEST_INSTRUCTION(RR___,"subfze.",	_XO(31,RD,RA,00,0,200,1));
	TEST_INSTRUCTION(RR___,"subfzeo.",	_XO(31,RD,RA,00,1,200,1));
	init_xer &= ~XER_CA_field::mask();
#endif
}

void powerpc_test_cpu::test_mul(void)
{
#if TEST_MUL
	TEST_INSTRUCTION(RRR__,"mulhw",		_XO(31,RD,RA,RB,0, 75,0));
	TEST_INSTRUCTION(RRR__,"mulhw.",	_XO(31,RD,RA,RB,0, 75,1));
	TEST_INSTRUCTION(RRR__,"mulhwu",	_XO(31,RD,RA,RB,0, 11,0));
	TEST_INSTRUCTION(RRR__,"mulhwu.",	_XO(31,RD,RA,RB,0, 11,1));
	TEST_INSTRUCTION(RRI__,"mulli",		_D ( 7,RD,RA,00));
	TEST_INSTRUCTION(RRR__,"mullw",		_XO(31,RD,RA,RB,0,235,0));
	TEST_INSTRUCTION(RRR__,"mullw.",	_XO(31,RD,RA,RB,0,235,1));
	TEST_INSTRUCTION(RRR__,"mullwo",	_XO(31,RD,RA,RB,1,235,0));
	TEST_INSTRUCTION(RRR__,"mullwo.",	_XO(31,RD,RA,RB,1,235,1));
#endif
}

void powerpc_test_cpu::test_div(void)
{
#if TEST_DIV
	TEST_INSTRUCTION(RRR__,"divw",		_XO(31,RD,RA,RB,0,491,0));
	TEST_INSTRUCTION(RRR__,"divw.",		_XO(31,RD,RA,RB,0,491,1));
	TEST_INSTRUCTION(RRR__,"divwo",		_XO(31,RD,RA,RB,1,491,0));
	TEST_INSTRUCTION(RRR__,"divwo.",	_XO(31,RD,RA,RB,1,491,1));
	TEST_INSTRUCTION(RRR__,"divwu",		_XO(31,RD,RA,RB,0,459,0));
	TEST_INSTRUCTION(RRR__,"divwu.",	_XO(31,RD,RA,RB,0,459,1));
	TEST_INSTRUCTION(RRR__,"divwuo",	_XO(31,RD,RA,RB,1,459,0));
	TEST_INSTRUCTION(RRR__,"divwuo.",	_XO(31,RD,RA,RB,1,459,1));
#endif
}

void powerpc_test_cpu::test_logical(void)
{
#if TEST_GENERIC_ARITH
	TEST_INSTRUCTION(RRR__,"and.",		_X (31,RA,RD,RB,28,1));
	TEST_INSTRUCTION(RRR__,"andc.",		_X (31,RA,RD,RB,60,1));
	TEST_INSTRUCTION(RRK__,"andi.",		_D (28,RA,RD,00));
	TEST_INSTRUCTION(RRK__,"andis.",	_D (29,RA,RD,00));
	TEST_INSTRUCTION(RR___,"cntlzw.",	_X (31,RA,RD,00,26,1));
	TEST_INSTRUCTION(RRR__,"eqv.",		_X (31,RA,RD,RB,284,1));
	TEST_INSTRUCTION(RR___,"extsb.",	_X (31,RA,RD,00,954,1));
	TEST_INSTRUCTION(RR___,"extsh.",	_X (31,RA,RD,00,922,1));
	TEST_INSTRUCTION(RRR__,"nand.",		_X (31,RA,RD,RB,476,1));
	TEST_INSTRUCTION(RR___,"neg.",		_XO(31,RD,RA,RB,0,104,1));
	TEST_INSTRUCTION(RR___,"nego.",		_XO(31,RD,RA,RB,1,104,1));
	TEST_INSTRUCTION(RRR__,"nor.",		_X (31,RA,RD,RB,124,1));
	TEST_INSTRUCTION(RRR__,"or.",		_X (31,RA,RD,RB,444,1));
	TEST_INSTRUCTION(RRR__,"orc.",		_X (31,RA,RD,RB,412,1));
	TEST_INSTRUCTION(RRK__,"ori",		_D (24,RA,RD,00));
	TEST_INSTRUCTION(RRK__,"oris",		_D (25,RA,RD,00));
	TEST_INSTRUCTION(RRR__,"xor.",		_X (31,RA,RD,RB,316,1));
	TEST_INSTRUCTION(RRK__,"xori",		_D (26,RA,RD,00));
	TEST_INSTRUCTION(RRK__,"xoris",		_D (27,RA,RD,00));
#endif
}

void powerpc_test_cpu::test_shift(void)
{
#if TEST_SHIFT
	TEST_INSTRUCTION(RRR__,"slw",		_X (31,RA,RD,RB, 24,0));
	TEST_INSTRUCTION(RRR__,"slw.",		_X (31,RA,RD,RB, 24,1));
	TEST_INSTRUCTION(RRR__,"sraw",		_X (31,RA,RD,RB,792,0));
	TEST_INSTRUCTION(RRR__,"sraw.",		_X (31,RA,RD,RB,792,1));
	TEST_INSTRUCTION(RRS__,"srawi",		_X (31,RA,RD,00,824,0));
	TEST_INSTRUCTION(RRS__,"srawi.",	_X (31,RA,RD,00,824,1));
	TEST_INSTRUCTION(RRR__,"srw",		_X (31,RA,RD,RB,536,0));
	TEST_INSTRUCTION(RRR__,"srw.",		_X (31,RA,RD,RB,536,1));
#endif
}

void powerpc_test_cpu::test_rotate(void)
{
#if TEST_ROTATE
	TEST_INSTRUCTION(RRIII,"rlwimi.",	_M (20,RA,RD,00,00,00,1));
	TEST_INSTRUCTION(RRIII,"rlwinm.",	_M (21,RA,RD,00,00,00,1));
	TEST_INSTRUCTION(RRRII,"rlwnm.",	_M (23,RA,RD,RB,00,00,1));
#endif
}

void powerpc_test_cpu::test_compare(void)
{
#if TEST_COMPARE
	TEST_INSTRUCTION(CRR__,"cmp",		_X (31,00,RA,RB,000,0));
	TEST_INSTRUCTION(CRI__,"cmpi",		_D (11,00,RA,00));
	TEST_INSTRUCTION(CRR__,"cmpl",		_X (31,00,RA,RB, 32,0));
	TEST_INSTRUCTION(CRK__,"cmpli",		_D (10,00,RA,00));
#endif
}

void powerpc_test_cpu::test_cr_logical(void)
{
#if TEST_CR_LOGICAL
	TEST_INSTRUCTION(CCC__,"crand",		_X (19,00,00,00,257,0));
	TEST_INSTRUCTION(CCC__,"crandc",	_X (19,00,00,00,129,0));
	TEST_INSTRUCTION(CCC__,"creqv",		_X (19,00,00,00,289,0));
	TEST_INSTRUCTION(CCC__,"crnand",	_X (19,00,00,00,225,0));
	TEST_INSTRUCTION(CCC__,"crnor",		_X (19,00,00,00, 33,0));
	TEST_INSTRUCTION(CCC__,"cror",		_X (19,00,00,00,449,0));
	TEST_INSTRUCTION(CCC__,"crorc",		_X (19,00,00,00,417,0));
	TEST_INSTRUCTION(CCC__,"crxor",		_X (19,00,00,00,193,0));
#endif
}

bool powerpc_test_cpu::test(void)
{
	// Tests initialization
	tests = errors = 0;
	init_cr = native_get_cr() & ~CR_field<0>::mask();
	init_xer = native_get_xer() & ~(XER_OV_field::mask() | XER_CA_field::mask());

	// Tests execution
	test_add();
	test_sub();
	test_mul();
	test_div();
	test_shift();
	test_rotate();
	test_logical();
	test_compare();
	test_cr_logical();

	printf("%u errors out of %u tests\n", errors, tests);
	return errors != 0;
}

int main()
{
	powerpc_test_cpu ppc;

	if (!ppc.test())
		return EXIT_FAILURE;

	return EXIT_SUCCESS;
}
