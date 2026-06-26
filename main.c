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

typedef struct iDisplay {
  int w, h;
  uint32_t *pixels;
} iDisplay;


typedef struct Pos {
  int x,y;
} Pos;
typedef struct Posf {
  float x,y;
} Posf;
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
  Posf pos, p1, p2, p3;
} Triangle;

Triangle tri_a;

Triangle create_equ_tri(Posf center, float s, float r) {
  float p1x = ((-s/2.) * cos(r) - (-sqrt(3)*(1./2.)*s) * sin(r));
  float p1y = ((-s/2.) * sin(r) + (-sqrt(3)*(1./2.)*s) * cos(r));
  float p2x = ((s) * cos(r));
  float p2y = ((s) * sin(r));
  float p3x = ((-s/2.) * cos(r) - (sqrt(3)*(1./2.)*s) * sin(r));
  float p3y = ((-s/2.) * sin(r) + (sqrt(3)*(1./2.)*s) * cos(r));
  
  Triangle t = {
	.pos = center,
	.p1 = {.x = p1x, .y = p1y},
	.p2 = {.x = p2x, .y = p2y},
	.p3 = {.x = p3x, .y = p3y},
  };
  return t;
}

Triangle rotate_tri(Triangle t, float r) {
  float p1x = ((t.p1.x) * cos(r) - (t.p1.y) * sin(r));
  float p1y = ((t.p1.x) * sin(r) + (t.p1.y) * cos(r));
  float p2x = ((t.p2.x) * cos(r) - (t.p2.y) * sin(r));
  float p2y = ((t.p2.x) * sin(r) + (t.p2.y) * cos(r));
  float p3x = ((t.p3.x) * cos(r) - (t.p3.y) * sin(r));
  float p3y = ((t.p3.x) * sin(r) + (t.p3.y) * cos(r));

  t.p1.x = p1x;
  t.p1.y = p1y;
  t.p2.x = p2x;
  t.p2.y = p2y;
  t.p3.x = p3x;
  t.p3.y = p3y;

  return t;
}

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


int draw_ball(iDisplay display, Ball ball, float bt, Color color) {
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

	  if (ball.mag-bt < dist && dist <= ball.mag) display.pixels[j*WIDTH + i] = CToUi32(color);
	}
  }

  return 0;
}

int draw_ball_filled(iDisplay display, Ball ball, Color color) {
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

	  if (dist <= ball.mag) display.pixels[j*WIDTH + i] = CToUi32(color);
	}
  }

  return 0;
}

// NOTE: b would be more accurate by using a norm. Maybe later
int draw_rect_filled(iDisplay display, Rect rect, Color color) {
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
		  display.pixels[dj*WIDTH + di] = CToUi32(color);
		}
	  }
	}
  }

  return 0;
}

// NOTE: Order of points is assumed to be clockwise.
int draw_tria(iDisplay display, Triangle t, Color color) {
  float tp1x = t.pos.x + t.p1.x;
  float tp1y = t.pos.y + t.p1.y;
  float tp2x = t.pos.x + t.p2.x;
  float tp2y = t.pos.y + t.p2.y;
  float tp3x = t.pos.x + t.p3.x;
  float tp3y = t.pos.y + t.p3.y;
  
  int xl =(int) floor(tp1x <= tp2x && tp1x <= tp3x ? tp1x : ( tp2x <= tp3x ? tp2x : tp3x));
  int xr =(int) floor(tp1x >= tp2x && tp1x >= tp3x ? tp1x : ( tp2x >= tp3x ? tp2x : tp3x));
  int yl =(int) floor(tp1y <= tp2y && tp1y <= tp3y ? tp1y : ( tp2y <= tp3y ? tp2y : tp3y));
  int yr =(int) floor(tp1y >= tp2y && tp1y >= tp3y ? tp1y : ( tp2y >= tp3y ? tp2y : tp3y));

  xl = 0<=xl ? xl : 0;
  xr = xr < WIDTH ? xr : WIDTH;
  yl = 0<=yl ? yl : 0;
  yr = yr < HEIGHT ? yr : HEIGHT;

  int dp1p2x = (int) floor(tp2x - tp1x);
  int dp1p2y = (int) floor(tp2y - tp1y);
  int dp2p3x = (int) floor(tp3x - tp2x);
  int dp2p3y = (int) floor(tp3y - tp2y);
  int dp3p1x = (int) floor(tp1x - tp3x);
  int dp3p1y = (int) floor(tp1y - tp3y);

  int stp1x = (int) floor(dp1p2y * tp1x);
  int stp1y = (int) floor(dp1p2x * tp1y);
  int stp2x = (int) floor(dp2p3y * tp2x);
  int stp2y = (int) floor(dp2p3x * tp2y);
  int stp3x = (int) floor(dp3p1y * tp3x);
  int stp3y = (int) floor(dp3p1x * tp3y);

  int C1 = (int) floor(stp1x - stp1y);
  int C2 = (int) floor(stp2x - stp2y);
  int C3 = (int) floor(stp3x - stp3y);

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
		  display.pixels[j*WIDTH + i] = CToUi32(color);
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

int draw_tria_rainbow(iDisplay display, Triangle t) {
  float tp1x = t.pos.x + t.p1.x;
  float tp1y = t.pos.y + t.p1.y;
  float tp2x = t.pos.x + t.p2.x;
  float tp2y = t.pos.y + t.p2.y;
  float tp3x = t.pos.x + t.p3.x;
  float tp3y = t.pos.y + t.p3.y;
  
  int xl =(int) floor(tp1x <= tp2x && tp1x <= tp3x ? tp1x : ( tp2x <= tp3x ? tp2x : tp3x));
  int xr =(int) floor(tp1x >= tp2x && tp1x >= tp3x ? tp1x : ( tp2x >= tp3x ? tp2x : tp3x));
  int yl =(int) floor(tp1y <= tp2y && tp1y <= tp3y ? tp1y : ( tp2y <= tp3y ? tp2y : tp3y));
  int yr =(int) floor(tp1y >= tp2y && tp1y >= tp3y ? tp1y : ( tp2y >= tp3y ? tp2y : tp3y));

  xl = 0<=xl ? xl : 0;
  xr = xr < WIDTH ? xr : WIDTH;
  yl = 0<=yl ? yl : 0;
  yr = yr < HEIGHT ? yr : HEIGHT;

  int dp1p2x = (int) floor(tp2x - tp1x);
  int dp1p2y = (int) floor(tp2y - tp1y);
  int dp2p3x = (int) floor(tp3x - tp2x);
  int dp2p3y = (int) floor(tp3y - tp2y);
  int dp3p1x = (int) floor(tp1x - tp3x);
  int dp3p1y = (int) floor(tp1y - tp3y);

  int stp1x = (int) floor(dp1p2y * tp1x);
  int stp1y = (int) floor(dp1p2x * tp1y);
  int stp2x = (int) floor(dp2p3y * tp2x);
  int stp2y = (int) floor(dp2p3x * tp2y);
  int stp3x = (int) floor(dp3p1y * tp3x);
  int stp3y = (int) floor(dp3p1x * tp3y);

  int C1 = (int) floor(stp1x - stp1y);
  int C2 = (int) floor(stp2x - stp2y);
  int C3 = (int) floor(stp3x - stp3y);

  int sstp1 = yl * dp1p2x - xl * dp1p2y + C1;
  int sstp2 = yl * dp2p3x - xl * dp2p3y + C2;
  int sstp3 = yl * dp3p1x - xl * dp3p1y + C3;

  int sstp1_ = tp3y * dp1p2x - tp3x * dp1p2y + C1;
  int sstp2_ = tp1y * dp2p3x - tp1x * dp2p3y + C2;
  int sstp3_ = tp2y * dp3p1x - tp2x * dp3p1y + C3;
  
  for (int j = yl; j<yr; j++) {
  	int i_sstp1 = sstp1;
	int i_sstp2 = sstp2;
	int i_sstp3 = sstp3;
	for (int i = xl; i<xr; i++) {
	  if (i_sstp1 > 0 &&
		  i_sstp2 > 0 &&
		  i_sstp3 > 0) {
		  int sstp1__ = j * dp1p2x - i * dp1p2y + C1;
		  int sstp2__ = j * dp2p3x - i * dp2p3y + C2;
		  int sstp3__ = j * dp3p1x - i * dp3p1y + C3;

		  Color color = {
			.a = 0xFF,
			.r = (uint8_t)(0xFF * ((float)(sstp1__) / (float)(sstp1_))),
			.g = (uint8_t)(0xFF * ((float)(sstp2__) / (float)(sstp2_))),
			.b = (uint8_t)(0xFF * ((float)(sstp3__) / (float)(sstp3_)))};
		  display.pixels[j*WIDTH + i] = CToUi32(color);
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


int draw_rect(iDisplay display, Rect rect, float bt, Color color) {
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
	  display.pixels[j*WIDTH + i] = CToUi32(color);
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


int init_scene(iDisplay display) {
  draw_ball_filled(display, ball, RED);

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

  tri_a = create_equ_tri((Posf){.x=WIDTH/2., .y=HEIGHT/2.}, 100., M_PI/4.);
  
  return 0;
}

int clear_scene(iDisplay display, Color color) {
  for (int i =0; i<WIDTH; i++) {
	for (int j=0; j<HEIGHT; j++) {
	  display.pixels[j*WIDTH + i] = CToUi32(color);
	}
  }
  
  return 0;
}

int update_scene(iDisplay display) {  
  update_move();
  ball = update_position(ball, move);
  
  draw_ball(display, ball, 1.,  RED);
  draw_ball_filled(display, ball_l, BLUE);
  draw_ball_filled(display, ball_r, GREEN);

  if (rect_collision(rect_l, ball, true)) {
	draw_rect_filled(display, rect_l, RED);
  } else {
	draw_rect_filled(display, rect_l, WHITE);
  }
  if (rect_collision(rect_r, ball, true)) draw_rect_filled(display, rect_r, RED);
  else draw_rect_filled(display, rect_r, WHITE);
  
  move.x = 0;
  move.y = 0;

  
  //draw_rect_filled(pdisplay.ixels, rect_c, RED);
  rect_c.angle += .01;

  draw_tria_rainbow(display, tri_a);
  tri_a = rotate_tri(tri_a, 1);

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
	iDisplay d = {
	  .w = WIDTH,
	  .h = HEIGHT,
	  .pixels = pixels,
	};
	
	image = XCreateImage(display,
						 wa.visual,
						 wa.depth,
						 ZPixmap,
						 0,
						 (char*) (d.pixels),
						 d.w,
						 d.h,
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

	init_scene(d);
	
    while (!quit) {  
	  while (XPending(display) > 0) {
		XEvent	event = {0};
		XNextEvent(display, &event);
		switch (event.type) {
		case KeyPress: {
		  char key = XLookupKeysym(&event.xkey, 0); 
		  switch (key) {
		  case 'q':
			quit = 1;
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
		  default:
			printf("This key was released %c\n", key);
		  }
		}	  
		  break;
	   
		  
		case KeyRelease: {
		  char key = XLookupKeysym(&event.xkey, 0);
		  switch(key) {
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
			printf("This key was released %c\n", key);
		  }
		}
		  break;

		case ClientMessage: {
		  if ((Atom) event.xclient.data.l[0] == wm_delete_window) {
			quit = 1;
		  }
		}
		  break;

		default: {
		  printf("default\n");
		}
		}
	  }
	  
	  clear_scene(d, BLACK);
	  update_scene(d);
	  XPutImage(display, window, gc, image, 0,0,0,0, d.w, d.h);
	}
	
    XCloseDisplay(display);
	
    return 0;
}

