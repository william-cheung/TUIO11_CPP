/*
	TUIO C++ Server Demo
    Copyright (c) 2005-2016 Martin Kaltenbrunner <martin@tuio.org>
 
    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program; if not, write to the Free Software
    Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
*/

#include <math.h>

#include "PavoDemo.h"

void PavoDemo::drawFrame() {

	if(!running) return;
	glClear(GL_COLOR_BUFFER_BIT);
	char id[3];

	// draw the cursors
	std::list<TuioCursor*> cursorList = tuioServer->getTuioCursors();
	for (std::list<TuioCursor*>::iterator iter = cursorList.begin(); iter!=cursorList.end(); iter++) {
		TuioCursor *tcur = (*iter);
		std::list<TuioPoint> path = tcur->getPath();
		if (path.size()>0) {

			TuioPoint last_point = path.front();
			glBegin(GL_LINES);
			glColor3f(0.0, 0.0, 1.0);

			for (std::list<TuioPoint>::iterator point = path.begin(); point!=path.end(); point++) {
				glVertex3f(last_point.getScreenX(width), last_point.getScreenY(height), 0.0f);
				glVertex3f(point->getScreenX(width), point->getScreenY(height), 0.0f);
				last_point.update(point->getX(),point->getY());
			}
			glEnd();

			// draw the finger tip
			glColor3f(0.75, 0.75, 0.75);
			std::list<TuioCursor*>::iterator joint = std::find( jointCursorList.begin(), jointCursorList.end(), tcur );
			if( joint != jointCursorList.end() ) {
				glColor3f(0.5, 0.5, 0.5);
			}
			glPushMatrix();
			glTranslatef(last_point.getScreenX(width), last_point.getScreenY(height), 0.0f);
			glBegin(GL_TRIANGLE_FAN);
			for(double a = 0.0f; a <= 2*M_PI; a += 0.2f) {
				glVertex2d(cos(a) * height/100.0f, sin(a) * height/100.0f);
			}
			glEnd();
			glPopMatrix();

			glColor3f(0.0, 0.0, 0.0);
			glRasterPos2f(tcur->getScreenX(width),tcur->getScreenY(height));
			sprintf(id,"%d",tcur->getCursorID());
			drawString(id);
		}
	}

	if (help) {
		glColor3f(0.0, 0.0, 1.0);
		glRasterPos2f(10,20);
		drawString("h - toggle help display");
		glRasterPos2f(10,35);
		if (verbose) drawString("v - disable verbose mode");
		else drawString("v - enable verbose mode");
		glRasterPos2f(10,50);
		drawString("r - reset session");
		glRasterPos2f(10,65);
		if (tuioServer->fullUpdateEnabled()) drawString("f - disable full update");
		else drawString("f - enable full update");
		glRasterPos2f(10,80);
		if (tuioServer->periodicMessagesEnabled()) drawString("p - disable periodic messages");
		else drawString("p - enable periodic messages");
		glRasterPos2f(10,95);
		drawString("SHIFT click - create persistent cursor");
		glRasterPos2f(10,110);
		drawString("CTRL click - add to cursor group");
		glRasterPos2f(10,125);
		if (fullscreen) drawString("F1 - exit fullscreen mode");
		else drawString("F1 - enter fullscreen mode");
		glRasterPos2f(10,140);
		drawString("ESC - Quit");
	}

	SDL_GL_SwapWindow(window);
}

void PavoDemo::drawString(const char *str) {
	if (str && strlen(str)) {
		while (*str) {
			glutBitmapCharacter(GLUT_BITMAP_8_BY_13, *str);
			str++;
		}
	}
}

void PavoDemo::processEvents() {
    SDL_Event event;
    while(pavoPollEvent(&event)) {
        switch( event.type ) {
		case SDL_KEYDOWN:
			if (event.key.repeat) continue;
			else if( event.key.keysym.sym == SDLK_ESCAPE ){
				running = false;
				SDL_Quit();
			} else if( event.key.keysym.sym == SDLK_F1 ){
				if(fullscreen)
				{
					width = window_width;
					height = window_height;
					SDL_SetWindowFullscreen(window, SDL_FALSE);
					fullscreen = false;
				}
				else
				{
					width = screen_width;
					height = screen_height;
					SDL_SetWindowFullscreen(window, SDL_WINDOW_FULLSCREEN_DESKTOP);
					fullscreen = true;
				}
				
				glClearColor(1.0f, 1.0f, 1.0f, 0.0f);
				glViewport(0, 0, (GLint)width, (GLint)height);
				glMatrixMode(GL_PROJECTION);
				glLoadIdentity();
				gluOrtho2D(0, (GLfloat)width, (GLfloat)height, 0);
				glMatrixMode(GL_MODELVIEW);
				glLoadIdentity();
			} else if( event.key.keysym.sym == SDLK_f ){
				fullupdate=!tuioServer->fullUpdateEnabled();
				if (fullupdate) tuioServer->enableFullUpdate();
				else tuioServer->disableFullUpdate();
			} else if( event.key.keysym.sym == SDLK_p ){
				periodic=!tuioServer->periodicMessagesEnabled();
				if (periodic) tuioServer->enablePeriodicMessages();
				else tuioServer->disablePeriodicMessages();
			} else if( event.key.keysym.sym == SDLK_v ){
				verbose = !verbose;
				tuioServer->setVerbose(verbose);
			} else if( event.key.keysym.sym == SDLK_h ){
				help = !help;
			} else if( event.key.keysym.sym == SDLK_r ){
				tuioServer->resetTuioCursors();
				stickyCursorList.clear();
				jointCursorList.clear();
				activeCursorList.clear();
			}
			break;

		case SDL_MOUSEMOTION:
			if (event.button.button == SDL_BUTTON_LEFT) mouseDragged((float)event.button.x/width, (float)event.button.y/height);
			break;
		case SDL_MOUSEBUTTONDOWN:
			if (event.button.button == SDL_BUTTON_LEFT) mousePressed((float)event.button.x/width, (float)event.button.y/height);
			break;
		case SDL_MOUSEBUTTONUP:
			if (event.button.button == SDL_BUTTON_LEFT) mouseReleased((float)event.button.x/width, (float)event.button.y/height);
			break;
		case SDL_QUIT:
			running = false;
			SDL_ShowCursor(true);
			SDL_Quit();
			break;
        }
    }
}

int PavoDemo::pavoPollEvent(SDL_Event *event) {
	static bool mouseButtonPressed = false; 
	static TuioPoint pressedPoint(0, 0);

	if (SDL_PollEvent(event)) {
		if (event->type != SDL_MOUSEBUTTONDOWN && 
			event->type != SDL_MOUSEBUTTONUP) {
			return 1;
		} 
	}

	int bufferSize = 8192;
	pavo_response_scan_t* buffer = new pavo_response_scan_t[bufferSize];
	memset(buffer, sizeof(pavo_response_scan_t)*bufferSize, 0);
	if (lidarDriver->get_scanned_data(buffer, bufferSize)) {
		delete[] buffer;
		return 0;
	}

	std::vector<TuioPoint> points;
	for (int i = 0; i < bufferSize; i++) {
		pavo_response_scan_t item = buffer[i];
		double radian = item.angle / 100.0 * 3.141592653 / 180;
		double r = item.distance * 2;
		double x = r * cos(radian) / 1000;
		double y = r * sin(radian) / 750;
		if (x <= 0 && x > -1 && y >= 0 && y < 1) {
			x = -x * width;
			y = y * height;
			points.push_back(TuioPoint(x, y));
		}
	}

	if (points.size() > 0) {
		float sumX = 0, sumY = 0;
		for (int i = 0, n = points.size(); i < n; i++) {
			sumX += points[i].getX();
			sumY += points[i].getY();
		}
		float centroidX = sumX / points.size();
		float centroidY = sumY / points.size();
		
		if (centroidX - pressedPoint.getX() > 1 && 
				centroidY - pressedPoint.getY() > 1 && 
				mouseButtonPressed) {
			mouseButtonPressed = false;
			event->type = SDL_MOUSEBUTTONUP;
			event->button.button = SDL_BUTTON_LEFT;
			event->button.x = (int) pressedPoint.getX();
			event->button.y = (int) pressedPoint.getY();
		} else if (!mouseButtonPressed) {
			pressedPoint.update(centroidX, centroidY);
			mouseButtonPressed = true;
			event->type = SDL_MOUSEBUTTONDOWN;
			event->button.button = SDL_BUTTON_LEFT;
			event->button.x = (int) pressedPoint.getX();
			event->button.y = (int) pressedPoint.getY();
		}
	} else if (mouseButtonPressed) {
		mouseButtonPressed = false;
		event->type = SDL_MOUSEBUTTONUP;
		event->button.button = SDL_BUTTON_LEFT;
		event->button.x = int(pressedPoint.getX());
		event->button.y = int(pressedPoint.getY());
	} else {
		delete[] buffer;
		return 0;
	}
	delete[] buffer;
	return 1;
}

void PavoDemo::mousePressed(float x, float y) {
	//printf("pressed %f %f\n",x,y);

	TuioCursor *match = NULL;
	float distance  = 0.01f;
	for (std::list<TuioCursor*>::iterator iter = stickyCursorList.begin(); iter!=stickyCursorList.end(); iter++) {
		TuioCursor *tcur = (*iter);
		float test = tcur->getDistance(x,y);
		if ((test < distance) && (test < 8.0f/width)) {
			distance = test;
			match = tcur;
		}
	}
	
	const Uint8 *keystate = SDL_GetKeyboardState(NULL);

	if ((keystate[SDL_SCANCODE_LSHIFT]) || (keystate[SDL_SCANCODE_RSHIFT]))  {

		if (match!=NULL) {
			std::list<TuioCursor*>::iterator joint = std::find( jointCursorList.begin(), jointCursorList.end(), match );
			if( joint != jointCursorList.end() ) {
				jointCursorList.erase( joint );
			}
			stickyCursorList.remove(match);
			activeCursorList.remove(match);
			tuioServer->removeTuioCursor(match);
		} else {
			TuioCursor *cursor = tuioServer->addTuioCursor(x,y);
			stickyCursorList.push_back(cursor);
			activeCursorList.push_back(cursor);
		}
	} else if ((keystate[SDL_SCANCODE_LCTRL]) || (keystate[SDL_SCANCODE_RCTRL])) {

		if (match!=NULL) {
			std::list<TuioCursor*>::iterator joint = std::find( jointCursorList.begin(), jointCursorList.end(), match );
			if( joint != jointCursorList.end() ) {
				jointCursorList.remove( match );
			} else jointCursorList.push_back(match);
		}
	} else {
		if (match==NULL) {
			TuioCursor *cursor = tuioServer->addTuioCursor(x,y);
			activeCursorList.push_back(cursor);
		} else activeCursorList.push_back(match);
	}
}

void PavoDemo::mouseDragged(float x, float y) {
	//printf("dragged %f %f\n",x,y);

	TuioCursor *cursor = NULL;
	float distance  = width;
	for (std::list<TuioCursor*>::iterator iter = activeCursorList.begin(); iter!=activeCursorList.end(); iter++) {
		TuioCursor *tcur = (*iter);
		float test = tcur->getDistance(x,y);
		if (test<distance) {
			distance = test;
			cursor = tcur;
		}
	}

	if (cursor==NULL) return;
	if (cursor->getTuioTime()==frameTime) return;

	std::list<TuioCursor*>::iterator joint = std::find( jointCursorList.begin(), jointCursorList.end(), cursor );
	if( joint != jointCursorList.end() ) {
		float dx = x-cursor->getX();
		float dy = y-cursor->getY();
		for (std::list<TuioCursor*>::iterator iter = jointCursorList.begin(); iter!=jointCursorList.end(); iter++) {
			TuioCursor *jointCursor = (*iter);
			 tuioServer->updateTuioCursor(jointCursor,jointCursor->getX()+dx,jointCursor->getY()+dy);
		}
	} else tuioServer->updateTuioCursor(cursor,x,y);
}

void PavoDemo::mouseReleased(float x, float y) {
	//printf("released %f %f\n",x,y);

	TuioCursor *cursor = NULL;
	float distance  = 0.01f;
	for (std::list<TuioCursor*>::iterator iter = stickyCursorList.begin(); iter!=stickyCursorList.end(); iter++) {
		TuioCursor *tcur = (*iter);
		float test = tcur->getDistance(x,y);
		if (test<distance) {
			distance = test;
			cursor = tcur;
		}
	}

	if (cursor!=NULL) {
		activeCursorList.remove(cursor);
		return;
	}

	distance = 0.01f;
	for (std::list<TuioCursor*>::iterator iter = activeCursorList.begin(); iter!=activeCursorList.end(); iter++) {
		TuioCursor *tcur = (*iter);
		float test = tcur->getDistance(x,y);
		if (test<distance) {
			distance = test;
			cursor = tcur;
		}
	}

	if (cursor!=NULL) {
		activeCursorList.remove(cursor);
		tuioServer->removeTuioCursor(cursor);
	}
}

PavoDemo::PavoDemo(TuioServer* sever)
	: verbose		(false)
	, fullupdate	(false)
	, periodic		(false)
	, fullscreen	(false)
	, help			(false)
	, screen_width	(1024)
	, screen_height	(768)
	, window_width	(640)
	, window_height	(480)
{
	if ( SDL_Init(SDL_INIT_VIDEO) < 0 ) {
		std::cerr << "SDL could not be initialized: " <<  SDL_GetError() << std::endl;
		SDL_Quit();
		exit(1);
	}
    
    TuioTime::initSession();
    frameTime = TuioTime::getSessionTime();

	initWindow();
	SDL_SetWindowTitle(window,"PavoDemo");

    lidarDriver = new pavo_driver("10.10.10.101", 2368, "10.10.10.100", 2368);
}

PavoDemo::~PavoDemo() {
	delete lidarDriver;
}

void PavoDemo::initWindow() {

	SDL_DisplayMode mode;
	SDL_GetCurrentDisplayMode(0, &mode);
	screen_width = mode.w;
	screen_height= mode.h;
	
	int videoFlags = SDL_WINDOW_OPENGL;
	if( fullscreen ) {
		videoFlags |= SDL_WINDOW_FULLSCREEN;
		width = screen_width;
		height = screen_height;
	} else {
		width = window_width;
		height = window_height;
	}
	
	SDL_CreateWindowAndRenderer(width, height, videoFlags, &window, &renderer);
	
	if ( window == NULL ) {
		std::cerr << "Could not open window: " << SDL_GetError() << std::endl;
		SDL_Quit();
	}

	SDL_GLContext context = SDL_GL_CreateContext(window);
	if (!context) {
		fprintf(stderr, "Couldn't create context: %s\n", SDL_GetError());
		return;
	}

	glClearColor(1.0f, 1.0f, 1.0f, 0.0f);
	glViewport(0, 0, width, height);
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	gluOrtho2D(0, (float)width,  (float)height, 0);
	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();
}

void PavoDemo::run() {
	running = true;
	while (running) {
		frameTime = TuioTime::getSessionTime();
		tuioServer->initFrame(frameTime);
		processEvents();
		tuioServer->stopUntouchedMovingCursors();
		tuioServer->commitFrame();
		drawFrame();

		// simulate 50Hz compensating the previous processing time
		int delay = 20 - (
			TuioTime::getSessionTime().getTotalMilliseconds() - 
			frameTime.getTotalMilliseconds());
		if (delay>0) SDL_Delay(delay);
	}
}

int main(int argc, char* argv[]) {
	TuioServer *server = NULL;
	if( argc == 3 ) {
		server = new TuioServer(argv[1],atoi(argv[2]));
	} else server = new TuioServer(); // default is UDP port 3333 on localhost

	PavoDemo *app = new PavoDemo(server);
	app->run();

	delete app;
	delete server;
	return 0;
}
