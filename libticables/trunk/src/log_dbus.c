/* Hey EMACS -*- linux-c -*- */
/* $Id: data_log.c 1720 2006-01-20 22:34:58Z roms $ */

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

/* 
	D-BUS logging.
*/

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include <glib/gstdio.h>
#include <stdio.h>
#include <glib.h>

#include "logging.h"
#include "data_log.h"

#define HEX_FILE	"ticables-hex.log"
#define LOG_FILE	"ticables-dbus.log"

static char *ifn = NULL;
static char *ofn = NULL;

int log_dbus_start(void)
{
  // build filenames
#ifdef __WIN32__
	ifn = g_strconcat("C:\\", HEX_FILE, NULL);
	ofn = g_strconcat("C:\\", LOG_FILE, NULL);
#else
	ifn = g_strconcat(g_get_home_dir(), "/", HEX_FILE, NULL);
	ofn = g_strconcat(g_get_home_dir(), "/", LOG_FILE, NULL);
#endif

  	return 0;
}

int log_dbus_1(int dir, uint8_t data)
{
  	return 0;
}

extern int pkdecomp(const char *filename, int resync);

int log_dbus_stop(void)
{
	char *r;

	r = strrchr(ifn, '.');
	if(r) *r = '\0';

	pkdecomp(ifn, 0);	// ticables-hex.log -> ticables-hex.pkt
	strcat(ifn, ".pkt");

	g_unlink(ofn);
	g_rename(ifn, ofn);

	g_free(ifn);
	g_free(ofn);

  	return 0;
}
