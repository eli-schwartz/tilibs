/* Hey EMACS -*- linux-c -*- */
/* $Id: link_nul.c 1059 2005-05-14 09:45:42Z roms $ */

/*  libCables - Ti Link Cable library, a part of the TiLP project
 *  Copyright (C) 1999-2005  Romain Lievin
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
 *  Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.
 */

/* Initialize the LinkCable structure with default functions */
/* This module can be used as sample code.*/

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include <stdio.h>
#include <time.h>

#include "ticalcs.h"
#include "gettext.h"
#include "logging.h"
#include "error.h"
#include "pause.h"

#include "cmd89.h"
#include "rom89.h"
#include "keys89.h"

// Screen coordinates of the TI89
#define TI89_ROWS          128
#define TI89_COLS          240
#define TI89_ROWS_VISIBLE  100
#define TI89_COLS_VISIBLE  160

#define DUMP_ROM89_FILE "dumprom.89z"
//#define ROMSIZE (2*1024)	// 2MB or 4MB (Titanium)

static int		is_ready	(CalcHandle* handle)
{
	uint16_t status;

	TRYF(ti89_send_RDY());
	TRYF(ti89_recv_ACK(&status));

	return (status & 0x01) ? ERR_NOT_READY : 0;
}

static int		send_key	(CalcHandle* handle, uint16_t key)
{
	TRYF(send_key(handle, key));
	TRYF(ti89_recv_ACK(NULL));

	return 0;
}

static int		recv_screen	(CalcHandle* handle, CalcScreenCoord* sc, uint8_t** bitmap)
{
	uint32_t max_cnt;
	int err;

	sc->width = TI89_COLS;
	sc->height = TI89_ROWS;
	switch (handle->model) 
	{
	case CALC_TI89:
	case CALC_TI89T:
		sc->clipped_width = TI89_COLS_VISIBLE;
		sc->clipped_height = TI89_ROWS_VISIBLE;
    break;
	case CALC_TI92P:
	case CALC_V200:
		sc->clipped_width = TI89_COLS;
		sc->clipped_height = TI89_ROWS;
    break;
	}

	*bitmap = (uint8_t *) malloc(TI89_COLS * TI89_ROWS * sizeof(uint8_t) / 8);
	if(*bitmap == NULL) 
		return ERR_MALLOC;

	TRYF(ti89_send_SCR());
	TRYF(ti89_recv_ACK(NULL));

	err = ti89_recv_XDP(&max_cnt, *bitmap);	// pb with checksum
	if (err != ERR_CHECKSUM) { TRYF(err) };
	TRYF(ti89_send_ACK());

	// Clip the unused part of the screen (nethertheless useable witha asm prog)
	if (((handle->model == CALC_TI89) || (handle->model == CALC_TI89T))
      && (sc->format == SCREEN_CLIPPED)) 
	{
		int i, j, k;

		for (i = 0, j = 0; j < TI89_ROWS_VISIBLE; j++)
			for (k = 0; k < (TI89_COLS_VISIBLE >> 3); k++)
				*bitmap[i++] = *bitmap[j * (TI89_COLS >> 3) + k];
	}
	

	return 0;
}

static int		get_dirlist	(CalcHandle* handle, TNode** vars, TNode** apps)
{
	return 0;
}

static int		get_memfree	(CalcHandle* handle, uint32_t* mem)
{
	return 0;
}

static int		send_backup	(CalcHandle* handle, BackupContent* content)
{
	return 0;
}

static int		recv_backup	(CalcHandle* handle, BackupContent* content)
{
	return 0;
}

static int		send_var	(CalcHandle* handle, CalcMode mode, FileContent* content)
{
	return 0;
}

static int		recv_var	(CalcHandle* handle, CalcMode mode, FileContent* content, VarRequest* vr)
{
	return 0;
}

static int		del_var		(CalcHandle* handle, VarRequest* vr)
{
	return 0;
}

static int		send_var_ns	(CalcHandle* handle, CalcMode mode, FileContent* content)
{
	return 0;
}

static int		recv_var_ns	(CalcHandle* handle, CalcMode mode, FileContent* content, VarEntry* ve)
{
	return 0;
}

static int		send_flash	(CalcHandle* handle, FlashContent* content)
{
	return 0;
}

static int		recv_flash	(CalcHandle* handle, FlashContent* content, VarRequest* vr)
{
	return 0;
}

static int		recv_idlist	(CalcHandle* handle, uint8_t* idlist)
{
	return 0;
}

static int		dump_rom	(CalcHandle* handle, CalcDumpSize size, const char *filename)
{
	int i, j;
	uint8_t data;
	time_t start, elapsed, estimated, remaining;
	char buffer[257];
	char tmp[257];
	int pad;
	FILE *f, *file;
	uint16_t checksum, sum;
	int ROMSIZE = (handle->model == CALC_TI89T) || 
  					(handle->model == CALC_V200) ? 4*1024 : 2*1024;
	FileContent content;

	// Copies ROM dump program into a file
	f = fopen(DUMP_ROM89_FILE, "wb");
	if (f == NULL)
		return ERR_FILE_OPEN;

	fwrite(romDump89, sizeof(unsigned char), romDumpSize89, f);
	fclose(f);

	// Transfer program to calc
	tifiles_file_read_regular(DUMP_ROM89_FILE, &content);
	TRYF(send_var(handle, MODE_SEND_ONE_VAR, &content));
	tifiles_content_free_regular(&content);
	unlink(DUMP_ROM89_FILE);

	// Launch calculator program by remote control
	sprintf(handle->update->text, _("Launching..."));
	handle->update->label();

	TRYF(send_key(handle, KEY89_CLEAR));
	PAUSE(50);
    TRYF(send_key(handle, KEY89_CLEAR));
    PAUSE(50);
    TRYF(send_key(handle, 'm'));
    TRYF(send_key(handle, 'a'));
    TRYF(send_key(handle, 'i'));
    TRYF(send_key(handle, 'n'));
    TRYF(send_key(handle, '\\'));
    TRYF(send_key(handle, 'r'));
    TRYF(send_key(handle, 'o'));
    TRYF(send_key(handle, 'm'));
    TRYF(send_key(handle, 'd'));
    TRYF(send_key(handle, 'u'));
    TRYF(send_key(handle, 'm'));
    TRYF(send_key(handle, 'p'));
    TRYF(send_key(handle, KEY89_LP));
    TRYF(send_key(handle, KEY89_RP));
    TRYF(send_key(handle, KEY89_ENTER));

	// Open file
	file = fopen(filename, "wb");
	if (file == NULL)
		return ERR_OPEN_FILE;

	// Receive it now blocks per blocks (1024 + CHK)
	sprintf(handle->update->text, _("Receiving..."));
	handle->update->label();

	start = time(NULL);
	handle->update->max1 = 1024;
	handle->update->max2 = 1024 * size;

	for (i = 0; i < ROMSIZE; i++) 
	{
		sum = 0;

		for (j = 0; j < 1024; j++) 
		{
			TRYF(ticables_cable_get(handle->cable, &data));
			fprintf(file, "%c", data);
			sum += data;

			handle->update->cnt1 = j;
			handle->update->pbar();
			if (handle->update->cancel)
				return -1;
		}

		TRYF(ticables_cable_get(handle->cable, &data));
		checksum = data << 8;
		TRYF(ticables_cable_get(handle->cable, &data));
		checksum |= data;
		if (sum != checksum)
		return ERR_CHECKSUM;
		TRYF(ticables_cable_put(handle->cable, 0xDA));

		handle->update->cnt2 = i;
		if (handle->update->cancel)
			return -1;

		elapsed = (long) difftime(time(NULL), start);
		estimated = (long) (elapsed * (float) (ROMSIZE) / i);
		remaining = (long) difftime(estimated, elapsed);
		sprintf(buffer, "%s", ctime(&remaining));
		sscanf(buffer, "%3s %3s %i %s %i", tmp, tmp, &pad, tmp, &pad);
		sprintf(handle->update->text, _("Remaining (mm:ss): %s"), tmp + 3);
		handle->update->label();
	}

	// make ROM dumping program exit.
	TRYF(ticables_cable_put(handle->cable, 0xCC));
	fclose(file);

	return 0;
}

static int		set_clock	(CalcHandle* handle, CalcClock* clock)
{
	return 0;
}

static int		get_clock	(CalcHandle* handle, CalcClock* clock)
{
	return 0;
}

const CalcFncts calc_89 = 
{
	CALC_TI89,
	"TI89",
	N_("TI-89"),
	N_("TI-89"),
	OPS_ISREADY | OPS_KEYS | OPS_SCREEN | OPS_DIRLIST | OPS_BACKUP | OPS_VARS | 
	OPS_FLASH | OPS_IDLIST | OPS_CLOCK | OPS_ROMDUMP |
	FTS_SILENT | FTS_FOLDER | FTS_FLASH,
	&is_ready,
	&send_key,
	&recv_screen,
	&get_dirlist,
	&get_memfree,
	&send_backup,
	&recv_backup,
	&send_var,
	&recv_var,
	&del_var,
	&send_var_ns,
	&recv_var_ns,
	&send_flash,
	&recv_flash,
	&recv_idlist,
	&dump_rom,
	&set_clock,
	&get_clock,
};

const CalcFncts calc_92p = 
{
	CALC_TI92P,
	"TI92+",
	N_("TI-92 Plus"),
	N_("TI-92 Plus"),
	OPS_ISREADY | OPS_KEYS | OPS_SCREEN | OPS_DIRLIST | OPS_BACKUP | OPS_VARS | 
	OPS_FLASH | OPS_IDLIST | OPS_CLOCK | OPS_ROMDUMP |
	FTS_SILENT | FTS_FOLDER | FTS_FLASH,
	&is_ready,
	&send_key,
	&recv_screen,
	&get_dirlist,
	&get_memfree,
	&send_backup,
	&recv_backup,
	&send_var,
	&recv_var,
	&del_var,
	&send_var_ns,
	&recv_var_ns,
	&send_flash,
	&recv_flash,
	&recv_idlist,
	&dump_rom,
	&set_clock,
	&get_clock,
};

const CalcFncts calc_89t = 
{
	CALC_TI89T,
	"TI89t",
	N_("TI-89 Titanium"),
	N_("TI-89 Titanium"),
	OPS_ISREADY | OPS_KEYS | OPS_SCREEN | OPS_DIRLIST | OPS_BACKUP | OPS_VARS | 
	OPS_FLASH | OPS_IDLIST | OPS_CLOCK | OPS_ROMDUMP |
	FTS_SILENT | FTS_FOLDER | FTS_FLASH,
	&is_ready,
	&send_key,
	&recv_screen,
	&get_dirlist,
	&get_memfree,
	&send_backup,
	&recv_backup,
	&send_var,
	&recv_var,
	&del_var,
	&send_var_ns,
	&recv_var_ns,
	&send_flash,
	&recv_flash,
	&recv_idlist,
	&dump_rom,
	&set_clock,
	&get_clock,
};

const CalcFncts calc_v2 = 
{
	CALC_V200,
	"V200",
	N_("V200PLT"),
	N_("V200 Portable Learning Tool"),
	OPS_ISREADY | OPS_KEYS | OPS_SCREEN | OPS_DIRLIST | OPS_BACKUP | OPS_VARS | 
	OPS_FLASH | OPS_IDLIST | OPS_CLOCK | OPS_ROMDUMP |
	FTS_SILENT | FTS_FOLDER | FTS_FLASH,
	&is_ready,
	&send_key,
	&recv_screen,
	&get_dirlist,
	&get_memfree,
	&send_backup,
	&recv_backup,
	&send_var,
	&recv_var,
	&del_var,
	&send_var_ns,
	&recv_var_ns,
	&send_flash,
	&recv_flash,
	&recv_idlist,
	&dump_rom,
	&set_clock,
	&get_clock,
};