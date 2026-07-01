#include "stdio.h"
#include "stdbool.h"
#include "stdint.h"
#include "time.h"
#include "errno.h"

#include <X11/Xlib.h>
#include <X11/extensions/XShm.h>

#define SHAPES_IMPL
#include "shapes.h"

#define WIDTH 800
#define HEIGHT 600
#define MAG 50

double global_time = 0.;



v2f move = {0};
v2f gravitation = {.y=1};

Tria tri_a, tri_b, tri_c, tri_d;
Ball ball, ball_l, ball_r;
Rect rect_l, rect_r, rect_c, screen_border;

bool keymap[256] = {0};

int find_orthogonal(iDisplay display, v2f p11, v2f p12, v2f p21, int sa) {
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

	if (dot < 1./(2*sa) && dot >= -1./(2*sa)) {
	  draw_line(display, p21, pc, GREEN);
	} else {
	  draw_line(display, p21, pc, RED);
	}
 	 

	
	pc.x += d_p11p12x/(float)sa;
	pc.y += d_p11p12y/(float)sa;
  }

  return 0;
}




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

  BLOCKED_DIR dir = rect_ball_collision(screen_border, vBall, false);

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
  ball_pos = (v2f) {.x=(float) WIDTH, .y=100.};
  ball_l = ball_create(ball_pos, 100);
  ball_pos = (v2f) {.x=(float) WIDTH, .y=0.};
  ball_r = ball_create(ball_pos, 100);
  
  draw_ball_filled(display, ball, RED);

  rect_l = rect_create((v2f) {.x=0., .y=HEIGHT-1.},
					   100., 100.,
					   0.);
  rect_r = rect_create((v2f) {.x=WIDTH/2., .y=HEIGHT/2.},
					   100., 100.,
					   0.);

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
  
  draw_ball(display, ball, 1.,  RED);
  draw_ball_filled(display, ball_l, BLUE);
  draw_ball_filled(display, ball_r, GREEN);

  if (rect_ball_collision(rect_l, ball, true)) {
	draw_rect_filled(display, rect_l, RED);
  } else {
	draw_rect_filled(display, rect_l, WHITE);
  }
  if (rect_ball_collision(rect_r, ball, true)) draw_rect_filled(display, rect_r, RED);
  else draw_rect_filled(display, rect_r, WHITE);
  
  move.x = 0;
  move.y = 0;

  
  /* draw_rect_filled(display, rect_c, RED); */
  /* rect_c = rect_rotate(rect_c, 0.01*global_time); */

  /* draw_tria_rainbow(display, tri_a); */
  /* tri_a = tria_rotate(tri_a, 0.01*global_time); */
  /* draw_tria_rainbow(display, tri_b); */
  /* tri_b = tria_rotate(tri_b, 0.01*global_time); */
  /* draw_tria_rainbow(display, tri_c); */
  /* tri_c = tria_rotate(tri_c, 0.01*global_time); */
  /* draw_tria_rainbow(display, tri_d); */
  /* tri_d = tria_rotate(tri_d, 0.01*global_time); */


  float d_btrp1x = ball.c.x-rect_r.c.x-rect_r.p1.x;
  float d_btrp1y = ball.c.y-rect_r.c.y-rect_r.p1.y;
  float d_btrp1 = pow(d_btrp1x,2) + pow(d_btrp1y,2);
  float d_btrp2x = ball.c.x-rect_r.c.x-rect_r.p2.x;
  float d_btrp2y = ball.c.y-rect_r.c.y-rect_r.p2.y;
  float d_btrp2 = pow(d_btrp2x,2) + pow(d_btrp2y,2);
  float d_btrp3x = ball.c.x-rect_r.c.x-rect_r.p3.x;
  float d_btrp3y = ball.c.y-rect_r.c.y-rect_r.p3.y;
  float d_btrp3 = pow(d_btrp3x,2) + pow(d_btrp3y,2);
  float d_btrp4x = ball.c.x-rect_r.c.x-rect_r.p4.x;
  float d_btrp4y = ball.c.y-rect_r.c.y-rect_r.p4.y;
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
	d_rminx = rect_r.p1.x;
	d_rminy = rect_r.p1.y;
	d_r_p1_x = rect_r.p2.x;
	d_r_p1_y = rect_r.p2.y;
	d_r_m1_x = rect_r.p4.x;
	d_r_m1_y = rect_r.p4.y;
  } else if (d_btrp2 <= d_btrp3 &&
			 d_btrp2 <= d_btrp4) {
	d_rminx = rect_r.p2.x;
	d_rminy = rect_r.p2.y;
	d_r_p1_x = rect_r.p3.x;
	d_r_p1_y = rect_r.p3.y;
	d_r_m1_x = rect_r.p1.x;
	d_r_m1_y = rect_r.p1.y;
  } else if (d_btrp3 <= d_btrp4) {
	d_rminx = rect_r.p3.x;
	d_rminy = rect_r.p3.y;
	d_r_p1_x = rect_r.p4.x;
	d_r_p1_y = rect_r.p4.y;
	d_r_m1_x = rect_r.p2.x;
	d_r_m1_y = rect_r.p2.y;
  } else {
	d_rminx = rect_r.p4.x;
	d_rminy = rect_r.p4.y;
	d_r_p1_x = rect_r.p1.x;
	d_r_p1_y = rect_r.p1.y;
	d_r_m1_x = rect_r.p3.x;
	d_r_m1_y = rect_r.p3.y;
  }

  v2f p = {.x=rect_r.c.x + d_rminx, .y=rect_r.c.y + d_rminy};
  v2f p1 = {.x=rect_r.c.x + d_r_p1_x, .y=rect_r.c.y + d_r_p1_y};
  find_orthogonal(display, p, p1, ball.c, 16);
  


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

