#pragma once

#include <limits>
#include "Shape.h"

#ifdef max
#undef max
#endif

using namespace std;

struct Triangle_Half_Edge;
struct Triangle_Face;
struct Triangle_Vertex;
struct Triangle_Edge;


struct Triangle_Half_Edge {
	size_t next;
	size_t prev;
	size_t flip;

	size_t face;
	size_t origin;
	size_t edge;

	// default constructor
	Triangle_Half_Edge();

	// print the half edge
	void print();
};

struct Triangle_Vertex {
	size_t he;

	Point* point;

	// constructor assigns Point pointer
	Triangle_Vertex(Point* p);

	// pring the vertex
	void print();
};

struct Triangle_Edge {
	size_t he;

	// default constructor
	Triangle_Edge();

	// print the edge
	void print();
};

struct Triangle_Face {
	size_t he;

	// default constructor
	Triangle_Face();

	//draw the triangle
	void draw();

	// check the constraints for the triangle
	bool constraint_check();

	// print the triangle
	void print();

	// calculate and return the quality of the triangle
	float quality();
};




