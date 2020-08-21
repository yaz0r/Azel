#pragma once

void battleResultScreen_createProgressiveTextNumber(p_workArea pThis, sSaturnPtr config, int value);
void battleResultScreen_updateSub8(int value, sVec2_S32* output);
void battleResultScreenProgressiveText_printNumber(const std::array<s32, 4>& data, int value);
