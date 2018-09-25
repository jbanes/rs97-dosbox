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


bool menu_active = false;
bool menu_last = false;
bool keystates[1024];

struct MENU_Block 
{
    SDL_Surface *surface;
    int selected;
    char *frameskip;
    char *cycles;
};

static MENU_Block menu;

const char *menuoptions[4] = {
    "Resume",
    "Frameskip: ",
    "Cycles: ",
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
}

void MENU_Deinit()
{
    
}

void MENU_Toggle()
{
    menu_active ^= 1;
    
    if(!menu_active) menu_last = true;
    
    for(int i=0; i<1024; i++) keystates[i] = false;
    
    sprintf(menu.frameskip, "%i", render.frameskip.max);
    
    if(CPU_CycleAutoAdjust) strcpy(menu.cycles, "auto");
    else sprintf(menu.cycles, "%i", CPU_CycleMax);
}

void MENU_MoveCursor(int direction)
{
    menu.selected += direction;
    
    if(menu.selected < 0) menu.selected = 3;
    if(menu.selected > 3) menu.selected = 0;
}

void MENU_Activate()
{
    switch(menu.selected)
    {
        case 0: // Resume
            MENU_Toggle();
            break;
            
        case 2: // Resume
            CPU_CycleAutoAdjust ^= 1;
            
            if(CPU_CycleAutoAdjust) strcpy(menu.cycles, "auto");
            else sprintf(menu.cycles, "%i", CPU_CycleMax);
            
            break;
            
        case 3: // Exit
            throw(0);
            break;
    }
}

void MENU_Increase()
{
    switch(menu.selected)
    {
        case 1: // Frameskip
            if(render.frameskip.max < 10) render.frameskip.max++;
            
            sprintf(menu.frameskip, "%i", render.frameskip.max);
            break;
            
        case 2: // CPU cycles
            CPU_CycleIncrease(true);
            
            if(CPU_CycleAutoAdjust) strcpy(menu.cycles, "auto");
            else sprintf(menu.cycles, "%i", CPU_CycleMax);
            
            break;
    }
}

void MENU_Decrease()
{
    switch(menu.selected)
    {
        case 1: // Frameskip
            if(render.frameskip.max > 0) render.frameskip.max--;
            
            sprintf(menu.frameskip, "%i", render.frameskip.max);
            break;
            
        case 2: // CPU cycles
            CPU_CycleDecrease(true);
            
            if(CPU_CycleAutoAdjust) strcpy(menu.cycles, "auto");
            else sprintf(menu.cycles, "%i", CPU_CycleMax);
            
            break;
    }
}

void GFX_ForceUpdate(); // in sdlmain.cpp

int MENU_CheckEvent(SDL_Event *event)
{
    bool keystate = (event->type == SDL_KEYDOWN) ? true : false;
    int sym = event->key.keysym.sym;
    
    if(keystate && event->key.keysym.scancode == 4) 
    {
        MENU_Toggle();
    
        return 1;
    }
    
    if(!menu_active) return 0;
    
    if(keystate && !keystates[sym])
    {
        if(sym == SDLK_UP) MENU_MoveCursor(-1);
        if(sym == SDLK_DOWN) MENU_MoveCursor(1);
        if(sym == SDLK_LEFT) MENU_Decrease();
        if(sym == SDLK_RIGHT) MENU_Increase();
        if(sym == SDLK_LCTRL) MENU_Activate(); // A - normal keypress
    }
    
    if(keystate && event->key.keysym.scancode == 107)
    {
        printf("Received shutdown command: %i, %i\n", event->key.keysym.scancode, event->key.keysym.sym);
        throw(0);
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
    int y = 40;
    int color = 0xFF;
    SDL_Rect dest;
    
    if(!menu_active) return;
    
    SDL_FillRect(menu.surface, NULL, SDL_MapRGBA(surface->format, 0x00, 0x00, 0xFF, 0xFF));
    
    for(int i=0; i<4; i++)
    {
        color = 0xFF;
        
        if(menu.selected == i)
        {
            dest.x = 20;
            dest.y = y-10;
            dest.w = 280;
            dest.h = 30;
            
            color = 0x00;

            SDL_FillRect(menu.surface, &dest, SDL_MapRGBA(menu.surface->format, 0xFF, 0xFF, 0xFF, 0xFF));
        }
        
        stringRGBA(menu.surface, 40, y, menuoptions[i], color, color, color, 0xFF);
        
        if(i == 1) stringRGBA(menu.surface, 125, y, menu.frameskip, color, color, color, 0xFF);
        if(i == 2) stringRGBA(menu.surface, 125, y, menu.cycles, color, color, color, 0xFF);
        
        y += 40;
    }
    
    MENU_BlitDoubledSurface(menu.surface, 0, 0, surface);
}

void MENU_CleanScreen(SDL_Surface *surface)
{
    SDL_FillRect(surface, NULL, 0);
    menu_last = false;
}

