/* GemRB - Infinity Engine Emulator
 * Copyright (C) 2003 The GemRB Project
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.

 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.

 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
 *
 * $Header: /data/gemrb/cvs2svn/gemrb/gemrb/gemrb/plugins/Core/WorldMapControl.cpp,v 1.10 2004/11/21 12:57:30 avenger_teambg Exp $
 */

#ifndef WIN32
#include <sys/time.h>
#endif
#include "../../includes/win32def.h"
#include "WorldMapControl.h"
#include "Interface.h"

#define MAP_TO_SCREENX(x) XPos - ScrollX + (x)
#define MAP_TO_SCREENY(y) YPos - ScrollY + (y)

#define SCREEN_TO_MAPX(x) (x) - XPos + ScrollX
#define SCREEN_TO_MAPY(y) (y) - YPos + ScrollY

WorldMapControl::WorldMapControl(void)
{
	ScrollX = 0;
	ScrollY = 0;
	MouseIsDown = false;
	Changed = true;
	//lastCursor = 0;
}


WorldMapControl::~WorldMapControl(void)
{
}

/** Draws the Control on the Output Display */
void WorldMapControl::Draw(unsigned short /*x*/, unsigned short /*y*/)
{
	WorldMap* worldmap = core->GetWorldMap();
	if (!Width || !Height) {
		return;
	}
	if(!Changed)
		return;
	Changed = false;
	Video* video = core->GetVideoDriver();
	Region r( XPos, YPos, Width, Height );
	video->BlitSprite( worldmap->MapMOS, XPos - ScrollX, YPos - ScrollY, true, &r );


	std::vector< WMPAreaEntry*>::iterator m;

	for (m = worldmap->area_entries.begin(); m != worldmap->area_entries.end(); ++m) {
		if (! (*m)->AreaStatus & WMP_ENTRY_VISIBLE) continue;

		Region r2 = Region( MAP_TO_SCREENX((*m)->X), MAP_TO_SCREENY((*m)->Y), (*m)->MapIcon->Width, (*m)->MapIcon->Height );
		// if (xm >= (*m)->X && xm < (*m)->X + (*m)->MapIcon->Width && ym >= (*m)->Y && ym < (*m)->Y + (*m)->MapIcon->Height)
		video->BlitSprite( (*m)->MapIcon, MAP_TO_SCREENX((*m)->X), MAP_TO_SCREENY((*m)->Y), true, &r );

		// wmpty.bam
	}

	Font* fnt = core->GetButtonFont();

	// alpha bit is unfortunately ignored
	Color fore = {0x00, 0x00, 0x00, 0xff};
	Color back = {0x00, 0x00, 0x00, 0x00};
	Color* text_pal = core->GetVideoDriver()->CreatePalette( fore, back );

	// Draw WMP entry labels
	for (m = worldmap->area_entries.begin(); m != worldmap->area_entries.end(); ++m) {
		if (! (*m)->AreaStatus & WMP_ENTRY_VISIBLE) continue;

		Region r2 = Region( MAP_TO_SCREENX((*m)->X), MAP_TO_SCREENY((*m)->Y), (*m)->MapIcon->Width, (*m)->MapIcon->Height );
		if (r2.y+r2.h<r.y) continue;

		char *text = core->GetString( (*m)->LocCaptionName );
		int tw = fnt->CalcStringWidth( text ) + 5;
		int th = fnt->maxHeight;

		fnt->Print( Region( r2.x + (r2.w - tw)/2, r2.y + r2.h, tw, th ),
			    ( unsigned char * ) text, text_pal, 0, true );
	}
}

#if 0
/** Key Press Event */
void WorldmapControl::OnKeyPress(unsigned char Key, unsigned short Mod)
{
	HotKey=tolower(Key);
}
#endif

/** Key Release Event */
void WorldMapControl::OnKeyRelease(unsigned char Key, unsigned short Mod)
{
	//unsigned int i;

	switch (Key) {
		case '\t':
			//not GEM_TAB
			printf( "TAB released\n" );
			return;
		case 'f':
			if (Mod & 64)
				core->GetVideoDriver()->ToggleFullscreenMode();
			break;
		case 'g':
			if (Mod & 64)
				core->GetVideoDriver()->ToggleGrabInput();
			break;
		default:
			break;
	}
	if (!core->CheatEnabled()) {
		return;
	}
	if (Mod & 64) //ctrl
	{
	}
}
void WorldMapControl::AdjustScrolling(short x, short y)
{
	WorldMap* worldmap = core->GetWorldMap();
	ScrollX += x;
	ScrollY += y;
	if (ScrollX > worldmap->MapMOS->Width - Width)
		ScrollX = worldmap->MapMOS->Width - Width;
	if (ScrollY > worldmap->MapMOS->Height - Height)
		ScrollY = worldmap->MapMOS->Height - Height;
	if (ScrollX < 0)
		ScrollX = 0;
	if (ScrollY < 0)
		ScrollY = 0;
	Changed = true;
}

/** Mouse Over Event */
void WorldMapControl::OnMouseOver(unsigned short x, unsigned short y)
{
	WorldMap* worldmap = core->GetWorldMap();
	int nextCursor = 44; //this is the grabbing hand

	if (MouseIsDown) {
		AdjustScrolling(lastMouseX-x, lastMouseY-y);
	}

	lastMouseX = x;
	lastMouseY = y;

	if (Value) {
		x += ScrollX;
		y += ScrollY;

		std::vector< WMPAreaEntry*>::iterator m;
		for (m = worldmap->area_entries.begin(); m != worldmap->area_entries.end(); ++m) {
			if ((*m)->X <= x &&
			(*m)->X + (*m)->MapIcon->Width > x &&
			(*m)->Y <= y &&
			(*m)->Y + (*m)->MapIcon->Height > y) {
				printf("A: %s\n", (*m)->AreaName);
				nextCursor = 0; //we are over an area!
			}
		}
	}

	( ( Window * ) Owner )->Cursor = nextCursor;
}

/** Mouse Leave Event */
void WorldMapControl::OnMouseLeave(unsigned short /*x*/, unsigned short /*y*/)
{
	( ( Window * ) Owner )->Cursor = 0;
}

/** Mouse Button Down */
void WorldMapControl::OnMouseDown(unsigned short x, unsigned short y,
	unsigned char Button, unsigned short /*Mod*/)
{
	if ((Button != GEM_MB_ACTION) ) {
		return;
	}
	//short WorldmapX = x, WorldmapY = y;
	//core->GetVideoDriver()->ConvertToWorldmap( WorldmapX, WorldmapY );
	MouseIsDown = true;
	lastMouseX = x;
	lastMouseY = y;
}
/** Mouse Button Up */
void WorldMapControl::OnMouseUp(unsigned short /*x*/, unsigned short /*y*/,
	unsigned char Button, unsigned short /*Mod*/)
{
	if (Button != GEM_MB_ACTION) {
		return;
	}

	MouseIsDown = false;
#if 0
		if(overInfoPoint) {
			if(HandleActiveRegion(overInfoPoint, selected[0])) {
				return;
			}
		}
#endif
}

/** Special Key Press */
void WorldMapControl::OnSpecialKeyPress(unsigned char Key)
{
	WorldMap* worldmap = core->GetWorldMap();
	switch (Key) {
		case GEM_LEFT:
			ScrollX -= 64;
			break;
		case GEM_UP:
			ScrollY -= 64;
			break;
		case GEM_RIGHT:
			ScrollX += 64;
			break;
		case GEM_DOWN:
			ScrollY += 64;
			break;
		case GEM_ALT:
			printf( "ALT pressed\n" );
			break;
		case GEM_TAB:
			printf( "TAB pressed\n" );
			break;
	}

	if (ScrollX > worldmap->MapMOS->Width - Width)
		ScrollX = worldmap->MapMOS->Width - Width;
	if (ScrollY > worldmap->MapMOS->Height - Height)
		ScrollY = worldmap->MapMOS->Height - Height;
	if (ScrollX < 0)
		ScrollX = 0;
	if (ScrollY < 0)
		ScrollY = 0;
}

