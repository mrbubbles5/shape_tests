#include "stdio.h"
#include "stdlib.h"
#include "stdbool.h"
#include "stdint.h"
#include "errno.h"
#include "math.h"

#include <X11/Xlib.h>
#include <X11/extensions/XShm.h>

#define SHAPES_IMPL
#include "shapes.h"

#define WIDTH 800
#define HEIGHT 600
#define MAG 50


v2f move = {0};
v2f gravitation = {.y=1};

Tria tri_a;
Ball ball, ball_l, ball_r;
Rect rect_l, rect_r, rect_c, screen_border;

bool keymap[256] = {0};


int update_move() {
  if (keymap['w']) move.y -= 2.;
  if (keymap['a']) move.x -= 1.;
  if (keymap['s']) move.y += 1.;
  if (keymap['d']) move.x += 1.;

  return 0;
}

Ball update_position(Ball ball, v2f move) {
  int dy = ball.c.y + gravitation.y + move.y;
  int dx = ball.c.x + move.x;

  Ball vBall = {
	.c = {dx,dy},
	.mag = ball.mag,
  };

  BLOCKED_DIR dir = rect_collision(screen_border, vBall, false);

  if (dir==NONE) {
	ball.c.x = dx;
	ball.c.y = dy;
  } else if (dir==X) {
	ball.c.y = dy;
  } else if (dir==Y) {
	ball.c.x = dx;
  } 

  return ball;
}


int init_scene(iDisplay display) {
  v2f ball_pos = {.x=WIDTH/2., .y=WIDTH/2.};
  ball = ball_create(ball_pos, 50);
  ball_pos = (v2f) {.x=0., .y=0.};
  ball_l = ball_create(ball_pos, 100);
  ball_pos = (v2f) {.x=(float) WIDTH, .y=0.};
  ball_r = ball_create(ball_pos, 100);
  
  draw_ball_filled(display, ball, RED);

  rect_l = rect_create((v2f) {.x=0., .y=HEIGHT-1.},
					   100., 100.,
					   0.);
  rect_r = rect_create((v2f) {.x=WIDTH-1., .y=HEIGHT-1.},
					   100., 100.,
					   0.);

  rect_c = rect_create((v2f) {.x=WIDTH/2., .y=HEIGHT/4.},
					   100., 100.,
					   0.125*M_PI);

  screen_border = rect_create((v2f) {.x=WIDTH/2., .y=HEIGHT/2.},
							  WIDTH/2., HEIGHT/2.,
							  0.);

  tri_a = tria_create_equ((v2f){.x=WIDTH/2., .y=3*HEIGHT/4.}, 100., M_PI/4.);
  
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

  
  draw_rect_filled(display, rect_c, RED);
  rect_c.a += .01;

  draw_tria_rainbow(display, tri_a);
  tri_a = tria_rotate(tri_a, .01);

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

