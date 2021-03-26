#include <SDL.h>
#ifndef _SDL_UTIL_H
#define _SDL_UTIL_H
// texture and surface convert functions
SDL_Surface* CreateSurfaceFromRenderer(SDL_Renderer* pRenderer,
	SDL_Rect* pRect, int depth, Uint32 format);
SDL_Surface* CreateSurfaceFromTexture(SDL_Renderer* pRenderer,
	SDL_Texture* pTexture, SDL_Rect* pRect,
	int depth, Uint32 foramt);
int SaveSurface(SDL_Surface* pSurface, const char* path);
int SaveRenderer(SDL_Renderer* pRenderer,
	SDL_Rect* pRect, const char* path);
int SaveTexture(SDL_Renderer* pRenderer, SDL_Texture* pTexture,
	SDL_Rect* pRect, const char* path);
int SaveTextureStream(SDL_Texture* pTexture,
	SDL_Rect* pRect, const char* path);
#endif