
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>
#include <SDL2/SDL2_gfxPrimitives.h>
#include <SDL2/SDL_ttf.h>


SDL_Renderer* renderer;
SDL_Window* window;

enum ShapeKind { SHAPE_RECT, SHAPE_CIRCLE, SHAPE_LINE, SHAPE_GRID, SHAPE_TEXT, SHAPE_ARROW, SHAPE_IMAGE };

struct Rect {
	int x;
	int y;
	int w;
	int h;
};

struct Circle {
	int x;
	int y;
	int r;
};

struct Line {
	int fx;
	int fy;
	int tx;
	int ty;
};

struct Grid {
	int x;
	int y;
	int w;
	int h;
};

struct Text {
	int x;
	int y;
	char* t;
};

struct Arrow {
	int fx;
	int fy;
	int tx;
	int ty;
};

struct Image {
	int x;
	int y;
	int w;
	int h;
	SDL_Texture* i;
};


struct Shape {
	enum ShapeKind kind;
	union {
		struct Rect r;
		struct Circle c;
		struct Line l;
		struct Grid g;
		struct Text t;
		struct Arrow a;
		struct Image i;
	};
};


struct Shape** h;
int hi = 0; // history index: first empty slot
int* hlens;
char rerender_requested = 1;

void update();
void render();


////////////////////////////
int main() {

	h = calloc( sizeof(void*), 100);
	hlens = calloc( sizeof(int), 100);

	SDL_Init( SDL_INIT_AUDIO | SDL_INIT_EVENTS | SDL_INIT_VIDEO | SDL_INIT_TIMER );

	// window = SDL_CreateWindow("Paint", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED,
	// 800, 600, SDL_WINDOW_RESIZABLE | SDL_WINDOW_MAXIMIZED);

	window = SDL_CreateWindow("Paint", 0, 0,
	100, 600, SDL_WINDOW_RESIZABLE);

	renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);

	// IMG_Init( IMG_INIT_PNG | IMG_INIT_JPG );

	while(1) {
		update();
		if (rerender_requested) render();
		SDL_Delay(1000/30);
	}

	return 0;
}



///////////////////////////
void render() {

	
	// SDL_RenderCopy(renderer, texture, &grass_src, &grass_dest);
	// SDL_RenderCopyEx(renderer, texture, &tank_src, &tank_dest, tank_th, NULL, SDL_FLIP_NONE);
	SDL_RenderPresent(renderer);
}


struct KeyStatus {
	// char pressed;
	// uint16_t keycode;
	// uint16_t scancode;
	// uint32_t tstart;
	int mx, my;
};

struct KeyStatus ks;
int mx, my;



//////////////////////////////////
void update() {

	SDL_Event event;
	int saveMousePos = 0;

	while(SDL_PollEvent(&event)) {

		switch (event.type) {
			case SDL_QUIT:
				// IMG_Quit();
				SDL_DestroyRenderer(renderer);
				SDL_DestroyWindow(window);
				SDL_Quit();
				exit(0);
			break;

			case SDL_MOUSEMOTION:
				mx = event.motion.x;
				my = event.motion.y;
			break;
				
			
			case SDL_KEYDOWN:
				if (event.key.repeat) break;
				// ks.pressed = 1;
				// ks.keycode = event.key.keysym.sym;
				// ks.scancode = event.key.keysym.scancode;
				// ks.tstart = event.key.timestamp;
				saveMousePos = 1;			
			break;
			
			case SDL_KEYUP:
				// ks.pressed = 0;
				if (ks.mx == mx && ks.my == my) {
					printf("%c", event.key.keysym.sym);
					fflush(stdout);
				}
				else {
					printf("\n'%c' (%d, %d) -> (%d, %d)\n", event.key.keysym.sym, ks.mx, ks.my, mx, my);
					pushToHistory(event.key.keysym.scancode);
				}

			break;
		}
	}

	if (saveMousePos) {
		ks.mx = mx;
		ks.my = my;
	}


}



///////////////////////////////////////
void pushToHistory( int scancode ) {
	switch (scancode) {
		case SDL_SCANCODE_R:
			duplicateLastHistoryEntry();
			struct Shape r;
			r.kind = SHAPE_RECT;
			r.r.x = ks.mx > mx ? mx : ks.mx;
			r.r.y = ks.my > my ? my : ks.my;
			r.r.w = ks.mx - r.r.x + mx - r.r.x;
			r.r.h = ks.my - r.r.y + my - r.r.y;
			// if (hlens[hi] )
		break;
	}
}




//////////////////////////////////////
void duplicateLastHistoryEntry() {
	if ( hi != 0) { // if hi = 0, dont duplicate
		memcpy( h[hi], h[hi-1], hlens[hi-1]);
		hlens[hi] = hlens[hi-1];
	}
}