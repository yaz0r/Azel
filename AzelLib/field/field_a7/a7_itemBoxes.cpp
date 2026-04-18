#include "PDS.h"
#include "o_fld_a7.h"
#include "field/fieldItemBox.h"
#include "field/fieldItemBoxDefinition.h"

// FLD_A7 overlay item boxes.
//
// These lists are the A7 overlay's local copies of the shared item box
// creation function (itemBoxType1Init). Each entry is an s_itemBoxDefinition
// in the A7 overlay's Saturn binary and is registered through the shared
// createFieldItemBox helper, which finds the owning grid cell
// task and creates an s_itemBoxType1 sub-task.

// 06059eb8 — create 10 item boxes for subfield 0
void createA7_itemBoxes_0(p_workArea parent)
{
    createFieldItemBox(readItemBoxDefinition({ 0x06085d20, gFLD_A7 }));
    createFieldItemBox(readItemBoxDefinition({ 0x06085d68, gFLD_A7 }));
    createFieldItemBox(readItemBoxDefinition({ 0x06085db0, gFLD_A7 }));
    createFieldItemBox(readItemBoxDefinition({ 0x06085df8, gFLD_A7 }));
    createFieldItemBox(readItemBoxDefinition({ 0x06085e40, gFLD_A7 }));
    createFieldItemBox(readItemBoxDefinition({ 0x06085e88, gFLD_A7 }));
    createFieldItemBox(readItemBoxDefinition({ 0x06085ed0, gFLD_A7 }));
    createFieldItemBox(readItemBoxDefinition({ 0x06085f18, gFLD_A7 }));
    createFieldItemBox(readItemBoxDefinition({ 0x06085f60, gFLD_A7 }));
    createFieldItemBox(readItemBoxDefinition({ 0x06085fa8, gFLD_A7 }));
}

// 06059f00 — create 4 item boxes for subfield 1
void createA7_itemBoxes_1(void)
{
    createFieldItemBox(readItemBoxDefinition({ 0x06085ff0, gFLD_A7 }));
    createFieldItemBox(readItemBoxDefinition({ 0x06086038, gFLD_A7 }));
    createFieldItemBox(readItemBoxDefinition({ 0x06086080, gFLD_A7 })); // map
    createFieldItemBox(readItemBoxDefinition({ 0x060860c8, gFLD_A7 })); // save
}
