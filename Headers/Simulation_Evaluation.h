#ifndef Evaluation /* Include guard */
#define Evaluation

#include "Simulation_Constants.h"

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <windows.h>

#define MAX_FILE_NAME_LENGTH 200
#define MAX_PATH_LENGTH 150
#define DATA_FOLDER "Evaluation data" /* Name of output folder */

void print_stats(simulation_state sim_state);

void output_statistics(simulation_state sim_state, char *solution_name);
void output_csv(char *directory, char *solution_name, char *stat_name, int* array, int array_len);
void output_double_csv(char *directory, char *solution_name, char *stat_name, double *array, int array_len);
void set_file_name(char *directory, char *file_name, char *solution_name, char *stat_name);

double get_avg_wait_time(simulation_state sim_state, int day);
int cmp_wait_time(const void * a, const void * b);

/* Prints a list of statistics in the console */
void print_stats(simulation_state sim_state){
  int day = 0, overall_car_count = 0;
  double avg_wait = 0.0, avg_max_wait = 0.0;

  /* Print stats for every simulated day */
  for(day = 0; day < sim_state.days_simulated; day++){
    double current_time = sim_state.current_time, sim_time = sim_state.stats[day].time_passed;
    printf("\nSimulation ended at: %02d:%02d:%02d", (int) current_time / (3600),  ((int) current_time / (60)) % 60, (int) current_time % 60);
    printf("\nTotal simulated time: %02d:%02d:%02d", (int) sim_time / (3600),  ((int) sim_time / (60)) % 60, (int) sim_time % 60);

    printf("\n\nTotal cars passed : %d", sim_state.stats[day].total_cars_passed);

    printf("\n\nMax queue length : %d", sim_state.stats[day].max_queue_length);

    printf("\n\nAverage wait time per car : %f", ((sim_state.stats[day].total_wait_time) / (double) sim_state.stats[day].total_cars_passed));
    printf("\nMaximum wait time for a car : %f\n\n\n", sim_state.stats[day].max_wait_time);
    printf("\n\n\n");

    /* Data for overall averages */
    overall_car_count += sim_state.stats[day].total_cars_passed;
    avg_wait += sim_state.stats[day].total_wait_time;
    avg_max_wait += sim_state.stats[day].max_wait_time;
  }
  /* Calculate average for all days */
  avg_max_wait /= (double) sim_state.days_simulated;
  avg_wait /= (double) overall_car_count;

  /* Print overall averages */
  printf("\n\n\n OVERALL: \n Average wait time : %f\nAverage max wait time : %f\n\n", avg_wait, avg_max_wait);
}

/* Writes gathered statistics to a comma seperated txt file */
void output_statistics(simulation_state sim_state, char *solution_name){
  char directory[MAX_PATH_LENGTH];
  double avg_accum_wait_time[DAILY_DATA_POINTS], avg_accum_cars[DAILY_DATA_POINTS];
  int i, j;

  /* Create directory for given solution */
  CreateDirectory(DATA_FOLDER, NULL);
  sprintf(directory, "%s\\%s", DATA_FOLDER, solution_name);
  CreateDirectory(directory, NULL);

  /* Calculate averages over simulated period */
  for(j = 0; j < DAILY_DATA_POINTS; j++){
    avg_accum_wait_time[j] = 0;
    avg_accum_cars[j] = 0;
    for(i = 0; i < sim_state.days_simulated; i++){
      avg_accum_wait_time[j] += sim_state.stats[i].accumulated_wait_time[j];
      avg_accum_cars[j] += sim_state.stats[i].cars_passed_over_time[j];
    }
    avg_accum_wait_time[j] /= (double) sim_state.days_simulated;
    avg_accum_cars[j] /= (double) sim_state.days_simulated;
  }

  output_double_csv(directory, solution_name, "accumulated_cars", avg_accum_cars, DAILY_DATA_POINTS);
  output_double_csv(directory, solution_name, "accumulated_wait_time", avg_accum_wait_time, DAILY_DATA_POINTS);
}

/* Combine file path and names to a sing string */
void set_file_name(char *directory, char *file_name, char *solution_name, char *stat_name){
  sprintf(file_name, "%s\\%s_%s.txt", directory, solution_name, stat_name);
}

/* Outputs a comma seperated file of integers */
void output_csv(char *directory, char *solution_name, char *stat_name, int* array, int array_len){
  int i;
  char file_name[MAX_FILE_NAME_LENGTH];
  FILE *fp ;

  set_file_name(directory, file_name, solution_name, stat_name);
  fp = fopen(file_name, "w");

  /* Print all values seperated by commas */
  for(i = 0; i < array_len; i++){
    fprintf(fp, "%d,", array[i]);
  }

  fclose(fp);
}

/* Outputs a comma seperated file of doubles */
void output_double_csv(char *directory, char *solution_name, char *stat_name, double* array, int array_len){
  int i;
  FILE *fp;
  char file_name[MAX_FILE_NAME_LENGTH];

  set_file_name(directory, file_name, solution_name, stat_name);
  fp = fopen(file_name, "w");

  for(i = 0; i < array_len; i++){
    fprintf(fp, "%f,", array[i]);
  }

  fclose(fp);
}

/* Returns average wait time for a given day */
double get_avg_wait_time(simulation_state sim_state, int day){
  if(day >= sim_state.days_simulated) return -1.0;
  return sim_state.stats[day].total_wait_time / (double) sim_state.stats[day].total_cars_passed;
}


#endif /* Evalution */
