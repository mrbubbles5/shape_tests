#include "stdio.h"
#include "stdlib.h"
#include "stdbool.h"
#include "stdint.h"
#include "errno.h"
#include "math.h"

#include <X11/Xlib.h>
#include <X11/extensions/XShm.h>

#define WIDTH 800
#define HEIGHT 600
#define MAG 50


typedef struct Pos {
  int x,y;
} Pos;

Pos move = {
  .x=0,
  .y=0,
};
Pos gravitation = {
  .x=0,
  .y=1,
};

typedef struct Ball {
  Pos pos;
  int mag;
} Ball;
Ball ball = {
  .pos = {.x = WIDTH/2,
		  .y = HEIGHT/2},
  .mag = MAG
};

Ball ball_l = {
  .pos = {  .x = 0,
			.y = 0},
  .mag = MAG,
};

Ball ball_r = {
  .pos = {.x = WIDTH-1,
		  .y = 0},
  .mag = MAG,
};


typedef struct Rect {
  Pos pos;
  float angle;
  int dx;
  int dy;
} Rect;

Rect screen_border = {
  .pos = {.x = WIDTH/2, .y = HEIGHT/2},
  .dx = (WIDTH-1)/2,
  .dy = (HEIGHT-1)/2,
};

Rect rect_l, rect_r, rect_c;

typedef struct Triangle {
  Pos pos, p1, p2, p3;
} Triangle;

Triangle tri_a;


bool keymap[256] = {0};

typedef struct Color {
  uint8_t a,r,g,b;
} Color;

#define WHITE (Color) {.a	= 255, .r = 255, .g = 255, .b = 255}
#define BLACK (Color) {.a	= 0, .r = 0, .g = 0, .b = 0}
#define RED (Color) {.a		= 255, .r = 255, .g = 0, .b = 0}
#define GREEN (Color) {.a	= 255, .r = 0, .g = 255, .b = 0}
#define BLUE (Color) {.a	= 255, .r = 0, .g = 0, .b = 255}

#define CToUi32(color) (uint32_t) color.a<<24 | color.r << 16 | color.g << 8 | color.b 


int draw_ball(uint32_t *pixels, Ball ball, float bt, Color color) {
  int lx = ball.pos.x - ball.mag;
  int rx = ball.pos.x + ball.mag;
  int ly = ball.pos.y - ball.mag;
  int ry = ball.pos.y + ball.mag;

  lx = lx < 0 ? 0 : lx;
  rx = rx >= WIDTH ? WIDTH - 1 : rx;
  ly = ly < 0 ? 0 : ly;
  ry = ry >= HEIGHT ? HEIGHT - 1 : ry;
  
  for (int i =lx; i<=rx; i++) {
	for (int j=ly; j<=ry; j++) {
	  float dist = sqrt(pow((float)i-ball.pos.x,2) + pow((float)j-ball.pos.y,2));

	  if (ball.mag-bt < dist && dist <= ball.mag) pixels[j*WIDTH + i] = CToUi32(color);
	}
  }

  return 0;
}

int draw_ball_filled(uint32_t *pixels, Ball ball, Color color) {
  int lx = ball.pos.x - ball.mag;
  int rx = ball.pos.x + ball.mag;
  int ly = ball.pos.y - ball.mag;
  int ry = ball.pos.y + ball.mag;

  lx = lx < 0 ? 0 : lx;
  rx = rx >= WIDTH ? WIDTH - 1 : rx;
  ly = ly < 0 ? 0 : ly;
  ry = ry >= HEIGHT ? HEIGHT - 1 : ry;
  
  for (int i =lx; i<=rx; i++) {
	for (int j=ly; j<=ry; j++) {
	  float dist = sqrt((float) (i-ball.pos.x)*(i-ball.pos.x) + (float) (j-ball.pos.y)*(j-ball.pos.y));

	  if (dist <= ball.mag) pixels[j*WIDTH + i] = CToUi32(color);
	}
  }

  return 0;
}

// NOTE: b would be more accurate by using a norm. Maybe later
int draw_rect_filled(uint32_t *pixels, Rect rect, Color color) {
  float rx = (cos(-rect.angle) - sin(-rect.angle)) * .5;
  float ry = (sin(-rect.angle) + cos(-rect.angle)) * .5;

  float c = cos(-rect.angle);
  float s = sin(-rect.angle);

  int b = (int) (rect.dx <= rect.dy ? rect.dy * 1.5 : rect.dx * 1.5);

  // TODO: Optimize like the triangle
  
  for (int i = -b; i<b; i++) {
	float _t_i = c * i;
	float _t_j = s * i;
	
	for (int j = -b; j<b; j++) {
	  int t_i = (int) floor(_t_i - s * j + rx);
	  int t_j = (int) floor(_t_j + c * j + ry);

	  if (-rect.dx <= t_i && t_i < rect.dx &&
		  -rect.dy <= t_j && t_j < rect.dy) {
		int di = i + rect.pos.x;
		int dj = j + rect.pos.y;
	  
		if (0<=di && di<WIDTH && 0<=dj && dj<HEIGHT) {
		  pixels[dj*WIDTH + di] = CToUi32(color);
		}
	  }
	}
  }

  return 0;
}

// NOTE: Order of points is assumed to be clockwise.
int draw_tria(uint32_t *pixels, Triangle t, Color color) {
  int tp1x = t.pos.x + t.p1.x;
  int tp1y = t.pos.y + t.p1.y;
  int tp2x = t.pos.x + t.p2.x;
  int tp2y = t.pos.y + t.p2.y;
  int tp3x = t.pos.x + t.p3.x;
  int tp3y = t.pos.y + t.p3.y;
  
  int xl =tp1x <= tp2x && tp1x <= tp3x ? tp1x : ( tp2x <= tp3x ? tp2x : tp3x);
  int xr =tp1x >= tp2x && tp1x >= tp3x ? tp1x : ( tp2x >= tp3x ? tp2x : tp3x);
  int yl =tp1y <= tp2y && tp1y <= tp3y ? tp1y : ( tp2y <= tp3y ? tp2y : tp3y);
  int yr =tp1y >= tp2y && tp1y >= tp3y ? tp1y : ( tp2y >= tp3y ? tp2y : tp3y);

  xl = 0<=xl ? xl : 0;
  xr = xr < WIDTH ? xr : WIDTH;
  yl = 0<=yl ? yl : 0;
  yr = yr < HEIGHT ? yr : HEIGHT;

  int dp1p2x = tp2x - tp1x;
  int dp1p2y = tp2y - tp1y;
  int dp2p3x = tp3x - tp2x;
  int dp2p3y = tp3y - tp2y;
  int dp3p1x = tp1x - tp3x;
  int dp3p1y = tp1y - tp3y;

  int stp1x = dp1p2y * tp1x;
  int stp1y = dp1p2x * tp1y;
  int stp2x = dp2p3y * tp2x;
  int stp2y = dp2p3x * tp2y;
  int stp3x = dp3p1y * tp3x;
  int stp3y = dp3p1x * tp3y;

  int C1 = stp1x - stp1y;
  int C2 = stp2x - stp2y;
  int C3 = stp3x - stp3y;

  int sstp1 = yl * dp1p2x - xl * dp1p2y + C1;
  int sstp2 = yl * dp2p3x - xl * dp2p3y + C2;
  int sstp3 = yl * dp3p1x - xl * dp3p1y + C3;

  for (int j = yl; j<yr; j++) {
  	int i_sstp1 = sstp1;
	int i_sstp2 = sstp2;
	int i_sstp3 = sstp3;
	for (int i = xl; i<xr; i++) {
	  if (i_sstp1 > 0 &&
		  i_sstp2 > 0 &&
		  i_sstp3 > 0) {
		  pixels[j*WIDTH + i] = CToUi32(color);
	  }
	  i_sstp1 -= dp1p2y;
	  i_sstp2 -= dp2p3y;
	  i_sstp3 -= dp3p1y;
	}
	sstp1 += dp1p2x;
	sstp2 += dp2p3x;
	sstp3 += dp3p1x;
  }

  return 0;
}


int draw_rect(uint32_t *pixels, Rect rect, float bt, Color color) {
  int lx = rect.pos.x - rect.dx;
  int rx = rect.pos.x + rect.dx;
  int ly = rect.pos.y - rect.dy;
  int ry = rect.pos.y + rect.dy;

  lx = lx < 0 ? 0 : lx;
  rx = rx >= WIDTH ? WIDTH - 1 : rx;
  ly = ly < 0 ? 0 : ly;
  ry = ry >= HEIGHT ? HEIGHT - 1 : ry;

  for (int i = lx; i<=rx; i++) {
	for (int j = ly; j<=ry; j++) {
	  pixels[j*WIDTH + i] = CToUi32(color);
	}
  }

  return 0;
}

int update_move() {
  if (keymap['w']) move.y -= 2;
  if (keymap['a']) move.x -= 1;
  if (keymap['s']) move.y += 1;
  if (keymap['d']) move.x += 1;

  return 0;
}


typedef enum BLOCKED_DIR {
  NONE=0,
  UP= 2,
  DOWN = 4,
  LEFT = 8,
  RIGHT = 16,
  X = 32,
  Y = 64,
} BLOCKED_DIR;

// Collision at the border
BLOCKED_DIR rect_collision(Rect rect, Ball ball, bool outer) {
  BLOCKED_DIR mask = NONE;
  
  if (outer) {
	if (rect.pos.x - rect.dx <= ball.pos.x &&  ball.pos.x <= rect.pos.x + rect.dx){
	  if (rect.pos.y - rect.dy <= ball.pos.y + ball.mag && rect.pos.y + rect.dy >= ball.pos.y - ball.mag) {
		mask = mask | Y;
	  }
	}
	
	if (rect.pos.y - rect.dy <= ball.pos.y &&  ball.pos.y <= rect.pos.y + rect.dy){
	  if (rect.pos.x - rect.dx <= ball.pos.x + ball.mag && rect.pos.x + rect.dx >= ball.pos.x - ball.mag) {
		mask = mask | X;
	  }
	}
  } else {
	if (rect.pos.x - rect.dx <= ball.pos.x &&  ball.pos.x <= rect.pos.x + rect.dx){
	  if (rect.pos.y - rect.dy >= ball.pos.y - ball.mag || rect.pos.y + rect.dy <= ball.pos.y + ball.mag) {
		mask = mask | Y;
	  }
	}
	
	if (rect.pos.y - rect.dy <= ball.pos.y &&  ball.pos.y <= rect.pos.y + rect.dy){
	  if (rect.pos.x - rect.dx >= ball.pos.x - ball.mag || rect.pos.x + rect.dx <= ball.pos.x + ball.mag) {
		mask = mask | X;
	  } 
	}
  }
  
  return mask;
}
 
Ball update_position(Ball ball, Pos move) {
  int dy = ball.pos.y + gravitation.y + move.y;
  int dx = ball.pos.x + move.x;

  Ball vBall = {
	.pos = {dx,dy},
	.mag = ball.mag,
  };

  BLOCKED_DIR dir = rect_collision(screen_border, vBall, false);

  if (dir==NONE) {
	ball.pos.x = dx;
	ball.pos.y = dy;
  } else if (dir==X) {
	ball.pos.y = dy;
  } else if (dir==Y) {
	ball.pos.x = dx;
  } 

  return ball;
}


int init_scene(uint32_t *pixels) {
  draw_ball_filled(pixels, ball, RED);

  rect_l = (Rect) {
	.pos = {.x=0, .y=HEIGHT-1},
	.dx = 100,
	.dy = 100};
  rect_r = (Rect) {
	.pos = {.x=WIDTH-1, .y=HEIGHT-1},
	.dx = 100,
	.dy = 100};

  rect_c = (Rect) { 
	.pos = {.x=WIDTH/2, .y=HEIGHT/2},
	.angle = 0.125*M_PI,
	.dx = 100,
	.dy = 100};

  tri_a = (Triangle) {
	.pos = {.x=WIDTH/2, .y=HEIGHT/2},
	.p1 = {.x = -100, .y = 0},
	.p2 = {.x = 100, .y = 0},
	.p3 = {.x = 0, .y = 100},
  };
  
  return 0;
}

int clear_scene(uint32_t *pixels, Color color) {
  for (int i =0; i<WIDTH; i++) {
	for (int j=0; j<HEIGHT; j++) {
	  pixels[j*WIDTH + i] = CToUi32(color);
	}
  }
  
  return 0;
}

int update_scene(uint32_t *pixels) {  
  update_move();
  ball = update_position(ball, move);
  
  draw_ball(pixels, ball, 1.,  RED);
  draw_ball_filled(pixels, ball_l, BLUE);
  draw_ball_filled(pixels, ball_r, GREEN);

  if (rect_collision(rect_l, ball, true)) {
	draw_rect_filled(pixels, rect_l, RED);
  } else {
	draw_rect_filled(pixels, rect_l, WHITE);
  }
  if (rect_collision(rect_r, ball, true)) draw_rect_filled(pixels, rect_r, RED);
  else draw_rect_filled(pixels, rect_r, WHITE);
  
  move.x = 0;
  move.y = 0;

  
  //draw_rect_filled(pixels, rect_c, RED);
  rect_c.angle += .01;

  draw_tria(pixels, tri_a, RED);
  tri_a.p1.x = (int) ((tri_a.p1.x+0.5) * cos(.001) - (tri_a.p1.x+ 0.5) * sin(.001));
  tri_a.p1.y = (int) ((tri_a.p1.x+0.5) * sin(.001) + (tri_a.p1.y+ 0.5) * cos(.001));
  tri_a.p2.x = (int) ((tri_a.p2.x+0.5) * cos(.001) - (tri_a.p2.x+ 0.5) * sin(.001));
  tri_a.p2.y = (int) ((tri_a.p2.x+0.5) * sin(.001) + (tri_a.p2.y+ 0.5) * cos(.001));
  tri_a.p3.x = (int) ((tri_a.p3.x+0.5) * cos(.001) - (tri_a.p3.x+ 0.5) * sin(.001));
  tri_a.p3.y = (int) ((tri_a.p3.x+0.5) * sin(.001) + (tri_a.p3.y+ 0.5) * cos(.001));
	

  
  return 0;
}


int main(void)
{
    Display *display = XOpenDisplay(NULL);
    if (display == NULL) {
        fprintf(stderr, "ERROR: could not open the default display\n");
        exit(1);
    }

    Bool	mit_shm_available = XShmQueryExtension(display);
    if (!mit_shm_available) {
        fprintf(stderr, "WARNING: could not find MIT-SHM extension\n");
    } else {
        fprintf(stderr, "INFO: detected MIT-SHM extension\n");
    }
	
    Window window = XCreateSimpleWindow(
                        display,
                        XDefaultRootWindow(display),
                        0, 0,
                        WIDTH, HEIGHT,
                        0,
                        0,
                        0);

    XWindowAttributes	wa = {0};
    XGetWindowAttributes(display, window, &wa);

    XImage			*image;
    XShmSegmentInfo	 shminfo = {0};

	uint32_t	pixels[WIDTH*HEIGHT];
	
	image = XCreateImage(display,
						 wa.visual,
						 wa.depth,
						 ZPixmap,
						 0,
						 (char*) &pixels,
						 WIDTH,
						 HEIGHT,
						 32,
						 WIDTH *	sizeof(uint32_t));


    GC	gc = XCreateGC(display, window, 0, NULL);

    Atom	wm_delete_window = XInternAtom(display, "WM_DELETE_WINDOW", False);
    XSetWMProtocols(display, window, &wm_delete_window, 1);

    XSelectInput(display, window, KeyPressMask | KeyReleaseMask | PointerMotionMask);

    XMapWindow(display, window);

    float	global_time = 0.0f;

    int CompletionType = XShmGetEventBase (display) + ShmCompletion;

    int quit = 0;

	init_scene(pixels);
	
    while (!quit) {  
	  while (XPending(display) > 0) {
		XEvent	event = {0};
		XNextEvent(display, &event);
		switch (event.type) {
		case KeyPress: {
		  switch (XLookupKeysym(&event.xkey, 0)) {
		  case 'q':
			quit	  = 1;
			break;
			
		  case 'w':
		    keymap['w'] = true;
			break;
		  case 's':
		    keymap['s'] = true;
			break;
		  case 'a':
		    keymap['a'] = true;
			break;
		  case 'd':
		    keymap['d'] = true;
			break;
		  }
		}	  
		  break;
	   
		  
		case KeyRelease: {
		  switch(XLookupKeysym(&event.xkey, 0)) {
		  case 'w':
		    keymap['w'] = false;
			break;
		  case 's':
		    keymap['s'] = false;
			break;
		  case 'a':
		    keymap['a'] = false;
			break;
		  case 'd':
		    keymap['d'] = false;
			break;
		  default:
			printf("Never reaches this\n");
		  }
		}
		  break;

		case ClientMessage: {
		  if ((Atom) event.xclient.data.l[0] == wm_delete_window) {
			quit							  = 1;
		  }
		}
		  break;

		default: {
		  printf("default\n");
		}
		}
	  }
	  
	  clear_scene(pixels, BLACK);
	  update_scene(pixels);
	  XPutImage(display, window, gc, image, 0,0,0,0, WIDTH, HEIGHT);
	}
	
    XCloseDisplay(display);
	
    return 0;
}
