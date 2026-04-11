#pragma once

#include "task.h"
#include "common.h"

// 0605ea52 — spawn the FLD_A7 dialog-for-choice sibling task. Reads the dialog
// script pointer from the table at FLD_A7::060861b0 indexed by (choice + 0x21),
// parents the new task, and flips the parent task's pause bit so it yields
// until the dialog completes.
void a7DialogChoice_spawn_0605ea52(p_workArea parent, s32 choiceIdx);
