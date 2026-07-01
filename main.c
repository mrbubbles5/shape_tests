#include "stdio.h"
#include "stdbool.h"
#include "stdint.h"
#include "time.h"
#include "errno.h"

#include <X11/Xlib.h>
#include <X11/extensions/XShm.h>

//#define DEBUG
#define SHAPES_IMPL
#include "shapes.h"

#define WIDTH 800
#define HEIGHT 600
#define MAG 50


// TODO: remove display





double global_time = 0.;


v2f move = {0};
v2f gravitation = {.y=1};

Tria tri_a, tri_b, tri_c, tri_d;
Ball ball, ball_l, ball_r, ball_c;
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
  int dy = ball.c.y + move.y; //gravitation.y + move.y;
  int dx = ball.c.x + move.x;

  Ball vBall = {
	.c = {dx,dy},
	.mag = ball.mag,
  };

  v2f dir = rect_ball_collision(screen_border, vBall, 64, false);

  if (dir.x == 0 && dir.y == 0) {
	ball.c.x = dx;
	ball.c.y = dy;
  } else if (dir.x == 0) {
	ball.c.y = dy;
  } else if (dir.y == 0) {
	ball.c.x = dx;
  } 

  return ball;
}


int init_scene(iDisplay display) {
  v2f ball_pos = {.x=WIDTH/2., .y=WIDTH/2.};
  ball = ball_create(ball_pos, 50);
  ball_pos = (v2f) {.x=(float) WIDTH, .y=100.};
  ball_r = ball_create(ball_pos, 100);
  ball_pos = (v2f) {.x=WIDTH/2., .y=HEIGHT/2.};
  ball_c = ball_create(ball_pos, 300);
  
  draw_ball_filled(display, ball, RED);

  rect_l = rect_create((v2f) {.x=0., .y=HEIGHT},
					   100., 100.,
					   .25*M_PI);
  rect_r = rect_create((v2f) {.x=WIDTH-1., .y=HEIGHT-1.},
					   100., 100.,
					   .25*M_PI);

  rect_c = rect_create((v2f) {.x=0., .y=0.},
					   100., 100.,
					   0.125*M_PI);

  screen_border = rect_create((v2f) {.x=WIDTH/2., .y=HEIGHT/2.},
							  WIDTH/2., HEIGHT/2.,
							  0.);

  tri_a = tria_create_equ((v2f){.x=WIDTH/2., .y=3*HEIGHT/4.}, 100., M_PI/4.);
  tri_b = tria_create_equ((v2f){.x=0.f, .y=3*HEIGHT/4.}, 100., M_PI/4.);
  tri_c = tria_create_equ((v2f){.x=WIDTH*1.-1, .y=3*HEIGHT/4.}, 100., M_PI/4.);
  tri_d = tria_create_equ((v2f){.x=WIDTH/2., .y=0.f}, 100., M_PI/4.);
  
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

  v2f cp = ball_ball_collision(ball_r, ball, true);
  if (cp.x>0 || cp.y>0) {
	draw_ball_filled(display, ball_r, RED);
	v2f f = {
	  .x = .1*(ball.c.x - cp.x),
	  .y = .1*(ball.c.y - cp.y),
	};
	ball = update_position(ball, f);
  } else {
	draw_ball_filled(display, ball_r, GREEN);
  }
  
  cp = ball_ball_collision(ball_c, ball, false);
  if (cp.x>0 || cp.y>0) {
	draw_ball_filled(display, ball_c, RED);
	/* v2f f = { */
	/*   .x = .1*(ball.c.x + cp.x), */
	/*   .y = .1*(ball.c.y + cp.y), */
	/* }; */
	/* ball = update_position(ball, f); */
  } else {
	draw_ball_filled(display, ball_c, BLUE);
  }

  cp = rect_ball_collision(rect_l, ball, 16, true);
  if (cp.x>0 || cp.y>0) {
	draw_rect_filled(display, rect_l, RED);
	v2f f = {
	  .x = .1*(ball.c.x - cp.x),
	  .y = .1*(ball.c.y - cp.y),
	};
	ball = update_position(ball, f);
  } else {
	 draw_rect_filled(display, rect_l, WHITE);
  }

  cp = rect_ball_collision(rect_r, ball, 16, true);
  if (cp.x>0 || cp.y>0) {
	draw_rect_filled(display, rect_r, RED);
	v2f f = {
	  .x = .1*(ball.c.x - cp.x),
	  .y = .1*(ball.c.y - cp.y),
	};
	ball = update_position(ball, f);
  } else {
	draw_rect_filled(display, rect_r, WHITE);
  }

  
  
  draw_ball(display, ball, 1.,  RED);
  
  move.x = 0;
  move.y = 0;

  
  /* draw_rect_filled(display, rect_c, RED); */
  /* rect_c = rect_rotate(rect_c, 0.01*global_time); */

  /* draw_tria_rainbow(display, tri_a); */
  /* tri_a = tria_rotate(tri_a, 0.01*global_time); */

  
  //v2f p = {.x=rect_r.c.x + rect_r.p1.x, .y=rect_r.c.y + rect_r.p1.y};
  //v2f p1 = {.x=rect_r.c.x + rect_r.p2.x, .y=rect_r.c.y + rect_r.p2.y};
  //find_orthogonal_visualize(display, p, p1, ball.c, 16);
  


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

	#ifdef DEBUG
	debug_display = d;
    #endif // DEBUG
	
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

    int CompletionType = XShmGetEventBase (display) + ShmCompletion;

    int quit = 0;

	init_scene(d);

	double target_fps = 60.0f;
	double current_fps = 0.f;
	int cycle_count = 5;
	int cycle = cycle_count;
	static struct timespec frame_t;
	static struct timespec cycle_start;
	clock_gettime(CLOCK_MONOTONIC, &cycle_start);

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

	  cycle -= 1;
	  
	  if (cycle==0) {
	    clock_gettime(CLOCK_MONOTONIC, &frame_t);
		
		current_fps = cycle_count / ((frame_t.tv_sec - cycle_start.tv_sec) + ((double)(frame_t.tv_nsec - cycle_start.tv_nsec)) / 1e9);
		global_time = target_fps / current_fps;

		printf("fps: %lf\n", current_fps);

		cycle = cycle_count;
	    clock_gettime(CLOCK_MONOTONIC, &cycle_start);
	  }
	}
	
    XCloseDisplay(display);
	
    return 0;
}

