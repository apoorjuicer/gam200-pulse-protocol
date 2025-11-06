/******************************************************************************/
/**
 * @file        Layer.h
 * @project     Pulse Protocol
 * @author      Goh Pin Kai - 100%
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
	//LAYER_UI = 1u << 2,	// User interface elements rendered on top
	LAYER_ALL = 0xFFFFFFFFu	// All Layers
};

struct LayerTag {
	LayerMask mask;
	int   orderInLayer;	//Higher = draws later = appears in front
	float z = 0.f;	// depth offset for fine-grained ordering (screen-space depth) <this is for 3D>
};