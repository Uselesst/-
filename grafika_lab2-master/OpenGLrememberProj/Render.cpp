#include "Render.h"

#include <sstream>
#include <iostream>

#include <windows.h>
#include <GL\GL.h>
#include <GL\GLU.h>

#include "MyOGL.h"

#include "Camera.h"
#include "Light.h"
#include "Primitives.h"

#include "GUItextRectangle.h"

bool textureMode = true;
bool lightMode = true;

//класс для настройки камеры
class CustomCamera : public Camera
{
public:
	//дистанция камеры
	double camDist;
	//углы поворота камеры
	double fi1, fi2;


	//значния масеры по умолчанию
	CustomCamera()
	{
		camDist = 15;
		fi1 = 1;
		fi2 = 1;
	}


	//считает позицию камеры, исходя из углов поворота, вызывается движком
	void SetUpCamera()
	{
		//отвечает за поворот камеры мышкой
		lookPoint.setCoords(0, 0, 0);

		pos.setCoords(camDist * cos(fi2) * cos(fi1),
			camDist * cos(fi2) * sin(fi1),
			camDist * sin(fi2));

		if (cos(fi2) <= 0)
			normal.setCoords(0, 0, -1);
		else
			normal.setCoords(0, 0, 1);

		LookAt();
	}

	void CustomCamera::LookAt()
	{
		//функция настройки камеры
		gluLookAt(pos.X(), pos.Y(), pos.Z(), lookPoint.X(), lookPoint.Y(), lookPoint.Z(), normal.X(), normal.Y(), normal.Z());
	}



}  camera;   //создаем объект камеры


//Класс для настройки света
class CustomLight : public Light
{
public:
	CustomLight()
	{
		//начальная позиция света
		pos = Vector3(1, 1, 3);
	}


	//рисует сферу и линии под источником света, вызывается движком
	void  DrawLightGhismo()
	{
		glDisable(GL_LIGHTING);


		glColor3d(0.9, 0.8, 0);
		Sphere s;
		s.pos = pos;
		s.scale = s.scale * 0.08;
		s.Show();

		if (OpenGL::isKeyPressed('G'))
		{
			glColor3d(0, 0, 0);
			//линия от источника света до окружности
			glBegin(GL_LINES);
			glVertex3d(pos.X(), pos.Y(), pos.Z());
			glVertex3d(pos.X(), pos.Y(), 0);
			glEnd();

			//рисуем окруность
			Circle c;
			c.pos.setCoords(pos.X(), pos.Y(), 0);
			c.scale = c.scale * 1.5;
			c.Show();
		}

	}

	void SetUpLight()
	{
		GLfloat amb[] = { 0.2, 0.2, 0.2, 0 };
		GLfloat dif[] = { 1.0, 1.0, 1.0, 0 };
		GLfloat spec[] = { .7, .7, .7, 0 };
		GLfloat position[] = { pos.X(), pos.Y(), pos.Z(), 1. };

		// параметры источника света
		glLightfv(GL_LIGHT0, GL_POSITION, position);
		// характеристики излучаемого света
		// фоновое освещение (рассеянный свет)
		glLightfv(GL_LIGHT0, GL_AMBIENT, amb);
		// диффузная составляющая света
		glLightfv(GL_LIGHT0, GL_DIFFUSE, dif);
		// зеркально отражаемая составляющая света
		glLightfv(GL_LIGHT0, GL_SPECULAR, spec);

		glEnable(GL_LIGHT0);
	}


} light;





int mouseX = 0, mouseY = 0;

void mouseEvent(OpenGL* ogl, int mX, int mY)
{
	int dx = mouseX - mX;
	int dy = mouseY - mY;
	mouseX = mX;
	mouseY = mY;


	if (OpenGL::isKeyPressed(VK_RBUTTON))
	{
		camera.fi1 += 0.01 * dx;
		camera.fi2 += -0.01 * dy;
	}



	if (OpenGL::isKeyPressed('G') && !OpenGL::isKeyPressed(VK_LBUTTON))
	{
		LPPOINT POINT = new tagPOINT();
		GetCursorPos(POINT);
		ScreenToClient(ogl->getHwnd(), POINT);
		POINT->y = ogl->getHeight() - POINT->y;

		Ray r = camera.getLookRay(POINT->x, POINT->y);

		double z = light.pos.Z();

		double k = 0, x = 0, y = 0;
		if (r.direction.Z() == 0)
			k = 0;
		else
			k = (z - r.origin.Z()) / r.direction.Z();

		x = k * r.direction.X() + r.origin.X();
		y = k * r.direction.Y() + r.origin.Y();

		light.pos = Vector3(x, y, z);
	}

	if (OpenGL::isKeyPressed('G') && OpenGL::isKeyPressed(VK_LBUTTON))
	{
		light.pos = light.pos + Vector3(0, 0, 0.02 * dy);
	}


}

void mouseWheelEvent(OpenGL* ogl, int delta)
{

	if (delta < 0 && camera.camDist <= 1)
		return;
	if (delta > 0 && camera.camDist >= 100)
		return;

	camera.camDist += 0.01 * delta;

}

void keyDownEvent(OpenGL* ogl, int key)
{
	if (key == 'L')
	{
		lightMode = !lightMode;
	}

	if (key == 'T')
	{
		textureMode = !textureMode;
	}

	if (key == 'R')
	{
		camera.fi1 = 1;
		camera.fi2 = 1;
		camera.camDist = 15;

		light.pos = Vector3(1, 1, 3);
	}

	if (key == 'F')
	{
		light.pos = camera.pos;
	}
}

void keyUpEvent(OpenGL* ogl, int key)
{

}



GLuint texId;


void initRender(OpenGL* ogl)
{

	glPixelStorei(GL_UNPACK_ALIGNMENT, 4);


	glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);


	glEnable(GL_TEXTURE_2D);



	RGBTRIPLE* texarray;


	char* texCharArray;
	int texW, texH;
	OpenGL::LoadBMP("texture.bmp", &texW, &texH, &texarray);
	OpenGL::RGBtoChar(texarray, texW, texH, &texCharArray);




	glGenTextures(1, &texId);

	glBindTexture(GL_TEXTURE_2D, texId);


	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, texW, texH, 0, GL_RGBA, GL_UNSIGNED_BYTE, texCharArray);


	free(texCharArray);
	free(texarray);


	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);



	ogl->mainCamera = &camera;
	ogl->mainLight = &light;


	glEnable(GL_NORMALIZE);

	glEnable(GL_LINE_SMOOTH);




	glLightModeli(GL_LIGHT_MODEL_TWO_SIDE, 0);

	camera.fi1 = -1.3;
	camera.fi2 = 0.8;
}

struct Vector {
	float x;
	float y;
	float z;
};

Vector FindNormal(const double(&p1)[3], const double(&p2)[3], const double(&p3)[3])
{
	Vector v1, v2, normal;


	v1.x = p2[0] - p1[0];
	v1.y = p2[1] - p1[1];
	v1.z = p2[2] - p1[2];

	v2.x = p3[0] - p1[0];
	v2.y = p3[1] - p1[1];
	v2.z = p3[2] - p1[2];


	normal.x = v1.y * v2.z - v1.z * v2.y;
	normal.y = -(v1.x * v2.z - v1.z * v2.x);
	normal.z = v1.x * v2.y - v1.y * v2.x;


	float length = sqrt(normal.x * normal.x + normal.y * normal.y + normal.z * normal.z);
	normal.x /= length;
	normal.y /= length;
	normal.z /= length;
	glNormal3d(normal.x, normal.y, normal.z);
	return normal;
}



void Render(OpenGL* ogl)
{



	glDisable(GL_TEXTURE_2D);
	glDisable(GL_LIGHTING);

	glEnable(GL_DEPTH_TEST);
	if (textureMode)
		glEnable(GL_TEXTURE_2D);

	if (lightMode)
		glEnable(GL_LIGHTING);


	//альфаналожение
	//glEnable(GL_BLEND);
	//glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);


	//настройка материала
	GLfloat amb[] = { 0.2, 0.2, 0.1, 1. };
	GLfloat dif[] = { 0.4, 0.65, 0.5, 1. };
	GLfloat spec[] = { 0.9, 0.8, 0.3, 1. };
	GLfloat sh = 0.1f * 256;


	//фоновая
	glMaterialfv(GL_FRONT, GL_AMBIENT, amb);
	//дифузная
	glMaterialfv(GL_FRONT, GL_DIFFUSE, dif);
	//зеркальная
	glMaterialfv(GL_FRONT, GL_SPECULAR, spec); \
		//размер блика
		glMaterialf(GL_FRONT, GL_SHININESS, sh);

	//чтоб было красиво, без квадратиков (сглаживание освещения)
	glShadeModel(GL_SMOOTH);
	//===================================
	//Прогать тут  


	//Начало рисования квадратика станкина
	double A[] = { 0, 0, 0 };
	double B[] = { 0, 0, 1 };
	double C[] = { 5,-2, 1 };
	double D[] = { 5,-2, 0 };
	FindNormal(A, B, C);
	glBegin(GL_QUADS); // Сторона 1
	glColor3d(0.2, 0.7, 0.7);
	glVertex3dv(A);
	glVertex3dv(B);
	glVertex3dv(C);
	glVertex3dv(D);
	glEnd();

	double A1[] = { 5, -2, 0 };
	double B1[] = { 5, -2, 1 };
	double C1[] = { 7,4, 1 };
	double D1[] = { 7,4, 0 };
	FindNormal(A1, B1, C1);
	glBegin(GL_QUADS); // Сторона 2
	glColor3d(0, 0, 0);
	glVertex3dv(A1);
	glVertex3dv(B1);
	glVertex3dv(C1);
	glVertex3dv(D1);
	glEnd();


	double A2[] = { 7, 4, 0 };
	double B2[] = { 7, 4, 1 };
	double C2[] = { 0,4, 1 };
	double D2[] = { 0,4, 0 };
	FindNormal(A2, B2, C2);
	glBegin(GL_QUADS); // Сторона 3
	glColor3d(1, 0, 0);
	glVertex3dv(A2);
	glVertex3dv(B2);
	glVertex3dv(C2);
	glVertex3dv(D2);
	glEnd();


	double A3[] = { 0, 4, 1 };
	double B3[] = { 0, 4, 0 };
	double C3[] = { -2,9, 0 };
	double D3[] = { -2,9, 1 };
	FindNormal(A3, B3, C3);
	glBegin(GL_QUADS); // Сторона 4
	glColor3d(0, 1, 0);
	glVertex3dv(A3);
	glVertex3dv(B3);
	glVertex3dv(C3);
	glVertex3dv(D3);
	glEnd();


	double A4[] = { -2, 9, 1 };
	double B4[] = { -2, 9, 0 };
	double C4[] = { -8,8, 0 };
	double D4[] = { -8,8, 1 };
	FindNormal(A4, B4, C4);
	glBegin(GL_QUADS); // Сторона 5
	glColor3d(0, 0, 1);
	glVertex3dv(A4);
	glVertex3dv(B4);
	glVertex3dv(C4);
	glVertex3dv(D4);
	glEnd();



	double A6[] = { -8, 8, 0 };
	double B6[] = { -8, 8, 1 };
	double C6[] = { -2, 3, 1 };
	double D6[] = { -2, 3, 0 };
	FindNormal(A6, B6, C6);
	glBegin(GL_QUADS); // Сторона 6
	glColor3d(0, 0, 0);
	glVertex3dv(A6);
	glVertex3dv(B6);
	glVertex3dv(C6);
	glVertex3dv(D6);
	glEnd();


	double A7[] = { -2, 3, 0 };
	double B7[] = { -2, 3, 1 };
	double C7[] = { -5, -4, 1 };
	double D7[] = { -5,-4, 0 };
	FindNormal(A7, B7, C7);
	glBegin(GL_QUADS); // Сторона 7
	glColor3d(0, 1, 0);
	glVertex3dv(A7);
	glVertex3dv(B7);
	glVertex3dv(C7);
	glVertex3dv(D7);
	glEnd();


	double A8[] = { -5, -4, 0 };
	double B8[] = { -5, -4, 1 };
	double C8[] = { 0,0, 1 };
	double D8[] = { 0,0, 0 };
	FindNormal(A8, B8, C8);
	glBegin(GL_QUADS); // Сторона 8
	glColor3d(1, 0, 0);
	glVertex3dv(A8);
	glVertex3dv(B8);
	glVertex3dv(C8);
	glVertex3dv(D8);
	glEnd();

	double A9[] = { 0, 0, 0 };
	double B9[] = { 0, 4, 0 };
	double C9[] = { -2, 3, 0 };
	FindNormal(A9, B9, C9);
	glBegin(GL_TRIANGLES); // Основание 4 1
	glColor3d(1, 0.7, 0.5);
	glVertex3dv(A9);
	glVertex3dv(B9);
	glVertex3dv(C9);
	glEnd();

	double A11[] = { 0, 0, 1 };
	double B11[] = { 0, 4, 1 };
	double C11[] = { -2, 3, 1 };
	FindNormal(A11, B11, C11);
	glBegin(GL_TRIANGLES); // Основание 4 2 
	glColor3d(1, 0.7, 0.5);
	glVertex3dv(A11);
	glVertex3dv(B11);
	glVertex3dv(C11);
	glEnd();

	double A12[] = { -2, 3, 0 };
	double B12[] = { -5, -4, 0 };
	double C12[] = { 0, 0, 0 };
	FindNormal(A12, B12, C12);
	glBegin(GL_TRIANGLES); // Основание 3 1
	glColor3d(1, 0, 1);
	glVertex3dv(A12);
	glVertex3dv(B12);
	glVertex3dv(C12);
	glEnd();

	double A13[] = { -2, 3, 1 };
	double B13[] = { -5, -4, 1 };
	double C13[] = { 0, 0, 1 };
	FindNormal(A13, B13, C13);
	glBegin(GL_TRIANGLES); // Основание 3 2
	glColor3d(1, 0.5, 1);
	glVertex3dv(A13);
	glVertex3dv(B13);
	glVertex3dv(C13);
	glEnd();

	double A14[] = { -8, 8, 0 };
	double B14[] = { -2, 9, 0 };
	double C14[] = { 0,4, 0 };
	double D14[] = { -2,3, 0 };
	FindNormal(A14, B14, C14);
	glBegin(GL_QUADS);
	glColor3d(1, 0, 0);
	glVertex3dv(A14);
	glVertex3dv(B14);
	glVertex3dv(C14);
	glVertex3dv(D14);
	glEnd();

	double A15[] = { -8, 8, 1 };
	double B15[] = { -2, 9, 1 };
	double C15[] = { 0,4, 1 };
	double D15[] = { -2,3, 1 };
	FindNormal(A15, B15, C15);
	glBegin(GL_QUADS);
	glColor3d(1, 0, 0);
	glVertex3dv(A15);
	glVertex3dv(B15);
	glVertex3dv(C15);
	glVertex3dv(D15);
	glEnd();

	double A16[] = { 0, 4, 0 };
	double B16[] = { 0, 0, 0 };
	double C16[] = { 5,-2, 0 };
	double D16[] = { 7,4, 0 };
	FindNormal(A16, B16, C16);
	glBegin(GL_QUADS);
	glColor3d(0, 1, 0);
	glVertex3dv(A16);
	glVertex3dv(B16);
	glVertex3dv(C16);
	glVertex3dv(D16);
	glEnd();

	double A17[] = { 0, 4, 1 };
	double B17[] = { 0, 0, 1 };
	double C17[] = { 5,-2, 1 };
	double D17[] = { 7,4, 1 };
	FindNormal(A17, B17, C17);
	glBegin(GL_QUADS);
	glColor3d(0, 1, 0);
	glVertex3dv(A17);
	glVertex3dv(B17);
	glVertex3dv(C17);
	glVertex3dv(D17);
	glEnd();


	glMatrixMode(GL_PROJECTION);

	glPushMatrix();
	glLoadIdentity();
	glOrtho(0, ogl->getWidth(), 0, ogl->getHeight(), 0, 1);

	glMatrixMode(GL_MODELVIEW);
	glPushMatrix();
	glLoadIdentity();
	glDisable(GL_LIGHTING);



	GuiTextRectangle rec;
	rec.setSize(300, 150);
	rec.setPosition(10, ogl->getHeight() - 150 - 10);


	std::stringstream ss;
	ss << "L - вкл/выкл освещение" << std::endl;
	ss << "F - Свет из камеры" << std::endl;
	ss << "G - двигать свет по горизонтали" << std::endl;
	ss << "G+ЛКМ двигать свет по вертекали" << std::endl;
	ss << "Коорд. света: (" << light.pos.X() << ", " << light.pos.Y() << ", " << light.pos.Z() << ")" << std::endl;
	ss << "Коорд. камеры: (" << camera.pos.X() << ", " << camera.pos.Y() << ", " << camera.pos.Z() << ")" << std::endl;
	ss << "Параметры камеры: R=" << camera.camDist << ", fi1=" << camera.fi1 << ", fi2=" << camera.fi2 << std::endl;

	rec.setText(ss.str().c_str());
	rec.Draw();

	glMatrixMode(GL_PROJECTION);
	glPopMatrix();


	glMatrixMode(GL_MODELVIEW);
	glPopMatrix();

}
