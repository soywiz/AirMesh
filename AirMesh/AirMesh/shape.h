#pragma once
#include <stdlib.h>
#include "gl\glut.h"
#include <vector>

using namespace std;

#define POINTSIZE 5

#define POLYGON 0
#define LINE 1

#define SLOW 10

struct Shape;

// 2D point
struct Point {
	// X position
	float x;
	// Y position
	float y;

	// pointer to shape, NULL if not point on shape
	Shape *parent_shape;

	// default constructor
	Point();

	// construction assigns location and parent
	Point(float _x, float _y, Shape* p = NULL);

	// draw the point in opengl
	void draw();

	// negate x and y
	void reverse();

	// subtract point from point
	Point operator-(Point p2);
};

// 2D shape
struct Shape {
	// list of points
	vector<Point> points;
	// direction shape is moving
	Point movement_vector;
	// type, either POLYGON or LINE
	int type;
	// true if object can't move
	bool stationary;

	// unique id
	int id;

	// default constructor
	Shape();

	// constructor to assign id
	Shape(int _id);

	// draw the points for this shape
	void draw_points();

	// draw the shape itself
	void draw();

	// move the shape by percentage of movement_vector
	void move(float percent);

	// test if two points are next to each other
	bool adjacent(Point* p1, Point* p2);

	// get the middle of the shape (average of all points)
	Point middle();
};
