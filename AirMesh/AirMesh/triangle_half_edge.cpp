#include "triangle_half_edge.h"

// surface representation globals
vector<Triangle_Vertex> triangle_vertices;
vector<Triangle_Face> triangle_faces;
vector<Triangle_Half_Edge> triangle_hes;
vector<Triangle_Edge> triangle_edges;

// default constructor
Triangle_Half_Edge::Triangle_Half_Edge() {
	next = numeric_limits<size_t>::max();
	prev = numeric_limits<size_t>::max();
	flip = numeric_limits<size_t>::max();
	face = numeric_limits<size_t>::max();
	origin = numeric_limits<size_t>::max();
	edge = numeric_limits<size_t>::max();
}

// print the half edge
void Triangle_Half_Edge::print() {
	printf("next: %u\n", next);
	printf("prev: %u\n", prev);
	printf("flip: %u\n", flip);
	printf("face: %u\n", face);
	printf("origin: %u\n", origin);
	printf("edge: %u\n", edge);
}

// constructor assigns Point pointer
Triangle_Vertex::Triangle_Vertex(Point* p) {
	point = p;

	he = numeric_limits<size_t>::max();
}

// pring the vertex
void Triangle_Vertex::print() {
	printf("half_edge: %u\n", he);
}

// default constructor
Triangle_Edge::Triangle_Edge() {
	he = numeric_limits<size_t>::max();
}

// print the edge
void Triangle_Edge::print() {
	printf("half_edge: %u\n", he);
}

// default constructor
Triangle_Face::Triangle_Face() {
	he = numeric_limits<size_t>::max();
}

//draw the triangle
void Triangle_Face::draw() {
	size_t v1 = triangle_hes[he].origin;
	size_t v2 = triangle_hes[triangle_hes[he].next].origin;
	size_t v3 = triangle_hes[triangle_hes[he].prev].origin;

	glBegin(GL_LINE_LOOP);
	glColor3f(1.0, 0, 0);
	glVertex2f(triangle_vertices[v1].point->x, triangle_vertices[v1].point->y);
	glVertex2f(triangle_vertices[v2].point->x, triangle_vertices[v2].point->y);
	glVertex2f(triangle_vertices[v3].point->x, triangle_vertices[v3].point->y);
	glEnd();
}

// check the constraints for the triangle
bool Triangle_Face::constraint_check() {
	size_t v1 = triangle_hes[he].origin;
	size_t v2 = triangle_hes[triangle_hes[he].next].origin;
	size_t v3 = triangle_hes[triangle_hes[he].prev].origin;

	float ba_x = triangle_vertices[v2].point->x - triangle_vertices[v1].point->x;
	float ba_y = triangle_vertices[v2].point->y - triangle_vertices[v1].point->y;
	float ca_x = triangle_vertices[v3].point->x - triangle_vertices[v1].point->x;
	float ca_y = triangle_vertices[v3].point->y - triangle_vertices[v1].point->y;
	float area = ba_x*ca_y - ba_y*ca_x;
	if (area >= 0) {
		return true;
	}
	else {
		return false;
	}
}

// print the triangle
void Triangle_Face::print() {
	printf("half_edge: %u\n", he);
}

// calculate and return the quality of the triangle
float Triangle_Face::quality() {
	Point* v1 = triangle_vertices[triangle_hes[he].origin].point;
	Point* v2 = triangle_vertices[triangle_hes[triangle_hes[he].next].origin].point;
	Point* v3 = triangle_vertices[triangle_hes[triangle_hes[he].prev].origin].point;

	float area = abs((v1->x * (v2->y - v3->y) + v2->x * (v3->y - v1->y) + v3->x * (v1->y - v2->y)) / 2.0f);
	float length1 = powf((v1->x - v2->x), 2) + powf((v1->y - v2->y), 2);
	float length2 = powf((v1->x - v3->x), 2) + powf((v1->y - v3->y), 2);
	float length3 = powf((v3->x - v2->x), 2) + powf((v3->y - v2->y), 2);
	return 4.0f / sqrt(3.0f) * area / (length1 + length2 + length3);
}