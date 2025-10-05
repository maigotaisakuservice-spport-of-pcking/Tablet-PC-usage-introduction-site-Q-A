#ifndef MARCHING_CUBES_H
#define MARCHING_CUBES_H

#include <windows.h>
#include <GL/gl.h>

// Define a structure for a 3D point/vector
typedef struct {
    GLfloat x, y, z;
} POINT3D;

// Define a structure for a triangle, consisting of 3 points
typedef struct {
    POINT3D p[3];
} TRIANGLE;

// Define a structure for a grid cell
typedef struct {
    POINT3D p[8];  // Position of the 8 vertices
    GLfloat val[8]; // Value at the 8 vertices
} GRIDCELL;

// Function prototype for the main Marching Cubes algorithm
int Polygonise(GRIDCELL g, GLfloat isolevel, TRIANGLE *triangles);

// Vertex interpolation function
POINT3D VertexInterp(GLfloat isolevel, POINT3D p1, POINT3D p2, GLfloat valp1, GLfloat valp2);

// The 256-entry triangulation table is defined in marching_cubes.cpp
extern const int triTable[256][16];

#endif // MARCHING_CUBES_H