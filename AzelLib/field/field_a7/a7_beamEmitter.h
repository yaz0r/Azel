#pragma once

// Beam emitter — registered in the FLD_A7 cell-object dispatcher under the
// create_A7_CellObj2 name (Saturn dispatch slot 06055da6). Emits a periodic
// charge-wobble / spark / beam sequence via three sibling sub-tasks.
void create_A7_CellObj2(struct s_visdibilityCellTask* r4, struct s_DataTable2Sub0& r5, s32 r6);
