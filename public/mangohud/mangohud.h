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

/* Raw values gathered on the Android side (Java BatteryManager/PowerManager).
 * All math (percent, volts, GPU temperature estimate) is done inside MangoHud. */
typedef struct MangoHudAndroidRaw
{
	/* Extras of the sticky ACTION_BATTERY_CHANGED intent (-1 if missing). */
	int level;
	int scale;
	int voltage;      /* mV on most devices, some report V */
	int temperature;  /* tenths of a degree C */
	int status;       /* BatteryManager.BATTERY_STATUS_*, 2 = charging, 5 = full */

	int api_level;    /* Build.VERSION.SDK_INT */

	/* PowerManager.getThermalHeadroom(10); NaN when not queried, unsupported
	 * (API < 30) or the device returned NaN. */
	float thermal_headroom;
} MangoHudAndroidRaw;

/* Fills *out, returns non-zero on success. Only queries the (more expensive)
 * thermal headroom when query_headroom is non-zero. */
typedef int (*MangoHudAndroidStatsFn)(MangoHudAndroidRaw *out, int query_headroom);

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

	/* Android only, may be NULL. Provides battery percent/voltage and the
	 * inputs of the GPU temperature estimate. */
	MangoHudAndroidStatsFn get_android_stats;
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
