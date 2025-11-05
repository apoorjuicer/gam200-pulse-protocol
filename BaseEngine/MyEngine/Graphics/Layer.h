/******************************************************************************/
/**
 * @file        Layer.h
 * @project     Pulse Protocol
 * @author      Goh Pin Kai - 100%
 * @brief       
 * @copyright   Copyright (C) 2025 DigiPen Institute of Technology.
 *              Reproduction or disclosure of this file or its contents without the
 *              prior written consent of DigiPen Institute of Technology is prohibited.
 */
 /******************************************************************************/
#pragma once
#include"pch/pch.h"
using LayerMask = uint32_t;

enum : LayerMask {
	LAYER_BACKGROUND = 1u << 0,
	LAYER_WORLD = 1u << 1,
	LAYER_UI = 1u << 2,
	LAYER_DEBUG = 1u << 3,
	LAYER_ALL = 0xFFFFFFFFu
};

struct LayerTag {
	LayerMask mask = LAYER_WORLD; // membership
	int   orderInLayer = 0;       // stable sort key within the layer
	float z = 0.f;                // optional secondary sort
};