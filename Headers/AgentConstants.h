#ifndef agentConstants
#define agentConstants

/* Constants */

  /* State constants */
  #define TOTAL_CAR_STATES 6
  #define TOTAL_SIGNAL_STATES 6
  #define TOTAL_TIME_STATES 3

  /* Traffic light */
  #define NUMBER_OF_LANES 1
  #define NUMBER_OF_DIRECTIONS 4
  #define YELLOW_TIME_LIMIT 4
  #define LAMDA 3
  #define SPAWNLIMIT 3

  /* Agent constants */
  #define TOTALACTIONS 2
  #define TIME_HORIZON 15

  const double reward[TOTAL_CAR_STATES] = {-0.5,2,6,12,20,77};
  const double penelty[TOTAL_CAR_STATES] = {0.5,-2,-6,-12,-20,-77};

  const int CarInterval[TOTAL_CAR_STATES][2] = { {0,0}, {1,3}, {4,8}, {9,15}, {16,25}, {26,126}};
  const int timeInterval[TOTAL_TIME_STATES][2] = { {0,15}, {16,119}, {120,10000}};

  const double spawnRate[NUMBER_OF_DIRECTIONS] = {48.69, 416.2, 313.74, 606.24};

/* Enum */

  typedef enum action {wait, ChangeSignal} action;

/* Structs */

  typedef struct agent_state {
    int carState[NUMBER_OF_DIRECTIONS];
    int signalState;
    int timeState;
  } agent_state ;

/* End of header */

#endif
