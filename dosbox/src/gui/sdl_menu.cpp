/*
 *  Copyright (C) 2002-2013  The DOSBox Team
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

#include <stdio.h>
#include <cstring>
 
#include <SDL/SDL.h>
#include <SDL/SDL_image.h>
#include <SDL/SDL_gfxPrimitives.h>
#include <SDL/SDL_gfxPrimitives_font.h>

#include "dosbox.h"
#include "video.h"
#include "render.h"
#include "cpu.h"

#define MENU_ITEMS 7

extern Bitu CPU_extflags_toggle;

bool dynamic_available = false;
bool menu_active = false;
bool menu_last = false;
bool keystates[1024];

struct MENU_Block 
{
    SDL_Surface *surface;
    int selected;
    char *frameskip;
    char *cycles;
    char *core;
    char *cpuType;
    bool doublebuf;
};

static MENU_Block menu;

const char *menuoptions[MENU_ITEMS] = {
    "Resume",
    "Frameskip: ",
    "Cycles: ",
    "CPU Core: ",
    "CPU Type: ",
    "Triple Buffer: ",
    "Exit"
};


void MENU_Init(int bpp)
{
    if(!menu.surface) 
    {
        menu.surface = SDL_CreateRGBSurface(SDL_SWSURFACE, 320, 240, bpp, 0, 0, 0, 0);
    }
    
    menu.selected = 0;
    menu.frameskip = (char*)malloc(16);
    menu.cycles = (char*)malloc(16);
    menu.core = (char*)malloc(16);
    menu.cpuType = (char*)malloc(16);
    menu.doublebuf = GFX_IsDoubleBuffering();
    
#if (C_DYNREC)
    if(cpudecoder == &CPU_Core_Dynrec_Run) dynamic_available = true;
#endif 
}

void MENU_Deinit()
{
    SDL_FreeSurface(menu.surface);
}

void MENU_UpdateMenu()
{
    // Frameskip
    sprintf(menu.frameskip, "%i", render.frameskip.max);
    
    // CPU Cycles
    if(CPU_AutoDetermineMode & CPU_AUTODETERMINE_CYCLES) strcpy(menu.cycles, "auto");
    else if(CPU_CycleAutoAdjust) strcpy(menu.cycles, "max");
    else sprintf(menu.cycles, "%i", CPU_CycleMax);
    
    // CPU Core
    if(CPU_AutoDetermineMode & CPU_AUTODETERMINE_CORE) strcpy(menu.core, "auto");
    else if(cpudecoder == &CPU_Core_Normal_Run) strcpy(menu.core, "normal");
    else if(cpudecoder == &CPU_Core_Simple_Run) strcpy(menu.core, "simple");
    else if(cpudecoder == &CPU_Core_Full_Run) strcpy(menu.core, "full");
#if (C_DYNREC)
    else if(cpudecoder == &CPU_Core_Dynrec_Run) strcpy(menu.core, "dynamic");
#endif
    else if(cpudecoder ==  &CPU_Core_Prefetch_Run) strcpy(menu.core, "prefetch");
    else strcpy(menu.core, "unknown");
    
    if(CPU_AutoDetermineMode & CPU_AUTODETERMINE_CORE) strcpy(menu.core, "auto");
    
    // CPU Type
    if(CPU_ArchitectureType == CPU_ARCHTYPE_MIXED) strcpy(menu.cpuType, "auto");
    else if(CPU_ArchitectureType == CPU_ARCHTYPE_386FAST && cpudecoder == &CPU_Core_Prefetch_Run) strcpy(menu.cpuType, "386 (Prefetch)");
    else if(CPU_ArchitectureType == CPU_ARCHTYPE_386FAST) strcpy(menu.cpuType, "386");
    else if(CPU_ArchitectureType == CPU_ARCHTYPE_386SLOW) strcpy(menu.cpuType, "386 (Slow)");
    else if(CPU_ArchitectureType == CPU_ARCHTYPE_486NEWSLOW && cpudecoder == &CPU_Core_Prefetch_Run) strcpy(menu.cpuType, "486 (Prefetch)");
    else if(CPU_ArchitectureType == CPU_ARCHTYPE_486NEWSLOW) strcpy(menu.cpuType, "486 (Slow)");
    else if(CPU_ArchitectureType == CPU_ARCHTYPE_PENTIUMSLOW) strcpy(menu.cpuType, "Pentium (Slow)");
    else strcpy(menu.cpuType, "Unknown");
}

void MENU_Toggle()
{
    menu_active = !menu_active;
    
    if(!menu_active)
    {
        if(GFX_IsDoubleBuffering() != menu.doublebuf) GFX_SwitchDoubleBuffering();
    }
    else
    {
        menu.doublebuf = GFX_IsDoubleBuffering();
    }
    
    for(int i=0; i<1024; i++) keystates[i] = false;
    
    MENU_UpdateMenu();
}

void MENU_MoveCursor(int direction)
{
    menu.selected += direction;
    
    if(menu.selected < 0) menu.selected = MENU_ITEMS-1;
    if(menu.selected > MENU_ITEMS-1) menu.selected = 0;
}

void MENU_Activate()
{
    switch(menu.selected)
    {
        case 0: // Resume
            MENU_Toggle();
            break;
            
        case 2: // Resume
            
            if(CPU_AutoDetermineMode & CPU_AUTODETERMINE_CYCLES)
            { // Max
                CPU_CycleAutoAdjust = 1;
                CPU_AutoDetermineMode ^= CPU_AUTODETERMINE_CYCLES;
            }
            else if(CPU_CycleAutoAdjust)
            { // Fixed
                CPU_CycleAutoAdjust = 0;
            }
            else
            { // Auto
                CPU_AutoDetermineMode |= CPU_AUTODETERMINE_CYCLES;
            }
            
            break;
            
        case 3:
            
            if(CPU_AutoDetermineMode & CPU_AUTODETERMINE_CORE) 
            {
                CPU_AutoDetermineMode ^= CPU_AUTODETERMINE_CORE;
            }
            else if(cpudecoder == &CPU_Core_Normal_Run) 
            {
                cpudecoder = &CPU_Core_Simple_Run;
            }
            else if(cpudecoder == &CPU_Core_Simple_Run) 
            {
                cpudecoder = &CPU_Core_Full_Run;
            }
#if (C_DYNREC)
            else if(cpudecoder == &CPU_Core_Full_Run && dynamic_available) 
            {
                cpudecoder = &CPU_Core_Dynrec_Run;
            }
            else if(cpudecoder == &CPU_Core_Dynrec_Run || (cpudecoder == &CPU_Core_Full_Run && !dynamic_available)) 
            {
                cpudecoder = &CPU_Core_Normal_Run;
            }
#else
            else if(cpudecoder == &CPU_Core_Full_Run) 
            {
                cpudecoder = &CPU_Core_Normal_Run;
                CPU_AutoDetermineMode |= CPU_AUTODETERMINE_CORE;
            }
#endif
            else 
            {
                cpudecoder = &CPU_Core_Normal_Run;
            }
            
            break;
            
        case 4: // CPU Type
            
            if(CPU_ArchitectureType == CPU_ARCHTYPE_MIXED)
            {
                CPU_ArchitectureType = CPU_ARCHTYPE_386FAST;
                CPU_PrefetchQueueSize = 16;
                cpudecoder = &CPU_Core_Prefetch_Run;
            }
            else if(CPU_ArchitectureType == CPU_ARCHTYPE_386FAST && cpudecoder == &CPU_Core_Prefetch_Run) 
            {
                cpudecoder = &CPU_Core_Normal_Run;
            }
            else if(CPU_ArchitectureType == CPU_ARCHTYPE_386FAST) 
            {
                CPU_ArchitectureType = CPU_ARCHTYPE_386SLOW;
            }
            else if(CPU_ArchitectureType == CPU_ARCHTYPE_386SLOW)
            {
                CPU_ArchitectureType = CPU_ARCHTYPE_486NEWSLOW;
                CPU_PrefetchQueueSize = 32;
                cpudecoder = &CPU_Core_Prefetch_Run;
            }
            else if(CPU_ArchitectureType == CPU_ARCHTYPE_486NEWSLOW && cpudecoder == &CPU_Core_Prefetch_Run) 
            {
                cpudecoder = &CPU_Core_Normal_Run;
            }
            else if(CPU_ArchitectureType == CPU_ARCHTYPE_486NEWSLOW) 
            {
                CPU_ArchitectureType = CPU_ARCHTYPE_PENTIUMSLOW;
            }
            else if(CPU_ArchitectureType == CPU_ARCHTYPE_PENTIUMSLOW) 
            {
                CPU_ArchitectureType = CPU_ARCHTYPE_MIXED;
            }
            
            if(CPU_ArchitectureType >= CPU_ARCHTYPE_486NEWSLOW) CPU_extflags_toggle = (FLAG_ID | FLAG_AC);
            else if(CPU_ArchitectureType >= CPU_ARCHTYPE_486OLDSLOW) CPU_extflags_toggle = (FLAG_AC);
            else CPU_extflags_toggle = 0;
            
            break;
            
        case 5: // Double Buffering
            menu.doublebuf = !menu.doublebuf;
            break;
            
        case 6: // Exit
            throw(0);
            break;
    }
    
    MENU_UpdateMenu();
}

void MENU_Increase()
{
    switch(menu.selected)
    {
        case 1: // Frameskip
            IncreaseFrameSkip(true);
            break;
            
        case 2: // CPU cycles
            CPU_CycleIncrease(true);
            break;
    }
    
    MENU_UpdateMenu();
}

void MENU_Decrease()
{
    switch(menu.selected)
    {
        case 1: // Frameskip
            DecreaseFrameSkip(true);
            break;
            
        case 2: // CPU cycles
            CPU_CycleDecrease(true);
            break;
    }
    
    MENU_UpdateMenu();
}

void GFX_ForceUpdate(); // in sdlmain.cpp

int MENU_CheckEvent(SDL_Event *event)
{
    bool keystate = (event->type == SDL_KEYDOWN) ? true : false;
    int sym = event->key.keysym.sym;
    
    if(!menu_active) return 0;
    
    if(keystate && !keystates[sym])
    {
        if(sym == SDLK_UP) MENU_MoveCursor(-1);
        if(sym == SDLK_DOWN) MENU_MoveCursor(1);
        if(sym == SDLK_LEFT) MENU_Decrease();
        if(sym == SDLK_RIGHT) MENU_Increase();
        if(sym == SDLK_LCTRL) MENU_Activate(); // A - normal keypress
        if(sym == SDLK_LALT) MENU_Toggle();    // B - exit
    }
    
    if(sym >= 0 && sym < 1024) keystates[sym] = keystate;
    
    return 1;
}

void MENU_BlitDoubledSurface(SDL_Surface *source, int left, int top, SDL_Surface *destination)
{
    int x, y;
    int w = source->pitch;
    int h = source->h;
    
    int bytes = source->format->BytesPerPixel;
    int offset = left * bytes;
    int trailing = destination->pitch - source->pitch - offset;
    
    uint8_t* s8 = (uint8_t*)source->pixels;
    uint64_t* s64 = (uint64_t*)source->pixels;
    
    uint8_t* d8 = (uint8_t*)destination->pixels;
    uint64_t* d64 = (uint64_t*)destination->pixels;

    // Move down the buffer to where we want to start rendering
    d8 += (destination->pitch * top);

    for(y=0; y<h; y++)
    {
        d8 += offset;
        
        for(x=0; x<w; )
        {
            if(w-x >= 64)
            {
                d64 = (uint64_t*)((void*)d8);
                s64 = (uint64_t*)((void*)s8);
                
                *d64++ = *s64++;
                *d64++ = *s64++;
                *d64++ = *s64++;
                *d64++ = *s64++;
                *d64++ = *s64++;
                *d64++ = *s64++;
                *d64++ = *s64++;
                *d64++ = *s64++;
                
                x += 64;
                d8 = (uint8_t*)((void*)d64);
                s8 = (uint8_t*)((void*)s64);
            }
            else
            {
                *d8++ = *s8++;
                x++;
            }
        }
        
        d8 += trailing;
        d8 += offset;
        s8 -= w;
        
        for(x=0; x<w; )
        {
            if(w-x >= 64)
            {
                d64 = (uint64_t*)((void*)d8);
                s64 = (uint64_t*)((void*)s8);
                
                *d64++ = *s64++;
                *d64++ = *s64++;
                *d64++ = *s64++;
                *d64++ = *s64++;
                *d64++ = *s64++;
                *d64++ = *s64++;
                *d64++ = *s64++;
                *d64++ = *s64++;
                
                x += 64;
                d8 = (uint8_t*)((void*)d64);
                s8 = (uint8_t*)((void*)s64);
            }
            else
            {
                *d8++ = *s8++;
                x++;
            }
        }

        d8 += trailing;
    }
}

void MENU_Draw(SDL_Surface *surface)
{
    int y = (surface->h - (MENU_ITEMS * 25)) / 2 + 12;
    int color = 0xFF;
    SDL_Rect dest;
    
    if(!menu_active) return;
 
    SDL_FillRect(menu.surface, NULL, SDL_MapRGB(menu.surface->format, 0x00, 0x00, 0xAA));

    for(int i=0; i<MENU_ITEMS; i++)
    {
        color = 0xFF;
        
        if(menu.selected == i)
        {
            dest.x = 20;
            dest.y = y-10;
            dest.w = 280;
            dest.h = 25;
            
            color = 0x00;

            SDL_FillRect(menu.surface, &dest, SDL_MapRGB(menu.surface->format, 0xFF, 0xFF, 0xFF));
        }
        
        stringRGBA(menu.surface, 40, y, menuoptions[i], color, color, color, 0xFF);
       
        if(i == 1) stringRGBA(menu.surface, 165, y, menu.frameskip, color, color, color, 0xFF);
        if(i == 2) stringRGBA(menu.surface, 165, y, menu.cycles, color, color, color, 0xFF);
        if(i == 3) stringRGBA(menu.surface, 165, y, menu.core, color, color, color, 0xFF);
        if(i == 4) stringRGBA(menu.surface, 165, y, menu.cpuType, color, color, color, 0xFF);
        if(i == 5) stringRGBA(menu.surface, 165, y, menu.doublebuf ? "On" : "Off", color, color, color, 0xFF);
        
        y += 24;
    }

    if(surface->h <= 240) SDL_BlitSurface(menu.surface, NULL, surface, NULL);
    else MENU_BlitDoubledSurface(menu.surface, 0, 0, surface);
    
    dest.x = 0;
    dest.y = 200;
    dest.w = 320;
    dest.h = 40;
    
    // I'm not fully sure why, but this fixes a flicker from left-over blue at the 
    // bottom of the screen. This *should* actually draw red down there. But it
    // doesn't. Best guess, this wipes out alpha information. I'll return and 
    // figure this out later.
    SDL_FillRect(menu.surface, &dest, SDL_MapRGBA(surface->format, 0x00, 0x00, 0xAA, 0xFF));
}

void MENU_CleanScreen(SDL_Surface *surface)
{
    SDL_FillRect(surface, NULL, 0);
    menu_last = false;
}

