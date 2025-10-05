#ifndef CHARACTER_RENDERER_H
#define CHARACTER_RENDERER_H

#include "character.h"
#include "globals.h"

void render_character_part(const VoxelPart& part, const CharacterAnimation& anim, float pos_x, float pos_y, float pos_z, float body_rot_y);
void render_voxel_zombie(float x, float y, float z, float rot_y, const CharacterAnimation& anim);
void render_advanced_drone(float x, float y, float z, float rot_y, const CharacterAnimation& anim);
void render_pig(float x, float y, float z, float rot_y, const CharacterAnimation& anim);
void render_cow(float x, float y, float z, float rot_y, const CharacterAnimation& anim);


#endif // CHARACTER_RENDERER_H