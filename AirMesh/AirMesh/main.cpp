#include <windows.h>
#include <stdio.h>
#include <ctime>
#include <iostream>

#include "triangle_half_edge.h"
#include "air_mesh.h"


#define MAX_SHAPES 50

#define SHAPE_MODE 0
#define VECTOR_MODE 1
#define TIME_MODE 2

#define MOUSE_PRECISION 5

int WINDOW_SIZE = 800;

int mouseX = -1, mouseY = -1;
bool mouseLeftDown = false, mouseRightDown = false, mouseMiddleDown = false;

vector<Shape> shapes;
int mode;
int current_shape;
clock_t previous_time;

Point c1(0,0);
Point c2(WINDOW_SIZE, 0);
Point c3(WINDOW_SIZE, WINDOW_SIZE);
Point c4(0, WINDOW_SIZE);

vector<Point> mesh_points;

bool show_triangles;
bool draw_vectors;

int next_shape_id = 4;

// globals from triangle_half_edge.cpp
extern vector<Triangle_Vertex> triangle_vertices;
extern vector<Triangle_Face> triangle_faces;
extern vector<Triangle_Half_Edge> triangle_hes;
extern vector<Triangle_Edge> triangle_edges;



void display(void) {
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	if (mode == SHAPE_MODE) {
		// draw all previous shapes
		for (int i = 0; i < current_shape; i++) {
			shapes[i].draw();
		}
		// draw points for current shape
		shapes[current_shape].draw();
		shapes[current_shape].draw_points();
	}
	else if (mode == VECTOR_MODE) {
		// draw all shapes 
		for (size_t i = 0; i < shapes.size(); i++) {
			shapes[i].draw();
		}
		// draw movement vectors
		for (size_t i = 0; i < shapes.size(); i++) {
			if (!shapes[i].stationary) {
				Point middle = shapes[i].middle();
				Point move = Point(middle.x + shapes[i].movement_vector.x, middle.y + shapes[i].movement_vector.y);
				middle.draw();
				move.draw();
				glColor3f(1.0, 0, 0);
				glBegin(GL_LINES);
					glVertex2d(middle.x, middle.y);
					glVertex2d(move.x, move.y);
				glEnd();
			}
		}
	}
	else if (mode == TIME_MODE) {
		clock_t current = clock();
		float move_percent = (current - previous_time) / (float)CLOCKS_PER_SEC;
		previous_time = current;

		// move all shapes
		for (size_t i = 0; i < shapes.size(); i++) {
			shapes[i].move(move_percent);
		}

		// detect and handle collisions
		collision_detection_handling(move_percent);

		// draw the shapes
		for (size_t i = 0; i < shapes.size(); i++) {
			shapes[i].draw();
		}

		if (draw_vectors) {
			// draw movement vectors
			for (size_t i = 0; i < shapes.size(); i++) {
				if (!shapes[i].stationary) {
					Point middle = shapes[i].middle();
					Point move = Point(middle.x + shapes[i].movement_vector.x, middle.y + shapes[i].movement_vector.y);
					middle.draw();
					move.draw();
					glColor3f(1.0, 0, 0);
					glBegin(GL_LINES);
						glVertex2d(middle.x, middle.y);
						glVertex2d(move.x, move.y);
					glEnd();
				}
			}
		}
		
		glutPostRedisplay();
	}

	// draw collision triangles
	if (show_triangles) {
		for (size_t i = 0; i < triangle_faces.size(); i++) {
			triangle_faces[i].draw();
		}
	}

	glFlush();
	glutSwapBuffers();
}

void mouse(int button, int state, int x, int y) {
	switch (button)
	{
	case GLUT_LEFT_BUTTON:
		mouseLeftDown = state == GLUT_DOWN;
		if (mode == SHAPE_MODE && mouseLeftDown) {
			// add point to current shape
			shapes[current_shape].points.push_back(Point(x, y, &shapes[current_shape]));
			cout << x << " " << y << endl;
			glutPostRedisplay();
		}
		else if (mode == VECTOR_MODE && mouseLeftDown) {
			for (size_t i = 0; i < shapes.size(); i++) {
				Point middle = shapes[i].middle();
				if (abs(middle.x - x) < MOUSE_PRECISION && abs(middle.y - y) < MOUSE_PRECISION) {
					current_shape = i;
					shapes[current_shape].movement_vector = Point(0, 0);
					break;
				}
			}
			glutPostRedisplay();
		}
		else if (mode == VECTOR_MODE && !mouseLeftDown) {
			current_shape = -1;
		}
		break;
	case GLUT_MIDDLE_BUTTON:
		mouseMiddleDown = state == GLUT_DOWN;
		break;
	case GLUT_RIGHT_BUTTON:
		mouseRightDown = state == GLUT_DOWN;
		break;
	}
	mouseX = x;
	mouseY = y;
}

void motion(int x, int y) {
	float dx, dy;
	dx = (x - mouseX);
	dy = (y - mouseY);
	mouseX = x;
	mouseY = y;

	if (mode == VECTOR_MODE && current_shape >= 0 && !shapes[current_shape].stationary) {
		// set movement vector for current shape in shape mode
		Point middle = shapes[current_shape].middle();
		shapes[current_shape].movement_vector = Point(x - middle.x, y - middle.y);
		glutPostRedisplay();
	}
}

void reshape(int w, int h) {
	glViewport(0, 0, w, h);
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	gluOrtho2D(0, glutGet(GLUT_WINDOW_WIDTH) - 1, glutGet(GLUT_WINDOW_HEIGHT) - 1, 0);
	glMatrixMode(GL_MODELVIEW);
}


void key(unsigned char c, int x, int y) {
	switch (c) {
	// pause simulation
	case 'p':
	case 'P':
		if (mode == TIME_MODE) {
			mode = VECTOR_MODE;
		}
		break;
	// add next shape in shape mode
	case 'n':
	case 'N':
		if (mode == SHAPE_MODE) {
			if (current_shape < MAX_SHAPES - 1) {
				shapes.push_back(Shape(next_shape_id++));
				current_shape++;
				glutPostRedisplay();
			}
		}
		break;
	// change shape movement mode
	case 'v':
	case 'V':
		if (mode == SHAPE_MODE) {
			shapes[current_shape].stationary = !shapes[current_shape].stationary;
			cout << "Shape is now ";
			if (shapes[current_shape].stationary) {
				cout << "stationary" << endl;
			}
			else {
				cout << "movable" << endl;
			}
		}
		break;
	// change shape type in shape mode
	case 'b':
	case 'B':
		if (mode == SHAPE_MODE) {
			if (shapes[current_shape].type == LINE) {
				shapes[current_shape].type = POLYGON;
			}
			else if (shapes[current_shape].type == POLYGON) {
				shapes[current_shape].type = LINE;
			}
			glutPostRedisplay();
		}
		break;
	// switch modes
	case 'm':
	case 'M':
		if (mode == SHAPE_MODE) {
			mode = VECTOR_MODE;
			if (shapes[current_shape].points.size() == 0) {
				shapes.pop_back();
			}
			current_shape = -1;
			glutPostRedisplay();
		}
		else if (mode == VECTOR_MODE) {
			if (triangle_faces.size() == 0) {
				triangulate_shapes();
				show_triangles = false;
				draw_vectors = false;
			}
			mode = TIME_MODE;
			previous_time = clock();
			glutPostRedisplay();
		}
		break;
	// triangulate
	case 't':
	case 'T':
		if (mode == VECTOR_MODE) {
			triangulate_shapes();
			show_triangles = true;
			draw_vectors = true;
			glutPostRedisplay();
		}
		break;
	// show or hide triangles
	case 's':
	case 'S':
		if (triangle_faces.size() > 0) {
			show_triangles = !show_triangles;
			if (mode == VECTOR_MODE) {
				glutPostRedisplay();
			}
		}
		break;
	// show or hide vectors
	case 'd':
	case 'D':
		if (mode == TIME_MODE) {
			draw_vectors = !draw_vectors;
		}
		break;
	}
}

void init(void) {
	/* select clearing color 	*/
	glClearColor(1.0, 1.0, 1.0, 0.0);

	glDisable(GL_DEPTH_TEST);

	glShadeModel(GL_SMOOTH);

	glDisable(GL_CULL_FACE);

	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	gluOrtho2D(0, glutGet(GLUT_WINDOW_WIDTH) - 1, glutGet(GLUT_WINDOW_HEIGHT) - 1, 0);

	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();

	mode = SHAPE_MODE;
	shapes.reserve(MAX_SHAPES);
	shapes.push_back(Shape(next_shape_id++));
	current_shape = 0;
	show_triangles = false;
	draw_vectors = false;
}

int main(int argc, char* argv[]) {
	glutInit(&argc, argv);
	glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGB);
	glutInitWindowSize(WINDOW_SIZE, WINDOW_SIZE);
	glutInitWindowPosition(100, 100);
	glutCreateWindow("Air Mesh - Clifton Rapier");
	init();
	glutReshapeFunc(reshape);
	glutDisplayFunc(display);
	glutMouseFunc(mouse);
	glutMotionFunc(motion);
	glutKeyboardFunc(key);
	glutMainLoop();
	return 0;
}