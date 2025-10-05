#ifndef CRAFTING_H
#define CRAFTING_H

// Function declarations related to the crafting system.
// All type and global variable definitions are now in globals.h
#include "globals.h"

void InitializeInventory();
void AddItemToInventory(ItemType item, int count);
void CheckRecipe();
void DoCraft();
void RenderCraftingScreen();

#endif // CRAFTING_H