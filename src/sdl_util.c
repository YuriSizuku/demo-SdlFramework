#include<string.h>
#include "sdl_util.h"

// texture and surface convert functions
SDL_Surface* CreateSurfaceFromRenderer(SDL_Renderer* pRenderer, 
	SDL_Rect* pRect, int depth, Uint32 format)
{
	int w, h, ret = 0;
	if (!pRect)
	{
		SDL_GetRendererOutputSize(pRenderer, &w, &h);
	}
	else
	{
		w = pRect->w;
		h = pRect->h;
	}

	SDL_Surface* pSurface = SDL_CreateRGBSurfaceWithFormat(0, w, h, depth, format);
	ret = SDL_RenderReadPixels(pRenderer, pRect,
		pSurface->format->format, pSurface->pixels, pSurface->pitch);
	if (ret == 0)
	{
		return pSurface;
	}
	else
	{
		SDL_FreeSurface(pSurface);
		return NULL;
	}
}

SDL_Surface* CreateSurfaceFromTexture(SDL_Renderer* pRenderer, 
	SDL_Texture* pTexture, SDL_Rect* pRect, 
	int depth, Uint32 foramt)
{
	Uint32 texture_format;
	int w, h, access, ret = -1;
	SDL_QueryTexture(pTexture, &texture_format, &access, &w, &h);
	SDL_Texture* pTmpTexture = NULL;
	SDL_Surface* pSurface = NULL;
	pTmpTexture = SDL_CreateTexture(pRenderer, texture_format, SDL_TEXTUREACCESS_TARGET, w, h);
	if (pTmpTexture)
	{
		SDL_Texture* pOldTarget = SDL_GetRenderTarget(pRenderer);
		SDL_SetRenderTarget(pRenderer, pTmpTexture);
		SDL_RenderCopy(pRenderer, pTexture, NULL, NULL);
		pSurface = CreateSurfaceFromRenderer(pRenderer, pRect, depth, foramt);
		SDL_SetRenderTarget(pRenderer, pOldTarget);
		SDL_DestroyTexture(pTmpTexture);
	}
	return pSurface;
}

int SaveSurface(SDL_Surface* pSurface, const char* path)
{
	int ret = -1;
	if (pSurface)
	{
		char* ext = strrchr((char*)path, (char)'.');
#ifdef SDL_IMAGE_H_
		if (!_stricmp(ext, ".png")) ret = IMG_SavePNG(pSurface, path);
		else if (!_stricmp(ext, ".jpg"))  ret = IMG_SaveJPG(pSurface, path, 90);
		else ret = SDL_SaveBMP(pSurface, path);
#else
		ret = SDL_SaveBMP(pSurface, path);
#endif
	}
	return ret;
}

int SaveRenderer(SDL_Renderer* pRenderer, 
	SDL_Rect* pRect, const char* path)
{
	int ret = -1;
	SDL_Surface* pSurface = CreateSurfaceFromRenderer(pRenderer, pRect, 32, SDL_PIXELFORMAT_RGBA8888);
	ret = SaveSurface(pSurface, path);
	SDL_FreeSurface(pSurface);
	return ret;
}

int SaveTexture(SDL_Renderer* pRenderer, SDL_Texture* pTexture, 
	SDL_Rect* pRect, const char* path)
{
	int ret = -1;
	SDL_Surface* pSurface = CreateSurfaceFromTexture(pRenderer, pTexture, pRect, 32, SDL_PIXELFORMAT_RGBA8888);
	ret = SaveSurface(pSurface, path);
	SDL_FreeSurface(pSurface);
	return ret;
}

int SaveTextureStream(SDL_Texture* pTexture, 
	SDL_Rect* pRect, const char* path)
{
	Uint32 format;
	int w, h, access, pitch = 0;
	void* pixel = NULL;
	int ret = 0;
	SDL_QueryTexture(pTexture, &format, &access, &w, &h);
	if (pRect)
	{
		w = pRect->w;
		h = pRect->h;
	}
	SDL_LockTexture(pTexture, pRect, &pixel, &pitch);
	SDL_Surface* pSurface =
		SDL_CreateRGBSurfaceWithFormatFrom(pixel, w, h, 32, pitch, format);
	if (pSurface)
	{
		char* ext = strrchr((char*)path, (char)'.');
#ifdef SDL_IMAGE_H_
		if (!_stricmp(ext, ".png")) ret = IMG_SavePNG(pSurface, path);
		else if (!_stricmp(ext, ".jpg"))  ret = IMG_SaveJPG(pSurface, path, 90);
		else ret = SDL_SaveBMP(pSurface, path);
#else
		ret = SDL_SaveBMP(pSurface, path);
#endif
	}
	else
	{
		ret = -1;
	}
	SDL_UnlockTexture(pTexture);
	SDL_FreeSurface(pSurface);
	return ret;
}