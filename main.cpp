
// DEPENDENCIES: "SDL2", "SDL2_GFX", "SDL2_TTF"
// TO COMPILE: g++ main.cpp -lSDL2 -lSDL2_gfx -lSDL2_ttf



#include <algorithm>
#include <math.h>
#include <iostream>
#include <memory>
#include <string>
#include <vector>
#include <SDL2/SDL.h>
// #include <SDL2/SDL_image.h>
#include <SDL2/SDL2_gfxPrimitives.h>
#include <SDL2/SDL_ttf.h>

// prototypes
void update();
void render();
void pushToHistory( int scancode );
void dupeCurrent();

SDL_Renderer* renderer;
SDL_Window* window;
TTF_Font* font;

// 0=black, 1=red, 2=yellow, 3=green, 4=cyan, 5=blue, 6=magenta, 7=white
Uint32 colors[] = {0x00202020, 0x0000007f, 0x00007f7f, 0x00007f00, 0x007f7f00, 0x007f0000, 0x007f007f, 0x007f7f7f};
char cci = 5; // current color index
#define LIGHTER_COLOR (c ? 0x00808080 : 0)

// transparent (ctrl+t), translucent (ctrl+g), opaque (ctrl+o)
Uint32 materials[] = {0x00000000, 0x7f000000, 0xff000000};
char cmi = 2;

/////////////////////////////
struct Shape {
	int x;
	int y;
	char c;
	char m;
	int t = 5;
	Shape();
	virtual void draw() = 0;
	virtual bool in(int cx, int cy) = 0;
};

Shape::Shape() {
	c = cci;
	m = cmi;
}

/////////////////////////////
struct Rect : Shape {
	int w;
	int h;
	Rect( int x1, int y1, int x2, int y2 );
	void draw();
	bool in(int cx, int cy);
};

Rect::Rect( int x1, int y1, int x2, int y2) {
	x = x1 > x2 ? x2 : x1;
	y = y1 > y2 ? y2 : y1;
	w = x1 - x + x2 - x;
	h = y1 - y + y2 - y;
}

void Rect::draw() {
	if (m != 1) {
		boxColor(renderer, x, y, x+w-t, y+t, colors[c] | materials[2]);
		boxColor(renderer, x, y+t, x+t, y+h, colors[c] | materials[2]);
		boxColor(renderer, x+w-t, y, x+w, y+h-t, colors[c] | materials[2]);
		boxColor(renderer, x+t, y+h-t, x+w, y+h, colors[c] | materials[2]);
	}
	boxColor(renderer, x+t, y+t, x+w-t, y+h-t, colors[c] | LIGHTER_COLOR | materials[m]);
}

bool Rect::in(int cx, int cy) {
	if ( cy >= y && cy <= y+t && cx >= x && cx <= x+w ) return true;
	if ( cx >= x && cx <= x+t && cy >= y && cy <= y+h ) return true;
	if ( cx >= x+w-t && cx <= x+w && cy >= y && cy <= y+h ) return true;
	if ( cy >= y+h-t && cy <= y+h && cx >= x && cx <= x+w ) return true;
	return false;
}




/////////////////////////////
struct Ellipse : Shape  {
	int rx;
	int ry;
	Ellipse(int x1, int y1, int x2, int y2);
	void draw();
	bool in(int cx, int cy);
};

Ellipse::Ellipse(int x1, int y1, int x2, int y2) {
	x = x1;
	y = y1;
	rx = abs(x2-x1);
	ry = abs(y2-y1);
	if (10 * rx < ry) rx = ry;
	else if (10 * ry < rx) ry = rx;
	else {
		rx *= 1.414;
		ry *= 1.414;
	}
}

void Ellipse::draw() {
	if (m != 1) {
		for (int i = 0; i<t; i++) {
			ellipseColor(renderer, x, y, rx-i, ry-i, colors[c] | materials[2]);
		}
	}
	filledEllipseColor(renderer, x, y, rx-t, ry-t, colors[c] | LIGHTER_COLOR | materials[m] );
}

bool Ellipse::in(int cx, int cy) {
	bool greaterThanMin = hypot((cx-x) * 1.0 / (rx-t), (cy-y) * 1.0 / (ry-t) ) >= 1;
	bool lessThanMax = hypot((cx-x) * 1.0 / (rx), (cy-y) * 1.0 / (ry) ) <= 1;
	return greaterThanMin && lessThanMax;
}


/////////////////////////////
struct Arrow : Shape  {
	int dx;
	int dy;
	Arrow( int x1, int y1, int x2, int y2);
	void draw();
	bool in(int cx, int cy);
};

Arrow::Arrow( int x1, int y1, int x2, int y2) {
	x = x1;
	y = y1;
	dx = x2-x1;
	dy = y2-y1;
	int rx = abs(dx);
	int ry = abs(dy);
	if (10 * rx < ry) dx = 0;
	else if (10 * ry < rx) dy = 0;
}

void Arrow::draw() {
	if (!dx && !dy) return;

	thickLineColor(renderer, x, y, x+dx, y+dy, t, colors[c] | LIGHTER_COLOR | materials[2]);
	if (m > 0) {
		double len = hypot(dx, dy);
		double ux = dx/len;
		double uy = dy/len;

		int v1x = x+dx - 20*ux - 10*uy;
		int v1y = y+dy - 20*uy + 10*ux;
		int v2x = x+dx - 20*ux + 10*uy;
		int v2y = y+dy - 20*uy - 10*ux;

		thickLineColor(renderer, x+dx, y+dy, v1x, v1y, t, colors[c] | LIGHTER_COLOR | materials[2]);
		thickLineColor(renderer, x+dx, y+dy, v2x, v2y, t, colors[c] | LIGHTER_COLOR | materials[2]);
	}
}

bool Arrow::in(int cx, int cy) {
	int nom = abs(dy*cx - dx*cy + (x+dx)*y - (y+dy)*x );
	double dist = nom / hypot(dy, dx);
	if (dist > 2*t) return false;
	int distCA = hypot(x-cx, y-cy );
	int distCB = hypot(x+dx-cx, y+dy-cy);
	int distAB = hypot(dx, dy);
	return distCA < distAB && distCB < distAB;
}


/////////////////////////////
struct Grid : Shape  {
	int w;
	int h;
	Grid(int x1, int y1, int x2, int y2);
	void draw();
	bool in(int cx, int cy);
};

Grid::Grid(int x1, int y1, int x2, int y2) {
	x = x1 > x2 ? x2 : x1;
	y = y1 > y2 ? y2 : y1;
	w = (x2-x + x1-x) / 100 * 100;
	h = (y2-y + y1-y) / 60 * 60;
}

void Grid::draw() {
	boxColor(renderer, x,y, x+w, y+h, colors[c] | LIGHTER_COLOR | materials[m]);

	for (int col = x; col<x+w+t; col += 100) {
		for (int i = 0; i<t; i++) {
			vlineColor(renderer, col+i, y, y+h+t, colors[c] | materials[2]);
		}
	}
	for (int row = y; row<y+h+t; row += 60) {
		for (int i = 0; i<t; i++) {
			hlineColor(renderer, x, x+w+t, row+i, colors[c] | materials[2]);
		}
	}
}
	

bool Grid::in(int cx, int cy) {
	int xal = cx-x;
	int yal = cy-y;
	if (xal < 0 || xal > w+t || yal < 0 || yal > h+t) return false;
	if (xal % 100 < t || yal % 60 < t) return true;
	return false;
}

/////////////////////////////
struct Text : Shape  {
	std::string t;
	int w, h;
	Text(char init, int x1, int y1);
	void add(char *c);
	void backspace();
	void draw();
	bool in(int cx, int cy);
};

Text::Text(char init, int x1, int y1) {
	t = init;
	TTF_SizeUTF8(font, t.c_str(), &w, &h);
	x = x1-15;
	y = y1-h+10;
}

void Text::add( char *c) {
	t += c;
	TTF_SizeUTF8(font, t.c_str(), &w, &h);
}

void Text::backspace( ) {
	t.pop_back();
	TTF_SizeUTF8(font, t.c_str(), &w, &h);
}

void Text::draw() {
	SDL_Color clr; 
	clr.r = ((colors[c] | LIGHTER_COLOR) & 0xFF);
	clr.g = ((colors[c] | LIGHTER_COLOR) & 0xFF00) >> 8;
	clr.b = ((colors[c] | LIGHTER_COLOR) & 0xFF0000) >> 16;
	clr.a = 0xFF;

	SDL_Rect dst;
	dst.x = x;
	dst.y = y;
	dst.w = w;
	dst.h = h;

	SDL_Surface* s = TTF_RenderUTF8_Solid(font, t.c_str(), clr);
	SDL_Texture* t = SDL_CreateTextureFromSurface(renderer, s);
	SDL_RenderCopy(renderer, t, NULL, &dst);
	SDL_FreeSurface(s);
}

bool Text::in(int cx, int cy) {
	if (cx > x && cx < x+w && cy > y && cy < y+h) return true;
	return false;
}



/////////////////////////////
// struct Image : Shape  {
// 	int w;
// 	int h;
// 	SDL_Texture* i;
// Image(int x1, int y1, int x2, int y2);
	// void draw();
	// bool in(int cx, int cy);
// };


/////////////////////////////
std::vector< std::vector< std::shared_ptr<Shape> > > h(100);
int hi = 0; // current entry of h
int hl = 0; // last entry of h
char rerender_requested = 1;
char preview_requested = 0;
char preview_shape;




////////////////////////////
int main() {

	
	SDL_Init( SDL_INIT_AUDIO | SDL_INIT_EVENTS | SDL_INIT_VIDEO | SDL_INIT_TIMER );

	window = SDL_CreateWindow("Paint", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED,
	800, 600, SDL_WINDOW_RESIZABLE | SDL_WINDOW_MAXIMIZED);

	// window = SDL_CreateWindow("Paint", 0, 0,
	// 200, 600, SDL_WINDOW_RESIZABLE);

	renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);
	
	// IMG_Init( IMG_INIT_PNG | IMG_INIT_JPG );
	TTF_Init();

	font = TTF_OpenFont("NotoSans-Bold.ttf", 40);

	SDL_StartTextInput();

	while(1) {
		update();
		if (rerender_requested) render();
		SDL_Delay(1000/30);
	}

	return 0;
}


//////////////////////
struct MouseStatus {
	int x, y, px, py;
	char typing, deleting;
};

struct MouseStatus ms = {0,0,0,0,0,0};



///////////////////////////
void render() {

	SDL_SetRenderDrawColor(renderer, 32, 32, 32, SDL_ALPHA_OPAQUE);
	SDL_RenderClear(renderer);

	for ( auto& ptr : h[hi]) {
		ptr->draw();
	}

	if (preview_requested) {
		Shape *s = NULL;

		switch (preview_shape) {
			case SDL_SCANCODE_R:
				s = new Rect(ms.px, ms.py, ms.x, ms.y);
			break;
			case SDL_SCANCODE_E:
				s = new Ellipse(ms.px, ms.py, ms.x, ms.y);
			break;
			case SDL_SCANCODE_A:
				s = new Arrow(ms.px, ms.py, ms.x, ms.y);
			break;
			case SDL_SCANCODE_G:
				s = new Grid(ms.px, ms.py, ms.x, ms.y);
			break;
		}

		if (s) {
			s->draw();
			delete s;
		}
	}

	SDL_RenderPresent(renderer);
	rerender_requested = 0;
}


//////////////////////////////////
void update() {

	SDL_Event event;
	int saveMousePos = 0;

	while(SDL_PollEvent(&event)) {

		switch (event.type) {
			case SDL_WINDOWEVENT:
				rerender_requested = 1;
			break;

			case SDL_QUIT:
				// IMG_Quit();
				TTF_Quit();
				SDL_DestroyRenderer(renderer);
				SDL_DestroyWindow(window);
				SDL_Quit();
				exit(0);
			break;

			case SDL_MOUSEMOTION:
				ms.x = event.motion.x;
				ms.y = event.motion.y;
				ms.typing = 0;
				if (ms.deleting) {

					char foundSmthToDelete = 0;

					for (auto& ptr : h[hi]) {
						if (ptr->in(ms.x, ms.y)) {
							ptr = nullptr;
							foundSmthToDelete = 1;
						}
					}
					if (foundSmthToDelete) {
						auto newend = std::remove(h[hi].begin(), h[hi].end(), nullptr);
						h[hi].erase(newend, h[hi].end() );
						rerender_requested = 1;
					}
				}

				if (preview_requested) {
					rerender_requested = 1;
				}
			break;
				
			
			case SDL_KEYDOWN:
				if (event.key.repeat) break;
				if (!ms.typing && event.key.keysym.scancode == SDL_SCANCODE_D) {
					ms.deleting = 1;
					dupeCurrent();
				}
				saveMousePos = 1;
				if (!ms.typing) {
					preview_requested = 1;
					preview_shape = event.key.keysym.scancode;
				}
			break;
			
			case SDL_KEYUP:
				preview_requested = 0;
				
				if (event.key.keysym.scancode == SDL_SCANCODE_Z && event.key.keysym.mod & KMOD_CTRL) {
					if (hi != 0) hi--;
					rerender_requested = 1;
					ms.typing = 0;
				}
				else if (event.key.keysym.scancode == SDL_SCANCODE_Y && event.key.keysym.mod & KMOD_CTRL) {
					if (hi < hl) hi++;
					rerender_requested = 1;
					ms.typing = 0;
				}
				else if (event.key.keysym.scancode == SDL_SCANCODE_V && event.key.keysym.mod & KMOD_CTRL) {
					char* cbd = SDL_GetClipboardText();
					if (cbd && *cbd) {
						if (ms.typing) {
							Text* t = (Text*) (h[hi].back().get());
							t->add(cbd);
						}
						else {
							ms.typing = 1;
							dupeCurrent();
							h[hi].push_back(std::make_shared<Text>(' ', ms.x, ms.y));
							Text* t = (Text*) (h[hi].back().get());
							t->add(cbd);
						}
						rerender_requested = 1;
						SDL_free(cbd);
					}
				}
				

				else if (ms.typing && event.key.keysym.sym == SDLK_BACKSPACE) {
					Text* t = (Text*) (h[hi].back().get());
					t->backspace();
					rerender_requested = 1;	
				}
				else if (event.key.keysym.scancode == SDL_SCANCODE_DELETE) {
					dupeCurrent();
					for ( auto& ptr : h[hi] ) {
						ptr.reset();
					}
					h[hi].clear();
					rerender_requested = 1;	
				}

				else if (!ms.typing && event.key.keysym.scancode == SDL_SCANCODE_GRAVE) {
					int s = h[hi].size();
					if (s) cmi = h[hi][s-1]->m = (h[hi][s-1]->m + 1) % 3;
					else cmi = (cmi + 1) % 3;
					rerender_requested = 1;
				}
				else if (!ms.typing && event.key.keysym.sym >= SDLK_0 && event.key.keysym.sym <= SDLK_7) {
					int s = h[hi].size();
					if (s) cci = h[hi][s-1]->c = event.key.keysym.sym - SDLK_0;
					else cci = event.key.keysym.sym - SDLK_0;
					rerender_requested = 1;
				}

				else if (ms.px == ms.x && ms.py == ms.y) {
					if (ms.typing == 0 && event.key.keysym.sym <= SDLK_z) {
						ms.typing = 1;
						dupeCurrent();
						h[hi].push_back(std::make_shared<Text>(event.key.keysym.sym, ms.x, ms.y));
					}

					rerender_requested = 1;

				}
				else {
					pushToHistory(event.key.keysym.scancode);
					rerender_requested = 1;
				}

				if (ms.deleting) {
					ms.deleting = 0;
				}
			break;


			case SDL_TEXTINPUT:
				if (ms.typing) {
					Text* t = (Text*) (h[hi].back().get());
					t->add(event.text.text);
					rerender_requested = 1;
				}
			break;	
		}
	}

	if (saveMousePos) {
		ms.px = ms.x;
		ms.py = ms.y;
	}


}



///////////////////////////////////////
void pushToHistory( int scancode ) {
	switch (scancode) {
		case SDL_SCANCODE_R:
			dupeCurrent();
			h[hi].push_back( std::make_shared<Rect>(ms.px, ms.py, ms.x, ms.y) );
		break;
		case SDL_SCANCODE_E:
			dupeCurrent();
			h[hi].push_back( std::make_shared<Ellipse>(ms.px, ms.py, ms.x, ms.y) );
		break;
		case SDL_SCANCODE_A:
			dupeCurrent();
			h[hi].push_back( std::make_shared<Arrow>(ms.px, ms.py, ms.x, ms.y) );
		break;
		case SDL_SCANCODE_G:
			dupeCurrent();
			h[hi].push_back( std::make_shared<Grid>(ms.px, ms.py, ms.x, ms.y) );
		break;
	}
}


////////////////////////////////
void dupeCurrent() {

	// boundary condition
	if (hi == 99) {
		for (int i = 0; i<50; i++) {
			// release shared ptrs
			for ( auto& ptr : h[i] ) {
				ptr.reset();
			}
			// clear
			h[i].clear();
			// copy
			for ( auto& ptr : h[50+i] ) {
				h[i].push_back(ptr);
				ptr.reset();
			}
			h[50+i].clear();
		}
		hi -= 50;
		hl -= 50;
	}

	// push after ctrl+z
	if (hi < hl) {
		for (int i = hi+1; i <= hl; i++) {
			// release shared ptrs
			for ( auto& ptr : h[i] ) {
				ptr.reset();
			}
			// clear
			h[i].clear();
		}
		hl = hi;
	}


	for ( auto& ptr : h[hi]) {
		h[hi+1].push_back(ptr);
	}
	hi++; hl++;
}