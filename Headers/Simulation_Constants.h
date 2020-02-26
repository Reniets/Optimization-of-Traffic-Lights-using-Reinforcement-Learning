#ifndef SimulationConstants /* Include guard */
#define SimulationConstants

/* ---------------Constants related to the intersection model---------------- */
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#define RAND_SEED 29707329 /* Seed for the random number generator used for spawning cars */

#define MAX_SIM_DAYS 14 /* Maximum amount of days worth of data saved */

#define AMOUNT_OF_STREETS 4
#define LANES_PER_STREET 2

#define MAX_NAME_LENGTH 20 /* Max length of streetnames */

#define MAX_SPAWNED_CARS 10
#define MAX_AMOUNT_OF_CARS 200 /* Max amount of cars per lane */

#define AMOUNT_OF_SIGNAL_STATES 6
#define AMOUNT_OF_SIGNAL_DIRECTIONS 4 /* Amount of lanes with signal light */

#define MAX_GREEN_TIME 120.0
#define MIN_GREEN_TIME 13.0
#define MAX_YELLOW_TIME 4.0

#define DATA_POINT_INTERVAL 60  /* Seconds between saved datapoints */

#define DAILY_DATA_POINTS (3600 * 24) / DATA_POINT_INTERVAL

typedef struct signal_state signal_state;
typedef struct street street;
typedef struct lane lane;
typedef struct car car;
typedef struct simulation_state simulation_state;
typedef struct statistics statistics;

struct car{
  int direction; /* The direction the car turns */
  int active; /* Whether the car is in the current simulation */
  double speed; /* Current speed measured in m/s */
  double position; /* Meters from being resolved */
  double wait_time; /* Time from spawn to being removed */
};

struct lane{
  char street_name[MAX_NAME_LENGTH];
  int lane_type; /* Left lane or right lane */
  int lane_direction;
  int index_front_car; /* Index of the foremost car in the array of cars */
  int amount_of_cars; /* Total amount of active cars in this lane */
  car cars[MAX_AMOUNT_OF_CARS];
};

struct street{
  char name[MAX_NAME_LENGTH]; /* Name of the street the car spawns on */
  lane lanes[LANES_PER_STREET];
};

/* Statistics used for  */
struct statistics{
  int gathered_data_points, gathered_car_data_points, total_cars_passed,
      cars_passed[AMOUNT_OF_STREETS][LANES_PER_STREET], max_queue_length,
      cars_passed_over_time[DAILY_DATA_POINTS];

  double time_since_data_save, time_passed, total_wait_time, max_wait_time,
         lane_wait_time[AMOUNT_OF_STREETS][LANES_PER_STREET],
         accumulated_wait_time[DAILY_DATA_POINTS];
};

/* Current simulation data */
struct simulation_state{
  street streets[AMOUNT_OF_STREETS];
  double current_time, time_since_change, last_spawn_time, time_scale;
  int current_signal_state, resolved_cars, render_simulation, days_simulated, sim_car_count;
  statistics *stats;
};

enum lane_type{
  left_lane, straight_right_lane
};

enum cardinal_direction{
  north, east, south, west
};

/* Lane directions that share the same signal */
enum signal_directions{
    north_south, north_south_left, east_west, east_west_left
};

enum signal_colors{
  red, yellow_to_red, yellow_to_green, green
};

/* Format : northsoth_eastwest */
enum signal_state_label{
  yg_r, g_r, yr_r, r_yg, r_g, r_yr
};

/* Signal states of the format :
{north_south, north_south_left, east_west, east_west_left} */
const int signals[AMOUNT_OF_SIGNAL_STATES][AMOUNT_OF_SIGNAL_DIRECTIONS] = {
     {yellow_to_green, red, red, red},
     {green, red, red, red},
     {yellow_to_red, red, red, red},

     {red, red, yellow_to_green, red},
     {red, red, green, red},
     {red, red, yellow_to_red, red}
 };

const char *street_names[] = {"Kjellerupsgade","Fyensgade","Soenderbro","Jyllandsgade"};

/* Functions for initializing structs */
void initialize_streets(street *streets);
simulation_state make_simulation_state();
street make_street(const char *streetname);
lane make_lane(const char *streetname, int lane_type);
car make_car(int direction, int speed, int position, int active);
void discard_simulation(simulation_state *sim_state);

int get_lane_direction(const char *street_name, int lane_type);
int get_opposing_street(const char *street_name);
const char* get_street_name(int street_id);
int get_signal_color(int current_signal_state, int lane_direction);
int get_street_index(char *street_name);
int is_yellow(int current_signal_state);


/* Returns direction of a lane based on the street name and lane type */
int get_lane_direction(const char *street_name, int lane_type){
  if(strcmp(street_name, street_names[0]) == 0 || strcmp(street_name, street_names[2]) == 0){
    if(lane_type == straight_right_lane)
      return north_south;
    else
      return north_south_left;

  }else{
    if(lane_type == straight_right_lane)
      return east_west;
    else
      return east_west_left;
  }
  return -1;
}

/* Returns index of street opposite to given street */
int get_opposing_street(const char *street_name){
  int i;
  for(i = 0; i < AMOUNT_OF_STREETS; i++){
    if(strcmp(street_name, street_names[i]) == 0)
      return ((i + 2) % 4);

  }
  return -1;
}

/* Returns the name of a street given its' index */
const char* get_street_name(int street_id){
  if(street_id >= 0 && street_id < AMOUNT_OF_STREETS)
    return street_names[street_id];
  return "";
}

/* Returns the current of color of the signal for a given lane */
int get_signal_color(int current_signal_state, int lane_direction){
  return signals[current_signal_state][lane_direction];
}

/* Returns true if the current signal is yellow in any direction */
int is_yellow(int current_signal_state){
  int i;
  for(i = 0; i < AMOUNT_OF_SIGNAL_DIRECTIONS; i++){
    int cur_sig = signals[current_signal_state][i];
    if(cur_sig == yellow_to_red || cur_sig == yellow_to_green){
      return 1;
    }
  }

  return 0;
}

/* Return index of a street given its' name */
int get_street_index(char *street_name){
  int i;
  for(i = 0; i < AMOUNT_OF_STREETS; i++){
    if(strcmp(street_name, street_names[i]) == 0)
      return i;
  }
  return 0;
}

/* Returns a simulation with default values */
simulation_state make_simulation_state(){
  int i, j;
  simulation_state new_sim;
  initialize_streets(new_sim.streets);
  new_sim.current_time = 0;
  new_sim.time_since_change = 0;
  new_sim.current_signal_state = r_g;
  new_sim.resolved_cars = 0;
  new_sim.render_simulation = 1;
  new_sim.days_simulated = 0;
  new_sim.sim_car_count = 0;
  new_sim.time_scale = 1;
  new_sim.last_spawn_time = 0;

  /* Allocate memory for statistic storage */
  new_sim.stats = (statistics *) malloc(sizeof(statistics) * MAX_SIM_DAYS);

  /* Initialize statistics structs for all days */
  for(j = 0; j < MAX_SIM_DAYS; j++){
    new_sim.stats[j].time_since_data_save = 0;
    new_sim.stats[j].time_passed = 0;
    new_sim.stats[j].total_wait_time = 0;
    new_sim.stats[j].max_wait_time = 0;
    new_sim.stats[j].total_cars_passed = 0;
    new_sim.stats[j].max_queue_length = 0;
    new_sim.stats[j].gathered_data_points = 0;

    for(i = 0; i < AMOUNT_OF_STREETS; i++){
      new_sim.stats[j].lane_wait_time[i][left_lane] = 0;
      new_sim.stats[j].lane_wait_time[i][straight_right_lane] = 0;
      new_sim.stats[j].cars_passed[i][left_lane] = 0;
      new_sim.stats[j].cars_passed[i][straight_right_lane] = 0;
    }
  }

  /* Initialize randomness used for spawning cars */
  srand(RAND_SEED);
  rand();

  return new_sim;
}

/* Initilizes street structs */
void initialize_streets(street streets[AMOUNT_OF_STREETS]){
  int i;
  for ( i = 0; i < AMOUNT_OF_STREETS; i++) {
    streets[i] = make_street(street_names[i]);
  }
}

street make_street(const char *streetname){
  street new_street;
  strcpy(new_street.name, streetname);

  new_street.lanes[straight_right_lane] = make_lane(streetname, straight_right_lane);
  new_street.lanes[left_lane] = make_lane(streetname, left_lane);

  return new_street;
}

lane make_lane(const char *streetname, int lane_type){
  int i;
  lane new_lane;
  strcpy(new_lane.street_name, streetname);
  new_lane.lane_type = lane_type;
  new_lane.lane_direction = get_lane_direction(streetname, lane_type);
  new_lane.index_front_car = 0;
  new_lane.amount_of_cars = 0;
  for(i = 0; i < MAX_AMOUNT_OF_CARS; i++)
    new_lane.cars[i] = make_car(0, 0, 0, 0);

  return new_lane;
}

car make_car(int direction, int speed, int position, int active){
  car new_car;
  new_car.direction = direction;
  new_car.speed = speed;
  new_car.position = position;
  new_car.active = active;
  new_car.wait_time = 0;
  return new_car;
}

/* Frees allocated memory in the given sim state */
void discard_simulation(simulation_state *sim_state){
    free(sim_state->stats);
}


#endif /* SimulationConstant */
