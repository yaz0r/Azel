#pragma once

#include <string>

std::string decompileScript(sSaturnPtr startAddress, int maxOpcodes = 200);
void drawScriptDecompilerWindow();
