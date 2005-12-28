/*****************************************************************************
 * pce                                                                       *
 *****************************************************************************/

/*****************************************************************************
 * File name:     src/devices/block/blkdosem.c                               *
 * Created:       2004-09-17 by Hampa Hug <hampa@hampa.ch>                   *
 * Last modified: 2004-12-03 by Hampa Hug <hampa@hampa.ch>                   *
 * Copyright:     (C) 2004 Hampa Hug <hampa@hampa.ch>                        *
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


#include "blkdosem.h"

#include <stdlib.h>
#include <string.h>


static
int dsk_dosemu_read (disk_t *dsk, void *buf, uint32_t i, uint32_t n)
{
	disk_dosemu_t *img;
	uint64_t      ofs, cnt;

	if ((i + n) > dsk->blocks) {
		return (1);
	}

	img = dsk->ext;

	ofs = img->start + 512 * (uint64_t) i;
	cnt = 512 * (uint64_t) n;

	if (dsk_read (img->fp, buf, ofs, cnt)) {
		return (1);
	}

	return (0);
}

static
int dsk_dosemu_write (disk_t *dsk, const void *buf, uint32_t i, uint32_t n)
{
	disk_dosemu_t *img;
	uint64_t      ofs, cnt;

	if (dsk->readonly) {
		return (1);
	}

	if ((i + n) > dsk->blocks) {
		return (1);
	}

	img = dsk->ext;

	ofs = img->start + 512 * (uint64_t) i;
	cnt = 512 * (uint64_t) n;

	if (dsk_write (img->fp, buf, ofs, cnt)) {
		return (1);
	}

	fflush (img->fp);

	return (0);
}

static
void dsk_dosemu_del (disk_t *dsk)
{
	disk_dosemu_t *img;

	img = dsk->ext;

	fclose (img->fp);
	free (img);
}

disk_t *dsk_dosemu_open_fp (FILE *fp, int ro)
{
	disk_dosemu_t *img;
	unsigned char buf[64];
	uint32_t      c, h, s;
	uint64_t      start;

	if (dsk_read (fp, buf, 0, 23)) {
		return (NULL);
	}

	if (memcmp (buf, "DOSEMU\x00", 7) != 0) {
		return (NULL);
	}

	c = dsk_get_uint32_le (buf, 15);
	h = dsk_get_uint32_le (buf, 7);
	s = dsk_get_uint32_le (buf, 11);
	start = dsk_get_uint32_le (buf, 19);

	if (start < 23) {
		return (NULL);
	}

	img = malloc (sizeof (disk_dosemu_t));
	if (img == NULL) {
		return (NULL);
	}

	dsk_init (&img->dsk, img, c, h, s);

	dsk_set_readonly (&img->dsk, ro);

	img->dsk.del = dsk_dosemu_del;
	img->dsk.read = dsk_dosemu_read;
	img->dsk.write = dsk_dosemu_write;

	img->start = start;

	img->fp = fp;

	return (&img->dsk);
}

disk_t *dsk_dosemu_open (const char *fname, int ro)
{
	disk_t *dsk;
	FILE   *fp;

	if (ro) {
		fp = fopen (fname, "rb");
	}
	else {
		fp = fopen (fname, "r+b");
	}

	if (fp == NULL) {
		return (NULL);
	}

	dsk = dsk_dosemu_open_fp (fp, ro);

	if (dsk == NULL) {
		fclose (fp);
		return (NULL);
	}

	return (dsk);
}

int dsk_dosemu_create_fp (FILE *fp, uint32_t c, uint32_t h, uint32_t s)
{
	uint64_t      cnt;
	unsigned char buf[128];

	cnt = 512 * (uint64_t) (c * h * s);
	if (cnt == 0) {
		return (1);
	}

	memset (buf, 0, 128);

	memcpy (buf, "DOSEMU\x00", 7);

	dsk_set_uint32_le (buf, 7, h);
	dsk_set_uint32_le (buf, 11, s);
	dsk_set_uint32_le (buf, 15, c);
	dsk_set_uint32_le (buf, 19, 128);

	if (dsk_write (fp, buf, 0, 128)) {
		return (1);
	}

	buf[0] = 0;
	if (dsk_write (fp, buf, 128 + cnt - 1, 1)) {
		return (1);
	}

	return (0);
}

int dsk_dosemu_create (const char *fname, uint32_t c, uint32_t h, uint32_t s)
{
	int  r;
	FILE *fp;

	fp = fopen (fname, "w+b");
	if (fp == NULL) {
		return (1);
	}

	r = dsk_dosemu_create_fp (fp, c, h, s);

	fclose (fp);

	return (r);
}
