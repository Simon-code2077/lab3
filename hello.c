/*
 * Userspace program that communicates with the vga_ball device driver
 * through ioctls
 *
 * Stephen A. Edwards
 * Columbia University
 */

#include <stdio.h>
#include "vga_ball.h"
#include <sys/ioctl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <string.h>
#include <unistd.h>

int vga_ball_fd;

/* Read and print the background color */
void print_background_color() {
  vga_ball_arg_t vla;
  
  if (ioctl(vga_ball_fd, VGA_BALL_READ_BACKGROUND, &vla)) {
      perror("ioctl(VGA_BALL_READ_BACKGROUND) failed");
      return;
  }
  printf("%02x %02x %02x\n",
	 vla.background.red, vla.background.green, vla.background.blue);
}

/* Set the background color */
void set_background_color(const vga_ball_color_t *c)
{
  vga_ball_arg_t vla;
  vla.background = *c;
  if (ioctl(vga_ball_fd, VGA_BALL_WRITE_BACKGROUND, &vla)) {
      perror("ioctl(VGA_BALL_SET_BACKGROUND) failed");
      return;
  }
}

/* change the ball's position */
void change_ball_position()
{
  vga_ball_arg_t vla;
  if (ioctl(vga_ball_fd, VGA_BALL_GET_POSITION, &vla)) {
      perror("ioctl(VGA_BALL_GET_POSITION) failed");
      return;
  }
  unsigned short x, y;
  x = vla.position.x;
  y = vla.position.y;
  printf("Current position: %02x %02x %02x\n",);
  // Move the ball in a simple pattern
  x += 10; // Move right
  if (x > 639) { // Wrap around
      x = 0;
      y += 10; // Move down
      if (y > 479) {
          y = 0; // Wrap vertically
      }
  }

  vla.position.x = x;
  vla.position.y = y;

  if (ioctl(vga_ball_fd, VGA_BALL_SET_POSITION, &vla)) {
      perror("ioctl(VGA_BALL_SET_POSITION) failed");
      return;
  }
}

int main()
{
  vga_ball_arg_t vla;
  int i;
  static const char filename[] = "/dev/vga_ball";

  static const vga_ball_color_t colors[] = {
    { 0x00ff, 0x0000, 0x0000 }, /* Red */
    { 0x0000, 0x00ff, 0x0000 }, /* Green */
    { 0x0000, 0x0000, 0x00ff }, /* Blue */
    { 0x00ff, 0x00ff, 0x0000 }, /* Yellow */
    { 0x0000, 0x00ff, 0x00ff }, /* Cyan */
    { 0x00ff, 0x0000, 0x00ff }, /* Magenta */
    { 0x0080, 0x0080, 0x0080 }, /* Gray */
    { 0x0000, 0x0000, 0x0000 }, /* Black */
    { 0x00ff, 0x00ff, 0x00ff }  /* White */
  };

# define COLORS 9

  printf("VGA ball Userspace program started\n");

  if ( (vga_ball_fd = open(filename, O_RDWR)) == -1) {
    fprintf(stderr, "could not open %s\n", filename);
    return -1;
  }

  printf("initial state: ");
  print_background_color();

  for (i = 0 ; i < 24 ; i++) {
    set_background_color(&colors[i % COLORS ]);
    print_background_color();
    change_ball_position();
    usleep(400000);
  }
  
  printf("VGA BALL Userspace program terminating\n");
  return 0;
}
