#include "character_renderer.h"
#include "globals.h" // For draw_cube and colors
#include <windows.h>
#include <GL/gl.h>
#include <cmath> // For sin, fabs

// Generic function to render a single part of a character
void render_character_part(const VoxelPart& part, const CharacterAnimation& anim, float pos_x, float pos_y, float pos_z, float body_rot_y) {
    glPushMatrix();

    // 1. Move to the character's base position and apply body rotation
    glTranslatef(pos_x, pos_y, pos_z);
    glRotatef(body_rot_y, 0.0f, 1.0f, 0.0f);

    // 2. Apply the part's specific offset from the character's center
    glTranslatef(part.offset_x, part.offset_y, part.offset_z);

    // 3. Apply animation rotation
    // rot_x is used for swinging (arms/legs). Value is 1.0 or -1.0 to control direction.
    if (fabs(part.rot_x) > 0.5f) {
        glRotatef(sin(anim.step_cycle) * anim.swing_angle * part.rot_x, 1.0f, 0.0f, 0.0f);
    }
    // rot_y is used for spinning (propellers)
    if (fabs(part.rot_y) > 0.5f) {
        glRotatef(anim.step_cycle * 200.0f, 0.0f, 1.0f, 0.0f);
    }

    // 4. Apply the part's base rotation and scale
    glRotatef(part.rot_z, 0.0f, 0.0f, 1.0f); // Base rotation if any
    glScalef(part.scale_x, part.scale_y, part.scale_z);

    // 5. Draw the cube
    glColor3fv(part.color);
    draw_cube(1.0f);

    glPopMatrix();
}

// Specific definition and rendering for a Voxel Zombie
void render_voxel_zombie(float x, float y, float z, float rot_y, const CharacterAnimation& anim) {
    const VoxelPart zombie_parts[] = {
        // Torso
        {0.0f, 0.0f, 0.0f,   0.5f, 0.7f, 0.25f,  0.0f, 0.0f, 0.0f, colorPalette[COLOR_GREEN_DARK]},
        // Head
        {0.0f, 0.6f, 0.0f,   0.4f, 0.4f, 0.4f,   0.0f, 0.0f, 0.0f, colorPalette[COLOR_GREEN]},
        // Left Arm (swings)
        {-0.45f, 0.0f, 0.0f, 0.2f, 0.6f, 0.2f,  1.0f, 0.0f, 0.0f, colorPalette[COLOR_BLUE_DARK]},
        // Right Arm (swings, opposite phase)
        {0.45f, 0.0f, 0.0f,  0.2f, 0.6f, 0.2f,  -1.0f, 0.0f, 0.0f, colorPalette[COLOR_BLUE_DARK]},
        // Left Leg
        {-0.15f, -0.7f, 0.0f, 0.2f, 0.7f, 0.2f,  -1.0f, 0.0f, 0.0f, colorPalette[COLOR_BROWN]},
        // Right Leg
        {0.15f, -0.7f, 0.0f,  0.2f, 0.7f, 0.2f,  1.0f, 0.0f, 0.0f, colorPalette[COLOR_BROWN]}
    };

    int num_parts = sizeof(zombie_parts) / sizeof(VoxelPart);
    for (int i = 0; i < num_parts; ++i) {
        render_character_part(zombie_parts[i], anim, x, y, z, rot_y);
    }
}

// Specific definition and rendering for an Advanced Drone
void render_advanced_drone(float x, float y, float z, float rot_y, const CharacterAnimation& anim) {
     const VoxelPart drone_parts[] = {
        // Main Body
        {0.0f, 0.0f, 0.0f,   0.6f, 0.4f, 0.6f,  0.0f, 0.0f, 0.0f, colorPalette[COLOR_GRAY_DARK]},
        // "Eye"
        {0.0f, 0.05f, -0.31f, 0.1f, 0.1f, 0.1f, 0.0f, 0.0f, 0.0f, colorPalette[COLOR_RED]},
        // Top Propeller (spins)
        {0.0f, 0.3f, 0.0f,   0.8f, 0.05f, 0.1f, 0.0f, 1.0f, 0.0f, colorPalette[COLOR_GRAY_LIGHT]},
        // Bottom Propeller (spins)
        {0.0f, -0.3f, 0.0f,  0.1f, 0.05f, 0.8f, 0.0f, 1.0f, 0.0f, colorPalette[COLOR_GRAY_LIGHT]}
    };

    int num_parts = sizeof(drone_parts) / sizeof(VoxelPart);
    for (int i = 0; i < num_parts; ++i) {
        render_character_part(drone_parts[i], anim, x, y, z, rot_y);
    }
}

// Specific definition and rendering for a Pig
void render_pig(float x, float y, float z, float rot_y, const CharacterAnimation& anim) {
    const VoxelPart pig_parts[] = {
        // Body
        {0.0f, 0.0f, 0.0f,   0.8f, 0.5f, 0.4f,   0.0f, 0.0f, 0.0f, colorPalette[COLOR_MAGENTA]},
        // Head
        {-0.6f, 0.1f, 0.0f,  0.4f, 0.4f, 0.35f,  0.0f, 0.0f, 0.0f, colorPalette[COLOR_MAGENTA]},
        // Snout
        {-0.85f, 0.05f, 0.0f, 0.1f, 0.15f, 0.15f, 0.0f, 0.0f, 0.0f, colorPalette[COLOR_RED]},
        // Front-Left Leg
        {-0.3f, -0.4f, 0.15f, 0.15f, 0.3f, 0.15f, 1.0f, 0.0f, 0.0f, colorPalette[COLOR_MAGENTA]},
        // Front-Right Leg
        {-0.3f, -0.4f, -0.15f,0.15f, 0.3f, 0.15f, 1.0f, 0.0f, 0.0f, colorPalette[COLOR_MAGENTA]},
        // Back-Left Leg
        {0.3f, -0.4f, 0.15f, 0.15f, 0.3f, 0.15f, -1.0f, 0.0f, 0.0f, colorPalette[COLOR_MAGENTA]},
        // Back-Right Leg
        {0.3f, -0.4f, -0.15f, 0.15f, 0.3f, 0.15f, -1.0f, 0.0f, 0.0f, colorPalette[COLOR_MAGENTA]}
    };

    int num_parts = sizeof(pig_parts) / sizeof(VoxelPart);
    for (int i = 0; i < num_parts; ++i) {
        render_character_part(pig_parts[i], anim, x, y, z, rot_y);
    }
}

// Specific definition and rendering for a Cow
void render_cow(float x, float y, float z, float rot_y, const CharacterAnimation& anim) {
    const VoxelPart cow_parts[] = {
        // Body
        {0.0f, 0.0f, 0.0f,   1.0f, 0.6f, 0.5f,   0.0f, 0.0f, 0.0f, colorPalette[COLOR_WHITE]},
        // Head
        {-0.8f, 0.3f, 0.0f,  0.4f, 0.4f, 0.4f,   0.0f, 0.0f, 0.0f, colorPalette[COLOR_WHITE]},
        // Snout
        {-1.05f, 0.2f, 0.0f, 0.1f, 0.2f, 0.3f, 0.0f, 0.0f, 0.0f, colorPalette[COLOR_BROWN]},
        // Horns
        {-0.8f, 0.6f, 0.15f, 0.1f, 0.2f, 0.1f, 0.0f, 0.0f, 0.0f, colorPalette[COLOR_GRAY_LIGHT]},
        {-0.8f, 0.6f, -0.15f,0.1f, 0.2f, 0.1f, 0.0f, 0.0f, 0.0f, colorPalette[COLOR_GRAY_LIGHT]},
        // Patches on body
        {0.2f, 0.1f, 0.2f, 0.3f, 0.3f, 0.2f, 0.0f, 0.0f, 0.0f, colorPalette[COLOR_BLACK]},
        {-0.3f, 0.0f, -0.2f, 0.4f, 0.25f, 0.2f, 0.0f, 0.0f, 0.0f, colorPalette[COLOR_BLACK]},
        // Legs
        {-0.4f, -0.5f, 0.2f, 0.2f, 0.4f, 0.2f, 1.0f, 0.0f, 0.0f, colorPalette[COLOR_WHITE]},
        {-0.4f, -0.5f, -0.2f,0.2f, 0.4f, 0.2f, 1.0f, 0.0f, 0.0f, colorPalette[COLOR_WHITE]},
        {0.4f, -0.5f, 0.2f, 0.2f, 0.4f, 0.2f, -1.0f, 0.0f, 0.0f, colorPalette[COLOR_WHITE]},
        {0.4f, -0.5f, -0.2f, 0.2f, 0.4f, 0.2f, -1.0f, 0.0f, 0.0f, colorPalette[COLOR_WHITE]}
    };

    int num_parts = sizeof(cow_parts) / sizeof(VoxelPart);
    for (int i = 0; i < num_parts; ++i) {
        render_character_part(cow_parts[i], anim, x, y, z, rot_y);
    }
}