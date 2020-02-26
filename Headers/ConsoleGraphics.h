
#ifndef ConsoleGraphics
#define ConsoleGraphics

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "Simulation_Constants.h"

#define SCREEN_WIDTH 85 /* Rendered width in characters */
#define SCREEN_HEIGHT 85 /* Rendered height in characters */

#define LANE_COUNT 4
#define LANE_WIDTH 5
#define LANE_LENGTH 30

#define RENDERED_CAR_LENGTH 3
#define RENDERED_CAR_WIDTH 2

#define METERS_PER_PIXEL 1.25 /* Amount of meters represented by a single char */

/* Color codes for signals */
#define KNRM  "\x1B[0m"
const char *signal_color_code[] = {"\x1B[31m", "\x1B[33m", "\x1B[33m", "\x1B[32m"};

/* Car orientation */
enum orientation{
  horizontal, vertical
};

/* Array of 'pixels' that are represented as characters and drawn in console */
char pixels[SCREEN_HEIGHT][SCREEN_WIDTH + 1];

/* Manipulating characters */
void clear_pixels();
void draw_intersection();
void draw_square(int x, int y, int width, int height, char c);
void draw_dotted_square(int x, int y, int width, int height, char c);
void draw_line(int x, int y, int length, int dir, int dotted);

void draw_car(char *road_name, double position, int lane);
void draw_car_shape(int x, int y, int facing_dir);
void draw_car_counts(street *streets);
void insert_number(int number, int x, int y, int offset);

void print_pixels(simulation_state sim_state);
void print_colored_pixels(char *string, const char *color, int x1, int x2, int str_len);
void print_colored_line(char *string, const char *color1, int x1, int x2, const char *color2, int x3, int x4);
const char *get_lane_color_code(int current_signal, lane l);

void clear_console();

/* Sets all 'pixels' to a space ' ' */
void clear_pixels(){
  int i, j;
  for(i = 0; i < SCREEN_HEIGHT; i++){
    for(j = 0; j < SCREEN_WIDTH; j++){
      pixels[i][j] = ' ';
    }
    pixels[i][SCREEN_WIDTH] = '\0';
  }
}

/* Draws the traffic grid */
void draw_intersection(){
  int i, total_length = 2 * LANE_LENGTH + LANE_COUNT * LANE_WIDTH;

  for(i = 1; i < LANE_COUNT - 1; i++){
    int dotted = (i == 1 || i == 3);
    /* Draw the lines between each lane */
    draw_line(LANE_LENGTH + (i * LANE_WIDTH), 0, LANE_LENGTH, vertical, dotted);
    draw_line(LANE_LENGTH + ((LANE_COUNT - i) * LANE_WIDTH), LANE_LENGTH + (LANE_WIDTH * LANE_COUNT), LANE_LENGTH, vertical, dotted);

    draw_line(0, LANE_LENGTH + ((LANE_COUNT - i) * LANE_WIDTH), LANE_LENGTH, horizontal, dotted);
    draw_line(LANE_LENGTH + (LANE_WIDTH * LANE_COUNT),LANE_LENGTH + (i * LANE_WIDTH), LANE_LENGTH, horizontal, dotted);
  }
  /* Draw the long intersecting lines that include stop lines */
  draw_line(LANE_LENGTH, 0, total_length, vertical, 0);
  draw_line(LANE_LENGTH + LANE_WIDTH * 4, 0, total_length, vertical, 0);

  draw_line(0, LANE_LENGTH, total_length, horizontal, 0);
  draw_line(0, LANE_LENGTH + (LANE_COUNT * LANE_WIDTH), total_length, horizontal, 0);
}

/* Draws a square in the pixels array with the given coordinates and dimensions */
void draw_square(int x, int y, int width, int height, char c){
  int i, j;
  for(i = y; i < height; i++){
    if(i >= SCREEN_HEIGHT || i < 0) continue;
    for(j = x; j < width; j++){
      if(j >= SCREEN_WIDTH || j < 0) continue;
      pixels[i][j] = c;
    }
  }
}

/* Draw a square where every other line is missing */
void draw_dotted_square(int x, int y, int width, int height, char c){
  int i, j;
  for(i = y; i < height; i++){
    if(i >= SCREEN_HEIGHT || i < 0) continue;
    for(j = x; j < width; j++){
      if(j >= SCREEN_WIDTH || j < 0) continue;
      pixels[i][j] = c;
    }
  }
}

/* Draws a line with width 1 */
void draw_line(int x, int y, int length, int dir, int dotted){
  int i;
  if(dir == horizontal){
    for(i = 0; i < length; i++){
      /* Skip every other character if dotted */
      if(dotted && i % 2 == 0) continue;

      pixels[y][x + i] = '-';
    }
  }else{
    for(i = 0; i < length; i++){
      /* Skip every other character if dotted */
      if(dotted && i % 2 == 0) continue;

      pixels[y + i][x] = '|';
    }
  }
}


/* Draw a car given its position and lane */
void draw_car(char *road_name, double position, int lane){
  int distance = (int) (position / METERS_PER_PIXEL);
  int lane_offset = 2 + ((lane == left_lane) ? LANE_WIDTH : 0);

  if(strcmp(road_name, street_names[0]) == 0)
  draw_car_shape(LANE_LENGTH + lane_offset, LANE_LENGTH - distance, south);

  else if(strcmp(road_name, street_names[1]) == 0)
  draw_car_shape(LANE_LENGTH + (LANE_COUNT * LANE_WIDTH) + distance + 1, LANE_LENGTH + lane_offset, west);

  else if(strcmp(road_name, street_names[2]) == 0)
  draw_car_shape(LANE_LENGTH + (4 * LANE_WIDTH) - lane_offset - 1, LANE_LENGTH + (LANE_COUNT * LANE_WIDTH) + distance + 1, north);

  else if(strcmp(road_name, street_names[3]) == 0)
  draw_car_shape(LANE_LENGTH - distance, LANE_LENGTH + (4 * LANE_WIDTH) - lane_offset - 1, east);

}

/* Draws 2x3 shape of 'x' characters */
void draw_car_shape(int x, int y, int facing_dir){
  switch(facing_dir){
    case(north):
    draw_square(x, y, x + RENDERED_CAR_WIDTH, y + RENDERED_CAR_LENGTH, 'X');
    break;
    case(south):
    draw_square(x, y - RENDERED_CAR_LENGTH, x + RENDERED_CAR_WIDTH, y, 'X');
    break;
    case(east):
    draw_square(x -  RENDERED_CAR_LENGTH, y, x, y + RENDERED_CAR_WIDTH, 'X');
    break;
    case(west):
    draw_square(x, y, x + RENDERED_CAR_LENGTH, y + RENDERED_CAR_WIDTH, 'X');
    break;
  }
}

/* Inserts number of cars next to each lane */
void draw_car_counts(street *streets){
  insert_number(streets[0].lanes[straight_right_lane].amount_of_cars, LANE_LENGTH - 4, 0, 0);
  insert_number(streets[0].lanes[left_lane].amount_of_cars, LANE_LENGTH + 2 * LANE_WIDTH + 1, 0, 1);

  insert_number(streets[2].lanes[left_lane].amount_of_cars, LANE_LENGTH + 2 * LANE_WIDTH - 4, LANE_LENGTH * 2 + LANE_WIDTH * 4 - 1, 0);
  insert_number(streets[2].lanes[straight_right_lane].amount_of_cars, LANE_LENGTH + 4 * LANE_WIDTH + 1, LANE_LENGTH * 2 + LANE_WIDTH * 4 - 1, 1);

  insert_number(streets[1].lanes[left_lane].amount_of_cars, 2 * LANE_LENGTH + 4 * LANE_WIDTH - 4, LANE_LENGTH + 2 * LANE_WIDTH + 1, 0);
  insert_number(streets[1].lanes[straight_right_lane].amount_of_cars, 2 * LANE_LENGTH + 4 * LANE_WIDTH - 4, LANE_LENGTH - 1, 0);

  insert_number(streets[3].lanes[left_lane].amount_of_cars, 0, LANE_LENGTH + 2 * LANE_WIDTH - 1, 1);
  insert_number(streets[3].lanes[straight_right_lane].amount_of_cars, 0, LANE_LENGTH + 4 * LANE_WIDTH + 1, 1);
}

/* Inserts a given number at x and y position */
void insert_number(int number, int x, int y, int offset){
  char str[] = "000";
  if(offset){
    sprintf(str, "%-3d", number);
  }else{
    sprintf(str, "%3d", number);
  }
  pixels[y][x] = str[0];
  pixels[y][x + 1] = str[1];
  pixels[y][x + 3] = str[2];
}


/* Prints the array of pixels with colored stop lines indicating signal colors */
void print_pixels(simulation_state sim_state){
  int i;
  const char *col1, *col2;
  for(i = 0; i < SCREEN_HEIGHT; i++){
    /* Print lines with colors according to current signal color */
    if(i == LANE_LENGTH){
      /* North stop lines */
      col1 = get_lane_color_code(sim_state.current_signal_state, sim_state.streets[0].lanes[1]);
      col2 = get_lane_color_code(sim_state.current_signal_state, sim_state.streets[0].lanes[0]);
      print_colored_line(pixels[i], col1, LANE_LENGTH, LANE_LENGTH + LANE_WIDTH, col2, LANE_LENGTH + LANE_WIDTH, LANE_LENGTH + 2 * LANE_WIDTH);

    }else if(i > LANE_LENGTH && i < LANE_LENGTH + 2 * LANE_WIDTH){
      /* East stop lines */
      int lane_num = (((i - LANE_LENGTH) / LANE_WIDTH) + 1) % 2;
      col1 = get_lane_color_code(sim_state.current_signal_state, sim_state.streets[1].lanes[lane_num]);
      print_colored_pixels(pixels[i], col1, LANE_LENGTH + 4 * LANE_WIDTH, LANE_LENGTH + 4 * LANE_WIDTH + 1, SCREEN_WIDTH);
      printf("\n");

    }else if(i > LANE_LENGTH + 2 * LANE_WIDTH && i < LANE_LENGTH + 4 * LANE_WIDTH){
      /* West stop lines */
      int lane_num = (i - (LANE_LENGTH + LANE_WIDTH * 2)) / LANE_WIDTH;
      col1 = get_lane_color_code(sim_state.current_signal_state, sim_state.streets[3].lanes[lane_num]);
      print_colored_pixels(pixels[i], col1, LANE_LENGTH, LANE_LENGTH + 1, SCREEN_WIDTH);
      printf("\n");

    }else if(i == LANE_LENGTH + LANE_WIDTH * 4){
      /* South stop lines */
      col1 = get_lane_color_code(sim_state.current_signal_state, sim_state.streets[2].lanes[0]);
      col2 = get_lane_color_code(sim_state.current_signal_state, sim_state.streets[2].lanes[1]);
      print_colored_line(pixels[i], col1, LANE_LENGTH + 2 * LANE_WIDTH, LANE_LENGTH + 3 * LANE_WIDTH, col2, LANE_LENGTH + 3 * LANE_WIDTH, LANE_LENGTH + 4 * LANE_WIDTH);

    }else{
      printf("%s\n", pixels[i]);
    }
  }
}

/* Print a line with a given color starting a x1 and ending at x2 */
void print_colored_pixels(char *string, const char *color, int x1, int x2, int str_len){
  int len = x2 - x1;
  printf("%.*s" "%s" "%.*s" KNRM "%.*s", x1, string, color,  len, string + x1, (str_len - x2), string + x2);
}

/* Print a line with multiple colored sections */
void print_colored_line(char *string, const char *color1, int x1, int x2, const char *color2, int x3, int x4){
  print_colored_pixels(string, color1, x1, x2, x2);
  print_colored_pixels(string + x2, color2, x3 - x2, x4 - x2, SCREEN_WIDTH - x2);
  printf("\n");
}

/* Returns a color code corresponding to the current signal color in a given lane */
const char *get_lane_color_code(int current_signal, lane l){
  return signal_color_code[get_signal_color(current_signal, l.lane_direction)];
}


void clear_console(){
  system("cls");
}


#endif
