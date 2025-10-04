#include "character_renderer.h"
#include "renderer.h" // For DrawCube
#include <GL/gl.h>
#include <math.h>

// Helper function to draw a single part of a character
void DrawVoxelPart(const VoxelPart& part) {
    glPushMatrix();
    glTranslatef(part.offset_x, part.offset_y, part.offset_z);
    glRotatef(part.rot_x, 1.0f, 0.0f, 0.0f);
    glRotatef(part.rot_y, 0.0f, 1.0f, 0.0f);
    glRotatef(part.rot_z, 0.0f, 0.0f, 1.0f);
    glScalef(part.scale_x, part.scale_y, part.scale_z);
    DrawCube(0, 0, 0, part.color, false); // Assuming parts are not in shadow by default
    glPopMatrix();
}

void DrawZombie(const Zombie& zombie) {
    if (!zombie.active) return;

    glPushMatrix();
    glTranslatef(zombie.x, zombie.y, zombie.z);

    // Define Zombie colors
    const float skinColor[] = {0.2f, 0.6f, 0.3f}; // Greenish
    const float shirtColor[] = {0.2f, 0.3f, 0.8f}; // Blue
    const float pantsColor[] = {0.1f, 0.1f, 0.4f}; // Dark Blue

    // --- Define Zombie Parts ---
    VoxelPart head = {0.0f, 1.0f, 0.0f, 0.5f, 0.5f, 0.5f, 0, 0, 0, skinColor};
    VoxelPart torso = {0.0f, 0.25f, 0.0f, 0.8f, 1.0f, 0.4f, 0, 0, 0, shirtColor};

    // Animated Arms
    VoxelPart leftArm = {0.6f, 0.25f, 0.0f, 0.3f, 1.0f, 0.3f, -zombie.anim.swing_angle, 0, 0, skinColor};
    VoxelPart rightArm = {-0.6f, 0.25f, 0.0f, 0.3f, 1.0f, 0.3f, zombie.anim.swing_angle, 0, 0, skinColor};

    // Animated Legs
    VoxelPart leftLeg = {0.25f, -0.75f, 0.0f, 0.4f, 1.0f, 0.4f, zombie.anim.swing_angle, 0, 0, pantsColor};
    VoxelPart rightLeg = {-0.25f, -0.75f, 0.0f, 0.4f, 1.0f, 0.4f, -zombie.anim.swing_angle, 0, 0, pantsColor};

    // --- Draw Parts ---
    DrawVoxelPart(head);
    DrawVoxelPart(torso);
    DrawVoxelPart(leftArm);
    DrawVoxelPart(rightArm);
    DrawVoxelPart(leftLeg);
    DrawVoxelPart(rightLeg);

    glPopMatrix();
}

void DrawPig(const Pig& pig) {
    if (!pig.active) return;

    glPushMatrix();
    glTranslatef(pig.x, pig.y, pig.z);

    const float bodyColor[] = {1.0f, 0.7f, 0.8f}; // Pink
    const float snoutColor[] = {0.9f, 0.6f, 0.7f}; // Darker Pink

    // --- Define Pig Parts ---
    VoxelPart body = {0.0f, 0.0f, 0.0f, 1.5f, 0.8f, 0.8f, 0, 0, 0, bodyColor};
    VoxelPart head = {0.9f, 0.2f, 0.0f, 0.6f, 0.6f, 0.6f, 0, 0, 0, bodyColor};
    VoxelPart snout = {1.2f, 0.15f, 0.0f, 0.2f, 0.3f, 0.3f, 0, 0, 0, snoutColor};

    // Animated Legs
    VoxelPart leg1 = {0.6f, -0.6f, 0.3f, 0.3f, 0.5f, 0.3f, pig.anim.swing_angle, 0, 0, bodyColor};
    VoxelPart leg2 = {-0.6f, -0.6f, 0.3f, 0.3f, 0.5f, 0.3f, -pig.anim.swing_angle, 0, 0, bodyColor};
    VoxelPart leg3 = {0.6f, -0.6f, -0.3f, 0.3f, 0.5f, 0.3f, -pig.anim.swing_angle, 0, 0, bodyColor};
    VoxelPart leg4 = {-0.6f, -0.6f, -0.3f, 0.3f, 0.5f, 0.3f, pig.anim.swing_angle, 0, 0, bodyColor};

    // --- Draw Parts ---
    DrawVoxelPart(body);
    DrawVoxelPart(head);
    DrawVoxelPart(snout);
    DrawVoxelPart(leg1);
    DrawVoxelPart(leg2);
    DrawVoxelPart(leg3);
    DrawVoxelPart(leg4);

    glPopMatrix();
}

void DrawCow(const Cow& cow) {
    if (!cow.active) return;

    glPushMatrix();
    glTranslatef(cow.x, cow.y, cow.z);

    const float bodyColor[] = {1.0f, 1.0f, 1.0f}; // White
    const float spotColor[] = {0.1f, 0.1f, 0.1f}; // Black
    const float hornColor[] = {0.8f, 0.8f, 0.7f}; // Light Gray

    // --- Define Cow Parts ---
    VoxelPart body = {0.0f, 0.0f, 0.0f, 1.8f, 1.0f, 1.0f, 0, 0, 0, bodyColor};
    VoxelPart head = {1.1f, 0.3f, 0.0f, 0.7f, 0.7f, 0.7f, 0, 0, 0, bodyColor};
    VoxelPart spot1 = {-0.5f, 0.2f, 0.4f, 0.5f, 0.5f, 0.1f, 0, 0, 0, spotColor};
    VoxelPart spot2 = {0.4f, 0.1f, -0.4f, 0.6f, 0.4f, 0.1f, 0, 0, 0, spotColor};
    VoxelPart horn1 = {1.2f, 0.7f, 0.2f, 0.1f, 0.3f, 0.1f, 0, 0, -20, hornColor};
    VoxelPart horn2 = {1.2f, 0.7f, -0.2f, 0.1f, 0.3f, 0.1f, 0, 0, 20, hornColor};

    // Animated Legs
    VoxelPart leg1 = {0.7f, -0.7f, 0.4f, 0.4f, 0.6f, 0.4f, cow.anim.swing_angle, 0, 0, bodyColor};
    VoxelPart leg2 = {-0.7f, -0.7f, 0.4f, 0.4f, 0.6f, 0.4f, -cow.anim.swing_angle, 0, 0, bodyColor};
    VoxelPart leg3 = {0.7f, -0.7f, -0.4f, 0.4f, 0.6f, 0.4f, -cow.anim.swing_angle, 0, 0, bodyColor};
    VoxelPart leg4 = {-0.7f, -0.7f, -0.4f, 0.4f, 0.6f, 0.4f, cow.anim.swing_angle, 0, 0, bodyColor};

    // --- Draw Parts ---
    DrawVoxelPart(body);
    DrawVoxelPart(head);
    DrawVoxelPart(spot1);
    DrawVoxelPart(spot2);
    DrawVoxelPart(horn1);
    DrawVoxelPart(horn2);
    DrawVoxelPart(leg1);
    DrawVoxelPart(leg2);
    DrawVoxelPart(leg3);
    DrawVoxelPart(leg4);

    glPopMatrix();
}


void DrawDrone(const Drone& drone) {
    if (!drone.active) return;

    glPushMatrix();
    glTranslatef(drone.x, drone.y, drone.z);
    glRotatef(drone.anim.swing_angle, 0.0f, 1.0f, 0.0f); // Hover rotation

    const float bodyColor[] = {0.2f, 0.2f, 0.2f};
    const float wingColor[] = {0.4f, 0.4f, 0.4f};

    VoxelPart body = {0,0,0, 1,1,1, 0,0,0, bodyColor};
    VoxelPart wing1 = {0.8f, 0, 0, 0.8f, 0.1f, 0.1f, 0,0,0, wingColor};
    VoxelPart wing2 = {-0.8f, 0, 0, 0.8f, 0.1f, 0.1f, 0,0,0, wingColor};

    DrawVoxelPart(body);
    DrawVoxelPart(wing1);
    DrawVoxelPart(wing2);

    glPopMatrix();
}