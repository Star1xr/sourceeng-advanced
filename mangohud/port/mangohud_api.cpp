#include "mangohud/mangohud.h"

#include <cstdlib>
#include <cstring>
#include <string>

#include <glad/glad.h>

#include "gl/gl_hud.h"

/* Read by the patched MangoHud file_utils.cpp. Kept in the global namespace
 * so the vendored code can find it without a new public header. */
std::string g_mangohud_config_dir_override;
std::string g_mangohud_data_dir_override;

namespace
{
MangoHudApi g_api = MANGOHUD_API_GL;
void *(*g_get_proc_address)(const char *) = nullptr;
bool g_initialized = false;
bool g_context_created = false;

/* MangoHud's renderer creates GL objects, so the context has to exist before
 * imgui_set_context() runs. The engine's SDL manager calls Init() during
 * startup, potentially before the GL context is current, so creation is
 * deferred until the first Render() call inside ShowPixels(). */
bool EnsureContext()
{
	if (g_context_created)
		return true;

	/* MangoHud uses a non-null token as its "this GL context is ready"
	 * marker. The EGL WSI path skips the GLX-only queries. */
	MangoHud::GL::imgui_set_context((void *)1, MangoHud::GL::GL_WSI_EGL);
	g_context_created = true;
	return true;
}
}

/* Called by the patched MangoHud::GL::imgui_create(). */
extern "C" int MangoHud_LoadGL(void)
{
	if (g_api == MANGOHUD_API_GLES)
	{
		if (!g_get_proc_address)
			return 0;
		return gladLoadGLES2Loader((GLADloadproc)g_get_proc_address);
	}

	return gladLoadGL();
}

extern "C" int MangoHud_Init(const MangoHudDesc *desc)
{
	if (!desc)
		return 0;

	if (g_initialized)
		return 1;

	g_api = desc->api;
	g_get_proc_address = desc->get_proc_address;

	if (desc->config_dir && desc->config_dir[0])
	{
		g_mangohud_config_dir_override = desc->config_dir;
		g_mangohud_data_dir_override = desc->config_dir;
	}

	if (desc->default_config && desc->default_config[0] && !getenv("MANGOHUD_CONFIG"))
		setenv("MANGOHUD_CONFIG", desc->default_config, 1);

	g_initialized = true;
	return 1;
}

extern "C" int MangoHud_Render(int width, int height)
{
	if (!g_initialized || width <= 0 || height <= 0)
		return 0;

	EnsureContext();
	MangoHud::GL::imgui_render((unsigned int)width, (unsigned int)height);
	return 1;
}

extern "C" void MangoHud_Shutdown(void)
{
	if (!g_initialized)
		return;

	if (g_context_created)
		MangoHud::GL::imgui_shutdown();

	g_context_created = false;
	g_initialized = false;
}

extern "C" int MangoHud_IsActive(void)
{
	return g_initialized ? 1 : 0;
}
