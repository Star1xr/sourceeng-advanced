#pragma once
#ifndef SOURCEENG_MANGOHUD_H
#define SOURCEENG_MANGOHUD_H

#ifdef __cplusplus
extern "C" {
#endif

typedef enum MangoHudApi
{
	MANGOHUD_API_GL = 0,
	MANGOHUD_API_GLES = 1,
	MANGOHUD_API_DX9 = 2,
} MangoHudApi;

typedef struct MangoHudDesc
{
	MangoHudApi api;

	/* IDirect3DDevice9* for DX9, unused for GL/GLES. */
	void *device;

	/* GL/GLES proc address loader, e.g. SDL_GL_GetProcAddress. */
	void *(*get_proc_address)(const char *name);

	/* Directory that contains engine.log. Used for MangoHud.conf and logs. */
	const char *config_dir;

	/* Used only when no MangoHud.conf is found. May be NULL. */
	const char *default_config;
} MangoHudDesc;

/* Returns non-zero on success. Safe to call more than once. */
int MangoHud_Init(const MangoHudDesc *desc);

/* Releases the ImGui context and stops MangoHud threads. */
void MangoHud_Shutdown(void);

/* Draws one frame. Returns non-zero if the HUD was rendered. */
int MangoHud_Render(int width, int height);

/* Returns non-zero while the HUD is initialized. */
int MangoHud_IsActive(void);

#ifdef __cplusplus
}
#endif

#endif /* SOURCEENG_MANGOHUD_H */
