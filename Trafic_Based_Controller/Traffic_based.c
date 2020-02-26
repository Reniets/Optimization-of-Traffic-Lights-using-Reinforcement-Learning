#include "..\Headers\Simulation.h"
#include "..\Headers\Simulation_Evaluation.h"

#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <string.h>
#include <math.h>

void run_cycle(simulation_state *sim_state);
void run_sequence(simulation_state *sim_state, double green_time);
int get_total_cars(simulation_state sim_state, int cardinal_direction, int cardinal_direction_2, int lane_type);

double fairness(int x);
double max_val(double a, double b);
double min_val(double a, double b);

/* Controls traffic based on car counts from censors */
int main() {
  int renderSim;
  double startTime, simTimeScale = 1;
  simulation_state sim_state = make_simulation_state();

  printf("Simulate with graphics OFF(0) or ON(1): ");
  scanf("%d", &renderSim);

  printf("\nStart time in seconds (0 = 00:00 and 28800 = 08:00): ");
  scanf("%lf", &startTime);

  if (renderSim){
    printf("\nSimulation timescale (1.0 = realtime and 2.5 = 1 sec in real life is 2.5 sec in the simulation): ");
    scanf("%lf", &simTimeScale);
  }
  printf("Simulating...\n");


  sim_state.current_time = startTime; /* Start time of day (measured in seconds past 00:00:00) */
  sim_state.time_scale = simTimeScale;
  sim_state.render_simulation = renderSim; /* Disable/enable rendering of simulation */

  while(sim_state.days_simulated < 1){
    run_cycle(&sim_state);
  }

  print_stats(sim_state);
  output_statistics(sim_state, "SemiIntelligent");
  discard_simulation(&sim_state);

  system("pause");
  return 0;
}


/* Calculate green-time for each lane using formula : green_time_lane = (cars_in_lane / total_cars) * (total_cars * time_factor) */
void run_cycle(simulation_state *sim_state) {
  int max_cars_ns, max_cars_ew;
  double green_time_ns, green_time_ew, car_ratio, default_green_time;

  /* Find lane with most traffic for each direction */
  max_cars_ns = max_val(sim_state->streets[north].lanes[straight_right_lane].amount_of_cars, sim_state->streets[south].lanes[straight_right_lane].amount_of_cars);
  max_cars_ew = max_val(sim_state->streets[east].lanes[straight_right_lane].amount_of_cars, sim_state->streets[west].lanes[straight_right_lane].amount_of_cars);

  /* Calculate the standard green time based on the direction with the fewest cars */
  default_green_time = 10.0 + (min_val(max_cars_ns, max_cars_ew));

  /* Set default value to 1 if there's no cars */
  max_cars_ns = (max_cars_ns > 0) ? max_cars_ns : 1;
  max_cars_ew = (max_cars_ew > 0) ? max_cars_ew : 1;

  /* Determine green time for each direction */
  if(max_cars_ns > max_cars_ew){
    /* Calculate ratio between cars and use it to calculate a fair distriution of green time */
    car_ratio = (double) max_cars_ns / (double) max_cars_ew;
    green_time_ew = default_green_time;
    green_time_ns = default_green_time * fairness(car_ratio);
  }else{
    car_ratio = (double) max_cars_ew / (double) max_cars_ns;
    green_time_ew = default_green_time * fairness(car_ratio);
    green_time_ns = default_green_time;
  }

  /* Run the sequence with the given calculated green times */
  run_sequence(sim_state, green_time_ns + 2 * MAX_YELLOW_TIME);
  run_sequence(sim_state, green_time_ew + 2 * MAX_YELLOW_TIME);
}

/* Changes to next signal and runs simulation until the given green time has passed
   or the lane becomes empty */
void run_sequence(simulation_state *sim_state, double green_time) {
  double time_step = 1.0;
  double elapsed_time = 0;
  update_simulation(sim_state, time_step, 1);


  /* Change the signal and wait until the signal is no longer yellow */
  while(is_yellow(sim_state->current_signal_state)){
    update_simulation(sim_state, time_step, 0);
    green_time -= time_step;
    elapsed_time += time_step;
  }

  /* Make sure the given green time is within the maximum/minimum limits */
  if(green_time > MAX_GREEN_TIME)
    green_time = MAX_GREEN_TIME;
  else if(green_time < MIN_GREEN_TIME)
    green_time = MIN_GREEN_TIME;

  /* Simulate the remaining green time */
  while(green_time > 0){
    /* Update time variable */
    if(green_time - time_step < 0)
      time_step = green_time;
    green_time -= time_step;
    elapsed_time += time_step;


    /* Skip signal if the green lanes are empty */
    if(are_green_lanes_empty(*sim_state))
      break;

    update_simulation(sim_state, time_step, 0);
  }
}

/* Returns total amount of cars with the same signal direction as the given lane*/
int get_total_cars(simulation_state sim_state, int cardinal_direction, int cardinal_direction_2, int lane_type){
  return sim_state.streets[cardinal_direction].lanes[lane_type].amount_of_cars + sim_state.streets[cardinal_direction_2].lanes[lane_type].amount_of_cars;
}

/* A fairness function dictacting how much more green time a lane should have
   where x is the ratio of cars between lanes */
double fairness(int x) {
   return pow(x, 0.5);
}

/* Returns the largest of the given values */
double max_val(double a, double b) {
  if(a > b) return a;
  return b;
}

/* Returns the smallest of the given values */
double min_val(double a, double b) {
  if(a > b) return b;
  return a;
}
