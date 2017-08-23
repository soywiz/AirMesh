#include <windows.h>
#include <iostream>
#include <unordered_set>
#include <map>
#define _USE_MATH_DEFINES
#include <math.h>

#include "triangle.h"
#include "triangle_half_edge.h"
#include "air_mesh.h"

#define LEFT_WALL 0
#define TOP_WALL 1
#define RIGHT_WALL 2
#define BOTTOM_WALL 3

// globals from triangle_half_edge.cpp
extern vector<Triangle_Vertex> triangle_vertices;
extern vector<Triangle_Face> triangle_faces;
extern vector<Triangle_Half_Edge> triangle_hes;
extern vector<Triangle_Edge> triangle_edges;

// globals from main.cpp
extern vector<Point> mesh_points;
extern vector<Shape> shapes;
extern Point c1;
extern Point c2;
extern Point c3;
extern Point c4;
extern int WINDOW_SIZE;

void triangulate_shapes() {
	triangle_vertices.clear();
	triangle_faces.clear();
	triangle_hes.clear();
	triangle_edges.clear();
	mesh_points.clear();


	struct triangulateio in, out;

	int shapes_vertex_count = 0;
	int shapes_with_holes = 0;
	for (size_t i = 0; i < shapes.size(); i++) {
		shapes_vertex_count += shapes[i].points.size();
		if (shapes[i].type == POLYGON) {
			shapes_with_holes++;
		}
	}

	triangle_vertices.push_back(Triangle_Vertex(&c1));
	triangle_vertices.push_back(Triangle_Vertex(&c2));
	triangle_vertices.push_back(Triangle_Vertex(&c3));
	triangle_vertices.push_back(Triangle_Vertex(&c4));

	in.numberofpoints = 4 + shapes_vertex_count;
	in.numberofpointattributes = 0;
	in.numberofsegments = 4 + shapes_vertex_count;
	in.numberofholes = shapes_with_holes;

	in.pointlist = (REAL *)malloc(in.numberofpoints * 2 * sizeof(REAL));
	in.pointmarkerlist = (int *)malloc(in.numberofpoints * sizeof(int));
	in.segmentlist = (int *)malloc(in.numberofsegments * 2 * sizeof(int));
	in.segmentmarkerlist = (int *)malloc(in.numberofsegments * sizeof(int));
	in.holelist = (REAL *)malloc(in.numberofholes * 2 * sizeof(REAL));

	// points for window border
	in.pointlist[0] = 0.0;
	in.pointlist[1] = 0.0;
	in.pointlist[2] = WINDOW_SIZE;
	in.pointlist[3] = 0.0;
	in.pointlist[4] = WINDOW_SIZE;
	in.pointlist[5] = WINDOW_SIZE;
	in.pointlist[6] = 0.0;
	in.pointlist[7] = WINDOW_SIZE;
	// marker boundry 1
	in.pointmarkerlist[0] = 0;
	in.pointmarkerlist[1] = 0;
	in.pointmarkerlist[2] = 0;
	in.pointmarkerlist[3] = 0;
	// segments to be in output
	in.segmentlist[0] = 0;
	in.segmentlist[1] = 1;
	in.segmentlist[2] = 1;
	in.segmentlist[3] = 2;
	in.segmentlist[4] = 2;
	in.segmentlist[5] = 3;
	in.segmentlist[6] = 3;
	in.segmentlist[7] = 0;
	// also boundry 1
	in.segmentmarkerlist[0] = 0;
	in.segmentmarkerlist[1] = 0;
	in.segmentmarkerlist[2] = 0;
	in.segmentmarkerlist[3] = 0;

	int point = 4;
	int point_pos = 8;
	int point_maker_pos = 4;
	int segement_pos = 8;
	int segment_marker_pos = 4;
	int hole_pos = 0;
	int marker_num = 1;
	for (size_t i = 0; i < shapes.size(); i++) {
		// save first pos for last line segment
		int first_pos = point;
		// set hole for this shape 
		if (shapes[i].type == POLYGON) {
			Point interior_point = shapes[i].middle();
			in.holelist[hole_pos++] = interior_point.x;
			in.holelist[hole_pos++] = interior_point.y;
		}

		for (size_t j = 0; j < shapes[i].points.size(); j++) {
			// set marker boundry for this point and segment
			in.pointmarkerlist[point_maker_pos++] = marker_num;
			in.segmentmarkerlist[segment_marker_pos++] = marker_num;

			// set segment based on point index
			if (j == shapes[i].points.size() - 1) {
				in.segmentlist[segement_pos++] = point;
				in.segmentlist[segement_pos++] = first_pos;
			}
			else {
				in.segmentlist[segement_pos++] = point;
				in.segmentlist[segement_pos++] = point + 1;
			}

			// set x and y positions for this point
			in.pointlist[point_pos++] = shapes[i].points[j].x;
			in.pointlist[point_pos++] = shapes[i].points[j].y;
			triangle_vertices.push_back(Triangle_Vertex(&shapes[i].points[j]));

			point++;
		}
	}

	// initialize output
	out.pointlist = (REAL *)NULL;
	out.pointmarkerlist = (int *)NULL;
	out.pointattributelist = (REAL *)NULL;
	out.trianglelist = (int *)NULL;
	out.triangleattributelist = (REAL *)NULL;
	out.segmentlist = (int *)NULL;
	out.segmentmarkerlist = (int *)NULL;

	// flags to use: pqzQ
	triangulate("pzQ", &in, &out, (struct triangulateio *) NULL);

	// make new points
	for (int i = triangle_vertices.size(); i < out.numberofpoints; i++) {
		mesh_points.push_back(Point(out.pointlist[2 * i], out.pointlist[2 * i + 1]));
		triangle_vertices.push_back(Triangle_Vertex(&(mesh_points[mesh_points.size() - 1])));
	}

	// hash for points
	map<pair<size_t, size_t>, size_t> edge_hash;

	for (int i = 0; i < out.numberoftriangles; i++) {

		size_t vert[3];
		vert[0] = out.trianglelist[3 * i];
		vert[1] = out.trianglelist[3 * i + 1];
		vert[2] = out.trianglelist[3 * i + 2];

		// create the new face
		triangle_faces.push_back(Triangle_Face());
		size_t face_index = triangle_faces.size() - 1;

		// create 3 new half edges
		triangle_hes.push_back(Triangle_Half_Edge());
		size_t he1_index = triangle_hes.size() - 1;
		triangle_hes.push_back(Triangle_Half_Edge());
		size_t he2_index = triangle_hes.size() - 1;
		triangle_hes.push_back(Triangle_Half_Edge());
		size_t he3_index = triangle_hes.size() - 1;

		// set face and vertex pointers
		triangle_faces[face_index].he = he1_index;
		triangle_vertices[vert[0]].he = he1_index;
		triangle_vertices[vert[1]].he = he2_index;
		triangle_vertices[vert[2]].he = he3_index;

		// set known half edge pointers
		triangle_hes[he1_index].face = face_index;
		triangle_hes[he2_index].face = face_index;
		triangle_hes[he3_index].face = face_index;

		triangle_hes[he1_index].origin = vert[0];
		triangle_hes[he2_index].origin = vert[1];
		triangle_hes[he3_index].origin = vert[2];

		triangle_hes[he1_index].next = he2_index;
		triangle_hes[he1_index].prev = he3_index;

		triangle_hes[he2_index].next = he3_index;
		triangle_hes[he2_index].prev = he1_index;

		triangle_hes[he3_index].next = he1_index;
		triangle_hes[he3_index].prev = he2_index;

		// check if edges already found

		// first edge
		pair<size_t, size_t> edge1;
		size_t edge1_index;
		if (vert[0] < vert[1]) {
			edge1.first = vert[0];
			edge1.second = vert[1];
		}
		else {
			edge1.first = vert[1];
			edge1.second = vert[0];
		}
		map<pair<size_t, size_t>, size_t>::iterator edge1_it = edge_hash.find(edge1);
		// create new edge
		if (edge1_it == edge_hash.end()) {
			triangle_edges.push_back(Triangle_Edge());
			edge1_index = triangle_edges.size() - 1;

			// add to hash
			edge_hash[edge1] = edge1_index;
		}
		// use existing edge
		else {
			edge1_index = edge1_it->second;

			// set flip pointers
			triangle_hes[he1_index].flip = triangle_edges[edge1_index].he;
			triangle_hes[triangle_edges[edge1_index].he].flip = he1_index;
		}

		// second edge
		pair<size_t, size_t> edge2;
		size_t edge2_index;
		if (vert[1] < vert[2]) {
			edge2.first = vert[1];
			edge2.second = vert[2];
		}
		else {
			edge2.first = vert[2];
			edge2.second = vert[1];
		}
		map<pair<size_t, size_t>, size_t>::iterator edge2_it = edge_hash.find(edge2);
		// create new edge
		if (edge2_it == edge_hash.end()) {
			triangle_edges.push_back(Triangle_Edge());
			edge2_index = triangle_edges.size() - 1;

			// add to hash
			edge_hash[edge2] = edge2_index;
		}
		// use existing edge
		else {
			edge2_index = edge2_it->second;

			// set flip pointers
			triangle_hes[he2_index].flip = triangle_edges[edge2_index].he;
			triangle_hes[triangle_edges[edge2_index].he].flip = he2_index;
		}

		// third edge 
		pair<size_t, size_t> edge3;
		size_t edge3_index;
		if (vert[2] < vert[0]) {
			edge3.first = vert[2];
			edge3.second = vert[0];
		}
		else {
			edge3.first = vert[0];
			edge3.second = vert[2];
		}
		map<pair<size_t, size_t>, size_t>::iterator edge3_it = edge_hash.find(edge3);
		// create new edge
		if (edge3_it == edge_hash.end()) {
			triangle_edges.push_back(Triangle_Edge());
			edge3_index = triangle_edges.size() - 1;

			// add to hash
			edge_hash[edge3] = edge3_index;
		}
		// use existing edge
		else {
			edge3_index = edge3_it->second;

			// set flip pointers
			triangle_hes[he3_index].flip = triangle_edges[edge3_index].he;
			triangle_hes[triangle_edges[edge3_index].he].flip = he3_index;
		}

		// set pointers for edges
		triangle_edges[edge1_index].he = he1_index;
		triangle_edges[edge2_index].he = he2_index;
		triangle_edges[edge3_index].he = he3_index;

		// set half edge pointers to edges
		triangle_hes[he1_index].edge = edge1_index;
		triangle_hes[he2_index].edge = edge2_index;
		triangle_hes[he3_index].edge = edge3_index;
	}

	// free memory
	free(in.pointlist);
	free(in.pointmarkerlist);
	free(in.segmentlist);
	free(in.segmentmarkerlist);
	free(in.holelist);

	free(out.pointlist);
	free(out.pointmarkerlist);
	free(out.pointattributelist);
	free(out.trianglelist);
	free(out.triangleattributelist);
	free(out.segmentlist);
	free(out.segmentmarkerlist);
}


struct pair_hash {
	inline std::size_t operator()(const std::pair<int, int> & v) const {
		return v.first * 1000 + v.second;
	}
};

bool safe_flip(Triangle_Edge* e) {
	if (e->he != numeric_limits<size_t>::max() && triangle_hes[e->he].flip != numeric_limits<size_t>::max()) {

		size_t he1 = e->he;
		size_t he2 = triangle_hes[he1].flip;


		// check that its not part of a rope
		Point* p1 = triangle_vertices[triangle_hes[he1].origin].point;
		Point* p2 = triangle_vertices[triangle_hes[he2].origin].point;

		if (p1->parent_shape != NULL && p2->parent_shape != NULL) {
			if (p1->parent_shape == p2->parent_shape && p1->parent_shape->type == LINE) {
				if (p1->parent_shape->adjacent(p1, p2)) {
					return false;
				}
			}
		}

		// also check that angles are less than 180
		float angle;
		Point vector[3];

		vector[0] = *triangle_vertices[triangle_hes[he2].origin].point;
		vector[1] = *triangle_vertices[triangle_hes[triangle_hes[he1].prev].origin].point - vector[0];
		vector[2] = *triangle_vertices[triangle_hes[triangle_hes[he2].prev].origin].point - vector[0];

		angle = atan2(vector[2].y, vector[2].x) - atan2(vector[1].y, vector[1].x);

		if (angle < -1 * M_PI) {
			angle += 2 * M_PI;
		}
		else if (angle > M_PI) {
			angle -= 2 * M_PI;
		}

		if (angle < 0) {
			angle += 2 * M_PI;
		}

		if (angle >= M_PI) {
			return false;
		}

		vector[0] = *triangle_vertices[triangle_hes[he1].origin].point;
		vector[1] = *triangle_vertices[triangle_hes[triangle_hes[he2].prev].origin].point - vector[0];
		vector[2] = *triangle_vertices[triangle_hes[triangle_hes[he1].prev].origin].point - vector[0];

		angle = atan2(vector[2].y, vector[2].x) - atan2(vector[1].y, vector[1].x);

		if (angle < -1 * M_PI) {
			angle += 2 * M_PI;
		}
		else if (angle > M_PI) {
			angle -= 2 * M_PI;
		}

		if (angle < 0) {
			angle += 2 * M_PI;
		}

		if (angle >= M_PI) {
			return false;
		}

		return true;
	}
	else {
		return false;
	}
}

void flip_edge(Triangle_Edge* e) {
	size_t he1 = e->he;
	size_t he2 = triangle_hes[e->he].flip;

	size_t he1p = triangle_hes[he1].prev;
	size_t he1n = triangle_hes[he1].next;
	size_t he2p = triangle_hes[he2].prev;
	size_t he2n = triangle_hes[he2].next;

	size_t t1 = triangle_hes[he1].face;
	size_t t2 = triangle_hes[he2].face;

	size_t p1 = triangle_hes[he1].origin;
	size_t p2 = triangle_hes[he2].origin;

	// adjust he origins for edge
	triangle_hes[he1].origin = triangle_hes[he2p].origin;
	triangle_hes[he2].origin = triangle_hes[he1p].origin;

	// adjust prev and next for each face
	triangle_hes[he1].next = he1p;
	triangle_hes[he1].prev = he2n;
	triangle_hes[he1p].next = he2n;
	triangle_hes[he1p].prev = he1;
	triangle_hes[he2n].next = he1;
	triangle_hes[he2n].prev = he1p;

	triangle_hes[he2].next = he2p;
	triangle_hes[he2].prev = he1n;
	triangle_hes[he2p].next = he1n;
	triangle_hes[he2p].prev = he2;
	triangle_hes[he1n].next = he2;
	triangle_hes[he1n].prev = he2p;

	// ajust faces for each half edge
	triangle_hes[he1].face = t1;
	triangle_hes[he1p].face = t1;
	triangle_hes[he2n].face = t1;

	triangle_hes[he2].face = t2;
	triangle_hes[he2p].face = t2;
	triangle_hes[he1n].face = t2;

	// make sure face it pointing to he in its face
	triangle_faces[t1].he = he1;
	triangle_faces[t2].he = he2;

	// make sure original edge points point to an out half edge
	triangle_vertices[p1].he = he2n;
	triangle_vertices[p2].he = he1n;

}

bool flip_if_reduce(Triangle_Edge* e) {
	float triangle1_quality = triangle_faces[triangle_hes[e->he].face].quality();
	float triangle2_quality = triangle_faces[triangle_hes[triangle_hes[e->he].flip].face].quality();

	// test flip
	flip_edge(e);

	float triangle1_update_quality = triangle_faces[triangle_hes[e->he].face].quality();
	float triangle2_update_quality = triangle_faces[triangle_hes[triangle_hes[e->he].flip].face].quality();

	// reverse if it doesn't help
	if (triangle1_quality + triangle2_quality > triangle1_update_quality + triangle2_update_quality) {
		flip_edge(e);
		return false;
	}

	return true;
}

map<pair<int, int>, size_t> collision_pairs;
unordered_set<int> colliding_shapes;

void collision_detection_handling(float move_percent) {
	for (size_t i = 0; i < triangle_edges.size(); i++) {
		if (safe_flip(&triangle_edges[i])) {
			flip_if_reduce(&triangle_edges[i]);
		}
	}

	collision_pairs.clear();
	colliding_shapes.clear();

	for (size_t i = 0; i < triangle_faces.size(); i++) {

		if (!triangle_faces[i].constraint_check()) {
			Point* a = triangle_vertices[triangle_hes[triangle_faces[i].he].origin].point;
			Point* b = triangle_vertices[triangle_hes[triangle_hes[triangle_faces[i].he].next].origin].point;
			Point* c = triangle_vertices[triangle_hes[triangle_hes[triangle_faces[i].he].prev].origin].point;

			// collision with wall
			if (a->parent_shape != NULL && b->parent_shape == NULL && c->parent_shape == NULL) {
				// top wall
				if (b == &c1 && c == &c2 ||
					b == &c2 && c == &c1) {
					collision_pairs[pair<int, int>(TOP_WALL, a->parent_shape->id)] = i;
				}
				// right wall
				if (b == &c2 && c == &c3 ||
					b == &c3 && c == &c2) {
					collision_pairs[pair<int, int>(RIGHT_WALL, a->parent_shape->id)] = i;
				}
				// bottom wall
				if (b == &c3 && c == &c4 ||
					b == &c4 && c == &c3) {
					collision_pairs[pair<int, int>(BOTTOM_WALL, a->parent_shape->id)] = i;
				}
				// left wall
				if (b == &c4 && c == &c1 ||
					b == &c1 && c == &c4) {
					collision_pairs[pair<int, int>(LEFT_WALL, a->parent_shape->id)] = i;
				}

				colliding_shapes.insert(a->parent_shape->id);
			}
			else if (b->parent_shape != NULL && a->parent_shape == NULL && c->parent_shape == NULL) {
				// top wall
				if (a == &c1 && c == &c2 ||
					a == &c2 && c == &c1) {
					collision_pairs[pair<int, int>(TOP_WALL, b->parent_shape->id)] = i;
				}
				// right wall
				if (a == &c2 && c == &c3 ||
					a == &c3 && c == &c2) {
					collision_pairs[pair<int, int>(RIGHT_WALL, b->parent_shape->id)] = i;
				}
				// bottom wall
				if (a == &c3 && c == &c4 ||
					a == &c4 && c == &c3) {
					collision_pairs[pair<int, int>(BOTTOM_WALL, b->parent_shape->id)] = i;
				}
				// left wall
				if (a == &c4 && c == &c1 ||
					a == &c1 && c == &c4) {
					collision_pairs[pair<int, int>(LEFT_WALL, b->parent_shape->id)] = i;
				}

				colliding_shapes.insert(b->parent_shape->id);
			}
			else if (c->parent_shape != NULL && a->parent_shape == NULL && b->parent_shape == NULL) {
				// top wall
				if (b == &c1 && a == &c2 ||
					b == &c2 && a == &c1) {
					collision_pairs[pair<int, int>(TOP_WALL, c->parent_shape->id)] = i;
				}
				// right wall
				if (b == &c2 && a == &c3 ||
					b == &c3 && a == &c2) {
					collision_pairs[pair<int, int>(RIGHT_WALL, c->parent_shape->id)] = i;
				}
				// bottom wall
				if (b == &c3 && a == &c4 ||
					b == &c4 && a == &c3) {
					collision_pairs[pair<int, int>(BOTTOM_WALL, c->parent_shape->id)] = i;
				}
				// left wall
				if (b == &c4 && a == &c1 ||
					b == &c1 && a == &c4) {
					collision_pairs[pair<int, int>(LEFT_WALL, c->parent_shape->id)] = i;
				}

				colliding_shapes.insert(c->parent_shape->id);
			}
			// two shapes colliding
			else if (a->parent_shape != b->parent_shape &&
				a->parent_shape != NULL && b->parent_shape != NULL) {
				if (a->parent_shape->id < b->parent_shape->id) {
					collision_pairs[pair<int, int>(a->parent_shape->id, b->parent_shape->id)] = i;
				}
				else {
					collision_pairs[pair<int, int>(b->parent_shape->id, a->parent_shape->id)] = i;
				}

				colliding_shapes.insert(a->parent_shape->id);
				colliding_shapes.insert(b->parent_shape->id);
			}
			else if (a->parent_shape != c->parent_shape &&
				a->parent_shape != NULL && c->parent_shape != NULL) {
				if (a->parent_shape->id < c->parent_shape->id) {
					collision_pairs[pair<int, int>(a->parent_shape->id, c->parent_shape->id)] = i;
				}
				else {
					collision_pairs[pair<int, int>(c->parent_shape->id, a->parent_shape->id)] = i;
				}

				colliding_shapes.insert(a->parent_shape->id);
				colliding_shapes.insert(c->parent_shape->id);
			}
			else if (b->parent_shape != c->parent_shape &&
				b->parent_shape != NULL && c->parent_shape != NULL) {
				if (b->parent_shape->id < c->parent_shape->id) {
					collision_pairs[pair<int, int>(b->parent_shape->id, c->parent_shape->id)] = i;
				}
				else {
					collision_pairs[pair<int, int>(c->parent_shape->id, b->parent_shape->id)] = i;
				}

				colliding_shapes.insert(b->parent_shape->id);
				colliding_shapes.insert(c->parent_shape->id);
			}

		}
	}

	if (collision_pairs.size() > 0) {
		cout << "\nColliding Shapes: ";
	}
	for (auto it = colliding_shapes.begin(); it != colliding_shapes.end(); it++) {
		if (!shapes[*it].stationary) {
			// unmove shape
			shapes[*it - 4].move(-1 * move_percent);
		}
		cout << *it << ", ";
	}

	//handle collisions
	if (collision_pairs.size() > 0) {
		cout << "\nColliding Pairs: ";
	}
	for (auto it = collision_pairs.begin(); it != collision_pairs.end(); it++) {
		// wall collisions
		if (it->first.first < 4) {

			// bounce off wall
			if (it->first.first == BOTTOM_WALL || it->first.first == TOP_WALL) {
				shapes[it->first.second - 4].movement_vector.y = -1 * shapes[it->first.second - 4].movement_vector.y;
			}
			else if (it->first.first == LEFT_WALL || it->first.first == RIGHT_WALL) {
				shapes[it->first.second - 4].movement_vector.x = -1 * shapes[it->first.second - 4].movement_vector.x;
			}
		}
		// fake wall collision
		else if (shapes[it->first.first - 4].stationary) {
			if (abs(shapes[it->first.second - 4].movement_vector.y) >= abs(shapes[it->first.second - 4].movement_vector.x)) {
				shapes[it->first.second - 4].movement_vector.y = -1 * shapes[it->first.second - 4].movement_vector.y;
			}
			else {
				shapes[it->first.second - 4].movement_vector.x = -1 * shapes[it->first.second - 4].movement_vector.x;
			}

			// if still colliding flip other
			shapes[it->first.second - 4].move(move_percent);
			if (!triangle_faces[it->second].constraint_check()) {
				shapes[it->first.second - 4].move(-1 * move_percent);
				shapes[it->first.second - 4].movement_vector.x = -1 * shapes[it->first.second - 4].movement_vector.x;
				shapes[it->first.second - 4].movement_vector.y = -1 * shapes[it->first.second - 4].movement_vector.y;
			}
			else {
				shapes[it->first.second - 4].move(-1 * move_percent);
			}
		}
		// fake wall collision
		else if (shapes[it->first.second - 4].stationary) {
			if (abs(shapes[it->first.first - 4].movement_vector.y) >= abs(shapes[it->first.first - 4].movement_vector.x)) {
				shapes[it->first.first - 4].movement_vector.y = -1 * shapes[it->first.first - 4].movement_vector.y;
			}
			else {
				shapes[it->first.first - 4].movement_vector.x = -1 * shapes[it->first.first - 4].movement_vector.x;
			}

			// if still colliding flip other
			shapes[it->first.first - 4].move(move_percent);
			if (!triangle_faces[it->second].constraint_check()) {
				shapes[it->first.first - 4].move(-1 * move_percent);
				shapes[it->first.first - 4].movement_vector.x = -1 * shapes[it->first.first - 4].movement_vector.x;
				shapes[it->first.first - 4].movement_vector.y = -1 * shapes[it->first.first - 4].movement_vector.y;
			}
			else {
				shapes[it->first.first - 4].move(-1 * move_percent);
			}
		}
		// two shape collision
		else {
			// trade movement vectors
			Point direction_a(shapes[it->first.first - 4].movement_vector.x, shapes[it->first.first - 4].movement_vector.y);

			shapes[it->first.first - 4].movement_vector.x = shapes[it->first.second - 4].movement_vector.x;
			shapes[it->first.first - 4].movement_vector.y = shapes[it->first.second - 4].movement_vector.y;

			shapes[it->first.second - 4].movement_vector.x = direction_a.x;
			shapes[it->first.second - 4].movement_vector.y = direction_a.y;
		}

		cout << "<" << it->first.first << ", " << it->first.second << ">, ";
	}
	if (collision_pairs.size() > 0) {
		cout << endl;
	}
}