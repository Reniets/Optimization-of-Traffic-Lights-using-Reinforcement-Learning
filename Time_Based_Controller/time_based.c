#include "..\Headers\Simulation.h"
#include "..\Headers\Simulation_Evaluation.h"

#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <string.h>
#include <math.h>


#define STANDARD_GREEN_TIME 20.0 /* Green time outside peak hours */
#define STANDARD_LEFT_TIME 12.0

/* Green time factor for lanes with most traffic according to data */
#define INCREASE_AFTERNOON_PEAK_TIME 1.5
#define INCREASE_MORNING_PEAK_TIME 1.25


void run_cycle(simulation_state *sim_state, double green_light, double increase_factor);
void sim_time_based(simulation_state *sim_state);


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

  /* Run time based solution */
  sim_time_based(&sim_state);

  /* Print and output statistics for the simulation */
  print_stats(sim_state);
  output_statistics(sim_state, "timebased");

  /* Free memory */
  discard_simulation(&sim_state);

  system("pause");
  return 0;
}

/* Run a full simulation with a timebased controller */
void sim_time_based(simulation_state *sim_state){
  double current_time;

  /* Run simulation until */
  while(sim_state->days_simulated < 1){
    current_time = sim_state->current_time;

    /* Morning peak hours*/
    if(current_time >= 26400 && current_time <= 30000){
      run_cycle(sim_state, STANDARD_GREEN_TIME, INCREASE_MORNING_PEAK_TIME);
    }
    /* Afternoon peak hours */
    else if(current_time >= 54600 && current_time <= 58200){
      run_cycle(sim_state, STANDARD_GREEN_TIME,  INCREASE_AFTERNOON_PEAK_TIME);
    }
    /* Other times of day */
    else{
      run_cycle(sim_state, STANDARD_GREEN_TIME, 1.0);
    }
  }
}

/* Runs a single cycle */
void run_cycle(simulation_state *sim_state, double green_light, double increase_factor){
  if(sim_state->current_signal_state != r_g){
    printf("Error : Expected signal %d but current signal is %d\n", r_yg, sim_state->current_signal_state);
    exit(0);
  }

  /* Run through a full cycle using the given green times */
  update_simulation(sim_state, green_light + MAX_YELLOW_TIME * 2, 1);
  update_simulation(sim_state, green_light * increase_factor + (MAX_YELLOW_TIME * 2), 1);
}
