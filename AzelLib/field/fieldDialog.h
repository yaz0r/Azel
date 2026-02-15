#pragma once

typedef void(*tDialogCallback)(s32);

void registerCallbackForDialog(tDialogCallback callback1, tDialogCallback callback2, s32 r6);
