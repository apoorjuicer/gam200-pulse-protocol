/******************************************************************************/
/**
 * @file        ImportExport.h
 * @project     Pulse Protocol
 * @author      Chia Wei Xuan Rachael - 100%
 * @brief       Defines DLL export/import macros for MyEngine
 *
 * @copyright   Copyright (C) 2025 DigiPen Institute of Technology.
 */
/******************************************************************************/
#pragma once

#ifndef DLL_API
#   if defined DLL_API_EXPORT
#       define DLL_API __declspec(dllexport)
#   else
#       define DLL_API __declspec(dllimport)
#   endif
#endif