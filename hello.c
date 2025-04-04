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
#include <stdlib.h>
#include <time.h>
#include <stdbool.h>

int vga_ball_fd;

/* Read and print the background color */
void print_background_color() {
  vga_ball_arg_t vla;
  
  if (ioctl(vga_ball_fd, VGA_BALL_READ_BACKGROUND, &vla)) {
      perror("ioctl(VGA_BALL_READ_BACKGROUND) failed");
      return;
  }
  printf("%04x %04x %04x\n",
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


bool move_ball(int *a, int *b, vga_ball_position_t *position)
{
  vga_ball_arg_t vla;
  int new_x, new_y;
  if (ioctl(vga_ball_fd, VGA_BALL_GET_POSITION, &vla)) {
      perror("ioctl(VGA_BALL_GET_POSITION) failed");
      return false;
  }
  unsigned short x, y;
  x = vla.position.x;
  y = vla.position.y;
  int flag = 0;
  new_x = x + *a;
  new_y = y + *b;
  while (new_x>625 || new_x<15 || new_y>465 || new_y<15) {
      if (new_x > 625) {
          new_x = 625*2 - new_x;
          *a = *a * -1;
          flag = 1;
      }
      else if (new_x < 15) {
          new_x = 15*2 -new_x;
          *a = *a * -1;
          flag = 1;
      }
      if (new_y > 465) {
          new_y = 465*2 - new_y;
          (*b) = (*b) * -1;
          flag = 1;
      }
      else if (new_y < 15) {
          new_y = 15*2 - new_y;
          *b = *b * -1;
          flag = 1;
      }
  }
  
  vla.position.x = new_x;
  vla.position.y = new_y;
  if (ioctl(vga_ball_fd, VGA_BALL_SET_POSITION, &vla)) {
      perror("ioctl(VGA_BALL_SET_POSITION) failed");
      return false;
  }
  position->x = x;
  position->y = y;
  return flag;
}
// void print_ball(vga_ball_position_t *position)
// {
//   vga_ball_arg_t vla;
//   if (ioctl(vga_ball_fd, VGA_BALL_GET_POSITION, &vla)) {
//       perror("ioctl(VGA_BALL_GET_POSITION) failed");
//       return;
//   }
//   printf("Ball position: %04x %04x\n", vla.position.x, vla.position.y);
// }

void read_ball_position(vga_ball_position_t *position)
{
  vga_ball_arg_t vla;
  if (ioctl(vga_ball_fd, VGA_BALL_GET_POSITION, &vla)) {
      perror("ioctl(VGA_BALL_GET_POSITION) failed");
      return;
  }
  position->x = vla.position.x;
  position->y = vla.position.y;
}

int main()
{
  srand(time(NULL));  // 初始化随机数种子（只执行一次）
  int a = rand() % 3+1;
  int b = rand() % 3+1;
  vga_ball_arg_t vla;
  int i;
  static const char filename[] = "/dev/vga_ball";

  static const vga_ball_color_t colors[] = {
    { 0xff, 0x00, 0x00 }, /* Red */
    { 0x00, 0xff, 0x00 }, /* Green */
    { 0x00, 0x00, 0xff }, /* Blue */
    { 0xff, 0xff, 0x00 }, /* Yellow */
    { 0x00, 0xff, 0xff }, /* Cyan */
    { 0xff, 0x00, 0xff }, /* Magenta */
    { 0x80, 0x80, 0x80 }, /* Gray */
    { 0x00, 0x00, 0x00 }, /* Black */
    // { 0xff, 0xff, 0xff }  /* White */
  };

# define COLORS 8

  printf("VGA ball Userspace program started\n");

  if ( (vga_ball_fd = open(filename, O_RDWR)) == -1) {
    fprintf(stderr, "could not open %s\n", filename);
    return -1;
  }
  vga_ball_position_t position;
  printf("initial state: \n");
  print_background_color();
  read_ball_position(&position);
  printf("Initial position: %04x %04x\n", position.x, position.y);
  position.x = position.x - a;
  position.y = position.y - b;
  unsigned char bouncing = 0;
  while (bouncing < 24) {
    if (move_ball(&a, &b, &position)) {
      printf("Ball bounced! New position: %04x %04x\n", position.x, position.y);
      set_background_color(&colors[bouncing % COLORS]);
      print_background_color();
      bouncing++;
    }
    else {
      printf("Ball moved! New position: %04x %04x\n", position.x, position.y);
    }
    usleep(75000);
  }

  // boundry check
  // print_ball(&position);
  // usleep(400000);
  // position.x = 640;
  // position.y = 480;
  // print_ball(&position);
  // usleep(400000);
  // position.x = 0;
  // position.y = 0;
  // print_ball(&position);
  // usleep(400000);
  // position.x = 640;
  // position.y = 0;
  // print_ball(&position);
  // usleep(400000);
  // position.x = 0;
  // position.y = 480;
  // print_ball(&position);
  // usleep(400000);

  // for (i = 0 ; i < 24 ; i++) {
  //   set_background_color(&colors[i % COLORS ]);
  //   print_background_color();
  //   change_ball_position();
  //   usleep(400000);
  // }

  // set_background_color(&colors[0]); // set to first color
  // printf("First state: RED\n");
  // print_background_color();

  // usleep(400000);
  // set_background_color(&colors[1]); // set to second color
  // printf("Second state: GREEN\n");
  // print_background_color();

  // usleep(400000);
  // set_background_color(&colors[7]); // set to seventh color
  // printf("Seventh state: BLACK\n");
  // print_background_color();
  // usleep(400000);
  // set_background_color(&colors[8]); // set to eighth color
  // printf("Eighth state: WHITE\n");
  // print_background_color();
  // usleep(400000);
  
  printf("VGA BALL Userspace program terminating\n");
  return 0;
}
