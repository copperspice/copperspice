/***********************************************************************
*
* Copyright (c) 2012-2018 Barbara Geller
* Copyright (c) 2012-2018 Ansel Sermersheim
* Copyright (c) 2012-2016 Digia Plc and/or its subsidiary(-ies).
* Copyright (c) 2008-2012 Nokia Corporation and/or its subsidiary(-ies).
* All rights reserved.
*
* This file is part of CopperSpice.
*
* CopperSpice is free software. You can redistribute it and/or
* modify it under the terms of the GNU Lesser General Public License
* version 2.1 as published by the Free Software Foundation.
*
* CopperSpice is distributed in the hope that it will be useful,
* but WITHOUT ANY WARRANTY; without even the implied warranty of
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
*
* <http://www.gnu.org/licenses/>.
*
***********************************************************************/

#include <qcolor.h>
#include <stdio.h>
#include <stdlib.h>
#include <limits.h>


#define APPLE_CMAP 1

struct Col {
    int r,g,b;
};

#if SPACE_SEARCH
#define MAPSIZE 256
#define ACCURACY 4     // Bits-per-channel
#define SPACESIZE ((1<<ACCURACY)*(1<<ACCURACY)*(1<<ACCURACY))
#define R(c) (((c>>(8-ACCURACY)*2)&((1<<ACCURACY)-1))<<ACCURACY)
#define G(c) (((c>>(8-ACCURACY))&((1<<ACCURACY)-1))<<ACCURACY)
#define B(c) (((c>>0)&((1<<ACCURACY)-1))<<ACCURACY)
#define COL(c) (((c.r>>(8-ACCURACY))<<8)|((c.b>>(8-ACCURACY))<<4)|(c.g>>(8-ACCURACY)))
#elif APPLE_CMAP
#define SPACESIZE 216
#define MAPSIZE 216
#define R(c) ((apple_cmap[c]>>16)&0xff)
#define G(c) ((apple_cmap[c]>>8)&0xff)
#define B(c) ((apple_cmap[c]>>0)&0xff)
#define COL(c) findapple(c)
static int apple_cmap[216] = {
0xffffff,
0xffffcc,
0xffff99,
0xffff66,
0xffff33,
0xffff00,
0xffccff,
0xffcccc,
0xffcc99,
0xffcc66,
0xffcc33,
0xffcc00,
0xff99ff,
0xff99cc,
0xff9999,
0xff9966,
0xff9933,
0xff9900,
0xff66ff,
0xff66cc,
0xff6699,
0xff6666,
0xff6633,
0xff6600,
0xff33ff,
0xff33cc,
0xff3399,
0xff3366,
0xff3333,
0xff3300,
0xff00ff,
0xff00cc,
0xff0099,
0xff0066,
0xff0033,
0xff0000,
0xccffff,
0xccffcc,
0xccff99,
0xccff66,
0xccff33,
0xccff00,
0xccccff,
0xcccccc,
0xcccc99,
0xcccc66,
0xcccc33,
0xcccc00,
0xcc99ff,
0xcc99cc,
0xcc9999,
0xcc9966,
0xcc9933,
0xcc9900,
0xcc66ff,
0xcc66cc,
0xcc6699,
0xcc6666,
0xcc6633,
0xcc6600,
0xcc33ff,
0xcc33cc,
0xcc3399,
0xcc3366,
0xcc3333,
0xcc3300,
0xcc00ff,
0xcc00cc,
0xcc0099,
0xcc0066,
0xcc0033,
0xcc0000,
0x99ffff,
0x99ffcc,
0x99ff99,
0x99ff66,
0x99ff33,
0x99ff00,
0x99ccff,
0x99cccc,
0x99cc99,
0x99cc66,
0x99cc33,
0x99cc00,
0x9999ff,
0x9999cc,
0x999999,
0x999966,
0x999933,
0x999900,
0x9966ff,
0x9966cc,
0x996699,
0x996666,
0x996633,
0x996600,
0x9933ff,
0x9933cc,
0x993399,
0x993366,
0x993333,
0x993300,
0x9900ff,
0x9900cc,
0x990099,
0x990066,
0x990033,
0x990000,
0x66ffff,
0x66ffcc,
0x66ff99,
0x66ff66,
0x66ff33,
0x66ff00,
0x66ccff,
0x66cccc,
0x66cc99,
0x66cc66,
0x66cc33,
0x66cc00,
0x6699ff,
0x6699cc,
0x669999,
0x669966,
0x669933,
0x669900,
0x6666ff,
0x6666cc,
0x666699,
0x666666,
0x666633,
0x666600,
0x6633ff,
0x6633cc,
0x663399,
0x663366,
0x663333,
0x663300,
0x6600ff,
0x6600cc,
0x660099,
0x660066,
0x660033,
0x660000,
0x33ffff,
0x33ffcc,
0x33ff99,
0x33ff66,
0x33ff33,
0x33ff00,
0x33ccff,
0x33cccc,
0x33cc99,
0x33cc66,
0x33cc33,
0x33cc00,
0x3399ff,
0x3399cc,
0x339999,
0x339966,
0x339933,
0x339900,
0x3366ff,
0x3366cc,
0x336699,
0x336666,
0x336633,
0x336600,
0x3333ff,
0x3333cc,
0x333399,
0x333366,
0x333333,
0x333300,
0x3300ff,
0x3300cc,
0x330099,
0x330066,
0x330033,
0x330000,
0x00ffff,
0x00ffcc,
0x00ff99,
0x00ff66,
0x00ff33,
0x00ff00,
0x00ccff,
0x00cccc,
0x00cc99,
0x00cc66,
0x00cc33,
0x00cc00,
0x0099ff,
0x0099cc,
0x009999,
0x009966,
0x009933,
0x009900,
0x0066ff,
0x0066cc,
0x006699,
0x006666,
0x006633,
0x006600,
0x0033ff,
0x0033cc,
0x003399,
0x003366,
0x003333,
0x003300,
0x0000ff,
0x0000cc,
0x000099,
0x000066,
0x000033,
0x000000,
};
int findapple(Col c)
{
    for (int i=0; i<216; i++)
	if (apple_cmap[i]==(c.r<<16)|(c.g<<8)|c.b) return i;
    abort();
}
#endif

#define SQ(x) ((x)*(x))
#define D(c1,c2) (SQ(R(c1)-R(c2))+SQ(G(c1)-G(c2))+SQ(B(c1)-B(c2)))

main()
{
    Col c[256] = {
	{ 0,0,0 },
	{ 255,255,255 },
	{ 255,0,0 }, { 0,255,0 }, { 0,0,255 },
	{ 255,255,0 }, { 0,255,255 }, { 255,0,255 },
	#define PREALLOC 8
	{ 96,96,96 }, { 192,192,192 },
	//#define PREALLOC 10
    };
    int done[SPACESIZE];
    for (int a=0; a<SPACESIZE; a++) done[a]=0;
    for (int a=0; a<PREALLOC; a++) done[COL(c[a])]=1;

    for (int allocated=PREALLOC; allocated<MAPSIZE; allocated++) {
	int mostdist;
	int dist=0;
	for (int a=0; a<SPACESIZE; a++) {
	    if (!done[a]) {
		int closeness=INT_MAX;
		for (int b=0; b<SPACESIZE; b++) {
		    if (done[b]) {
			int d=D(a,b);
			if (d < closeness) {
			    closeness=d;
			}
		    }
		}
		if (closeness > dist) {
		    mostdist=a;
		    dist=closeness;
		}
	    }
	}
	c[allocated].r=R(mostdist);
	c[allocated].g=G(mostdist);
	c[allocated].b=B(mostdist);
	done[mostdist]=1;
	fprintf(stderr,"Done %d of %d (%06x dist %d)\n",allocated+1,MAPSIZE,
	    qRgb(c[allocated].r, c[allocated].g, c[allocated].b), dist);
    }

    for (int i=0; i<256; i++) {
	printf("0x%06x,%c", qRgb(c[i].r, c[i].g, c[i].b), i%4==3 ? '\n' : ' ');
    }
}
