/*****************************************************************************
 * pce                                                                       *
 *****************************************************************************/

/*****************************************************************************
 * File name:     src/arch/sims32/cmd_s32.c                                  *
 * Created:       2004-09-28 by Hampa Hug <hampa@hampa.ch>                   *
 * Last modified: 2005-02-23 by Hampa Hug <hampa@hampa.ch>                   *
 * Copyright:     (C) 2004-2005 Hampa Hug <hampa@hampa.ch>                   *
 *****************************************************************************/

/*****************************************************************************
 * This program is free software. You can redistribute it and / or modify it *
 * under the terms of the GNU General Public License version 2 as  published *
 * by the Free Software Foundation.                                          *
 *                                                                           *
 * This program is distributed in the hope  that  it  will  be  useful,  but *
 * WITHOUT  ANY   WARRANTY,   without   even   the   implied   warranty   of *
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU  General *
 * Public License for more details.                                          *
 *****************************************************************************/

/* $Id$ */


#include "main.h"


int ss32_match_reg (cmd_t *cmd, sims32_t *sim, uint32_t **reg)
{
	sparc32_t *cpu;

	cpu = sim->cpu;

	cmd_match_space (cmd);

	if (cmd_match (cmd, "%r")) {
		unsigned n;

		if (!cmd_match_uint (cmd, &n, 10)) {
			cmd_error (cmd, "missing register number");
			return (0);
		}

		*reg = &cpu->reg[n & 0x3f];

		return (1);
	}
	else if (cmd_match (cmd, "%npc")) {
		*reg = &cpu->npc;
		return (1);
	}
	else if (cmd_match (cmd, "%psr")) {
		*reg = &cpu->psr;
		return (1);
	}
	else if (cmd_match (cmd, "%pc")) {
		*reg = &cpu->pc;
		return (1);
	}
	else if (cmd_match (cmd, "%tbr")) {
		*reg = &cpu->tbr;
		return (1);
	}
	else if (cmd_match (cmd, "%wim")) {
		*reg = &cpu->wim;
		return (1);
	}
	else if (cmd_match (cmd, "%y")) {
		*reg = &cpu->y;
		return (1);
	}

	return (0);
}

void ss32_dasm_str (char *dst, s32_dasm_t *op)
{
	switch (op->argn) {
		case 0:
			sprintf (dst, "%08lX  %s", (unsigned long) op->ir, op->op);
			break;

		case 1:
			sprintf (dst, "%08lX  %-8s %s", (unsigned long) op->ir, op->op, op->arg1);
			break;

		case 2:
			sprintf (dst, "%08lX  %-8s %s, %s",
				(unsigned long) op->ir, op->op, op->arg1, op->arg2);
			break;

		case 3:
			sprintf (dst, "%08lX  %-8s %s, %s, %s",
				(unsigned long) op->ir, op->op, op->arg1, op->arg2, op->arg3);
			break;

		default:
			strcpy (dst, "---");
			break;
	}
}

void ss32_prt_state_cpu (sparc32_t *c, FILE *fp)
{
	unsigned           i;
	unsigned long long opcnt, clkcnt;
	unsigned long      delay;
	s32_dasm_t         op;
	char               str[256];

	prt_sep (fp, "SPARC32");

	opcnt = s32_get_opcnt (c);
	clkcnt = s32_get_clkcnt (c);
	delay = s32_get_delay (c);

	fprintf (fp, "CLK=%llx  OP=%llx  DLY=%lx  CPI=%.4f\n",
		clkcnt, opcnt, delay,
		(opcnt > 0) ? ((double) (clkcnt + delay) / (double) opcnt) : 1.0
	);

	fprintf (fp, " PSR=%08lX  ICC=[%c%c%c%c]\n",
		(unsigned long) s32_get_psr (c),
		(s32_get_psr_c (c)) ? 'C' : '-',
		(s32_get_psr_v (c)) ? 'V' : '-',
		(s32_get_psr_z (c)) ? 'Z' : '-',
		(s32_get_psr_n (c)) ? 'N' : '-'
	);

	fprintf (fp, " r00=%08lX  r08=%08lX  r16=%08lX  r24=%08lX    PC=%08lX\n",
		(unsigned long) s32_get_reg (c, 0),
		(unsigned long) s32_get_reg (c, 8),
		(unsigned long) s32_get_reg (c, 16),
		(unsigned long) s32_get_reg (c, 24),
		(unsigned long) s32_get_pc (c)
	);

	fprintf (fp, " r01=%08lX  r09=%08lX  r17=%08lX  r25=%08lX   nPC=%08lX\n",
		(unsigned long) s32_get_reg (c, 1),
		(unsigned long) s32_get_reg (c, 9),
		(unsigned long) s32_get_reg (c, 17),
		(unsigned long) s32_get_reg (c, 25),
		(unsigned long) s32_get_npc (c)
	);

	fprintf (fp, " r02=%08lX  r10=%08lX  r18=%08lX  r26=%08lX   WIM=%08lX\n",
		(unsigned long) s32_get_reg (c, 2),
		(unsigned long) s32_get_reg (c, 10),
		(unsigned long) s32_get_reg (c, 18),
		(unsigned long) s32_get_reg (c, 26),
		(unsigned long) s32_get_wim (c)
	);

	fprintf (fp, " r03=%08lX  r11=%08lX  r19=%08lX  r27=%08lX   TBR=%08lX\n",
		(unsigned long) s32_get_reg (c, 3),
		(unsigned long) s32_get_reg (c, 11),
		(unsigned long) s32_get_reg (c, 19),
		(unsigned long) s32_get_reg (c, 27),
		(unsigned long) s32_get_tbr (c)
	);

	fprintf (fp, " r04=%08lX  r12=%08lX  r20=%08lX  r28=%08lX     Y=%08lX\n",
		(unsigned long) s32_get_reg (c, 4),
		(unsigned long) s32_get_reg (c, 12),
		(unsigned long) s32_get_reg (c, 20),
		(unsigned long) s32_get_reg (c, 28),
		(unsigned long) s32_get_y (c)
	);

	for (i = 5; i < 8; i++) {
		fprintf (fp, " r%02u=%08lX  r%02u=%08lX  r%02u=%08lX  r%02u=%08lX\n",
			i + 0, (unsigned long) s32_get_reg (c, i + 0),
			i + 8, (unsigned long) s32_get_reg (c, i + 8),
			i + 16, (unsigned long) s32_get_reg (c, i + 16),
			i + 24, (unsigned long) s32_get_reg (c, i + 24)
		);
	}

	s32_dasm_mem (c, &op, s32_get_pc (c), c->asi_text, par_xlat);
	ss32_dasm_str (str, &op);

	fprintf (fp, "%08lX  %s\n", (unsigned long) s32_get_pc (c), str);
}


static
void ss32_exec (sims32_t *sim)
{
	unsigned long long old;

	old = s32_get_opcnt (sim->cpu);

	while (s32_get_opcnt (sim->cpu) == old) {
		ss32_clock (sim, 1);
	}
}

static
void ss32_run_bp (sims32_t *sim)
{
	pce_start();

	while (1) {
		ss32_exec (sim);

		if (bp_check (&sim->brkpt, s32_get_pc (sim->cpu), 0)) {
			break;
		}

		if (sim->brk) {
			break;
		}
	}

	pce_stop();
}

static
int ss32_exec_to (sims32_t *sim, unsigned long addr)
{
	while (s32_get_pc (sim->cpu) != addr) {
		ss32_clock (sim, 1);

		if (sim->brk) {
			return (1);
		}
	}

	return (0);
}

static
int ss32_exec_off (sims32_t *sim, unsigned long addr)
{
	while (s32_get_pc (sim->cpu) == addr) {
		ss32_clock (sim, 1);

		if (sim->brk) {
			return (1);
		}
	}

	return (0);
}


#if 0
static
void ss32_log_opcode (void *ext, unsigned long ir)
{
	sims32_t *sim = ext;
}
#endif

static
void ss32_log_undef (void *ext, unsigned long ir)
{
	sims32_t *sim = ext;

	pce_log (MSG_DEB,
		"%08lX: undefined operation [%08lX]\n",
		(unsigned long) s32_get_pc (sim->cpu), ir
	);

	ss32_break (sim, PCE_BRK_STOP);
}

static
void ss32_log_trap (void *ext, unsigned tn)
{
	sims32_t *sim = ext;
	char     *name;

	switch (tn) {
		case 0x0300:
			name = "data store";
			return;

		default:
			name = "unknown";
			break;
	}

	pce_log (MSG_DEB, "%08lX: trap %x (%s)\n",
		(unsigned long) s32_get_pc (sim->cpu), tn, name
	);
}

static
void do_bc (cmd_t *cmd, sims32_t *sim)
{
	unsigned long addr;

	if (cmd_match_eol (cmd)) {
		bp_clear_all (&sim->brkpt);
		return;
	}

	if (!cmd_match_uint32 (cmd, &addr)) {
		cmd_error (cmd, "expecting address");
		return;
	}

	if (!cmd_match_end (cmd)) {
		return;
	}

	if (bp_clear (&sim->brkpt, addr, 0)) {
		printf ("no breakpoint cleared at %08lX\n", addr);
	}
}

static
void do_bl (cmd_t *cmd, sims32_t *sim)
{
	if (!cmd_match_end (cmd)) {
		return;
	}

	bp_list (sim->brkpt, 0);
}

static
void do_bs (cmd_t *cmd, sims32_t *sim)
{
	unsigned long  addr;
	unsigned short pass, reset;

	addr = s32_get_pc (sim->cpu);
	pass = 1;
	reset = 0;

	if (!cmd_match_uint32 (cmd, &addr)) {
		cmd_error (cmd, "expecting address");
		return;
	}

	cmd_match_uint16 (cmd, &pass);
	cmd_match_uint16 (cmd, &reset);

	if (!cmd_match_end (cmd)) {
		return;
	}

	if (pass > 0) {
		printf ("Breakpoint at %08lX  %04X  %04X\n",
			addr, pass, reset
		);

		bp_add (&sim->brkpt, addr, 0, pass, reset);
	}
}

static
void do_b (cmd_t *cmd, sims32_t *sim)
{
	if (cmd_match (cmd, "l")) {
		do_bl (cmd, sim);
	}
	else if (cmd_match (cmd, "s")) {
		do_bs (cmd, sim);
	}
	else if (cmd_match (cmd, "c")) {
		do_bc (cmd, sim);
	}
	else {
		cmd_error (cmd, "b: unknown command");
	}
}

static
void do_c (cmd_t *cmd, sims32_t *sim)
{
	unsigned long cnt;

	cnt = 1;

	cmd_match_uint32 (cmd, &cnt);

	if (!cmd_match_end (cmd)) {
		return;
	}

	while (cnt > 0) {
		ss32_clock (sim, 1);
		cnt -= 1;
	}

	ss32_prt_state_cpu (sim->cpu, stdout);
}

static
void do_d (cmd_t *cmd, sims32_t *sim)
{
	unsigned long        i, j;
	unsigned long        cnt;
	unsigned long        addr1, addr2;
	static int           first = 1;
	static unsigned long saddr = 0;
	unsigned long        p, p1, p2;
	char                 buf[256];

	if (first) {
		first = 0;
		saddr = s32_get_pc (sim->cpu);
	}

	addr1 = saddr;
	cnt = 256;

	if (cmd_match_uint32 (cmd, &addr1)) {
		cmd_match_uint32 (cmd, &cnt);
	}

	if (!cmd_match_end (cmd)) {
		return;
	}

	addr2 = (addr1 + cnt - 1) & 0xffffffffUL;
	if (addr2 < addr1) {
		addr2 = 0xffffffffUL;
		cnt = addr2 - addr1 + 1;
	}

	saddr = addr1 + cnt;

	p1 = addr1 / 16;
	p2 = addr2 / 16 + 1;

	for (p = p1; p < p2; p++) {
		j = 16 * p;

		sprintf (buf,
			"%08lX  xx xx xx xx xx xx xx xx-xx xx xx xx xx xx xx xx  xxxxxxxxxxxxxxxx\n",
			j
		);

		for (i = 0; i < 16; i++) {
			if ((j >= addr1) && (j <= addr2)) {
				uint8_t  val;
				unsigned val1, val2;

				if (s32_get_mem8 (sim->cpu, j, sim->cpu->asi_data, par_xlat, &val)) {
					val = 0xff;
				}

				val1 = (val >> 4) & 0x0f;
				val2 = val & 0x0f;

				buf[10 + 3 * i + 0] = (val1 < 10) ? ('0' + val1) : ('A' + val1 - 10);
				buf[10 + 3 * i + 1] = (val2 < 10) ? ('0' + val2) : ('A' + val2 - 10);

				if ((val >= 32) && (val <= 127)) {
					buf[59 + i] = val;
				}
				else {
					buf[59 + i] = '.';
				}
			}
			else {
				buf[10 + 3 * i] = ' ';
				buf[10 + 3 * i + 1] = ' ';
				buf[59 + i] = ' ';
			}

			j += 1;
		}

		fputs (buf, stdout);
	}
}

static
void do_e (cmd_t *cmd, sims32_t *sim)
{
	unsigned long  addr;
	unsigned short val;

	addr = 0;

	if (!cmd_match_uint32 (cmd, &addr)) {
		cmd_error (cmd, "need an address");
		return;
	}

	while (cmd_match_uint16 (cmd, &val)) {
		if (s32_set_mem8 (sim->cpu, addr, sim->cpu->asi_data, par_xlat, val)) {
			printf ("TLB miss: %08lx <- %02x\n", addr, (unsigned) val);
		}

		addr += 1;
	}
}

static
void do_g (cmd_t *cmd, sims32_t *sim)
{
	int           run;
	unsigned long addr;

	if (cmd_match (cmd, "b")) {
		run = 0;
	}
	else {
		run = 1;
	}

	if (cmd_match_uint32 (cmd, &addr)) {
		bp_add (&sim->brkpt, addr, 0, 1, 0);
		run = 0;
	}

	if (!cmd_match_end (cmd)) {
		return;
	}

	if (run) {
		pce_run();
	}
	else {
		ss32_run_bp (sim);

		fputs ("\n", stdout);
		ss32_prt_state_cpu (sim->cpu, stdout);
	}
}

static
void do_h (cmd_t *cmd, sims32_t *sim)
{
	fputs (
		"bc [addr]                 clear a breakpoint or all\n"
		"bl                        list breakpoints\n"
		"bs addr [pass [reset]]    set a breakpoint [pass=1 reset=0]\n"
		"c [cnt]                   clock\n"
		"d [addr [cnt]]            dump memory\n"
		"e addr [val...]           enter bytes into memory\n"
		"g [b]                     run with or without breakpoints (ESC to stop)\n"
		"key [val...]              send keycodes to the serial console\n"
		"p [cnt]                   execute cnt instructions, skip calls [1]\n"
		"q                         quit\n"
		"rett                      execute to next rett\n"
		"r reg [val]               set a register\n"
		"s [what]                  print status (cpu)\n"
		"t [cnt]                   execute cnt instructions [1]\n"
		"u [addr [cnt]]            disassemble\n"
		"v [expr...]               evaluate expressions\n",
		stdout
	);
}

static
void do_key (cmd_t *cmd, sims32_t *sim)
{
	unsigned short c;

	while (cmd_match_uint16 (cmd, &c)) {
		ss32_set_keycode (sim, c);
	}

	if (!cmd_match_end (cmd)) {
		return;
	}
}

static
void do_p (cmd_t *cmd, sims32_t *sim)
{
	unsigned long cnt;
	s32_dasm_t    da;

	cnt = 1;

	cmd_match_uint32 (cmd, &cnt);

	if (!cmd_match_end (cmd)) {
		return;
	}

	pce_start();

	while (cnt > 0) {
		s32_dasm_mem (sim->cpu, &da, s32_get_pc (sim->cpu), sim->cpu->asi_text, S32_XLAT_CPU);

		if (da.flags & S32_DFLAG_CALL) {
			if (ss32_exec_to (sim, s32_get_pc (sim->cpu) + 8)) {
				break;
			}
		}
		else {
			uint32_t psr;

			psr = s32_get_psr (sim->cpu);

			if (ss32_exec_off (sim, s32_get_pc (sim->cpu))) {
				break;
			}

			if ((psr & S32_PSR_S) == 0) {
				/* check if exception occured */
				while (s32_get_psr_s (sim->cpu)) {
					ss32_clock (sim, 1);

					if (sim->brk) {
						break;
					}
				}
			}
		}

		cnt -= 1;
	}

	pce_stop();

	ss32_prt_state_cpu (sim->cpu, stdout);
}

static
void do_rett (cmd_t *cmd, sims32_t *sim)
{
	s32_dasm_t da;

	if (!cmd_match_end (cmd)) {
		return;
	}

	pce_start();

	while (1) {
		s32_dasm_mem (sim->cpu, &da, s32_get_pc (sim->cpu), sim->cpu->asi_text, S32_XLAT_CPU);

		if (ss32_exec_off (sim, s32_get_pc (sim->cpu))) {
			break;
		}

		if (da.flags & S32_DFLAG_RETT) {
			break;
		}
	}

	pce_stop();

	ss32_prt_state_cpu (sim->cpu, stdout);
}

static
void do_r (cmd_t *cmd, sims32_t *sim)
{
	unsigned long val;
	uint32_t      *reg;

	if (!ss32_match_reg (cmd, sim, &reg)) {
		printf ("missing register\n");
		return;
	}

	if (cmd_match_eol (cmd)) {
		printf ("%08lx\n", (unsigned long) *reg);
		return;
	}

	if (!cmd_match_uint32 (cmd, &val)) {
		printf ("missing value\n");
		return;
	}

	if (!cmd_match_end (cmd)) {
		return;
	}

	*reg = val;

	ss32_prt_state_cpu (sim->cpu, stdout);
}

static
void do_s (cmd_t *cmd, sims32_t *sim)
{
	if (cmd_match_eol (cmd)) {
		ss32_prt_state_cpu (sim->cpu, stdout);
		return;
	}

	prt_state (sim, stdout, cmd_get_str (cmd));
}

static
void do_t (cmd_t *cmd, sims32_t *sim)
{
	unsigned long i, n;

	n = 1;

	cmd_match_uint32 (cmd, &n);

	if (!cmd_match_end (cmd)) {
		return;
	}

	pce_start();

	for (i = 0; i < n; i++) {
		ss32_exec (sim);
	}

	pce_stop();

	ss32_prt_state_cpu (sim->cpu, stdout);
}

static
void do_u (cmd_t *cmd, sims32_t *sim)
{
	unsigned             i;
	int                  to;
	unsigned long        addr, cnt;
	static unsigned int  first = 1;
	static unsigned long saddr = 0;
	s32_dasm_t           op;
	char                 str[256];

	if (first) {
		first = 0;
		saddr = s32_get_pc (sim->cpu);
	}

	to = 0;
	addr = saddr;
	cnt = 16;

	if (cmd_match (cmd, "-")) {
		to = 1;
	}

	if (cmd_match_uint32 (cmd, &addr)) {
		cmd_match_uint32 (cmd, &cnt);
	}

	if (!cmd_match_end (cmd)) {
		return;
	}

	if (to) {
		addr -= 4 * (cnt - 1);
	}

	for (i = 0; i < cnt; i++) {
		s32_dasm_mem (sim->cpu, &op, addr, sim->cpu->asi_text, par_xlat);
		ss32_dasm_str (str, &op);

		fprintf (stdout, "%08lX  %s\n", addr, str);

		addr += 4;
	}

	saddr = addr;
}

static
void do_v (cmd_t *cmd)
{
	unsigned long val;

	while (cmd_match_uint32 (cmd, &val)) {
		printf ("%lX\n", val);
	}

	if (!cmd_match_end (cmd)) {
		return;
	}
}

int ss32_do_cmd (cmd_t *cmd, sims32_t *sim)
{
	if (cmd_match (cmd, "b")) {
		do_b (cmd, sim);
	}
	else if (cmd_match (cmd, "c")) {
		do_c (cmd, sim);
	}
	else if (cmd_match (cmd, "d")) {
		do_d (cmd, sim);
	}
	else if (cmd_match (cmd, "e")) {
		do_e (cmd, sim);
	}
	else if (cmd_match (cmd, "g")) {
		do_g (cmd, sim);
	}
	else if (cmd_match (cmd, "h")) {
		do_h (cmd, sim);
	}
	else if (cmd_match (cmd, "key")) {
		do_key (cmd, sim);
	}
	else if (cmd_match (cmd, "p")) {
		do_p (cmd, sim);
	}
	else if (cmd_match (cmd, "rett")) {
		do_rett (cmd, sim);
	}
	else if (cmd_match (cmd, "r")) {
		do_r (cmd, sim);
	}
	else if (cmd_match (cmd, "s")) {
		do_s (cmd, sim);
	}
	else if (cmd_match (cmd, "t")) {
		do_t (cmd, sim);
	}
	else if (cmd_match (cmd, "u")) {
		do_u (cmd, sim);
	}
	else if (cmd_match (cmd, "v")) {
		do_v (cmd);
	}
	else {
		cmd_error (cmd, "unknown command");
		return (1);
	}

	return (0);
}

void ss32_cmd_init (sims32_t *sim)
{
	sim->cpu->log_ext = sim;
	sim->cpu->log_opcode = NULL;
	sim->cpu->log_undef = &ss32_log_undef;
	sim->cpu->log_exception = &ss32_log_trap;
}
