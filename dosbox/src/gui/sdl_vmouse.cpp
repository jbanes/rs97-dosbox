

#include <SDL/SDL.h>
#include <SDL/SDL_image.h>

#include "dosbox.h"
#include "mouse.h"
#include "sdl_vmouse_image.h"
#include "sdl_vmouse.h"

static struct {
    double x, y;
    double accelx, accely;
    
    bool left, right, up, down;
    bool enabled;
    
    SDL_Surface *cursor;
    SDL_Surface *icon;
    SDL_Surface *icon2x;
} vmouse;

extern bool vkeyb_active;

void GFX_ForceUpdate(); // in sdlmain.cpp

void VMOUSE_Init(int bpp)
{
    vmouse.x = 0.0;
    vmouse.y = 0.0;
    vmouse.accelx = 0.0;
    vmouse.accely = 0.0;
    
    vmouse.left = false;
    vmouse.right = false;
    vmouse.up = false;
    vmouse.down = false;
    vmouse.enabled = false;
    
    vmouse.cursor = IMG_LoadPNG_RW(SDL_RWFromMem(pointer_image, sizeof(pointer_image)));
    vmouse.icon = IMG_LoadPNG_RW(SDL_RWFromMem(mouse_image, sizeof(mouse_image)));
    vmouse.icon2x = 0;

    // Need to explicitly set width and height as they're not read from the file
    vmouse.cursor->w = 24;
    vmouse.cursor->h = 24;
    vmouse.icon->w = 16;
    vmouse.icon->h = 16;
}

void VMOUSE_Deinit()
{
    
}

bool VMOUSE_IsEnabled(void)
{
    return vmouse.enabled;
}

void VMOUSE_SetEnabled(bool enabled)
{
    vmouse.enabled = enabled;
    
    if(!enabled)
    {
        vmouse.accelx = 0.0;
        vmouse.accely = 0.0;

        vmouse.left = false;
        vmouse.right = false;
        vmouse.up = false;
        vmouse.down = false;
    }
    
    GFX_ForceUpdate();
}

bool VMOUSE_CheckEvent(SDL_Event *event)
{
    if(vkeyb_active) 
    {
        vmouse.enabled = false;
        
        return false;
    }
    
    // Don't block buttons we're not using
    if(event->key.keysym.sym == SDLK_RETURN) return false;
    if(event->key.keysym.sym == SDLK_ESCAPE) return false;
    
    if(event->key.keysym.sym == SDLK_BACKSPACE)
    {
        if(event->type == SDL_KEYDOWN) VMOUSE_SetEnabled(!VMOUSE_IsEnabled());
        
        return true;
    }
    
    if(!vmouse.enabled) return false;

    if(event->key.keysym.sym == SDLK_LEFT) vmouse.left = (event->type != SDL_KEYUP);
    if(event->key.keysym.sym == SDLK_RIGHT) vmouse.right = (event->type != SDL_KEYUP);
    if(event->key.keysym.sym == SDLK_UP) vmouse.up = (event->type != SDL_KEYUP);
    if(event->key.keysym.sym == SDLK_DOWN) vmouse.down = (event->type != SDL_KEYUP);
    
    if(!vmouse.left && !vmouse.right) vmouse.accelx = 0.0;
    if(!vmouse.up && !vmouse.down) vmouse.accely = 0.0;
    
    if(event->key.keysym.sym == SDLK_LCTRL) (event->type == SDL_KEYUP) ? Mouse_ButtonReleased(0) : Mouse_ButtonPressed(0); // A
    if(event->key.keysym.sym == SDLK_LALT) (event->type == SDL_KEYUP) ? Mouse_ButtonReleased(2) : Mouse_ButtonPressed(2); // B
    if(event->key.keysym.sym == SDLK_SPACE) (event->type == SDL_KEYUP) ? Mouse_ButtonReleased(2) : Mouse_ButtonPressed(2); // X
    if(event->key.keysym.sym == SDLK_LSHIFT) (event->type == SDL_KEYUP) ? Mouse_ButtonReleased(1) : Mouse_ButtonPressed(1); // Y
    
    GFX_ForceUpdate();
    
    return true;
}

void VMOUSE_BlitDoubledSurface(SDL_Surface *source, SDL_Surface *destination)
{
    int x, y;
    int w = source->pitch;
    int h = source->h;
    int left = 0;
    int top = 0;
    
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

SDL_Surface* load_icon2x()
{
    if(vmouse.icon2x) return vmouse.icon2x;

    vmouse.icon2x = SDL_CreateRGBSurface(SDL_SWSURFACE, vmouse.icon->w, vmouse.icon->h * 2, vmouse.icon->format->BitsPerPixel, 0, 0, 0, 0);
    
    VMOUSE_BlitDoubledSurface(vmouse.icon, vmouse.icon2x);
    
    return vmouse.icon2x;
}

void VMOUSE_BlitVMouse(SDL_Surface *surface)
{
    SDL_Rect position;
    
    if(!vmouse.enabled) return;

    vmouse.accelx += vmouse.left ? -1 : vmouse.right ? 1 : 0.0;
    vmouse.accely += vmouse.up ? -1 : vmouse.down ? 1 : 0.0;
    vmouse.x += vmouse.accelx;
    vmouse.y += vmouse.accely;
    
    Mouse_CursorMoved(vmouse.accelx, vmouse.accely, vmouse.accelx, vmouse.accely, true);
    
    if(!Mouse_IsHidden())
    {
        position.x = vmouse.x;
        position.y = vmouse.y;
        position.w = vmouse.cursor->w;
        position.h = vmouse.cursor->h;

        SDL_BlitSurface(vmouse.cursor, NULL, surface, &position);
    }
    else
    {
        position.x = surface->w - vmouse.icon->w;
        position.y = surface->h - ((surface->h > surface->w) ? vmouse.icon->h * 2 : vmouse.icon->h) ;
        position.w = vmouse.icon->w;
        position.h = (surface->h > surface->w) ? vmouse.icon->h * 2 : vmouse.icon->h;

        SDL_BlitSurface((surface->h > surface->w) ? load_icon2x() : vmouse.icon, NULL, surface, &position);
    }
}