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



#ifndef SHAPES
#define SHAPES

typedef struct Color Color;
#define WHITE (WHITE_IMPL)
#define BLACK (BLACK_IMPL)
#define RED   (RED_IMPL)
#define GREEN (GREEN_IMPL)
#define BLUE  (BLUE_IMPL)

#define CToUi32(color) CToUi32_IMPL(color)


typedef struct iDisplay iDisplay;


typedef struct v2i v2i;


typedef struct v2f v2f;

typedef enum BLOCKED_DIR {
  NONE=0,
  UP= 2,
  DOWN = 4,
  LEFT = 8,
  RIGHT = 16,
  X = 32,
  Y = 64,
} BLOCKED_DIR;

typedef struct Ball Ball;
Ball ball_create(v2f c, float mag);

int draw_ball(iDisplay display, Ball ball, float bt, Color color);
int draw_ball_filled(iDisplay display, Ball ball, Color color);

typedef struct Rect Rect;
Rect rect_create(v2f center, float dx, float dy, float a);
Rect rect_rotate(Rect r, float a);
BLOCKED_DIR rect_collision(Rect rect, Ball ball, bool outer);

int draw_rect(iDisplay display, Rect rect, Color color);
int draw_rect_filled(iDisplay display, Rect rect, Color color);


typedef struct Tria Tria;
Tria tri_create(v2f center, v2f p1, v2f p2, v2f p3);
Tria tri_create_equ(v2f center, float s, float a);
Tria rotate_tri(Tria t, float a);

int draw_tria(iDisplay display, Tria t, Color color);
int draw_tria_rainbow(iDisplay display, Tria t);


#ifdef SHAPES_IMPL
typedef struct iDisplay {
  int w, h;
  uint32_t *pixels;
} iDisplay;



typedef struct Color {
  uint8_t a,r,g,b;
} Color;
#define WHITE_IMPL (Color) {.a=0xFF, .r=0xFF, .g=0xFF, .b=0xFF}
#define BLACK_IMPL (Color) {.a=0xFF, .r=0x00, .g=0x00, .b=0x00}
#define RED_IMPL (Color) {.a=0xFF, .r=0xFF, .g=0x00, .b=0x00}
#define GREEN_IMPL (Color) {.a=0xFF, .r=0x00, .g=0xFF, .b=0x00}
#define BLUE_IMPL (Color) {.a=0xFF, .r=0x00, .g=0x00, .b=0xFF}

#define CToUi32_IMPL(color) (uint32_t) color.a<<24 | color.r << 16 | color.g << 8 | color.b



typedef struct v2i {
  int x,y;
} v2i;



typedef struct v2f {
  float x,y;
} v2f;



typedef struct Ball {
  v2f c;
  int mag;
} Ball;

Ball ball_create(v2f c, float mag) {
  return (Ball) {
	.c = c,
	.mag = mag,
  };
}

int draw_ball(iDisplay display, Ball ball, float bt, Color color) {
  int lx = (int) floor(ball.c.x - ball.mag);
  int rx = (int) floor(ball.c.x + ball.mag);
  int ly = (int) floor(ball.c.y - ball.mag);
  int ry = (int) floor(ball.c.y + ball.mag);

  lx = lx < 0 ? 0 : lx;
  rx = rx >= WIDTH ? WIDTH - 1 : rx;
  ly = ly < 0 ? 0 : ly;
  ry = ry >= HEIGHT ? HEIGHT - 1 : ry;
  
  for (int i =lx; i<=rx; i++) {
	for (int j=ly; j<=ry; j++) {
	  float dist = sqrt(pow((float)i-ball.c.x,2) + pow((float)j-ball.c.y,2));

	  if (ball.mag-bt < dist && dist <= ball.mag) display.pixels[j*WIDTH + i] = CToUi32(color);
	}
  }

  return 0;
}

int draw_ball_filled(iDisplay display, Ball ball, Color color) {
  int lx = (int) floor(ball.c.x - ball.mag);
  int rx = (int) floor(ball.c.x + ball.mag);
  int ly = (int) floor(ball.c.y - ball.mag);
  int ry = (int) floor(ball.c.y + ball.mag);

  lx = lx < 0 ? 0 : lx;
  rx = rx >= WIDTH ? WIDTH - 1 : rx;
  ly = ly < 0 ? 0 : ly;
  ry = ry >= HEIGHT ? HEIGHT - 1 : ry;
  
  for (int i =lx; i<=rx; i++) {
	for (int j=ly; j<=ry; j++) {
	  float dist = sqrt((float) (i-ball.c.x)*(i-ball.c.x) + (float) (j-ball.c.y)*(j-ball.c.y));

	  if (dist <= ball.mag) display.pixels[j*WIDTH + i] = CToUi32(color);
	}
  }

  return 0;
}




typedef struct Rect {
  v2f c;
  float a;
  int dx;
  int dy;
} Rect;


Rect rect_create(v2f c, float dx, float dy, float a) {
  return (Rect) { 
	.c = c,
	.a = a,
	.dx = dx,
	.dy = dy};
}

// Collision at the border
BLOCKED_DIR rect_collision(Rect rect, Ball ball, bool outer) {
  BLOCKED_DIR mask = NONE;
  
  if (outer) {
	if (rect.c.x - rect.dx <= ball.c.x &&  ball.c.x <= rect.c.x + rect.dx){
	  if (rect.c.y - rect.dy <= ball.c.y + ball.mag && rect.c.y + rect.dy >= ball.c.y - ball.mag) {
		mask = mask | Y;
	  }
	}
	
	if (rect.c.y - rect.dy <= ball.c.y &&  ball.c.y <= rect.c.y + rect.dy){
	  if (rect.c.x - rect.dx <= ball.c.x + ball.mag && rect.c.x + rect.dx >= ball.c.x - ball.mag) {
		mask = mask | X;
	  }
	}
  } else {
	if (rect.c.x - rect.dx <= ball.c.x &&  ball.c.x <= rect.c.x + rect.dx){
	  if (rect.c.y - rect.dy >= ball.c.y - ball.mag || rect.c.y + rect.dy <= ball.c.y + ball.mag) {
		mask = mask | Y;
	  }
	}
	
	if (rect.c.y - rect.dy <= ball.c.y &&  ball.c.y <= rect.c.y + rect.dy){
	  if (rect.c.x - rect.dx >= ball.c.x - ball.mag || rect.c.x + rect.dx <= ball.c.x + ball.mag) {
		mask = mask | X;
	  } 
	}
  }
  
  return mask;
}

int draw_rect(iDisplay display, Rect rect, Color color) {
  int lx = rect.c.x - rect.dx;
  int rx = rect.c.x + rect.dx;
  int ly = rect.c.y - rect.dy;
  int ry = rect.c.y + rect.dy;

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

// NOTE: b would be more accurate by using a norm. Maybe later
int draw_rect_filled(iDisplay display, Rect rect, Color color) {
  float rx = (cos(-rect.a) - sin(-rect.a)) * .5;
  float ry = (sin(-rect.a) + cos(-rect.a)) * .5;

  float c = cos(-rect.a);
  float s = sin(-rect.a);

  int b = (int) floor(rect.dx <= rect.dy ? rect.dy * 1.5 : rect.dx * 1.5);

  float t_i = c * (-b) - s*(-b) + rx;
  float t_j = s * (-b) + c*(-b) + ry;
	
  
  for (int i = -b; i<b; i++) {
	float _t_i = t_i;
	float _t_j = t_j;
	
	for (int j = -b; j<b; j++) {
	  //int t_i = (int) floor(_t_i - s * j);
	  //int t_j = (int) floor(_t_j + c * j);

	  if (-rect.dx <= _t_i && _t_i < rect.dx &&
		  -rect.dy <= _t_j && _t_j < rect.dy) {
		int di = i + (int) floor(rect.c.x);
		int dj = j + (int) floor(rect.c.y);
	  
		if (0<=di && di<WIDTH && 0<=dj && dj<HEIGHT) {
		  display.pixels[dj*WIDTH + di] = CToUi32(color);
		}
	  }

	  _t_i += c;
	  _t_j += s;
	}
	
	t_i -= s;
	t_j += c;
  }

  return 0;
}



typedef struct Tria {
  v2f pos, p1, p2, p3;
} Tria;

Tria tria_create_equ(v2f center, float s, float r) {
  float p1x = ((-s/2.) * cos(r) - (-sqrt(3)*(1./2.)*s) * sin(r));
  float p1y = ((-s/2.) * sin(r) + (-sqrt(3)*(1./2.)*s) * cos(r));
  float p2x = ((s) * cos(r));
  float p2y = ((s) * sin(r));
  float p3x = ((-s/2.) * cos(r) - (sqrt(3)*(1./2.)*s) * sin(r));
  float p3y = ((-s/2.) * sin(r) + (sqrt(3)*(1./2.)*s) * cos(r));
  
  Tria t = {
	.pos = center,
	.p1 = {.x = p1x, .y = p1y},
	.p2 = {.x = p2x, .y = p2y},
	.p3 = {.x = p3x, .y = p3y},
  };
  return t;
}

Tria tria_rotate(Tria t, float r) {
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

// NOTE: Order of points is assumed to be clockwise.
// TODO: Update to 8x8 rasters to reduce costs
int draw_tria(iDisplay display, Tria t, Color color) {
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

int draw_tria_rainbow(iDisplay display, Tria t) {
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

#endif // SHAPES_IMPL
#endif // SHAPES

