#ifndef INCLUDED_TUIODEMO_H
#define INCLUDED_TUIODEMO_H

#include "TuioServer.h"
#include "TuioCursor.h"
#include "osc/OscTypes.h"
#include <list>
#include <math.h>

#ifdef __APPLE__
#include <SDL2/SDL.h>
#include <OpenGL/gl.h>
#include <OpenGL/glu.h>
#include <GLUT/glut.h>
#else
#include "SDL.h"
#include <GL/gl.h>
#include <GL/glu.h>
#include <GL/glut.h>
#endif

#include "pavo_driver.h"

using namespace TUIO;
using namespace pavo;

class PavoDemo { 
	
public:
	PavoDemo(TuioServer *server);
	~PavoDemo() {};
	
	void run();
	TuioServer *tuioServer;
	std::list<TuioCursor*> stickyCursorList;
	std::list<TuioCursor*> jointCursorList;
	std::list<TuioCursor*> activeCursorList;
private:
	void drawFrame();
	void drawString(const char *str);
	void processEvents();
	void initWindow();

	SDL_Window *window;
	SDL_Renderer *renderer;
	bool verbose, fullupdate, periodic, fullscreen, running, help;
	
	int width, height;
	int screen_width, screen_height;
	int window_width, window_height;
	TuioTime frameTime;

    pavo_driver *lidarDriver;
	int pavoPollEvent(SDL_Event* event);
	
	void mousePressed(float x, float y);
	void mouseReleased(float x, float y);
	void mouseDragged(float x, float y);
	//int s_id;
};

#endif