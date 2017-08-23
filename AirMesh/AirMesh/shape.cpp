
#include "shape.h"

// default constructor
Point::Point() {
	x = 0;
	y = 0;
}

// construction assigns location and parent
Point::Point(float _x, float _y, Shape* p) {
	x = _x;
	y = _y;
	parent_shape = p;
}

// draw the point in opengl
void Point::draw() {
	glColor3f(0, 0, 0);
	glBegin(GL_QUADS);
	glVertex2f(x - POINTSIZE / 2.0, y - POINTSIZE / 2.0);
	glVertex2f(x + POINTSIZE / 2.0, y - POINTSIZE / 2.0);
	glVertex2f(x + POINTSIZE / 2.0, y + POINTSIZE / 2.0);
	glVertex2f(x - POINTSIZE / 2.0, y + POINTSIZE / 2.0);
	glEnd();
}

// negate x and y
void Point::reverse() {
	x = -x;
	y = -y;
}

// subtract point from point
Point Point::operator-(Point p2) {
	return Point(x - p2.x, y - p2.y);
}


// default constructor
Shape::Shape() {
	id = -1;
	type = POLYGON;
}

// constructor to assign id
Shape::Shape(int _id) {
	id = _id;
	type = POLYGON;
	stationary = false;
}

// draw the points for this shape
void Shape::draw_points() {
	for (size_t i = 0; i < points.size(); i++) {
		points[i].draw();
	}
}

// draw the shape itself
void Shape::draw() {
	glColor3f(0, 0, 1.0);
	if (type == POLYGON) {
		glBegin(GL_POLYGON);
	}
	else {
		glBegin(GL_LINE_STRIP);
	}
	for (size_t i = 0; i < points.size(); i++) {
		glVertex2f(points[i].x, points[i].y);
	}
	glEnd();
}

// move the shape by percentage of movement_vector
void Shape::move(float percent) {
	for (size_t i = 0; i < points.size(); i++) {
		points[i].x += movement_vector.x * percent;
		points[i].y += movement_vector.y * percent;
	}
}

// test if two points are next to each other
bool Shape::adjacent(Point* p1, Point* p2) {
	for (size_t i = 0; i < points.size(); i++) {
		if (&points[i] == p1) {
			if (i != 0) {
				if (&points[i - 1] == p2) {
					return true;
				}
			}
			if (i != points.size() - 1) {
				if (&points[i + 1] == p2) {
					return true;
				}
			}
		}
	}
	return false;
}

// get the middle of the shape (average of all points)
Point Shape::middle() {
	if (points.size() > 1) {
		float x = 0;
		float y = 0;

		for (size_t i = 0; i < points.size(); i++) {
			x += points[i].x;
			y += points[i].y;
		}

		x /= points.size();
		y /= points.size();

		return Point(x, y);
	}
	else {
		return Point(0, 0);
	}
}