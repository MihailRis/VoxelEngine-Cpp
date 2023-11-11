#include "settings.h"

#include <memory>
#include <string>

#include "files/files.h"
#include "coders/json.h"

using std::string;
using std::unique_ptr;

void load_settings(EngineSettings& settings, string filename) {
	string source = files::read_string(filename);
	unique_ptr<json::JObject> obj(json::parse(filename, source));
	{
		auto& display = settings.display;
		obj->num("display-width", display.width);
		obj->num("display-height", display.height);
		obj->num("display-samples", display.samples);
		obj->num("display-swap-interval", display.swapInterval);
	}

	{
		auto& chunks = settings.chunks;
		obj->num("chunks-load-distance", chunks.loadDistance);
		obj->num("chunks-load-speed", chunks.loadSpeed);
		obj->num("chunks-padding", chunks.padding);
	}
	{
		auto& camera = settings.camera;
		obj->flag("camera-fov-effects", camera.fovEvents);
        obj->flag("camera-shaking", camera.shaking);
	}
	obj->num("fog-curve", settings.fogCurve);
}

void save_settings(EngineSettings& settings, string filename) {
	json::JObject obj;
	{
		auto& display = settings.display;
		obj.put("display-width", display.width);
		obj.put("display-height", display.height);
		obj.put("display-samples", display.samples);
		obj.put("display-swap-interval", display.swapInterval);
	}

	{
		auto& chunks = settings.chunks;
		obj.put("chunks-load-distance", chunks.loadDistance);
		obj.put("chunks-load-speed", chunks.loadSpeed);
		obj.put("chunks-padding", chunks.padding);
	}

	{
		auto& camera = settings.camera;
		obj.put("camera-fov-effects", camera.fovEvents);
        obj.put("camera-shaking", camera.shaking);
	}
	obj.put("fog-curve", settings.fogCurve);
	files::write_string(filename, json::stringify(&obj, true, "  "));
}