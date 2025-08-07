#pragma once

/*
 * ===== MASM ASSEMBLER PLUGIN HEADER =====
 * C++ Implementation for MASM Integration
 * Compatible with BenignPacker Framework
 */

#include "../../PluginFramework/IPlugin.h"
#include <windows.h>
#include <string>
#include <vector>
#include <map>

// Plugin export functions
extern "C" {
    __declspec(dllexport) BenignPacker::PluginFramework::IPlugin* CreatePlugin();
    __declspec(dllexport) void DestroyPlugin(BenignPacker::PluginFramework::IPlugin* plugin);
    __declspec(dllexport) uint32_t GetApiVersion();
}