
#ifndef SDL_VMOUSE_H
#define SDL_VMOUSE_H

void VMOUSE_Init(int bpp);
void VMOUSE_Deinit();
bool VMOUSE_IsEnabled(void);
void VMOUSE_SetEnabled(bool enabled);
bool VMOUSE_CheckEvent(SDL_Event *event);
void VMOUSE_BlitVMouse(SDL_Surface *surface);


#endif /* SDL_VMOUSE_H */

