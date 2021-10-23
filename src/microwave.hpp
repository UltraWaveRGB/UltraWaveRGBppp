#define OFF 0
#define ON 1
#define MAX_POWER 255

#define TRUE 1
#define FALSE 0

#define ON_DOOR_CLOSED 0
#define OFF_DOOR_CLOSED 1
#define OFF_DOOR_OPEN 2
#define PAUSED_DOOR_CLOSED 3
#define PAUSED_DOOR_OPEN 4

#define TONE_DURATION 750
#define INTERVAL_BETWEEN_TONES 750
#define NUMBER_OF_TONES 3

void update_timer();
int is_door_open();
int start_button_was_pressed();
int stop_button_was_pressed();
int execution_finished();
void ring_buzzer();