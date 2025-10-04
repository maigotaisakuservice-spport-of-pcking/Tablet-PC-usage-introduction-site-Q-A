#ifndef CHARACTER_H
#define CHARACTER_H

#define MAX_CHARACTER_PARTS 10

struct VoxelPart {
    float offset_x, offset_y, offset_z;
    float scale_x, scale_y, scale_z;
    float rot_x, rot_y, rot_z;
    const float* color;
};

struct CharacterAnimation {
    float swing_angle;
    float swing_speed;
    float step_cycle;
};

#endif // CHARACTER_H