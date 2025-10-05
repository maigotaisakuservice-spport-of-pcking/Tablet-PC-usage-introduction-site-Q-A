#include "crafting.h"

void InitializeInventory() {
    for (int i = 0; i < INVENTORY_SLOTS; ++i) {
        inventory[i].type = ITEM_NONE;
        inventory[i].count = 0;
    }
}

// A simple inventory add function. Stacks items if a slot with the same type exists.
void AddItemToInventory(ItemType item, int count) {
    // First, try to stack with existing items
    for (int i = 0; i < INVENTORY_SLOTS; ++i) {
        if (inventory[i].type == item) {
            inventory[i].count += count;
            return;
        }
    }
    // If no existing stack, find an empty slot
    for (int i = 0; i < INVENTORY_SLOTS; ++i) {
        if (inventory[i].type == ITEM_NONE) {
            inventory[i].type = item;
            inventory[i].count = count;
            return;
        }
    }
    // Inventory is full, item is lost (for now)
}

// Helper function to sort item types for shapeless recipe checking
void sort_item_list(ItemType* list, int size) {
    for (int i = 0; i < size - 1; i++) {
        for (int j = 0; j < size - i - 1; j++) {
            if (list[j] > list[j + 1]) {
                ItemType temp = list[j];
                list[j] = list[j + 1];
                list[j + 1] = temp;
            }
        }
    }
}

void CheckRecipe() {
    crafting_output.type = ITEM_NONE;
    crafting_output.count = 0;

    for (int i = 0; i < recipe_count; ++i) {
        // 1. Check central item (must be present and count must be 1)
        if (crafting_grid[0].type != recipes[i].ingredients[0] || crafting_grid[0].count != 1) {
            continue;
        }

        // 2. Check surrounding items (shapeless)
        ItemType grid_ingredients[CRAFTING_GRID_SIZE - 1];
        ItemType recipe_ingredients[CRAFTING_GRID_SIZE - 1];
        int grid_item_count = 0;

        for (int j = 1; j < CRAFTING_GRID_SIZE; ++j) {
            if (crafting_grid[j].type != ITEM_NONE) {
                // For items with count > 1, expand them into individual items
                for(int k=0; k < crafting_grid[j].count; ++k) {
                    if(grid_item_count < CRAFTING_GRID_SIZE - 1) {
                         grid_ingredients[grid_item_count++] = crafting_grid[j].type;
                    }
                }
            }
            recipe_ingredients[j-1] = recipes[i].ingredients[j];
        }
        // Fill the rest of the grid_ingredients with ITEM_NONE
        for(int j = grid_item_count; j < CRAFTING_GRID_SIZE -1; ++j) grid_ingredients[j] = ITEM_NONE;


        sort_item_list(grid_ingredients, CRAFTING_GRID_SIZE - 1);
        sort_item_list(recipe_ingredients, CRAFTING_GRID_SIZE - 1);

        bool match = true;
        for (int j = 0; j < CRAFTING_GRID_SIZE - 1; ++j) {
            if (grid_ingredients[j] != recipe_ingredients[j]) {
                match = false;
                break;
            }
        }

        if (match) {
            crafting_output.type = recipes[i].output;
            crafting_output.count = recipes[i].output_count;
            return; // Found a matching recipe
        }
    }
}

void DoCraft() {
    if (crafting_output.type != ITEM_NONE) {
        // Transfer output to held item
        held_item.type = crafting_output.type;
        held_item.count = crafting_output.count;

        // Clear grid and output
        for (int i = 0; i < CRAFTING_GRID_SIZE; ++i) {
            crafting_grid[i].type = ITEM_NONE;
            crafting_grid[i].count = 0;
        }
        crafting_output.type = ITEM_NONE;
        crafting_output.count = 0;
    }
}