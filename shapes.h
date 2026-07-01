#include "stdlib.h"
#include "stdbool.h"
#include "stdint.h"
#include "math.h"


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
iDisplay debug_display;

// TODO: lin alg lib
typedef struct v2i v2i;
typedef struct v2f v2f;

float v2f_dist_eukl(v2f p1, v2f p2);
int draw_line(iDisplay display, v2f p1, v2f p2, Color color);

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
v2f ball_ball_collision(Ball b1, Ball b2, bool o);

int draw_ball(iDisplay display, Ball ball, float bt, Color color);
int draw_ball_filled(iDisplay display, Ball ball, Color color);


// TODO: rename Rect to Quadliteral
typedef struct Rect Rect;
Rect rect_quad_create(v2f c, v2f p1, v2f p2, v2f p3, v2f p4);
Rect rect_create(v2f center, float dx, float dy, float a);
Rect rect_rotate(Rect r, float a);
v2f rect_ball_collision(Rect rect, Ball ball, int p, bool outer);

int draw_rect(iDisplay display, Rect rect, Color color);
int draw_rect_filled(iDisplay display, Rect rect, Color color);


typedef struct Tria Tria;
Tria tri_create(v2f center, v2f p1, v2f p2, v2f p3);
Tria tri_create_equ(v2f center, float s, float a);
Tria tri_rotate(Tria t, float a);

int draw_tria(iDisplay display, Tria t, Color color);
int draw_tria_rainbow(iDisplay display, Tria t);

v2f find_orthogonal(v2f p11, v2f p12, v2f p21, int sa);
v2f find_orthogonal_visualized(iDisplay display, v2f p11, v2f p12, v2f p21, int sa);


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



float v2f_dist_eukl(v2f p1, v2f p2) {
  return sqrt(pow(p1.x-p2.x,2) + pow(p1.y-p2.y,2));
}


int draw_line(iDisplay display, v2f p1, v2f p2, Color color){
  float d_p1p2x = p1.x - p2.x;
  float d_p1p2y = p1.y - p2.y;
  float d_p1p2 = sqrt(pow(d_p1p2x,2) + pow(d_p1p2y,2));
  d_p1p2 = d_p1p2==0 ? 1 : d_p1p2;
  
  for (int i = 0; i <= (int) floor(d_p1p2); i++) {
	int x = (int) floor(p2.x + i/d_p1p2 * d_p1p2x);
	int y = (int) floor(p2.y + i/d_p1p2 * d_p1p2y);

	if (0<=x && x<display.w && 0<=y && y<display.h) {
	  display.pixels[y*display.w + x] = CToUi32(color);
	}
  }

  
  return 0;
}


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

v2f ball_ball_collision(Ball b1, Ball b2, bool o) {
  float d_b1b2 = v2f_dist_eukl(b1.c,b2.c);

  if (o) {
	if (d_b1b2 <= (b1.mag + b2.mag)) {
	  v2f cp = {
		.x = b1.c.x + b1.mag/(b1.mag + b2.mag) * (b1.c.x - b2.c.x),
		.y = b1.c.y + b1.mag/(b1.mag + b2.mag) * (b1.c.y - b2.c.y),
	  };
	  return cp;
	}
  } else {
	if (d_b1b2 + b2.mag >= b1.mag) {
	  v2f cp = {
		.x = - (b2.mag/b1.mag)*(b1.c.x - b2.c.x),
		.y = - (b2.mag/b1.mag)*(b1.c.y - b2.c.y),
	  };
	  return cp;
	}
  }

  return (v2f) {0};
}

int draw_ball(iDisplay display, Ball ball, float bt, Color color) {
  int lx = (int) floor(ball.c.x - ball.mag);
  int rx = (int) floor(ball.c.x + ball.mag);
  int ly = (int) floor(ball.c.y - ball.mag);
  int ry = (int) floor(ball.c.y + ball.mag);

  lx = lx < 0 ? 0 : lx;
  rx = rx >= display.w ? display.w - 1 : rx;
  ly = ly < 0 ? 0 : ly;
  ry = ry >= display.h ? display.h - 1 : ry;
  
  for (int i =lx; i<=rx; i++) {
	for (int j=ly; j<=ry; j++) {
	  float dist = sqrt(pow((float)i-ball.c.x,2) + pow((float)j-ball.c.y,2));

	  if (ball.mag-bt < dist && dist <= ball.mag) display.pixels[j*display.w + i] = CToUi32(color);
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
  rx = rx >= display.w ? display.w - 1 : rx;
  ly = ly < 0 ? 0 : ly;
  ry = ry >= display.h ? display.h - 1 : ry;
  
  for (int i =lx; i<=rx; i++) {
	for (int j=ly; j<=ry; j++) {
	  float dist = sqrt((float) (i-ball.c.x)*(i-ball.c.x) + (float) (j-ball.c.y)*(j-ball.c.y));

	  if (dist <= ball.mag) display.pixels[j*display.w + i] = CToUi32(color);
	}
  }

  return 0;
}




typedef struct Rect {
  v2f c;
  v2f p1,p2,p3,p4;
} Rect;


Rect rect_create(v2f c, float dx, float dy, float a) {
  v2f p1 = {
	(-dx)* cos(a) - (-dy) * sin(a),
	(-dx)* sin(a) + (-dy) * cos(a),
  };
  v2f p2 = {
	(dx)* cos(a) - (-dy) * sin(a),
	(dx)* sin(a) + (-dy) * cos(a),
  };
  v2f p3 = {
	(dx)* cos(a) - (dy) * sin(a),
	(dx)* sin(a) + (dy) * cos(a),
  };
  v2f p4 = {
	(-dx)* cos(a) - (dy) * sin(a),
	(-dx)* sin(a) + (dy) * cos(a),
  };

  return (Rect) { 
	.c = c,
	.p1 = p1,
	.p2 = p2,
	.p3 = p3,
	.p4 = p4}; 
}

Rect rect_quad_create(v2f c, v2f p1, v2f p2, v2f p3, v2f p4) {
  return (Rect) { 
	.c = c,
	.p1 = p1,
	.p2 = p2,
	.p3 = p3,
	.p4 = p4};
}

Rect rect_rotate(Rect r, float a) {
    v2f p1 = {
	(r.p1.x)* cos(a) - (r.p1.y) * sin(a),
	(r.p1.x)* sin(a) + (r.p1.y) * cos(a),
  };
  v2f p2 = {
	(r.p2.x)* cos(a) - (r.p2.y) * sin(a),
	(r.p2.x)* sin(a) + (r.p2.y) * cos(a),
  };
  v2f p3 = {
	(r.p3.x)* cos(a) - (r.p3.y) * sin(a),
	(r.p3.x)* sin(a) + (r.p3.y) * cos(a),
  };
  v2f p4 = {
	(r.p4.x)* cos(a) - (r.p4.y) * sin(a),
	(r.p4.x)* sin(a) + (r.p4.y) * cos(a),
  };

  return (Rect) { 
	.c = r.c,
	.p1 = p1,
	.p2 = p2,
	.p3 = p3,
	.p4 = p4}; 
}

v2f rect_ball_collision(Rect r, Ball b, int p, bool o) {
  float d_btrp1x = b.c.x-r.c.x-r.p1.x;
  float d_btrp1y = b.c.y-r.c.y-r.p1.y;
  float d_btrp1 = pow(d_btrp1x,2) + pow(d_btrp1y,2);
  float d_btrp2x = b.c.x-r.c.x-r.p2.x;
  float d_btrp2y = b.c.y-r.c.y-r.p2.y;
  float d_btrp2 = pow(d_btrp2x,2) + pow(d_btrp2y,2);
  float d_btrp3x = b.c.x-r.c.x-r.p3.x;
  float d_btrp3y = b.c.y-r.c.y-r.p3.y;
  float d_btrp3 = pow(d_btrp3x,2) + pow(d_btrp3y,2);
  float d_btrp4x = b.c.x-r.c.x-r.p4.x;
  float d_btrp4y = b.c.y-r.c.y-r.p4.y;
  float d_btrp4 = pow(d_btrp4x,2) + pow(d_btrp4y,2);

  float d_rminx;
  float d_rminy;
  float d_r_p1_x;
  float d_r_p1_y;
  float d_r_m1_x;
  float d_r_m1_y;
  
	
  if (d_btrp1 <= d_btrp2 &&
	  d_btrp1 <= d_btrp3 &&
	  d_btrp1 <= d_btrp4) {
	d_rminx = r.p1.x;
	d_rminy = r.p1.y;
	d_r_p1_x = r.p2.x;
	d_r_p1_y = r.p2.y;
	d_r_m1_x = r.p4.x;
	d_r_m1_y = r.p4.y;
  } else if (d_btrp2 <= d_btrp3 &&
			 d_btrp2 <= d_btrp4) {
	d_rminx = r.p2.x;
	d_rminy = r.p2.y;
	d_r_p1_x = r.p3.x;
	d_r_p1_y = r.p3.y;
	d_r_m1_x = r.p1.x;
	d_r_m1_y = r.p1.y;
  } else if (d_btrp3 <= d_btrp4) {
	d_rminx = r.p3.x;
	d_rminy = r.p3.y;
	d_r_p1_x = r.p4.x;
	d_r_p1_y = r.p4.y;
	d_r_m1_x = r.p2.x;
	d_r_m1_y = r.p2.y;
  } else {
	d_rminx = r.p4.x;
	d_rminy = r.p4.y;
	d_r_p1_x = r.p1.x;
	d_r_p1_y = r.p1.y;
	d_r_m1_x = r.p3.x;
	d_r_m1_y = r.p3.y;
  }

  v2f d_rmin = {
	.x = r.c.x + d_rminx,
	.y = r.c.y + d_rminy,
  };
  v2f d_r_p1 = {
	.x = r.c.x + d_r_p1_x,
	.y = r.c.y + d_r_p1_y,
  };
  v2f d_r_m1 = {
	.x = r.c.x + d_r_m1_x,
	.y = r.c.y + d_r_m1_y,
  };
  
  float c1 = ((d_rmin.x - d_r_m1.x))*(b.c.y - d_r_m1.y) - (d_rmin.y - d_r_m1.y)*(b.c.x - d_r_m1.x);
  float c2 = ((d_r_p1.x - d_rmin.x))*(b.c.y - d_rmin.y) - (d_r_p1.y - d_rmin.y)*(b.c.x - d_rmin.x);

  #ifdef DEBUG
  draw_line(debug_display, b.c, d_rmin, BLUE);
  draw_line(debug_display, b.c, d_r_p1, BLUE);
  draw_line(debug_display, b.c, d_r_m1, BLUE);
  #endif // DEBUG
  
  float d_bcrmin = v2f_dist_eukl(d_rmin, b.c);

  if (o) {
	if (c1 <= 0 && c2 <= 0) {
	  if (d_bcrmin < b.mag) {
        #ifdef DEBUG
		draw_line(debug_display, b.c, d_rmin, GREEN);
        #endif // DEBUG

		return d_rmin;
	  }

      #ifdef DEBUG
	  if (d_bcrmin >= b.mag) {
		draw_line(debug_display, b.c, d_rmin, RED);
	  }
      #endif // DEBUG
   
	} else if (c1 <= 0) {
	  #ifdef DEBUG
	  v2f orth = find_orthogonal_visualized(debug_display, d_rmin, d_r_m1, b.c, p);
	  #else
	  v2f orth = find_orthogonal(d_rmin, d_r_m1, b.c, p);
      #endif // DEBUG

	  float d_bo = v2f_dist_eukl(b.c, orth);

	  if (d_bo <= b.mag) return orth;
    
	} else if (c2 <= 0) {
	  #ifdef DEBUG
	  v2f orth = find_orthogonal_visualized(debug_display, d_rmin, d_r_p1, b.c, p);
	  #else
	  v2f orth = find_orthogonal(d_rmin, d_r_p1, b.c, p);
	  #endif // DEBUG
	  
	  float d_bo = v2f_dist_eukl(b.c, orth);

	  if (d_bo <= b.mag) return orth;
	}
  } else {
	if (c1 > 0 && c2 > 0) {
	  #ifdef DEBUG
	  v2f orth1 = find_orthogonal_visualized(debug_display, d_rmin, d_r_m1, b.c, p);
	  #else
	  v2f orth1 = find_orthogonal(d_rmin, d_r_m1, b.c, p);
      #endif // DEBUG

	  float d_bo1 = v2f_dist_eukl(b.c, orth1);

	  if (d_bo1 < b.mag) return orth1;
	  
	  #ifdef DEBUG
	  v2f orth2 = find_orthogonal_visualized(debug_display, d_rmin, d_r_p1, b.c, p);
	  #else
	  v2f orth2 = find_orthogonal(d_rmin, d_r_p1, b.c, p);
	  #endif // DEBUG
	  
	  float d_bo2 = v2f_dist_eukl(b.c, orth2);

	  if (d_bo2 < b.mag) return orth2;
	}
  }

  return (v2f){0};
}

int draw_rect(iDisplay display, Rect rect, Color color) {
  float rp1x = rect.c.x + rect.c.x;
  float rp1y = rect.c.y + rect.c.y;
  float rp2x = rect.c.x + rect.c.x;
  float rp2y = rect.c.y + rect.c.y;
  float rp3x = rect.c.x + rect.c.x;
  float rp3y = rect.c.y + rect.c.y;
  float rp4x = rect.c.x + rect.c.x;
  float rp4y = rect.c.y + rect.c.y;
  
  int xl =(int) floor(rp1x <= rp2x && rp1x <= rp3x  && rp1x <= rp4x ? rp1x : ( rp2x <= rp3x && rp2x <= rp4x ? rp2x : ( rp3x <= rp4x ? rp3x : rp4x)));
  int xr =(int) floor(rp1x >= rp2x && rp1x >= rp3x  && rp1x >= rp4x ? rp1x : ( rp2x >= rp3x && rp2x >= rp4x ? rp2x : ( rp3x >= rp4x ? rp3x : rp4x)));
  int yl =(int) floor(rp1y <= rp2y && rp1y <= rp3y  && rp1y <= rp4y ? rp1y : ( rp2y <= rp3y && rp2y <= rp4y ? rp2y : ( rp3y <= rp4y ? rp3y : rp4y)));
  int yr =(int) floor(rp1y >= rp2y && rp1y >= rp3y  && rp1y >= rp4y ? rp1y : ( rp2y >= rp3y && rp2y >= rp4y ? rp2y : ( rp3y >= rp4y ? rp3y : rp4y)));

  xl = xl < display.w ? (0<=xl ? xl : 0) : display.w;
  xr = 0 <= xr ? (xr < display.w ? xr : display.w) : 0;
  yl = yl < display.h ? (0<=yl ? yl : 0) : display.h;
  yr = 0 <= yr ? (yr < display.h ? yr : display.h) : 0;

  int dp1p2x = (int) floor(rp2x - rp1x);
  int dp1p2y = (int) floor(rp2y - rp1y);
  int dp2p3x = (int) floor(rp3x - rp2x);
  int dp2p3y = (int) floor(rp3y - rp2y);
  int dp3p4x = (int) floor(rp4x - rp3x);
  int dp3p4y = (int) floor(rp4y - rp3y);
  int dp4p1x = (int) floor(rp1x - rp4x);
  int dp4p1y = (int) floor(rp1y - rp4y);

  int srp1x = (int) floor(dp1p2y * rp1x);
  int srp1y = (int) floor(dp1p2x * rp1y);
  int srp2x = (int) floor(dp2p3y * rp2x);
  int srp2y = (int) floor(dp2p3x * rp2y);
  int srp3x = (int) floor(dp3p4y * rp3x);
  int srp3y = (int) floor(dp3p4x * rp3y);
  int srp4x = (int) floor(dp4p1y * rp4x);
  int srp4y = (int) floor(dp4p1x * rp4y);

  int C1 = (int) floor(srp1x - srp1y);
  int C2 = (int) floor(srp2x - srp2y);
  int C3 = (int) floor(srp3x - srp3y);
  int C4 = (int) floor(srp4x - srp4y);

  int ssrp1 = yl * dp1p2x - xl * dp1p2y + C1;
  int ssrp2 = yl * dp2p3x - xl * dp2p3y + C2;
  int ssrp3 = yl * dp3p4x - xl * dp3p4y + C3;
  int ssrp4 = yl * dp4p1x - xl * dp4p1y + C4;

  for (int j = yl; j<yr; j++) {
  	int i_ssrp1 = ssrp1;
	int i_ssrp2 = ssrp2;
	int i_ssrp3 = ssrp3;
	int i_ssrp4 = ssrp4;
	
	for (int i = xl; i<xr; i++) {
	  if (i_ssrp1 == 0 &&
		  i_ssrp2 == 0 &&
		  i_ssrp3 == 0 &&
		  i_ssrp4 == 0) {
		  display.pixels[j*display.w + i] = CToUi32(color);
	  }
	  i_ssrp1 -= dp1p2y;
	  i_ssrp2 -= dp2p3y;
	  i_ssrp3 -= dp3p4y;
	  i_ssrp4 -= dp4p1y;
	}
	ssrp1 += dp1p2x;
	ssrp2 += dp2p3x;
	ssrp3 += dp3p4x;
	ssrp4 += dp4p1x;
  }

  return 0;
}

// NOTE: b would be more accurate by using a norm. Maybe later
// TODO: if xl and xr are far left in the negative, clipping result into a shift -> xr <= xl
int draw_rect_filled(iDisplay display, Rect rect, Color color) {
  float rp1x = rect.c.x + rect.p1.x;
  float rp1y = rect.c.y + rect.p1.y;
  float rp2x = rect.c.x + rect.p2.x;
  float rp2y = rect.c.y + rect.p2.y;
  float rp3x = rect.c.x + rect.p3.x;
  float rp3y = rect.c.y + rect.p3.y;
  float rp4x = rect.c.x + rect.p4.x;
  float rp4y = rect.c.y + rect.p4.y;
  
  int xl =(int) floor(rp1x <= rp2x && rp1x <= rp3x  && rp1x <= rp4x ? rp1x : ( rp2x <= rp3x && rp2x <= rp4x ? rp2x : ( rp3x <= rp4x ? rp3x : rp4x)));
  int xr =(int) floor(rp1x >= rp2x && rp1x >= rp3x  && rp1x >= rp4x ? rp1x : ( rp2x >= rp3x && rp2x >= rp4x ? rp2x : ( rp3x >= rp4x ? rp3x : rp4x)));
  int yl =(int) floor(rp1y <= rp2y && rp1y <= rp3y  && rp1y <= rp4y ? rp1y : ( rp2y <= rp3y && rp2y <= rp4y ? rp2y : ( rp3y <= rp4y ? rp3y : rp4y)));
  int yr =(int) floor(rp1y >= rp2y && rp1y >= rp3y  && rp1y >= rp4y ? rp1y : ( rp2y >= rp3y && rp2y >= rp4y ? rp2y : ( rp3y >= rp4y ? rp3y : rp4y)));

  xl = xl < display.w ? (0<=xl ? xl : 0) : display.w;
  xr = 0 <= xr ? (xr < display.w ? xr : display.w) : 0;
  yl = yl < display.h ? (0<=yl ? yl : 0) : display.h;
  yr = 0 <= yr ? (yr < display.h ? yr : display.h) : 0;

  int dp1p2x = (int) floor(rp2x - rp1x);
  int dp1p2y = (int) floor(rp2y - rp1y);
  int dp2p3x = (int) floor(rp3x - rp2x);
  int dp2p3y = (int) floor(rp3y - rp2y);
  int dp3p4x = (int) floor(rp4x - rp3x);
  int dp3p4y = (int) floor(rp4y - rp3y);
  int dp4p1x = (int) floor(rp1x - rp4x);
  int dp4p1y = (int) floor(rp1y - rp4y);

  int srp1x = (int) floor(dp1p2y * rp1x);
  int srp1y = (int) floor(dp1p2x * rp1y);
  int srp2x = (int) floor(dp2p3y * rp2x);
  int srp2y = (int) floor(dp2p3x * rp2y);
  int srp3x = (int) floor(dp3p4y * rp3x);
  int srp3y = (int) floor(dp3p4x * rp3y);
  int srp4x = (int) floor(dp4p1y * rp4x);
  int srp4y = (int) floor(dp4p1x * rp4y);

  int C1 = (int) floor(srp1x - srp1y);
  int C2 = (int) floor(srp2x - srp2y);
  int C3 = (int) floor(srp3x - srp3y);
  int C4 = (int) floor(srp4x - srp4y);

  int ssrp1 = yl * dp1p2x - xl * dp1p2y + C1;
  int ssrp2 = yl * dp2p3x - xl * dp2p3y + C2;
  int ssrp3 = yl * dp3p4x - xl * dp3p4y + C3;
  int ssrp4 = yl * dp4p1x - xl * dp4p1y + C4;

  for (int j = yl; j<yr; j++) {
  	int i_ssrp1 = ssrp1;
	int i_ssrp2 = ssrp2;
	int i_ssrp3 = ssrp3;
	int i_ssrp4 = ssrp4;
	
	for (int i = xl; i<xr; i++) {
	  if (i_ssrp1 > 0 &&
		  i_ssrp2 > 0 &&
		  i_ssrp3 > 0 &&
		  i_ssrp4 > 0) {
		  display.pixels[j*display.w + i] = CToUi32(color);
	  }
	  i_ssrp1 -= dp1p2y;
	  i_ssrp2 -= dp2p3y;
	  i_ssrp3 -= dp3p4y;
	  i_ssrp4 -= dp4p1y;
	}
	ssrp1 += dp1p2x;
	ssrp2 += dp2p3x;
	ssrp3 += dp3p4x;
	ssrp4 += dp4p1x;
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
  xr = xr < display.w ? xr : display.w;
  yl = 0<=yl ? yl : 0;
  yr = yr < display.h ? yr : display.h;

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
		  display.pixels[j*display.w + i] = CToUi32(color);
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
  xr = xr < display.w ? xr : display.w;
  yl = 0<=yl ? yl : 0;
  yr = yr < display.h ? yr : display.h;

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
		  display.pixels[j*display.w + i] = CToUi32(color);
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


// NOTE: approximates the point on line p11 -> p12 that represents
//       an nearly orthogonal line to p21. sa represents the amount of
//       grid steps including the starting points.
v2f find_orthogonal(v2f p11, v2f p12, v2f p21, int sa) {
  float d_p11p12x = p12.x - p11.x;
  float d_p11p12y = p12.y - p11.y;
  v2f pc = {.x=p11.x + 1/(float)sa*d_p11p12x, .y=p11.y + 1/(float)sa*d_p11p12y};
 	
  for (int i=1; i<=sa; i++) {
	v2f dpcb = {.x=pc.x-p21.x,
				.y=pc.y-p21.y};
	float d1 = sqrt(pow(dpcb.x,2) + pow(dpcb.y,2));
	
	v2f dp1p2 = {.x = p11.x - p12.x,
				 .y = p11.y - p12.y};
	float d2 = sqrt(pow(dp1p2.x,2) + pow(dp1p2.y,2));
	
	// NOTE: normalized dot product of crossing lines 
	float dot = (dpcb.x/d1)*(dp1p2.x/d2) + (dpcb.y/d1)*(dp1p2.y/d2);

	float m = d2 / d1;
	if (-m/(2.*sa) <= dot && dot < m/(2.*sa)) {
	  return pc;
	}
 	 
	pc.x += d_p11p12x/(float)sa;
	pc.y += d_p11p12y/(float)sa;
  }

  return (v2f) {0};
}


v2f find_orthogonal_visualized(iDisplay display, v2f p11, v2f p12, v2f p21, int sa) {
  draw_line(display, p11, p21, BLUE);
  draw_line(display, p12, p21, BLUE);
  
  float d_p11p12x = p12.x - p11.x;
  float d_p11p12y = p12.y - p11.y;
  v2f pc = {.x=p11.x + 1/(float)sa*d_p11p12x, .y=p11.y + 1/(float)sa*d_p11p12y};
 	
  for (int i=1; i<=sa; i++) {
	v2f dpcb = {.x=pc.x-p21.x,
				.y=pc.y-p21.y};
	float d1 = sqrt(pow(dpcb.x,2) + pow(dpcb.y,2));
	
	v2f dp1p2 = {.x = p11.x-p12.x,
				 .y = p11.y - p12.y};
	float d2 = sqrt(pow(dp1p2.x,2) + pow(dp1p2.y,2));
	
	float dot = (dpcb.x/d1)*(dp1p2.x/d2) + (dpcb.y/d1)*(dp1p2.y/d2);

	float m = d2 / d1;
	if (-m/(2.*sa) <= dot && dot < m/(2.*sa)) {
	  draw_line(display, p21, pc, GREEN);
	  return pc;
	} else {
	  draw_line(display, p21, pc, RED);
	}
 	 
	pc.x += d_p11p12x/(float)sa;
	pc.y += d_p11p12y/(float)sa;
  }

  return (v2f) {0};
}

#endif // SHAPES_IMPL
#endif // SHAPES

