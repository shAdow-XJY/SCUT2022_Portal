extern int snippetMain(int, const char*const*);
int main(int argc, char** argv)
{
	return snippetMain(argc, argv);
}


//#include <glut.h>
//#include <Animation/Animation.h>
//
//Animation task1;
//void init()
//{
//	glutInitDisplayMode(GLUT_RGB | GLUT_DOUBLE | GLUT_DEPTH);
//	glutInitWindowSize(600, 600);
//	glutCreateWindow("Model Loader");
//
//
//	//--------------------OpenGL initialization------------------------
//	glClearColor(0.7f, 0.8f, 1.0f, 1.0f);
//	glEnable(GL_LIGHTING);
//	glEnable(GL_LIGHT0);
//	glEnable(GL_DEPTH_TEST);
//	glEnable(GL_NORMALIZE);
//	glLightModeli(GL_LIGHT_MODEL_TWO_SIDE, GL_TRUE);
//	glColorMaterial(GL_FRONT_AND_BACK, GL_AMBIENT_AND_DIFFUSE);
//
//	glMatrixMode(GL_PROJECTION);
//	glLoadIdentity();
//	gluPerspective(45, 1, 1.0, 100000.0);
//	
//	task1.init();
//}
//
//
////----Keyboard callback to toggle initial model orientation---
//int a = 0;
//void keyboard(unsigned char key, int x, int y)
//{
//	task1.keyboard(key);
//
//	task1.update(1000*a);
//	a++;
//	glutPostRedisplay();
//}
//
//void cleanup()
//{
//	task1.cleanup();
//}
//
////------The main display function---------
////----The model is first drawn using a display list so that all GL commands are
////    stored for subsequent display updates.
//void display()
//{
//	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
//
//	glMatrixMode(GL_MODELVIEW);
//	glLoadIdentity();
//
//	task1.display();
//
//	glutSwapBuffers();
//}
//
//int main(int argc, char** argv)
//{
//	glutInit(&argc, argv);
//
//	init();
//
//	glutDisplayFunc(display);
//	glutKeyboardFunc(keyboard);
//	glutMainLoop();
//
//	cleanup();
//}

