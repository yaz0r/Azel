#pragma once

// Enables the town-module validation interceptors (collision registry + cell grid). Called once from
// enableValidationHooks() in validationHooks.cpp. Defined in validationHooks_town.cpp.
void enableTownHooks();
