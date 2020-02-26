#ifndef Simulering /* Include guard */
#define Simulering

#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <string.h>
#include <math.h>

#include "ConsoleGraphics.h"
#include "Simulation_Constants.h"

/* ------------------------------------ Adjustable time constants ------------------------------------ */
#define TICK_RATE 10 /* Amount of times to update simulation per. simulated sceond */
#define MILLISEC_PER_TICK 1000 / TICK_RATE

#define SPAWN_INTERVAL 5.0  /* Simulated time between each potential car spawn */

#define MAX_FRAME_RATE 10.0 /* Maximum rendered frames per second */
#define MAX_MILLIS_PER_FRAME 1000 / MAX_FRAME_RATE
#define MAX_SKIPPED_FRAMES 5 /* Maximum amount of simulation physics updates between each rendered frame */

 /* Seed for random number generator used for spawning cars */

/* --------------------------------------- Simulation Constants --------------------------------------- */
#define CAR_LENGTH 4 /* In meters */
#define SAFTETY_DISTANCE 1 /* Distance kept between cars */


#define AMOUNT_OF_STREETS 4
#define MAX_NAME_LENGTH 20
#define MAX_SPAWNED_CARS 10


#define MAX_SPEED 14 /* Measured in meters/second - 50km/h */
#define CAR_SPAWN_POSITION 200 /* Meters from stop line to spawn cars */
#define DESPAWN_POSITION -20.0 /* Default position where right/straight lane cars despawn */
#define LEFT_DESPAWN_POSITION -10.0 /* Position where left lane cars stop and despawn */

/* Mathematical constants */
#define E_C 2.71828
#define SEC_PER_HOUR 3600



/* Functions for updating the simulation */
void update_simulation(simulation_state *sim_state, double time_step, int new_signal);
void tick(simulation_state *sim_state);

/* Update functions for individual cars */
void update_car(simulation_state *sim_State, lane *current_lane, int car_index);
void move_car(simulation_state *sim_state, lane *current_lane, int current_car_index, int car_in_front_index);
void accelerate_car(lane *current_lane, int car_index, int car_in_front_index);
double get_acceleration(double current_speed);
void remove_car(simulation_state *sim_state, lane *current_lane, int car_index);

/* Car spawning functions */
void spawn_cars(simulation_state *sim_state);
int get_car_spawn_count(double current_time, street road);
void add_car(lane *l);

/* Math functions */
int factorial(int a);
double poisson_probability(double k, double lambda);
double get_probability(double current_time, street road, int k);


/* Functions for determining the spawn rate of cars at a given time */
double get_soenderbro_spawn_rate(int time_step);
double get_kjellerup_spawn_rate(int time_step);
double get_jylland_spawn_rate(int time_step);
double get_fyensgade_spawn_rate(int time_step);

/* Graphics */
void render(simulation_state sim_state);

/* Misc */
void change_signal(simulation_state *sim_state, int new_signal);
int are_green_lanes_empty(simulation_state sim_state);
int are_all_lanes_empty(simulation_state sim_state);
void update_statistics(simulation_state *sim_state);



/* Runs simulation for 'time_step' amount of seconds. Changes current signal to new_signal  */
void update_simulation(simulation_state *sim_state, double time_step, int new_signal){
  int skipped_frames = 0, frame_count = 0, simulation_running = 1, tick_count = 0, ticks_per_timestep = TICK_RATE * time_step;
  clock_t next_tick_time = clock(), last_render_time = clock();

  /* Change signal if needed */
  change_signal(sim_state, new_signal);

  if(!sim_state->render_simulation){
    int i;
    for(i = 0; i < ticks_per_timestep; i++){
      tick(sim_state);
    }
    return;
  }

  /* Simulate the given timeframe  */
  /* Run in a loop that ensures the appropriate amount of ticks and renders per second */
  while(simulation_running){
    /* Update simulation logic enough times to meet tickrate requirements */
    while(clock() > next_tick_time && skipped_frames < MAX_SKIPPED_FRAMES){
      tick(sim_state);
      tick_count++;
      skipped_frames ++;

      /* Determine when to tick() next, taking time scale into account */
      next_tick_time += (MILLISEC_PER_TICK) / sim_state->time_scale;

      /* End the current simulation if the given timeframe has been simulated */
      if(tick_count > ticks_per_timestep){
        simulation_running = 0;
        break;
      }
    }
    skipped_frames = 0;

    /* Render display if neccesary */
    if(clock() - last_render_time > MAX_MILLIS_PER_FRAME){
      int sim_seconds = (int) sim_state->current_time;
      last_render_time = clock();
      render(*sim_state);
      frame_count++;

      /* Prints simulation time*/
      printf("Time : %02d:%02d:%02d\n", sim_seconds / (3600),  (sim_seconds / (60)) % 60, sim_seconds % 60);
    }
  }
}

/* Update simulation logic */
void tick(simulation_state *sim_state){
  street *streets = sim_state->streets;
  int i, j, k, day;

  /* Check if the yellow period has been exceeded and change signal if so */
  if(is_yellow(sim_state->current_signal_state)){
    if(sim_state->time_since_change > MAX_YELLOW_TIME)
      change_signal(sim_state, 1);

  }else if(are_all_lanes_empty(*sim_state)){
    /* Remove switching limitations if all lanes are empty */
    sim_state->time_since_change = MIN_GREEN_TIME;
  }

  /* Spawn cars if needed */
  if((sim_state->current_time - sim_state->last_spawn_time) > SPAWN_INTERVAL){
    spawn_cars(sim_state);
    sim_state->last_spawn_time = sim_state->current_time;
  }

  /* Update cars in all lanes */
  for(i = 0; i < AMOUNT_OF_STREETS; i++){
    for(j = 0; j < LANES_PER_STREET; j++){
      int car_count = streets[i].lanes[j].amount_of_cars;
      int front_car_ind = streets[i].lanes[j].index_front_car;

      for(k = front_car_ind; k < (car_count + front_car_ind); k++){
        update_car(sim_state, &(streets[i].lanes[j]), k % MAX_AMOUNT_OF_CARS);
      }
    }
  }

  day = sim_state->days_simulated;

  /* Update time variables */
  sim_state->current_time += (1.0 / TICK_RATE);
  sim_state->time_since_change += (1.0 / TICK_RATE);
  sim_state->stats[day].time_since_data_save += (1.0 / TICK_RATE);
  sim_state->stats[day].time_passed += (1.0 / TICK_RATE);

  /* Save statistics every minute */
  if(sim_state->stats[day].time_since_data_save > DATA_POINT_INTERVAL){
    update_statistics(sim_state);
    sim_state->stats[day].time_since_data_save -= DATA_POINT_INTERVAL;
  }

  /* Check if current simulated time exceeds a day */
  if(sim_state->current_time > (3600.0 * 24.0)){
    sim_state->current_time -= (3600.0 * 24.0);
    sim_state->last_spawn_time -= (3600.0 * 24.0);
    printf("\nSimulated day : %d\n", sim_state->days_simulated);
    sim_state->days_simulated += 1;
  }
}

/* Change current signal if needed */
void change_signal(simulation_state *sim_state, int new_signal){
  /* Don't change if yellow and within the 4 seconds minimum yellow time */
  if(new_signal && !(is_yellow(sim_state->current_signal_state) && sim_state->time_since_change < MAX_YELLOW_TIME)){
    sim_state->time_since_change = 0;
    sim_state->current_signal_state += 1;
    sim_state->current_signal_state %= AMOUNT_OF_SIGNAL_STATES;
  }
}

/* Displays current state with console graphics */
void render(simulation_state sim_state){
  int i, j, k;
  clear_pixels();
  draw_intersection();

  /* Render cars in all lanes */
  for(i = 0; i < AMOUNT_OF_STREETS; i++){
    for(j = 0; j < LANES_PER_STREET; j++){
      int start_index = sim_state.streets[i].lanes[j].index_front_car;
      for(k = start_index; k < sim_state.streets[i].lanes[j].amount_of_cars + start_index; k++)
        draw_car(sim_state.streets[i].name, sim_state.streets[i].lanes[j].cars[k % MAX_AMOUNT_OF_CARS].position, j);
    }
  }

  /* Print number of cars in each lane */
  draw_car_counts(sim_state.streets);

  /* Clear console and render graphics */
  clear_console();
  print_pixels(sim_state);
}


/* Updates simulation for a single car */
void update_car(simulation_state *sim_state, lane *current_lane, int car_index){
  car *current_car = &(current_lane->cars[car_index]);
  int car_in_front_index = -1;

  /* Find index of the car in front of this one */
  if(current_lane->amount_of_cars > 1 && car_index != current_lane->index_front_car){
    car_in_front_index = (car_index - 1) % MAX_AMOUNT_OF_CARS;
  }

  /* Update speed and position of the car */
  accelerate_car(current_lane, car_index, car_in_front_index);
  move_car(sim_state, current_lane, car_index, car_in_front_index);

  /* Remove cars if possible */
  if(current_car->position <= DESPAWN_POSITION){
    remove_car(sim_state, current_lane, car_index);
  }else{
    current_car->wait_time += 1.0 / TICK_RATE;
  }
}

/* Update speed of the car */
void accelerate_car(lane *current_lane, int car_index, int car_in_front_index){
  car *current_car, car_in_front;
  double front_car_distance, acceleration = 0.0;
  current_car = &(current_lane->cars[car_index]);

  /* If there is no car in front of this car set acceleration to a default value */
  if(car_in_front_index == -1){
    acceleration = (get_acceleration(current_car->speed));
  }else{
    /* Calculate distance to car in front */
    car_in_front = current_lane->cars[car_in_front_index];
    front_car_distance = current_car->position - (car_in_front.position + CAR_LENGTH);

    if(front_car_distance > SAFTETY_DISTANCE + 1.0)
      acceleration = get_acceleration(current_car->speed);
  }

  /* Match acceleration to the simulated time step */
  acceleration /= TICK_RATE;

  /* Make sure car does not accelerate above max speed */
  if((current_car->speed + acceleration) >= MAX_SPEED){
    acceleration = MAX_SPEED - current_car->speed;
  }else if(current_car->speed + acceleration < 0.0){
    acceleration = 0 - current_car->speed;
  }

  current_car->speed += acceleration;
}

/* Updates the position of the car */
void move_car(simulation_state *sim_state, lane *current_lane, int current_car_index, int car_in_front_index){
  car *current_car, car_in_front;
  double move_dist;
  int lane_signal = get_signal_color(sim_state->current_signal_state, current_lane->lane_direction);

  current_car = &(current_lane->cars[current_car_index]);
  move_dist = current_car->speed / TICK_RATE;

  /* If there is a car in front */
  if(car_in_front_index != -1){
    double front_car_distance;
    car_in_front = current_lane->cars[car_in_front_index];

    front_car_distance = (current_car->position - move_dist) - (car_in_front.position + CAR_LENGTH);

    if(front_car_distance <= SAFTETY_DISTANCE){
      current_car->speed = car_in_front.speed * 0.8;
      current_car->position = car_in_front.position + SAFTETY_DISTANCE + CAR_LENGTH;
      return;
    }
  }


  if(lane_signal == green || (lane_signal == yellow_to_green && sim_state->time_since_change > 1) || (lane_signal == yellow_to_red && sim_state->time_since_change < 2) || current_car->position < 0){
    /* Stop left lane drivers in the middle of the intersection*/
    if(current_lane->lane_type == left_lane && (current_car->position - move_dist) < LEFT_DESPAWN_POSITION){
      move_dist = current_car->position - LEFT_DESPAWN_POSITION;
      current_car->speed = 0;
    }

  }else{
    if((current_car->position - move_dist) < 0){
      move_dist = current_car->position;
      current_car->speed = 0;
    }
  }

  current_car->position -= move_dist;

  /* Hvis current_car->speed > fornt_car_speed - current_car_speed =  front_car_speed*/

}


/* Spawns cars in all streets */
void spawn_cars(simulation_state *sim_state){
  int i, j, spawned_cars;

  /* Loop through all streets */
  for(i = 0; i < AMOUNT_OF_STREETS; i++){
    /* Get amount of cars that should spawn in this street */
    spawned_cars = get_car_spawn_count(sim_state->current_time, sim_state->streets[i]);

    /* Spawn all the cars needed */
    for(j = 0; j < spawned_cars; j++){
      int spawn_lane = straight_right_lane, day = sim_state->days_simulated;

      /* Add new car to the simulation */
      add_car(&(sim_state->streets[i].lanes[spawn_lane]));

      /* Update max queue length statistics if applicable */
      if(sim_state->stats[day].max_queue_length < sim_state->streets[i].lanes[spawn_lane].amount_of_cars){
        sim_state->stats[day].max_queue_length = sim_state->streets[i].lanes[spawn_lane].amount_of_cars;
      }
    }
  }
}

/* Return amount of cars that should be spawned on the given road in the next spawn interval seconds */
int get_car_spawn_count(double current_time, street road){
  int car_count = 0, i;
  double sum_probability[MAX_SPAWNED_CARS], rand_num;

  /* Store probability of 1, 2 ... 10 cars spawning in the next time step */
  /* sum_probability[0] is the probability of 0 cars spawning */
  sum_probability[0] = get_probability(current_time, road, 0);

  /* sum_probability[i] is the probability of i cars spawning */
  for(i = 1; i < MAX_SPAWNED_CARS; i++){
    sum_probability[i] = sum_probability[i - 1] + get_probability(current_time, road, i);
  }

  /* Generate random number */
  rand_num = (double)rand()/RAND_MAX;

  /* Check if the random numer is within the probability intervals */
  for(i = 1; i < MAX_SPAWNED_CARS; i++){
    if(rand_num >= sum_probability[i - 1] && rand_num < sum_probability[i]){
      car_count = i;
      break;
    }
  }

  /* Return amount of cars to spawn */
  return car_count;
}

/* Adds a single car to a given lane */
void add_car(lane *l){
  int new_car_index = 0;
  double spawn_position = CAR_SPAWN_POSITION, extra_distance = 0;

  /* If there's already cars in the lane, find out where to spawn new car */
  if(l->amount_of_cars > 0){
    /* Find the index of car that is the furthest from the intersection */
    int last_car_index = ((l->index_front_car) + (l->amount_of_cars) - 1) % MAX_AMOUNT_OF_CARS;
    new_car_index = (last_car_index + 1) % MAX_AMOUNT_OF_CARS;

    /* If the last car is at spawn location or further away, spawn new car just behind the last car */
    if(l->cars[last_car_index].position > (CAR_SPAWN_POSITION - (CAR_LENGTH + SAFTETY_DISTANCE)))
    spawn_position = l->cars[last_car_index].position + CAR_LENGTH + SAFTETY_DISTANCE;
  }else{
    l->index_front_car = 0;
  }

  /* Add a random distance between cars */
  extra_distance = ((double)rand()/RAND_MAX) * 5.0;
  spawn_position += extra_distance;

  /* Add new car */
  l->cars[new_car_index] = make_car(l->lane_type, MAX_SPEED, spawn_position, 1);
  l->amount_of_cars += 1;
}


/* Removes the given car from the simulation */
void remove_car(simulation_state *sim_state, lane *current_lane, int car_index){
  int day = sim_state->days_simulated;

  /* Gather data for statistics */
  /* wait time = time spent - minimum time required to drive through the intersection */
  current_lane->cars[car_index].wait_time -= ((CAR_SPAWN_POSITION - DESPAWN_POSITION) / MAX_SPEED);

  /* Rounding error due to tick rate can cause wait time to go negative */
  if(current_lane->cars[car_index].wait_time < 0)
    current_lane->cars[car_index].wait_time = 0;

  /* Add wait time to total wait time */
  sim_state->sim_car_count += 1;
  sim_state->stats[day].total_wait_time += current_lane->cars[car_index].wait_time;

  /* Check if this is maximum wait time recorded so far */
  if(current_lane->cars[car_index].wait_time > sim_state->stats[day].max_wait_time)
    sim_state->stats[day].max_wait_time = current_lane->cars[car_index].wait_time;

  /* Stats for individual lanes */
  sim_state->stats[day].lane_wait_time[get_street_index(current_lane->street_name)][current_lane->lane_type] += current_lane->cars[car_index].wait_time;
  sim_state->stats[day].cars_passed[get_street_index(current_lane->street_name)][current_lane->lane_type] += 1;

  /* Count car in stats */
  sim_state->stats[day].total_cars_passed += 1;

  /* Remove car from array and simulation */
  current_lane->cars[car_index].active = 0;
  current_lane->amount_of_cars -= 1;
  current_lane->index_front_car++;
  current_lane->index_front_car %= MAX_AMOUNT_OF_CARS;
}

/* Updates statistics every simulated minute */
void update_statistics(simulation_state *sim_state){
  int day = sim_state->days_simulated, index = 0;
  index = sim_state->stats[day].gathered_data_points;

  if(index > DAILY_DATA_POINTS) return;

  sim_state->stats[day].accumulated_wait_time[index] = sim_state->stats[day].total_wait_time;
  sim_state->stats[day].cars_passed_over_time[index] = sim_state->stats[day].total_cars_passed;
  sim_state->stats[day].gathered_data_points += 1;
}

/* Returns the probability of k cars spawning within the SPAWN_INTERVAL */
double get_probability(double current_time, street road, int k){
  double cars_per_hour = 0, cars_per_timestep = 0;

  /* Determine spawnrate for given street */
  if(strcmp(road.name, street_names[0]) == 0)
  cars_per_hour = get_kjellerup_spawn_rate(current_time);

  else if(strcmp(road.name, street_names[1]) == 0)
  cars_per_hour = get_fyensgade_spawn_rate(current_time);

  else if(strcmp(road.name, street_names[2]) == 0)
  cars_per_hour = get_soenderbro_spawn_rate(current_time);

  else if(strcmp(road.name, street_names[3]) == 0)
  cars_per_hour = get_jylland_spawn_rate(current_time);

  /* Convert to cars per spawn interval */
  cars_per_timestep = (cars_per_hour / SEC_PER_HOUR) * SPAWN_INTERVAL;

  return poisson_probability(k, cars_per_timestep);
}


/* Return true (1) if all lanes are empty */
int are_all_lanes_empty(simulation_state sim_state) {
  int i, j, is_empty = 1;

  /* Loop through all lanes */
  for(i = 0; i < AMOUNT_OF_STREETS; i++){
    for(j = 0; j < LANES_PER_STREET; j++){
      lane l = sim_state.streets[i].lanes[j];
      /* Check if a lane has green signal and has cars in it */
      if(l.amount_of_cars > 0)
        is_empty = 0;
    }
  }

  return is_empty;
}

/* Returns true (1) if all green lanes are empty */
int are_green_lanes_empty(simulation_state sim_state) {
  int i, j, is_empty = 1;

  /* Loop through all lanes */
  for(i = 0; i < AMOUNT_OF_STREETS; i++){
    for(j = 0; j < LANES_PER_STREET; j++){
      /* Check if a lane has green signal and has cars in it */
      lane l = sim_state.streets[i].lanes[j];
      if(get_signal_color(sim_state.current_signal_state, l.lane_direction) == green && l.amount_of_cars > 0)
        is_empty = 0;
    }
  }

  return is_empty;
}


/* Returns acceleration in m/s^2 */
double get_acceleration(double current_speed){
  return (1.9625 * exp(0.093*(current_speed / 3.6)));
}

/* Returns the factorial of a */
int factorial(int a){
  int i, result = 1;
  for(i = 1; i <= a; i++)
    result *= i;
  return result;
}

/* Returns the probability of k events happening in an interval*/
double poisson_probability(double k, double lambda){
  return (pow(E_C, -lambda) * ((pow(lambda, k) /  ((double) factorial(k)))));
}


/* ----------- Functions that determine cars/hour for each lane ------------- */

double get_soenderbro_spawn_rate(int time_step){
  double x = time_step / SEC_PER_HOUR;

  if(x >= 0 && x <= 7.95)
    return (50.68 * pow(1.4, x));

  else if(x > 7.95 && x <= 15.92 )
    return ((38.52 * pow(x, 2)) - (931.02 * x) + 5702.66);

  else if(x > 15.92 && x <= 24)
    return (40255.22 * pow(0.77, x));

  return 0;
}

double get_jylland_spawn_rate(int time_step){
  double x = time_step / SEC_PER_HOUR;

  if(x >= 0 && x <= 7.97)
    return (200 * pow(1.2, x));

  else if(x > 7.97 && x <= 15.92 )
    return ((51.08 * pow(x, 2)) - (1176.09 * x) + 6958.85);

  else if(x > 15.92 && x <= 24)
    return (38200.08 * pow(0.79, x));

  return 0;
}

double get_fyensgade_spawn_rate(int time_step){
  double x = time_step / SEC_PER_HOUR;

  if(x >= 0 && x <= 7.75)
    return (34.44 * pow(1.45, x));

  else if(x > 7.75 && x <= 15.92 )
    return ((39.96 * pow(x, 2)) - (923.21 * x) + 5354.93);

  else if(x > 15.92 && x <= 24)
    return (124606.72 * pow(0.71, x));

  return 0;
}

double get_kjellerup_spawn_rate(int time_step){
  double x = time_step / SEC_PER_HOUR;

  if(x >= 0 && x <= 7.77)
    return ( 5.33 * pow(1.44, x));

  else if(x > 7.77 && x <= 15.92 )
    return ((5.76 * pow(x, 2)) - (129.5 * x) + 726.51);

  else if(x > 15.92 && x <= 24)
    return (128215.48 * pow(0.64, x));

  return 0;
}

#endif /* Simulering */
