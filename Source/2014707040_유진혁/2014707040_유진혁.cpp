#include <Windows.h>
#include <gl\GL.h>
#include <gl\GLU.h>
#include <gl\glut.h>
#include <cmath>

typedef GLfloat Position[3];	// Position[0]: x, Position[1]: y, Position[2]: z
Position p[36][18];				// Coordinates of points
Position normal[36][18];		// Normal vectors of planes
static int controlAngleY = 36, controlAngleZ = 18;	// Rotational angle control value
static bool toggleNormalOfPolygon = false;
static bool toggleNormalOfPoint = false;

// Computing a vector between two points
void Vector(const Position& p1, const Position& p2, Position& v)
{
	v[0] = p2[0] - p1[0];
	v[1] = p2[1] - p1[1];
	v[2] = p2[2] - p1[2];
}

// Normalize a vector
void Normalize(Position& v)
{
	GLfloat d = v[0] * v[0] + v[1] * v[1] + v[2] * v[2];
	if (d > 0)
	{
		GLfloat len = (1.0f / sqrt(d));
		v[0] *= len;
		v[1] *= len;
		v[2] *= len;
	}
}

// Cross product of v1 and v2
void CrossProduct(const Position& v1, const Position& v2, Position& r)
{
	r[0] = v1[1] * v2[2] - v1[2] * v2[1];
	r[1] = v1[2] * v2[0] - v1[0] * v2[2];
	r[2] = v1[0] * v2[1] - v1[1] * v2[0];
	Normalize(r);
}

// Initializes the coordinates of each point
void InitializePoint(float radius)
{
	float cos_10d = cos(10 * 3.14159 / 180.0);
	float sin_10d = sin(10 * 3.14159 / 180.0);
	// Initial circle on plane xy
	for (int j = 0; j < 18; j++)
	{
		float cos_th = cos(j * 20 * 3.14159 / 180.0);
		float sin_th = sin(j * 20 * 3.14159 / 180.0);
		p[0][j][0] = cos_th * radius + 3.0;
		p[0][j][1] = sin_th * radius + 3.0;
		p[0][j][2] = 0;
	}
	// Rotate initial circle around the y-axis
	for (int i = 1; i < 36; i++)
		for (int j = 0; j < 18; j++)
		{
			float sin_th = sin(j * 20 * 3.14159 / 180.0);
			p[i][j][0] = p[i - 1][j][0] * cos_10d + p[i - 1][j][2] * sin_10d;
			p[i][j][1] = sin_th * radius + 3.0;
			p[i][j][2] = -p[i - 1][j][0] * sin_10d + p[i - 1][j][2] * cos_10d;
		}
	// Assign normal vectors of planes to array
	// Normal vector of p[i][j], p[i+1][j], p[i+1][j+1], p[i][j+1] four-point polygon = normal[i+1][j+1]
	for (int i = 0; i < 36; i++)
		for (int j = 0; j < 18; j++)
		{
			Position v1, v2, v3, v4;
			Position n1, n2;
			Vector(p[i][j], p[(i + 1) % 36][j], v1);
			Vector(p[i][j], p[i][(j + 1) % 18], v2);
			CrossProduct(v1, v2, n1);
			Vector(p[(i + 1) % 36][(j + 1) % 18], p[i][(j + 1) % 18], v3);
			Vector(p[(i + 1) % 36][(j + 1) % 18], p[(i + 1) % 36][j], v4);
			CrossProduct(v3, v4, n2);
			normal[(i + 1) % 36][(j + 1) % 18][0] = (n1[0] + n2[0]) / 2.0f;
			normal[(i + 1) % 36][(j + 1) % 18][1] = (n1[1] + n2[1]) / 2.0f;
			normal[(i + 1) % 36][(j + 1) % 18][2] = (n1[2] + n2[2]) / 2.0f;
		}
}

// Draw axes
void DrawAxis()
{
	glLineWidth(1);
	glBegin(GL_LINES);
	{
		glColor3f(1, 0, 0);		// red x-axis
		glVertex3f(0, 0, 0);
		glVertex3f(20, 0, 0);
		glColor3f(0, 1, 0);		// green y-axis
		glVertex3f(0, 0, 0);
		glVertex3f(0, 20, 0);
		glColor3f(0, 0, 1);		// blue z-axis
		glVertex3f(0, 0, 0);
		glVertex3f(0, 0, 20);

	}
	glEnd();
}

// Draw polygons
void DrawQuads()
{
	glBegin(GL_QUADS);
	{
		for (int i = 0; i < controlAngleY; i++)
		{
			for (int j = 0; j < controlAngleZ; j++)
			{
				glVertex3fv(p[i][j]);
				glVertex3fv(p[(i + 1) % 36][j]);
				glVertex3fv(p[(i + 1) % 36][(j + 1) % 18]);
				glVertex3fv(p[i][(j + 1) % 18]);
			}
		}
	}
	glEnd();
}

// Draw normal vectors of polygons
void DrawNormalOfPolygon()
{
	glColor3f(0.5, 0.5, 0.5); // gray
	glBegin(GL_LINES);
	{
		for (int i = 0; i < controlAngleY; i++)
		{
			for (int j = 0; j < controlAngleZ; j++)
			{
				Position center;	// the center of a polygon
				center[0] = (p[i][j][0] + p[(i + 1) % 36][j][0] + p[(i + 1) % 36][(j + 1) % 18][0] + p[i][(j + 1) % 18][0]) / 4.0f;
				center[1] = (p[i][j][1] + p[(i + 1) % 36][j][1] + p[(i + 1) % 36][(j + 1) % 18][1] + p[i][(j + 1) % 18][1]) / 4.0f;
				center[2] = (p[i][j][2] + p[(i + 1) % 36][j][2] + p[(i + 1) % 36][(j + 1) % 18][2] + p[i][(j + 1) % 18][2]) / 4.0f;
				glVertex3fv(center);
				glVertex3f(center[0] + normal[(i + 1) % 36][(j + 1) % 18][0], center[1] + normal[(i + 1) % 36][(j + 1) % 18][1], center[2] + normal[(i + 1) % 36][(j + 1) % 18][2]);
			}
		}
	}
	glEnd();
}

// Draw normal vectors of points
void DrawNormalOfPoint()
{
	glColor3f(0.5, 0.5, 0.5);
	glBegin(GL_LINES);
	{
		for (int i = 0; i < controlAngleY; i++)
		{
			for (int j = 0; j < controlAngleZ; j++)
			{
				// Calculate a normal vector of a point by averaging normal vectors of polygons surrounding the point
				Position n;
				n[0] = (normal[i][j][0] + normal[(i + 1) % 36][j][0] + normal[i][(j + 1) % 18][0] + normal[(i + 1) % 36][(j + 1) % 18][0]) / 4.0f;
				n[1] = (normal[i][j][1] + normal[(i + 1) % 36][j][1] + normal[i][(j + 1) % 18][1] + normal[(i + 1) % 36][(j + 1) % 18][1]) / 4.0f;
				n[2] = (normal[i][j][2] + normal[(i + 1) % 36][j][2] + normal[i][(j + 1) % 18][2] + normal[(i + 1) % 36][(j + 1) % 18][2]) / 4.0f;
				glVertex3fv(p[i][j]);
				glVertex3f(p[i][j][0] + n[0], p[i][j][1] + n[1], p[i][j][2] + n[2]);
			}
		}
	}
	glEnd();
}

void RenderAxis()
{
	// Clear the window with current clearing color
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();

	gluLookAt(1, 1, 1, 0, 0, 0, 0, 1, 0);

	DrawAxis();

	glutSwapBuffers();
}

void RenderPoints()
{
	// Clear the window with current clearing color
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();

	gluLookAt(1, 1, 1, 0, 0, 0, 0, 1, 0);

	DrawAxis();

	InitializePoint(1.7);
	glColor3f(0, 0, 0);	// black
	glPointSize(3);
	glBegin(GL_POINTS);
	{
		for (int i = 0; i < controlAngleY; i++)
			for (int j = 0; j < controlAngleZ; j++)
			{
				glVertex3fv(p[i][j]);
			}
	}
	glEnd();

	if (toggleNormalOfPolygon)
		DrawNormalOfPolygon();
	if (toggleNormalOfPoint)
		DrawNormalOfPoint();

	glutSwapBuffers();
}

void RenderWireframe()
{
	// Clear the window with current clearing color
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();

	gluLookAt(1, 1, 1, 0, 0, 0, 0, 1, 0);

	DrawAxis();

	InitializePoint(1.7);
	glColor3f(0, 0, 0);	// black
	glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
	DrawQuads();

	if (toggleNormalOfPolygon)
		DrawNormalOfPolygon();
	if (toggleNormalOfPoint)
		DrawNormalOfPoint();

	glutSwapBuffers();
}

void RenderQuads()
{
	// Clear the window with current clearing color
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();

	gluLookAt(1, 1, 1, 0, 0, 0, 0, 1, 0);

	DrawAxis();

	InitializePoint(1.7);
	glColor3f(0, 0, 1);	// blue
	glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
	DrawQuads();

	if (toggleNormalOfPolygon)
		DrawNormalOfPolygon();
	if (toggleNormalOfPoint)
		DrawNormalOfPoint();

	glutSwapBuffers();
}

void RenderQuadsAndWireframe()
{
	// Clear the window with current clearing color
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();

	gluLookAt(1, 1, 1, 0, 0, 0, 0, 1, 0);

	DrawAxis();

	// inside wireframe
	InitializePoint(1.69);
	glColor3f(0, 0, 0);	// black
	glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
	DrawQuads();

	// quad plane
	InitializePoint(1.7);
	glColor3f(0, 0, 1);	// blue
	glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
	DrawQuads();

	if (toggleNormalOfPolygon)
		DrawNormalOfPolygon();
	if (toggleNormalOfPoint)
		DrawNormalOfPoint();

	// outside wireframe
	InitializePoint(1.71);
	glColor3f(0, 0, 0);	// black
	glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
	DrawQuads();

	glutSwapBuffers();
}

void RenderTwoSidedShading()
{
	// Clear the window with current clearing color
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();

	gluLookAt(1, 1, 1, 0, 0, 0, 0, 1, 0);

	DrawAxis();

	// inside wireframe
	InitializePoint(1.69);
	glColor3f(0, 0, 0);	// black
	glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
	DrawQuads();

	// shaded quad plane
	InitializePoint(1.7);
	glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
	glBegin(GL_QUADS);
	{
		for (int i = 0; i < controlAngleY; i++)
		{
			for (int j = 0; j < controlAngleZ; j++)
			{
				// Dot product
				GLfloat dot = normal[(i + 1) % 36][(j + 1) % 18][0] + normal[(i + 1) % 36][(j + 1) % 18][1] + normal[(i + 1) % 36][(j + 1) % 18][2];
				if (dot >= 0)
					glColor3f(0, 0, 1);	// blue
				else
					glColor3f(1, 0, 0);	// red
				glVertex3fv(p[i][j]);
				glVertex3fv(p[(i + 1) % 36][j]);
				glVertex3fv(p[(i + 1) % 36][(j + 1) % 18]);
				glVertex3fv(p[i][(j + 1) % 18]);
			}
		}
	}
	glEnd();

	if (toggleNormalOfPolygon)
		DrawNormalOfPolygon();
	if (toggleNormalOfPoint)
		DrawNormalOfPoint();

	// outisde wireframe
	InitializePoint(1.71);
	glColor3f(0, 0, 0);	// black
	glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
	DrawQuads();

	glutSwapBuffers();
}

// Setup the rendering state
void SetupRC(void)
{
	glClearColor(1.0f, 1.0f, 1.0f, 1.0f);
	glEnable(GL_DEPTH_TEST);
}

// Called by GLUT library when the window has changed size
void ChangeSize(int w, int h)
{
	// Prevent a divide by zero
	if (h == 0)
		h = 1;

	// Set viewport to window dimension
	glViewport(0, 0, w, h);

	// Reset coordinate system
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();

	// Establish clipping volume (left, right, bottom, top, near, far)
	if (w <= h)
		glOrtho(-10.0, 10.0, -10.0*(float)h / (float)w, 10.0*(float)h / (float)w, -50.0, 50.0);
	else
		glOrtho(-10.0*(float)w / (float)h, 10.0*(float)w / (float)h, -10.0, 10.0, -50.0, 50.0);
}

void keyboard(unsigned char key, int x, int y)
{
	switch (key)
	{
	case '1':
		glutDisplayFunc(RenderPoints);
		glutPostRedisplay();
		break;
	case '2':
		glutDisplayFunc(RenderWireframe);
		glutPostRedisplay();
		break;
	case '3':
		glutDisplayFunc(RenderQuads);
		glutPostRedisplay();
		break;
	case '4':
		glutDisplayFunc(RenderQuadsAndWireframe);
		glutPostRedisplay();
		break;
	case '5':
		glutDisplayFunc(RenderTwoSidedShading);
		glutPostRedisplay();
		break;
	case '6':
		if (toggleNormalOfPolygon)
			toggleNormalOfPolygon = false;
		else
			toggleNormalOfPolygon = true;
		glutPostRedisplay();
		break;
	case '7':
		if (toggleNormalOfPoint)
			toggleNormalOfPoint = false;
		else
			toggleNormalOfPoint = true;
		glutPostRedisplay();
		break;
	case 'a':
		if (controlAngleY < 36)
			controlAngleY++;
		glutPostRedisplay();
		break;
	case 's':
		if (controlAngleY > 0)
			controlAngleY--;
		glutPostRedisplay();
		break;
	case 'j':
		if (controlAngleZ < 18)
			controlAngleZ++;
		glutPostRedisplay();
		break;
	case 'k':
		if (controlAngleZ > 0)
			controlAngleZ--;
		glutPostRedisplay();
		break;
	default:
		break;
	}
}

// Main program entry point
void main(int argc, char* argv[])
{
	glutInit(&argc, argv);
	glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGB);
	glutInitWindowSize(600, 600);
	glutCreateWindow("OpenGL HW 1");
	glutDisplayFunc(RenderAxis);
	glutReshapeFunc(ChangeSize);
	glutKeyboardFunc(keyboard);

	SetupRC();

	glutMainLoop();
}