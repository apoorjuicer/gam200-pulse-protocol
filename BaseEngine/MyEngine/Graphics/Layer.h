/******************************************************************************/
/**
 * @file        Layer.h
 * @project     Pulse Protocol
 * @author      Goh Pin Kai
 * @brief       Defines the rendering layer system used to control draw order.
 *              Objects can be assigned one or more layers (e.g., background,
 *              world, UI) through a bitmask. Rendering passes filter by these
 *              layers to determine which objects to draw in each stage.
 *              The LayerTag component stores the object's assigned layer(s)
 *              as well as ordering information used to ensure predictable
 *              and stable sorting within a layer (e.g., foreground before
 *              background, UI always drawn last, etc.).
 * 
 * @copyright   Copyright (C) 2025 DigiPen Institute of Technology.
 *              Reproduction or disclosure of this file or its contents without the
 *              prior written consent of DigiPen Institute of Technology is prohibited.
 */
 /******************************************************************************/
#pragma once
#include"pch/pch.h"
using LayerMask = uint32_t;

enum : LayerMask {
	LAYER_BACKGROUND = 1u << 0,	// Elements drawn first (e.g., static backgrounds)
	LAYER_WORLD = 1u << 1,	 // Game world objects (e.g., characters, props)
	LAYER_UI = 1u << 2,	// User interface elements rendered on top
	LAYER_ALL = 0xFFFFFFFFu	// All Layers
};

struct LayerTag {
	LayerMask mask;
	int   orderInLayer;	//Higher = draws later = appears in front
	float z = 0.f;	// depth offset for fine-grained ordering (screen-space depth) <this is for 3D>

	void Serialize(rapidjson::Value& out, rapidjson::Document::AllocatorType& alloc) const {
		using namespace rapidjson;
		out.SetObject();

		// name of layer, easier to see
		const char* layerName = "Unknown";
		if (mask == LAYER_BACKGROUND) layerName = "Background";
		else if (mask == LAYER_WORLD) layerName = "World";
		else if (mask == LAYER_UI) layerName = "UI";
		else if (mask == LAYER_ALL) layerName = "All";

		Value nameVal;
		nameVal.SetString(layerName, alloc);
		out.AddMember("layerName", nameVal, alloc);

		out.AddMember("mask", mask, alloc);
		out.AddMember("orderInLayer", orderInLayer, alloc);
		out.AddMember("z", z, alloc);
	}

	void Deserialize(const rapidjson::Value& in) {
		if (!in.IsObject()) return;

		// load from mask
		if (in.HasMember("mask")) {
			mask = in["mask"].GetUint();
		}

		// or name of layer
		else if (in.HasMember("layerName") && in["layerName"].IsString()) {
			std::string name = in["layerName"].GetString();
			if (name == "Background") mask = LAYER_BACKGROUND;
			else if (name == "World") mask = LAYER_WORLD;
			else if (name == "UI") mask = LAYER_UI;
			else if (name == "All") mask = LAYER_ALL;
		}

		if (in.HasMember("orderInLayer")) orderInLayer = in["orderInLayer"].GetInt();
		if (in.HasMember("z")) z = in["z"].GetFloat();
	}
};