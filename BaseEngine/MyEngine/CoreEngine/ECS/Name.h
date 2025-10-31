/******************************************************************************/
/**
 * @file        Name.h
 * @project     Pulse Protocol
 * @author      Chloe Lau Rey En (primary) - 100%
 * @brief       Simple Name component for ECS entities to store entity names (for now only using for gui buttons, if wan use for others also can)
 *              for identification and debugging purposes in the hierarchy.
 * @copyright   Copyright (C) 2025 DigiPen Institute of Technology.
 *              Reproduction or disclosure of this file or its contents without the
 *              prior written consent of DigiPen Institute of Technology is prohibited.
 */
 /******************************************************************************/

#pragma once
#include <string>

struct Name {
    std::string name;
};