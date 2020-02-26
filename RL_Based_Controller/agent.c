#include <stdlib.h>
#include <stdio.h>
#include <math.h>
#include <windows.h>
#include <float.h>

#include "..\Headers\AgentConstants.h"
#include "..\Headers\Simulation.h"
#include "..\Headers\Simulation_Evaluation.h"

#define min(a, b) (((a) < (b)) ? (a) : (b))                                                     /* Returns the minimum value of 2 inputs*/
#define max(a, b) (((a) > (b)) ? (a) : (b))                                                     /* Returns the maximum value of 2 inputs*/

void initializeValueArray();                                                                    /* Will initialize the V_last array to all zerro*/
void GenerateValueArray();                                                                      /* Generates and saves every V array for each time horizon step*/
double valueIteration(agent_state currentState);                                                /* Performs one value iteration*/
int argmax(agent_state currentState);                                                           /* Outputs the best action possible given the current state*/

double R(int action, agent_state currentState, agent_state newState, double probability);       /* The reward for a agent_state transition*/
double Pr(int action, agent_state currentState, agent_state newState);                          /* The probability for a agent_state transition*/

double Pr_TimeChange(action action, agent_state currentState, agent_state newState);            /* The probability of a time interval change*/
double Pr_SignalChange(action action, agent_state currentState, agent_state newState);          /* The probability of a signal change*/
double Pr_CarIntervalChange(action action, agent_state currentState, agent_state newState);     /* The probability of all car interval changees*/

double Pr_OPEN_stayCarInterval(agent_state currentState, int dir);                              /* The probability of staying in an interval when the lane is open*/
double Pr_CLOSED_stayCarInterval(agent_state currentState, int dir);                            /* The probability of staying in an interval when the lane is closed*/
double Pr_OPEN_upCarInterval(agent_state currentState, agent_state newState, int dir);          /* The probability of going up an interval when the lane is open*/
double Pr_CLOSED_upCarInterval(agent_state currentState, agent_state newState, int dir);        /* The probability of going up an interval when the lane is closed*/
double Pr_OPEN_downCarInterval(agent_state currentState, agent_state newState, int dir);        /* The probability of going down an interval when the lane is open*/

double Pr_resolve(int R, int A, int T);                                                         /* The probability of resolveing R cars*/
double Pr_resolveCounterAction(int Z, int T, int C);                                            /* The probability of not resolveing enough cars for a counter action*/
double Pr_resolveStayCounterAction(int Z, int T, int A, int B);                                 /* The probability of not resolveing enough cars for a counter action when staying*/
double Pr_arrival(int Z, int dir);                                                              /* The probability of k cars arriving*/
double Pr_arrivalCounterAction(int R, int B, int T, int dir);                                   /* The probability of not arriving enough cars for a counter action*/
double Pr_carLoad(int C, int D);                                                                /* The probability of the current car load in an interval*/

int isActionAvailable(action action, agent_state currentState);                                 /* Check if a certain action is available in the current state*/
int isLaneOpen(agent_state currentState, int dir);                                              /* Check if a certain lane is available in the current state*/
int isCarIntervalChangePossible(action action, agent_state currentState, agent_state newState); /* Check if a total car interval change is possible*/

int factorialAgent(int f);                                                                      /* Returns the value of f!*/
double poisson(int k, double lamda);                                                            /* Returns the probability according to a poisson distribution*/

agent_state readCurrentState(simulation_state simState);                                        /* Returns the current state*/
int convertCarInterval(int cars);                                                               /* Converts a ceartain amount of cars into it's interval ID*/
int convertTimeInterval(double time_sec);                                                       /* Converts a ceartain amount of seconds into it's interval ID*/

void output_ValueArray(int H);                                                                  /* Outputs a formatted file of the current value array*/
void readData(double discount, int H);                                                          /* Reads and includes a formatted file of a previous value array*/

int sizeOfFullInterval(int A, int B);                                                           /* Calculates the size of a full interval. Example: |[A;B]|*/
int sizeOfInnerInterval(int A, int B);                                                          /* Calculates the size of an inner interval. Example: |]A;B[]|*/
int sizeOfEdgeInterval(int A, int B);                                                           /* Calculates the size of an edge interval. Example: |[A;B[| or |]A;B]|*/

void checkForErrors(int error, char *error_Msg);                                                /* Exits the program with an error msg if an error is detected*/

double V[TOTAL_CAR_STATES][TOTAL_CAR_STATES][TOTAL_CAR_STATES][TOTAL_CAR_STATES][TOTAL_SIGNAL_STATES][TOTAL_TIME_STATES];         /* The value array*/
double V_last[TOTAL_CAR_STATES][TOTAL_CAR_STATES][TOTAL_CAR_STATES][TOTAL_CAR_STATES][TOTAL_SIGNAL_STATES][TOTAL_TIME_STATES];    /* The last value array*/

int fac[10] = {1,1,2,6,24,120,720,5040,40320,32880};  /* A factorial look up table */
double discountValue; /* The discount value */
int timeHorizon;      /* The time horizon   */

int main(void) {
  simulation_state simState;
  agent_state currentState;
  int action, scans, sim, simGraphics;
  double startTime, simTimeScale = 1;
  char outputFileName[100];

  printf("Do you wish to train(0) or simulate(1) an agent?: ");
  scans = scanf("%d", &sim);

  printf("\nWhich discount value (0 < x > 1): ");
  scans = scanf("%lf", &discountValue);
  checkForErrors(scans != 1, "An input was unable to be loaded...");

  printf("\nTime horizon: ");
  scans = scanf("%d", &timeHorizon);
  checkForErrors(scans != 1, "An input was unable to be loaded...");

  if (sim){
    printf("\nTurn graphics OFF(0) or ON(1): ");
    scans = scanf("%d", &simGraphics);
    checkForErrors(scans != 1, "An input was unable to be loaded...");

    printf("\nStart time in seconds (0 = 00:00 and 28800 = 08:00): ");
    scans = scanf("%lf", &startTime);
    checkForErrors(scans != 1, "An input was unable to be loaded...");

    if (simGraphics){
      printf("\nSimulation timescale (1.0 = realtime and 2.5 = 1 sec in real life is 2.5 sec in the simulation): ");
      scans = scanf("%lf", &simTimeScale);
      checkForErrors(scans != 1, "An input was unable to be loaded...");
    }

    printf("\nData output filename: ");
    scans = scanf("%s", outputFileName);
    checkForErrors(scans != 1, "An input was unable to be loaded...");

    printf("Simulating...\n");
  }


  if (sim){

    /* Prepare simulation */
    readData(discountValue, timeHorizon);

    simState = make_simulation_state();
    simState.render_simulation = simGraphics;
    simState.current_time = startTime;
    simState.time_scale = simTimeScale;

    /* Run simulation for 1 day */
    while (simState.days_simulated != 1){

      currentState = readCurrentState(simState);

      /* If max time in signal has been reached, then change signal */
      if (currentState.timeState == (TOTAL_TIME_STATES - 1)){
        update_simulation(&simState, 1, ChangeSignal);

      /* Else if action ChangeSignal is available then calculate the best action */
      } else if (isActionAvailable(ChangeSignal, currentState)){
        action = argmax(currentState);
        update_simulation(&simState, 1, action);

      /* Else just wait */
      } else {
        update_simulation(&simState, 1, wait);
      }
    }

    /* Prints stats and generate output file. And free the memory */
    print_stats(simState);
    output_statistics(simState, outputFileName);
    discard_simulation(&simState);

  } else {
    /* initialize value arrays and begin the agent training */
    initializeValueArray();
    GenerateValueArray();
  }

  system("pause");

  return 0;
}

/* Will initialize the V_last array to all zerro*/
void initializeValueArray(){
  memset(V_last, 0, sizeof(V_last));
}

/* Generates and saves every V array for each time horizon step*/
void GenerateValueArray(){
  int h, car_N, car_S, car_E, car_W, signalState, timeState;
  agent_state currentState;
  int max_progress = ((TOTAL_CAR_STATES-1)*TOTAL_CAR_STATES*TOTAL_SIGNAL_STATES) + ((TOTAL_CAR_STATES-1)*TOTAL_SIGNAL_STATES) + (TOTAL_CAR_STATES-1);

  for (h = 1; h <= timeHorizon; h++){
    for (car_N = 0; car_N < TOTAL_CAR_STATES; car_N++){
      for (car_S = 0; car_S < TOTAL_CAR_STATES; car_S++){
        for (car_E = 0; car_E < TOTAL_CAR_STATES; car_E++){

          /* Progress output */
          system("cls");
          printf("Discount: %0.2f\n", discountValue);
          printf("H[%d/%d] %0.2f%%\n", h, timeHorizon,  ( ((double) ((car_N*TOTAL_CAR_STATES*TOTAL_SIGNAL_STATES) + (car_S*TOTAL_SIGNAL_STATES) + car_E)) / max_progress) * 100);

          for (car_W = 0; car_W < TOTAL_CAR_STATES; car_W++){
            for (signalState = 0; signalState < TOTAL_SIGNAL_STATES; signalState++){
              for (timeState = 0; timeState < TOTAL_TIME_STATES; timeState++){
                currentState.carState[0] = car_N;
                currentState.carState[1] = car_S;
                currentState.carState[2] = car_E;
                currentState.carState[3] = car_W;

                currentState.signalState = signalState;
                currentState.timeState = timeState;

                V[car_N][car_S][car_E][car_W][signalState][timeState] = valueIteration(currentState);
              }
            }
          }
        }
      }
    }

    output_ValueArray(h);
    memcpy(V_last, V, sizeof(V));
  }
}

/* Performs one value iteration*/
double valueIteration(agent_state currentState){
  int action, car_N, car_S, car_E, car_W, signalState, timeState;
  agent_state newState;

  double current, max = -DBL_MAX, probability;

  for (action = 0; action < TOTALACTIONS; action++){
    current = 0;

    if (isActionAvailable(action, currentState)){
      for (car_N = 0; car_N < TOTAL_CAR_STATES; car_N++){
        for (car_S = 0; car_S < TOTAL_CAR_STATES; car_S++){
          for (car_E = 0; car_E < TOTAL_CAR_STATES; car_E++){
            for (car_W = 0; car_W < TOTAL_CAR_STATES; car_W++){
              for (signalState = 0; signalState < TOTAL_SIGNAL_STATES; signalState++){
                for (timeState = 0; timeState < TOTAL_TIME_STATES; timeState++){

                  newState.carState[0] = car_N;
                  newState.carState[1] = car_S;
                  newState.carState[2] = car_E;
                  newState.carState[3] = car_W;

                  newState.signalState = signalState;
                  newState.timeState = timeState;

                  probability = Pr(action, currentState, newState);

                  current += probability * (R(action, currentState, newState, probability) + (discountValue * V_last[car_N][car_S][car_E][car_W][signalState][timeState]));

                }
              }
            }
          }
        }
      }
      if (current > max){
        max = current;
      }
    }
  }

  return max;
}

/* Outputs the best action possible given the current state*/
int argmax(agent_state currentState){
  int move;
  double current, max, probability;
  int action;
  int car_N, car_S, car_E, car_W, signalState, timeState;

  agent_state newState;

  for (action = 0; action < TOTALACTIONS; action++){
    current = 0;

    for (car_N = 0; car_N < TOTAL_CAR_STATES; car_N++){
      for (car_S = 0; car_S < TOTAL_CAR_STATES; car_S++){
        for (car_E = 0; car_E < TOTAL_CAR_STATES; car_E++){
          for (car_W = 0; car_W < TOTAL_CAR_STATES; car_W++){
            for (signalState = 0; signalState < TOTAL_SIGNAL_STATES; signalState++){
              for (timeState = 0; timeState < TOTAL_TIME_STATES; timeState++){

                newState.carState[0] = car_N;
                newState.carState[1] = car_S;
                newState.carState[2] = car_E;
                newState.carState[3] = car_W;

                newState.signalState = signalState;
                newState.timeState = timeState;

                probability = Pr(action, currentState, newState);

                current += probability * (R(action, currentState, newState, probability) + (discountValue * V[car_N][car_S][car_E][car_W][signalState][timeState]));

              }
            }
          }
        }
      }
    }


    if (current > max || action == 0){
      max = current;
      move = action;
    }
  }

  return move;
}

/* The reward for a agent_state transition*/
double R(int action, agent_state currentState, agent_state newState, double probability){
  double output = 0;
  int dir, intervalID;

  if (isActionAvailable(action, currentState) && probability != 0){

    for (dir = 0; dir < NUMBER_OF_DIRECTIONS; dir++){
        intervalID = newState.carState[dir];

        if (isLaneOpen(newState, dir)){
          output += reward[intervalID];
        } else {
          output += penelty[intervalID];
        }

    }
  }

  return output;
}

/* The probability for a agent_state transition*/
double Pr(int action, agent_state currentState, agent_state newState){
  double output;

  if (isActionAvailable(action, currentState) && isCarIntervalChangePossible(action, currentState, newState)){
    output = 1;

    /* The probability of the signal change */
    output *= Pr_SignalChange(action, currentState, newState);
    if (output == 0){
      return output;
    }

    /* The probability of the time change */
    output *= Pr_TimeChange(action, currentState, newState);
    if (output == 0){
      return output;
    }

    /* The probability of all car interval changes */
    output *= Pr_CarIntervalChange(action, currentState, newState);

  } else {
    output = 0;
  }

  return output;
}

/* The probability of a time interval change*/
double Pr_TimeChange(action action, agent_state currentState, agent_state newState){
  double output;
  int currentTime = currentState.timeState;
  int newTime = newState.timeState;

  if (action == wait){
    if (currentTime + 1 == newTime){ /* The chance of moving 1 time interval up */
      output = (double) 1 / sizeOfFullInterval(timeInterval[currentTime][0], timeInterval[currentTime][1]);

    } else if (currentTime == newTime){ /* The chance of staying in same interval */
      output = (double) (sizeOfFullInterval(timeInterval[currentTime][0], timeInterval[currentTime][1]) - 1) / sizeOfFullInterval(timeInterval[currentTime][0], timeInterval[currentTime][1]);

    } else if (currentTime == 2 && newTime == 0){ /* If max time has been reached, and the new time is the first time interval, then 100% */
      output = 1;

    } else {

      output = 0;
    }

  } else if (action == ChangeSignal){

    /* If we perform action ChangeSignal we will always go the the first time interval */
    if (newTime == 0){
      output = 1;
    } else {
      output = 0;
    }

  } else {
    checkForErrors(1, "An invalid action was used ?");
  }

  return output;
}

/* The probability of a signal change*/
double Pr_SignalChange(action action, agent_state currentState, agent_state newState){
  double output;

  int currentSignalState = currentState.signalState;
  int newSignalState = newState.signalState;

  int inYellowPhase = (currentSignalState == yg_r || currentSignalState == yr_r || currentSignalState == r_yg || currentSignalState == r_yr);
  int inGreenRedPhase = (currentSignalState == g_r || currentSignalState == r_g);

  if (action == wait){

    /* If currently in yellow phase and the new signalstate is the next in the signal order */
    if ( (inYellowPhase && currentSignalState + 1 == newSignalState) || (currentSignalState == (TOTAL_SIGNAL_STATES - 1) && newSignalState == 0) ){
      output = (double) 1/YELLOW_TIME_LIMIT;

    /* If in yellophase, and statys in the same yello phase */
    } else if (inYellowPhase && currentSignalState == newSignalState) {
      output = (double) (YELLOW_TIME_LIMIT - 1)/YELLOW_TIME_LIMIT;

    /* If we stay in the same signal (And therefor not also in yellow phase) */
    } else if (currentSignalState == newSignalState){
      output = 1;

    /* Otherwise */
    } else {
      output = 0;

    }

  } else if (action == ChangeSignal) {

    /* if in a green / red phase and the next signal in the next in the signal order */
    if (inGreenRedPhase && currentSignalState + 1 == newSignalState){
      output = 1;
    } else {
      output = 0;
    }

  } else {
    checkForErrors(1, "invalid action");
  }

  return output;
}

/* The probability of all car interval changees*/
double Pr_CarIntervalChange(action action, agent_state currentState, agent_state newState){
  int dir, currentCarState, newCarState, laneOPEN;

  double output = 1;

  for (dir = 0; dir < NUMBER_OF_DIRECTIONS; dir++){

    currentCarState = currentState.carState[dir];
    newCarState = newState.carState[dir];

    laneOPEN = isLaneOpen(currentState, dir);

    /* If we are going up in intervals */
    if (currentCarState < newCarState){

      /* If the lane is open */
      if ( laneOPEN ){
        output *= Pr_OPEN_upCarInterval(currentState, newState, dir);

      /* If the lane is closed */
      } else {
        output *= Pr_CLOSED_upCarInterval(currentState, newState, dir);
      }

    /* If we are staying in an interval */
    } else if (currentCarState == newCarState){

      /* If the lane is open */
      if ( laneOPEN ){
        output *= Pr_OPEN_stayCarInterval(currentState, dir);

      /* If the lane is closed */
      } else {
        output *= Pr_CLOSED_stayCarInterval(currentState, dir);
      }

    /* If we are going down in intervals */
    } else if (currentCarState > newCarState){

      /* If the lane is open */
      if ( laneOPEN ){
        output *= Pr_OPEN_downCarInterval(currentState, newState, dir);

      /* If the lane is closed */
      } else {
        output *= 0;
      }

    } else{
      checkForErrors(1, "SOMETHING WENT TOTALLY WRONG WITH CAR STATES");
    }

    if (output == 0){
      return output;
    }

  }

  return output;
}

/* The probability of staying in an interval when the lane is open*/
double Pr_OPEN_stayCarInterval(agent_state currentState, int dir){
  double output = 0;
  int T, Z, limit,
  currentIntervalID = currentState.carState[dir],

  A = CarInterval[currentIntervalID][0],
  B = CarInterval[currentIntervalID][1];

    for (T = A; T <= B; T++){
      limit = min(B + LAMDA - T, SPAWNLIMIT);
      for (Z = 0; Z <= limit; Z++){
        output += Pr_carLoad(A, B) * Pr_arrival(Z, dir) * Pr_resolveStayCounterAction(Z, T, A, B);
      }
    }

  return output;
}

/* The probability of staying in an interval when the lane is closed*/
double Pr_CLOSED_stayCarInterval(agent_state currentState, int dir){
  double output = 0;
  int T, Z, limit,
  currentIntervalID = currentState.carState[dir],

  A = CarInterval[currentIntervalID][0],
  B = CarInterval[currentIntervalID][1]; /* Optimering */

    for (T = A; T <= B; T++){
      limit = min(B-T, SPAWNLIMIT);
      for (Z = 0; Z <= limit; Z++){
        output += Pr_carLoad(A, B) * Pr_arrival(Z, dir);
      }
    }

  return output;
}

/* The probability of going up an interval when the lane is open*/
double Pr_OPEN_upCarInterval(agent_state currentState, agent_state newState, int dir){
  double output = 0;
  int T, Z, limit,
  currentIntervalID = currentState.carState[dir],
  newIntervalID = newState.carState[dir],

  A = CarInterval[currentIntervalID][0],
  B = CarInterval[currentIntervalID][1],
  C = CarInterval[newIntervalID][0],
  D = CarInterval[newIntervalID][1],

  maxT = SPAWNLIMIT - sizeOfInnerInterval(B,C);

  if (maxT > 0){
    for (T = max(A, B - SPAWNLIMIT + C - B); T <= B; T++){
      limit = min(sizeOfEdgeInterval(T, D), SPAWNLIMIT);
      for (Z = sizeOfEdgeInterval(T, C); Z <= limit; Z++){
        output += Pr_carLoad(A, B) * Pr_arrival(Z, dir) * Pr_resolveCounterAction(Z, T, C);
      }
    }
  }

  return output;
}

/* The probability of going up an interval when the lane is closed*/
double Pr_CLOSED_upCarInterval(agent_state currentState, agent_state newState, int dir){
  double output = 0;
  int T, Z, limit,
  currentIntervalID = currentState.carState[dir],
  newIntervalID = newState.carState[dir],

  A = CarInterval[currentIntervalID][0],
  B = CarInterval[currentIntervalID][1],
  C = CarInterval[newIntervalID][0],
  D = CarInterval[newIntervalID][1];

    for (T = max(C - SPAWNLIMIT, A); T <= B; T++){
      limit = min(sizeOfEdgeInterval(T, D), SPAWNLIMIT);
      for (Z = min(sizeOfEdgeInterval(T, C), SPAWNLIMIT); Z <= limit; Z++){
        output += Pr_carLoad(A, B) * Pr_arrival(Z, dir);
      }
    }

  return output;
}

/* The probability of going down an interval when the lane is open*/
double Pr_OPEN_downCarInterval(agent_state currentState, agent_state newState, int dir){
  double output = 0;
  int T, R, limitR,
  currentIntervalID = currentState.carState[dir],
  newIntervalID = newState.carState[dir],

  A = CarInterval[newIntervalID][0],
  B = CarInterval[newIntervalID][1],
  C = CarInterval[currentIntervalID][0],
  D = CarInterval[currentIntervalID][1],

  total_T = min(LAMDA - sizeOfInnerInterval(B,C), sizeOfFullInterval(C, D)),
  limitT = C + total_T - 1;

  for (T = C; T <= limitT; T++){
    limitR = min(LAMDA, sizeOfEdgeInterval(A, T));
    for (R = sizeOfEdgeInterval(B, T); R <= limitR; R++){
      output += Pr_carLoad(C, D) * Pr_resolve(R, A, T) * Pr_arrivalCounterAction(R, B, T, dir);
    }
  }

  return output;
}

/* The probability of resolveing R cars*/
double Pr_resolve(int R, int A, int T){
  double output;
  int i;
  int Rmax = min(LAMDA, sizeOfEdgeInterval(A,T));
  double poisSum = poisson(0, 1) + poisson(1, 1) + poisson(2, 1) + poisson(3, 1);

  if (R == T && Rmax >= R) {
    output = 0;
    for (i = R; i <= LAMDA; i++){
      output += poisson(i, 1);
    }
  } else  {

    if (R < 4 && Rmax >= R){
      output = poisson(R, 1);
    } else if (R > Rmax){
      output = 0;
    }
  }

  output /= poisSum;

  return output;
}

/* The probability of not resolveing enough cars for a counter action*/
double Pr_resolveCounterAction(int Z, int T, int C){
  int R;
  double output = 0;

  int limit = min((T+Z)-C, LAMDA);

  for (R = 0; R <= limit; R++){
    output += Pr_resolve(R, 0, (T+Z));
  }

  return output;
}

/* The probability of not resolveing enough cars for a counter action when staying*/
double Pr_resolveStayCounterAction(int Z, int T, int A, int B){
  int R;
  double output = 0;

  int limit = min(T+Z-A, LAMDA);

  for (R = max(0, T + Z - B); R <= limit; R++){
    output += Pr_resolve(R, 0, (T+Z));
  }

  return output;
}

/* The probability of k cars arriving*/
double Pr_arrival(int Z, int dir){
  return poisson(Z, (spawnRate[dir] / 3600));
}

/* The probability of not arriving enough cars for a counter action*/
double Pr_arrivalCounterAction(int R, int B, int T, int dir){
  int k;
  double output = 0;

  int limit = R - sizeOfEdgeInterval(B,T);

  for (k = 0; k <= limit; k++){
    output += Pr_arrival(k, dir);
  }

  return output;
}

/* The probability of the current car load in an interval*/
double Pr_carLoad(int C, int D){
  return 1.0/sizeOfFullInterval(C,D);
}

/* Check if a certain action is available in the current state*/
int isActionAvailable(action action, agent_state currentState){
  int signalState = currentState.signalState,
      output;

  /* action wait is always available */
  if (action == wait){
    output = 1;

  /* Else if we are in a green / red state, and the minimum time in signal state has been reached */
  } else if ((signalState == g_r || signalState == r_g) && currentState.timeState != 0){
    output = 1;

  /* Else if it's green in the north and south direction, and both lanes are empty */
  } else if ( signalState == g_r && currentState.carState[0] == 0 && currentState.carState[1] == 0){
    output = 1;

  /* Else if it's green in the east and west direction, and both lanes are empty */
  } else if ( signalState == r_g && currentState.carState[2] == 0 && currentState.carState[3] == 0){
    output = 1;

  /* Else if in any yellow phase or in a green / red phase, but minimum time has not yet been reached*/
  } else if(signalState == yg_r || signalState == yr_r || signalState == r_yg || signalState == r_yr || ((signalState == g_r || signalState == r_g) && currentState.timeState == 0)){
    output = 0;
  } else {
    checkForErrors(1, "SOME INVALID ACTION WAS CALLED");
  }

  return output;
}

/* Check if a certain lane is available in the current state*/
int isLaneOpen(agent_state currentState, int dir){
  int signalState = currentState.signalState,
      output;

  if (dir == 0 || dir == 1){

    if (signalState == yg_r || signalState == g_r || signalState == yr_r){
      output = 1;
    }else{
      output = 0;
    }

  } else if (dir == 2 || dir == 3){

    if (signalState == r_yg || signalState == r_g || signalState == r_yr){
      output = 1;
    }else{
      output = 0;
    }

  } else {

    checkForErrors(1, "SOME INVALID DIRECTION WAS CHECKED?");
  }

  return output;
}

/* Check if a total car interval change is possible*/
int isCarIntervalChangePossible(action action, agent_state currentState, agent_state newState){
  int dir, currentIntervalID, newIntervalID, currentIntervalStart, currentIntervalEnd, newIntervalStart, newIntervalEnd;
  int output;

  /* For every direction */
  for (dir = 0; dir < NUMBER_OF_DIRECTIONS; dir++){
    currentIntervalID = currentState.carState[dir];
    newIntervalID = newState.carState[dir];

    currentIntervalStart = CarInterval[currentIntervalID][0];
    currentIntervalEnd = CarInterval[currentIntervalID][1];
    newIntervalStart = CarInterval[newIntervalID][0];
    newIntervalEnd = CarInterval[newIntervalID][1];

    /* If the lane is open and the new interval is bigger than the current and it's possible reach within our spawn and despawn limits */
    if (isLaneOpen(currentState, dir) && (currentIntervalID > newIntervalID) && (currentIntervalStart - LAMDA <= newIntervalEnd)){
      output = 1;

    /* If the new interval is lower than the current and it's possible to reach within our spawn and despawn limits */
    } else if ((currentIntervalID < newIntervalID) && (currentIntervalEnd + LAMDA >= newIntervalStart)){
      output = 1;

    /* Else if we stay in the same interval */
    } else if (currentIntervalID == newIntervalID){
      output = 1;

    /* Else it's not possible to change interval, and 0 is returned */
    } else {
      output = 0;
      return output;
    }
  }

  return output;
}

/* Returns the value of f!*/
int factorialAgent(int f){
  return fac[f];
}

/* Returns the probability according to a poisson distribution*/
double poisson(int k, double lamda){
  return (pow(lamda, k) * exp(-lamda))/factorialAgent(k);
}

/* Returns the current state*/
agent_state readCurrentState(simulation_state simState){
  agent_state currentState;

  /* Car intervals */
  currentState.carState[0] = convertCarInterval(simState.streets[north].lanes[straight_right_lane].amount_of_cars);
  currentState.carState[1] = convertCarInterval(simState.streets[south].lanes[straight_right_lane].amount_of_cars);
  currentState.carState[2] = convertCarInterval(simState.streets[east].lanes[straight_right_lane].amount_of_cars);
  currentState.carState[3] = convertCarInterval(simState.streets[west].lanes[straight_right_lane].amount_of_cars);

  /* Signal and time states */
  currentState.signalState = simState.current_signal_state;
  currentState.timeState = convertTimeInterval(simState.time_since_change);

  return currentState;
}

/* Converts a ceartain amount of cars into it's interval ID*/
int convertCarInterval(int cars){
  int i;

  for (i = 0; i < TOTAL_CAR_STATES; i++){
    if (cars >= CarInterval[i][0] && cars <= CarInterval[i][1]){
      return i;
    }
  }

  return -1;
}

 /* Converts a ceartain amount of seconds into it's interval ID*/
int convertTimeInterval(double time_d){
  int i;

  int time_i = (int) time_d;

  for (i = 0; i < TOTAL_TIME_STATES; i++){
    if (time_i >= timeInterval[i][0] && time_i <= timeInterval[i][1]){
      return i;
    }
  }

  return -1;
}

/* Outputs a formatted file of the current value array*/
void output_ValueArray(int H){
  FILE *fp ;
  int car_N, car_S, car_E, car_W, signalState, timeState;
  char PATH[20];

  sprintf(PATH, "Agents\\D [%0.2f]", discountValue);
  CreateDirectory(PATH, NULL);

  sprintf(PATH, "%s\\%d.txt", PATH, H);

  fp = fopen(PATH, "w");

  /* For every datapoint, we print it in the format [Number] */
  for (car_N = 0; car_N < TOTAL_CAR_STATES; car_N++){
    for (car_S = 0; car_S < TOTAL_CAR_STATES; car_S++){
      for (car_E = 0; car_E < TOTAL_CAR_STATES; car_E++){
        for (car_W = 0; car_W < TOTAL_CAR_STATES; car_W++){
          for (signalState = 0; signalState < TOTAL_SIGNAL_STATES; signalState++){
            for (timeState = 0; timeState < TOTAL_TIME_STATES; timeState++){
              fprintf(fp, "[%0.5f]", V[car_N][car_S][car_E][car_W][signalState][timeState]);
            }
          }
        }
      }
    }
  }

  fclose(fp);
}

/* Reads and includes a formatted file of a previous value array*/
void readData(double discount, int H){
  int car_N, car_S, car_E, car_W, signalState, timeState, scans;
  char PATH[25];
  FILE *fp;

  sprintf(PATH, "Agents\\D [%0.2f]\\%d.txt", discount, H);

  fp = fopen(PATH, "r");
  checkForErrors(!fp, "Unable to open the required datafile");

  /* For every expected datapoint, read and store it */
  for (car_N = 0; car_N < TOTAL_CAR_STATES; car_N++){
    for (car_S = 0; car_S < TOTAL_CAR_STATES; car_S++){
      for (car_E = 0; car_E < TOTAL_CAR_STATES; car_E++){
        for (car_W = 0; car_W < TOTAL_CAR_STATES; car_W++){
          for (signalState = 0; signalState < TOTAL_SIGNAL_STATES; signalState++){
            for (timeState = 0; timeState < TOTAL_TIME_STATES; timeState++){
              scans = fscanf(fp, " [%lf]", &V[car_N][car_S][car_E][car_W][signalState][timeState]);
              checkForErrors(scans != 1, "Unable to read a datapoint form datafile");
            }
          }
        }
      }
    }
  }

  memcpy(V_last, V, sizeof(V));
}

/* Calculates the size of a full interval. Example: |[A;B]|*/
int sizeOfFullInterval(int A, int B){
  return B - A + 1;
}

/* Calculates the size of an inner interval. Example: |]A;B[]|*/
int sizeOfInnerInterval(int A, int B){
  return B - A - 1;
}

/* Calculates the size of an edge interval. Example: |[A;B[| or |]A;B]|*/
int sizeOfEdgeInterval(int A, int B){
  return B - A;
}

void checkForErrors(int error, char *error_Msg){
  if (error){
    printf("%s\n", error_Msg);
    system("pause");
    exit(EXIT_FAILURE);
  }
}
