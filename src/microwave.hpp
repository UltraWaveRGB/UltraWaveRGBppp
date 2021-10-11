#define OFF 0
#define ON 1
#define CLOSED 0
#define OPEN 1
#define MAX_BRIGHTNESS 255
#define STATE_OFF 0
#define STATE_ON 1
#define STATE_PAUSED 2

void spin();
void start();
void stop();
void open_door();
void close_door();
void update_door_state();
void read_button();
void wait();