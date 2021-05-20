#include <WiFi.h> //Connect to WiFi Network
#include <SPI.h>
#include <TFT_eSPI.h>
#include <mpu6050_esp32.h>
#include <math.h>
#include <string.h>
#include <cmath>
#include <DFRobotDFPlayerMini.h>

HardwareSerial mySoftwareSerial(2);
DFRobotDFPlayerMini myDFPlayer;
TFT_eSPI tft = TFT_eSPI();

int playing = 0;

const int SELECT_BUTTON_PIN = 0;
const int CHANGE_BUTTON_PIN = 5;
const int REVERSE_BUTTON_PIN = 14;
const int LOOP_PERIOD = 40;

char network[] = "MIT";  //SSID for 6.08 Lab
char password[] = ""; //Password for 6.08 Lab


//Some constants and some resources:
const int RESPONSE_TIMEOUT = 6000; //ms to wait for response from host
const uint16_t OUT_BUFFER_SIZE = 10000; //size of buffer to hold HTTP response
const int MAX_INT_ARRAY_SIZE = 800; //longest int section in library
const int NOTE_OFFSET = 3000;
const int SCROLL_TIME = 2100;

char old_response[1500]; //char array buffer to hold HTTP request
char response[OUT_BUFFER_SIZE]; //char array buffer to hold HTTP request
char host[] = "608dev-2.net";
char leader_board_response[2000];

char song[] = "photograph_guitar_medium";
int mp3_song;

class Note {       // The class
  public:             // Access specifier
    int x_coordinate;        // Attribute (int variable)
    int y_coordinate;  // Attribute (string variable)
    int rect_height = 0;
    int start_time;
    int duration;
    int finished = 0; //0 == note_line, 1 = finished
    int lane;
};

char username[200];
char roomname[200];
char action[10];
int times[MAX_INT_ARRAY_SIZE];
int notes[MAX_INT_ARRAY_SIZE];
int durations[MAX_INT_ARRAY_SIZE];
Note squares[100];

Note note_sequence[2000];
int global_index = 0;
int start = 0;
int start_time = 0;
int still_on_screen = 0;
int score_index = 0;
int done = 0;
int prev_button_state[5] = {0,0,0,0};  //Gives previous button state   -> gets set in main loop
int current_button_state[5] = {0,0,0,0}; //Gives current button state -> gets set in main loop
int can_transition[5] = {0,0,0,0}; //Indicates that it is within range to press down to start the note -> gets set to 0 at beginning of draw_notes, then gets set to 1 if front of note is within buffer
int could_count[5] = {0,0,0,0}; //Means that the line is inside the note and has the potential to be counted -> gets set to 0 at beginning of draw_notes, gets set to 1 if line is inside the note
int should_count[5] = {0,0,0,0}; // Means that we saw a transition at the beginning of the note -> gets set to 1 in main loop if there is a button transition and can transition is 1, gets set to 0 when a note moves beyond the line that was being counted before
int ending_time;

uint32_t primary_timer;

int old_val;
bool ready = false;

enum MENU {UserNameInput, Player_Choice, Multiplayer_Menu, HostRoomNameInput, 
  Host_Waiting_Room, JoinRoomNameInput, Join_Waiting_Room, 
  Song_Menu, Instrument_Menu, Difficulty_Menu, Waiting_Room, PlaySong, Exit_Menu} menu = UserNameInput;

const char *song_choices[12] = { "Every Morning", "Fluorescent Adolescent", "The Less I Know the Better", "Own Worst Enemy", "Photograph", "Still Into You", "Under Cover of Darkness", "What You Know", "When I Come Around"};
const char *instrument_choices[3] = { "Guitar", "Drums" };
const char *multiplayer_choices[4] = { "Host Room", "Join Room", "View Rooms" };
const char *host_waiting_choices[3] = { "Start", "Cancel" };
const char *player_choices[3] = { "Singleplayer", "Multiplayer" };
const char *difficulty_choices[5] = {"Easy", "Medium", "Hard","Expert"};
const char *exit_choices[4] = { "Restart", "Return to Lobby", "Exit to Main Menu"};

char song_choice[200];
char instrument[200];
char difficulty[200]; 

const uint8_t PIN1 = 26; //pin we use for software PWM
const uint8_t PIN2 = 25; //pin we use for hardware PWM
const uint8_t PIN3 = 27;
const uint8_t PIN4 = 12;
const uint8_t PIN5 = 13;

int points = 0;
int ammount = 0;
int ammount1 = 0;
int ammount2 = 0;
int ammount3 = 0;

const int BUTTON = 19;
const int BUTTON2 = 14;
const int BUTTON3 = 5;
const int BUTTON4 = 0;

const uint32_t PWM_CHANNEL = 0; //hardware pwm channel used in secon part
const uint32_t PWM_CHANNEL2 = 1;
const uint32_t PWM_CHANNEL3 = 2;
const uint32_t PWM_CHANNEL4 = 4;
const uint32_t PWM_CHANNEL5 = 5;

/*----------------------------------
   do_http_request Function:
   Arguments:
      char* host: null-terminated char-array containing host to connect to
      char* request: null-terminated char-arry containing properly formatted HTTP request
      char* response: char-array used as output for function to contain response
      uint16_t response_size: size of response buffer (in bytes)
      uint16_t response_timeout: duration we'll wait (in ms) for a response from server
      uint8_t serial: used for printing debug information to terminal (true prints, false doesn't)
   Return value:
      void (none)
*/
void do_http_request(char* host, char* request, char* response, uint16_t response_size, uint16_t response_timeout, uint8_t serial) {
  WiFiClient client; //instantiate a client object
  if (client.connect(host, 80)) { //try to connect to host on port 80
    if (serial) Serial.print(request);//Can do one-line if statements in C without curly braces
    client.print(request);
    memset(response, 0, response_size); //Null out (0 is the value of the null terminator '\0') entire buffer
    uint32_t count = millis();
    while (client.connected()) { //while we remain connected read out data coming back
      client.readBytesUntil('\n', response, response_size);
      if (serial) Serial.println(response);
      if (strcmp(response, "\r") == 0) { //found a blank line!
        break;
      }
      memset(response, 0, response_size);
      if (millis() - count > response_timeout) break;
    }
    memset(response, 0, response_size);
    count = millis();
    while (client.available()) { //read out remaining text (body of response)
      char_append(response, client.read(), OUT_BUFFER_SIZE);
    }
    if (serial) Serial.println(response);
    client.stop();
    if (serial) Serial.println("-----------");
  } else {
    if (serial) Serial.println("connection failed :/");
    if (serial) Serial.println("wait 0.5 sec...");
    client.stop();
  }
}

class PWM_608
{
  public:
    int pin; //digital pin class controls
    int period; //period of PWM signal (in milliseconds)
    int on_amt; //duration of "on" state of PWM (in milliseconds)
    int invert; //if invert==1, the "HIGH" value of the PWM signal should be 0 and the "LOW" value should be 1
    //DECLARE  three class methods below:
    PWM_608(int op, float frequency, int invert_in); //constructor op = output pin, frequency=freq of pwm signal in Hz, invert_in, if output should be inverted
    void set_duty_cycle(float duty_cycle); //sets duty cycle to be value between 0 and 100
    void update(); //updates state of system based on millis() and duty cycle
};

//DEFINE the three class methods below:
PWM_608::PWM_608(int op, float frequency, int invert_in) {
  pin = op;
  period = 1000/frequency;
  invert = invert_in;
  on_amt = period*0.5;
}

void PWM_608::update() {
  if(millis()%period < on_amt){
      digitalWrite(pin, !invert);
  }
  else if(millis()%period >= on_amt){
    digitalWrite(pin, invert);
  }
}

void PWM_608::set_duty_cycle(float duty_cycle) {
  if(duty_cycle < 0){
    duty_cycle = 0;
  } 
  else if(duty_cycle > 100){
    duty_cycle = 100;
  }
  on_amt = (period * duty_cycle / 100.0);
}

PWM_608 backlight(PIN1, 120, 1); //create instance of PWM to control backlight on pin 1, operating at 50 Hz

class Button {
  public:
    uint32_t state_2_start_time;
    uint32_t button_change_time;
    uint32_t debounce_duration;
    uint32_t long_press_duration;
    uint8_t pin;
    uint8_t flag;
    bool button_pressed;
    uint8_t state; // This is public for the sake of convenience
    Button(int p) {
      flag = 0;
      state = 0;
      pin = p;
      state_2_start_time = millis(); //init
      button_change_time = millis(); //init
      debounce_duration = 10;
      long_press_duration = 1000;
      button_pressed = 0;
    }
    void read() {
      uint8_t button_state = digitalRead(pin);
      button_pressed = !button_state;
    }

    int last_state = 0;
    //feel free to use case instead of if-else-if!!
    int update() {
      read();
      flag = 0;
      if (state == 0) {
        if (button_pressed) {
          state = 1;
          button_change_time = millis();
        }
      } else if (state == 1) {
        if (button_pressed && millis() - button_change_time >= debounce_duration) {
          state = 2;
          state_2_start_time = millis();
        } else if (!button_pressed && millis() - button_change_time >= debounce_duration) {
          state = 0;
          button_change_time = millis();
        }
      } else if (state == 2) {
        if (button_pressed && millis() - state_2_start_time >= long_press_duration) {
          state = 3;
        } else if (!button_pressed) {
          state = 4;
          button_change_time = millis();
          last_state = 2;
        }
      } else if (state == 3) {
        if (!button_pressed) {
          state = 4;
          last_state = 3;
          button_change_time = millis();
        }
      } else if (state == 4) {
        if (millis() - button_change_time >= debounce_duration) {
          if (last_state == 3) {
            flag = 2;
          } else {
            flag = 1;
          }
          state = 0;
        } else if (button_pressed && millis() - button_change_time < debounce_duration) {
          state = last_state;
          last_state = 0;
        }
      }
      return flag;
    }
};

class NameGetter {
    char alphabet[50] = "ABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789_";
    char msg[400] = {0}; //contains previous query response
    char query_string[50] = {0};
    int char_index;
    int state;
    uint32_t scrolling_timer;
    const int scrolling_threshold = 150;
    const float angle_threshold = 0.3;
    MENU menu_choice;
    char* string_to_save_to;
    bool do_http_request_at_end;
  public:

    NameGetter(char* message, MENU menu_to_go_to, char* save_string, bool req) {
      state = 0;
      memset(msg, 0, sizeof(msg));
      strcat(msg, message);
      char_index = 0;
      scrolling_timer = millis();
      menu_choice = menu_to_go_to;
      string_to_save_to = save_string;
      do_http_request_at_end = req;
    }

    void reset_namegetter(){
      state = 1;
      for(int i = 0; i < strlen(msg); i++){
        msg[i] = '\0';
      }   
      for(int i = 0; i < strlen(query_string); i++){
        query_string[i] = '\0';
      }
      for(int i = 0; i < strlen(roomname); i++){
        roomname[i] = '\0';
      }
      
      char_index = 0;
      scrolling_timer = millis();

    }

    void update_menu_to_go_to(MENU menu_to_go_to){
      menu_choice = menu_to_go_to;
    }

    int second_timer = 0;
    void update(int left_button, int right_button, int button, char* output) {
      if (state == 0) {
        strcpy(output, msg);
        if (button == 2) {
          char_index = 0;
          strcpy(query_string, "");
          scrolling_timer = millis();
          state = 1;
        }
      }
      else if (state == 1) {
        if (button == 1) {
          int len = strlen(query_string);
          if (len <= sizeof(query_string)) {
            query_string[len] = alphabet[char_index];
            query_string[len + 1] = '\0';
          }
          char_index = 0;
          char current[200];
          strcpy(current, query_string);
          len = strlen(current);
          if (len <= sizeof(current)) {
            current[len] = alphabet[char_index];
            current[len + 1] = '\0';
          }
          strcpy(output, current);
        }
        else if (button == 2) {
          state = 2;
        }
        else if (millis() - scrolling_timer >= scrolling_threshold) {
          if (left_button == 1 || right_button == 1) {
            if (right_button == 1) char_index += 1;
            else char_index -= 1;
            if (char_index <= -1) char_index = strlen(alphabet) - 1;
            else if (char_index >= strlen(alphabet)) char_index = 0;
          }
          char current[200];
          strcpy(current, query_string);
          int len = strlen(current);
          if (len <= sizeof(current)) {
            current[len] = alphabet[char_index];
            current[len + 1] = '\0';
          }
          strcpy(output, current);
          scrolling_timer = millis();
        }
        else {
          char current[200];
          strcpy(current, query_string);
          int len = strlen(current);
          if (len <= sizeof(current)) {
            current[len] = alphabet[char_index];
            current[len + 1] = '\0';
          }
          strcpy(output, current);
        }
      }

      else if (state == 2) {
        strncpy(string_to_save_to, output, strlen(output) - 1);
        strcat(string_to_save_to, "\0");
        state = 3;
        menu = menu_choice;
        //POST Request
        if (do_http_request_at_end) {
          char request[500];
          char body[200];
          sprintf(body, "user=%s&roomname=%s&action=%s&password=PASSWORD", username, roomname, action);
          Serial.println("finishes thing");
          sprintf(request, "POST /sandbox/sc/team49/server.py HTTP/1.1\r\n");
          sprintf(request + strlen(request), "Host: %s\r\n", host);
          strcat(request, "Content-Type: application/x-www-form-urlencoded\r\n");
          sprintf(request + strlen(request), "Content-Length: %d\r\n\r\n", strlen(body));
          strcat(request, body);
          Serial.println("Finishes copying");
          do_http_request(host, request, response, OUT_BUFFER_SIZE, RESPONSE_TIMEOUT, true);
          Serial.println(response);
          Serial.println(menu);
        }
      }
    }
};

NameGetter ng("Input Username after long press.", Player_Choice, username, false);
NameGetter room_ng("Input Room name after long press.", Host_Waiting_Room, roomname, true);
Button button(SELECT_BUTTON_PIN);
Button change_button(CHANGE_BUTTON_PIN);
Button reverse_button(REVERSE_BUTTON_PIN);

class Menu {
  public:
    int state;
    int choice = 0;
    uint32_t check_timer;
    Menu() {
      state = 0;
      check_timer = millis();
    }

    void reset_menu() {
      state = 0;
      choice = 0;
      check_timer = millis();
    }
};

class PlayerChoiceMenu: public Menu {
  public:
    int update(int change_button, int select_button) {
      if (change_button == 1) {
        choice = (choice + 1) % 2;
      }
      if (select_button == 1) {
        if (choice == 0) {
          menu = Song_Menu;
        } else if (choice == 1) {
          menu = Multiplayer_Menu;
        }
      }
      return choice;
    }
};

class SongMenu: public Menu {
  public:
    int update(int change_button, int select_button) {
      if (change_button == 1) {
        choice = (choice + 1) % 9;

      }
      if (select_button == 1) {
        strcpy(song_choice, song_choices[choice]);
        mp3_song = choice;
        menu = Instrument_Menu;
      }
      return choice;
    }
};

class InstrumentMenu: public Menu {
  public:
    int update(int change_button, int select_button) {
      if (change_button == 1) {
        choice = (choice + 1) % 2;
        Serial.println(choice);
      }
      if (select_button == 1) {
        strcpy(instrument, instrument_choices[choice]);
        ready = true;
        menu = Difficulty_Menu;
      }

      return choice;
    }
};

class ExitMenu: public Menu {
  public:
    int update(int change_button, int select_button) {
      if (change_button == 1) {
        choice = (choice+1) % 3;
        Serial.println(choice);
      }
      if (select_button == 1) {
        ready = true;
        if (choice == 0) {
          start_time = millis();
          global_index = 0;
          start=0;
          score_index = 1;
          
          points = 0;
          ammount = 0;
          ammount1 = 0;
          ammount2 = 0;
          ammount3 = 0;
          menu = PlaySong; 
          tft.fillScreen(TFT_BLACK);
        } else if (choice == 1) {
          menu = Song_Menu;
        } else if (choice == 2) {
          memset(difficulty, 0, sizeof(difficulty));
          memset(song_choice, 0, sizeof(song_choice));
          memset(instrument, 0, sizeof(instrument));
          memset(roomname, 0, sizeof(roomname));
          global_index = 0;
          start=0;
          start_time;
          score_index = 1;
          
          points = 0;
          ammount = 0;
          ammount1 = 0;
          ammount2 = 0;
          ammount3 = 0;

          menu = Player_Choice;
          room_ng.reset_namegetter();

          //http request to leave room.
          char request[500];
          char body[200];
          sprintf(body, "user=%s&roomname=PLACEHOLDER&action=leave&password=PASSWORD", username);
          sprintf(request, "POST /sandbox/sc/team49/server.py HTTP/1.1\r\n");
          sprintf(request + strlen(request), "Host: %s\r\n", host);
          strcat(request, "Content-Type: application/x-www-form-urlencoded\r\n");
          sprintf(request + strlen(request), "Content-Length: %d\r\n\r\n", strlen(body));
          sprintf(request + strlen(request), body);
          Serial.println(request);
          do_http_request(host, request, response, OUT_BUFFER_SIZE, RESPONSE_TIMEOUT, true);
          Serial.println(response);

        }
      }

      return choice;
    }
};

class MultiplayerMenu: public Menu {
  public:
    int update(int change_button, int select_button) {
      if (change_button == 1) {
        choice = (choice + 1) % 3;
        Serial.println(choice);
      }
      if (select_button == 1) {
        if (choice == 0) {
          menu = HostRoomNameInput;
          room_ng.update_menu_to_go_to(Host_Waiting_Room);
          Serial.println(menu);
        } else if (choice == 1) {
          menu = JoinRoomNameInput;
          room_ng.update_menu_to_go_to(Join_Waiting_Room);
          Serial.println(menu);
        } else if (choice == 2) {
          //        menu = View_Room;
          Serial.println(menu);
        }
      }
      return choice;
    }
};

class HostWaitingRoomMenu: public Menu {
  public:
    int update(int change_button, int select_button) {
      if (change_button == 1) {
        choice = (choice + 1) % 2;
        Serial.println(choice);
      }
      if (select_button == 1) {
        if (choice == 0) {
          char request[500];
          char body[200];
          sprintf(body, "user=%s&roomname=%s&action=ready&password=PASSWORD", username, roomname);
          Serial.println("finishes thing");
          sprintf(request, "POST /sandbox/sc/team49/server.py HTTP/1.1\r\n");
          sprintf(request + strlen(request), "Host: %s\r\n", host);
          strcat(request, "Content-Type: application/x-www-form-urlencoded\r\n");
          sprintf(request + strlen(request), "Content-Length: %d\r\n\r\n", strlen(body));
          strcat(request, body);
          Serial.println("Finishes copying");
          do_http_request(host, request, response, OUT_BUFFER_SIZE, RESPONSE_TIMEOUT, true);
          Serial.println(response);
          menu = Song_Menu;
          Serial.println(menu);
          //READY UP!
        } else if (choice == 1) {
          room_ng.reset_namegetter();
          menu = Multiplayer_Menu;
          reset_menu();
          Serial.println(menu);

          //http request to leave room.
          char request[500];
          char body[200];
          sprintf(body, "user=%s&roomname=PLACEHOLDER&action=leave&password=PASSWORD", username);
          sprintf(request, "POST /sandbox/sc/team49/server.py HTTP/1.1\r\n");
          sprintf(request + strlen(request), "Host: %s\r\n", host);
          strcat(request, "Content-Type: application/x-www-form-urlencoded\r\n");
          sprintf(request + strlen(request), "Content-Length: %d\r\n\r\n", strlen(body));
          sprintf(request + strlen(request), body);
          Serial.println(request);
          do_http_request(host, request, response, OUT_BUFFER_SIZE, RESPONSE_TIMEOUT, true);
          Serial.println(response);

        }
      }
      return choice;
    }
};

class DifficultyMenu: public Menu {
  public:
    int update(int change_button, int select_button) {
      if (change_button == 1) {
        choice = (choice + 1) % 4;
        Serial.println(choice);
      }
      if (select_button == 1) {
        strcpy(difficulty, difficulty_choices[choice]);
        menu = Waiting_Room;
      }

      return choice;
    }
};

InstrumentMenu instrument_menu;
SongMenu song_menu;
MultiplayerMenu multiplayer_menu;
HostWaitingRoomMenu hostWaitingRoomMenu;
PlayerChoiceMenu player_choice_menu;
DifficultyMenu difficulty_menu;
ExitMenu exit_menu;

void setup() {
  mySoftwareSerial.begin(9600, SERIAL_8N1, 32, 33);  // speed, type, RX, TX
  Serial.begin(115200);
  
  Serial.println();
  Serial.println(F("DFRobot DFPlayer Mini Demo"));
  Serial.println(F("Initializing DFPlayer ... (May take 3~5 seconds)"));
  delay(1000);
  while (!myDFPlayer.begin(mySoftwareSerial)) {  //Use softwareSerial to communicate with mp3.
    
    Serial.println(myDFPlayer.readType(),HEX);
    Serial.println(F("Unable to begin:"));
    Serial.println(F("1.Please recheck the connection!"));
    Serial.println(F("2.Please insert the SD card!"));
    //while(true);
  }
  Serial.println(F("DFPlayer Mini online."));
  
  myDFPlayer.setTimeOut(500); //Set serial communictaion time out 500ms
  
  //----Set volume----
  myDFPlayer.volume(20);  //Set volume value (0~30).
  myDFPlayer.volumeUp(); //Volume Up
  myDFPlayer.volumeDown(); //Volume Down
  
  //----Set different EQ----
  myDFPlayer.EQ(DFPLAYER_EQ_NORMAL);
  myDFPlayer.outputDevice(DFPLAYER_DEVICE_SD);


  WiFi.begin(network, password); //attempt to connect to wifi
  uint8_t count = 0; //count used for Wifi check times
  Serial.print("Attempting to connect to ");
  Serial.println(network);
  while (WiFi.status() != WL_CONNECTED && count < 12) {
    delay(500);
    Serial.print(".");
    count++;
  }
  delay(2000);
  if (WiFi.isConnected()) { //if we connected then print our IP, Mac, and SSID we're on
    Serial.println("CONNECTED!");
    Serial.printf("%d:%d:%d:%d (%s) (%s)\n", WiFi.localIP()[3], WiFi.localIP()[2],
                  WiFi.localIP()[1], WiFi.localIP()[0],
                  WiFi.macAddress().c_str() , WiFi.SSID().c_str());    delay(500);
  } else { //if we failed to connect just Try again.
    Serial.println("Failed to Connect :/  Going to restart");
    Serial.println(WiFi.status());
    ESP.restart(); // restart the ESP (proper way)
  }

  tft.init();
  tft.setRotation(2);
  tft.setTextSize(1);
  tft.fillScreen(TFT_BLACK);
  tft.setTextColor(TFT_GREEN, TFT_BLACK); //set color of font to green foreground, black background
  pinMode(SELECT_BUTTON_PIN, INPUT_PULLUP);
  pinMode(CHANGE_BUTTON_PIN, INPUT_PULLUP);
  pinMode(BUTTON, INPUT_PULLUP);
  pinMode(BUTTON2, INPUT_PULLUP);
  strcpy(instrument, instrument_choices[0]);
  primary_timer = millis();

  ledcSetup(PWM_CHANNEL, 60, 12);//create pwm channel, @50 Hz, with 12 bits of precision
  ledcAttachPin(PIN2, PWM_CHANNEL); //link pwm channel to IO pin 25
  ledcSetup(PWM_CHANNEL2, 60, 12);//create pwm channel, @50 Hz, with 12 bits of precision
  ledcAttachPin(PIN1, PWM_CHANNEL2);
  ledcSetup(PWM_CHANNEL3, 60, 12);//create pwm channel, @50 Hz, with 12 bits of precision
  ledcAttachPin(PIN3, PWM_CHANNEL3);
  ledcSetup(PWM_CHANNEL4, 60, 12);//create pwm channel, @50 Hz, with 12 bits of precision
  ledcAttachPin(PIN4, PWM_CHANNEL4);
  ledcSetup(PWM_CHANNEL5, 60, 12);//create pwm channel, @50 Hz, with 12 bits of precision
  ledcAttachPin(PIN5, PWM_CHANNEL5);

  tft.fillScreen(TFT_BLACK);
  tft.setCursor(0, 0, 2);

  global_index = 0;
  start_time = millis();
  backlight.set_duty_cycle(60); //initialize the software PWM to be at 30%
  parse_song_file(response);
  for (int i = 0; i < 2000; i++){
    times[i] += 4000;
  }
}


MENU last_menu = UserNameInput;
int last_choice = 0;
bool change = true;

void loop() {
  int change_button_val = change_button.update();
  int select_button_val = button.update();
  int reverse_button_val = reverse_button.update();
  int choice = 0;

  if (menu == Multiplayer_Menu) {
    choice = multiplayer_menu.update(change_button_val, select_button_val);
    if (change) {
      tft.fillScreen(TFT_BLACK);
      tft.setCursor(0, 0, 2);
      for (int i = 0; i < 3; i++) {
        if (i == choice) {
          tft.setTextColor(TFT_WHITE, TFT_GREEN);
          tft.println(multiplayer_choices[i]);
          tft.setTextColor(TFT_GREEN, TFT_BLACK);
        } else {
          tft.println(multiplayer_choices[i]);
        }
      }
      change = false;
    }
  } else if (menu == Exit_Menu) {

    choice = exit_menu.update(change_button_val, select_button_val);
    if (change) {
      tft.fillScreen(TFT_BLACK);
      tft.setTextColor(TFT_GREEN, TFT_BLACK);
      tft.setCursor(30, 30, 2);
      int y_index[6] = {50, 65, 80};
      int x_index[6] = {60, 45, 40};
      for (int i = 0; i < 3; i++) {
        tft.setCursor(x_index[i], y_index[i], 2);
        if (i == choice) {
          tft.setTextColor(TFT_WHITE, TFT_GREEN);
          tft.println(exit_choices[i]);
          tft.setTextColor(TFT_GREEN, TFT_BLACK);
        } else {
          tft.println(exit_choices[i]);
        }
      }
      change = false;
    }
  } else if (menu == HostRoomNameInput) {
    int right_value = !digitalRead(CHANGE_BUTTON_PIN);
    int left_value = !digitalRead(REVERSE_BUTTON_PIN);
    strcpy(action, "create");
    room_ng.update(left_value, right_value, select_button_val, response); //input: angle and button, output String to display on this timestep
    if (change || strcmp(response, old_response) != 0) {//only draw if changed!
      tft.fillScreen(TFT_BLACK);
      tft.setCursor(0, 0, 2);
      tft.println(response);
      change = false;
    }
    memset(old_response, 0, sizeof(old_response));
    strcat(old_response, response);
    while (millis() - primary_timer < LOOP_PERIOD); //wait for primary timer to increment
    primary_timer = millis();
    choice = last_choice;
  } else if (menu == Host_Waiting_Room) {
    //Updating based on the room
    if (millis() - primary_timer > 2000) {
      Serial.println("2 second timer");
      primary_timer = millis();
      char request[500];
      sprintf(request, "GET /sandbox/sc/team49/server.py?user=%s&action=lobby HTTP/1.1\r\n", username);
      sprintf(request + strlen(request), "Host: %s\r\n\r\n", host);
      do_http_request(host, request, response, OUT_BUFFER_SIZE, RESPONSE_TIMEOUT, true);
      Serial.println(response);
      change = true;
    }

    choice = hostWaitingRoomMenu.update(change_button_val, select_button_val);
    if (change) {
      tft.fillScreen(TFT_BLACK);
      tft.setCursor(0, 0, 2);
      for (int i = 0; i < 2; i++) {
        if (i == choice) {
          tft.setTextColor(TFT_WHITE, TFT_GREEN);
          tft.println(host_waiting_choices[i]);
          tft.setTextColor(TFT_GREEN, TFT_BLACK);
        } else {
          tft.println(host_waiting_choices[i]);
        }
      }
      tft.println();
      tft.println();
      tft.println(response);
      change = false;
    }
  } else if (menu == JoinRoomNameInput) {
    int right_value = !digitalRead(CHANGE_BUTTON_PIN);
    int left_value = !digitalRead(REVERSE_BUTTON_PIN);
    strcpy(action, "join");
    room_ng.update(left_value, right_value, select_button_val, response); //input: angle and button, output String to display on this timestep
    if (change || strcmp(response, old_response) != 0) {//only draw if changed!
      tft.fillScreen(TFT_BLACK);
      tft.setCursor(0, 0, 2);
      tft.println(response);
      change = false;
    }
    memset(old_response, 0, sizeof(old_response));
    strcat(old_response, response);
    while (millis() - primary_timer < LOOP_PERIOD); //wait for primary timer to increment
    primary_timer = millis();
    choice = last_choice;
  } else if (menu == Join_Waiting_Room) {
    if (millis() - primary_timer > 2000) {
      Serial.println("2 second timer");
      primary_timer = millis();
      char request[500];
      sprintf(request, "GET /sandbox/sc/team49/server.py?user=%s&action=lobby HTTP/1.1\r\n", username);
      sprintf(request + strlen(request), "Host: %s\r\n\r\n", host);
      do_http_request(host, request, response, OUT_BUFFER_SIZE, RESPONSE_TIMEOUT, true);
      Serial.println(response);
      if(strstr(response, "Readying") != NULL){
        menu = Song_Menu;
      }
      change = true;
    }
    if (change) {
      tft.fillScreen(TFT_BLACK);
      tft.setCursor(0, 0, 2);
      tft.println(response);
      change = false;
    }
  } else if (menu == PlaySong) {
    if(millis() - start_time >= ending_time && done != 1){
      char request[500];
      char body[200];
      tft.fillScreen(TFT_BLACK);
      tft.setCursor(75, 0, 2);
      tft.println("WELL DONE!");
      tft.setCursor(65, 25, 2);
      tft.println("Your Score Is:");
      tft.setCursor(85, 40, 2);
      tft.println(points);
      sprintf(body, "user=%s&song=%s&instruments=%s&score=%i&action=leaderboard", username, song, instrument, points);
      sprintf(request, "POST /sandbox/sc/nfaro/server.py HTTP/1.1\r\n");
      sprintf(request + strlen(request), "Host: %s\r\n", host);
      strcat(request, "Content-Type: application/x-www-form-urlencoded\r\n");
      sprintf(request + strlen(request), "Content-Length: %d\r\n\r\n", strlen(body));
      strcat(request, body);
      Serial.println("This is the request");
      Serial.println(request);
      do_http_request(host, request, leader_board_response, OUT_BUFFER_SIZE, RESPONSE_TIMEOUT, true);
      delay(5000);
      tft.fillScreen(TFT_BLACK);
      tft.setCursor(0, 0, 2);
      tft.println(leader_board_response);
      done = 1;
    }
    else if (done == 1){
      delay(5000);
      tft.fillScreen(TFT_BLACK);
      tft.setCursor(0, 0, 2);
      menu = Exit_Menu;
      done = 0;          
      playing = 0;
    }
    else{

      //NEEDS TO BE ADDED, CONTROLS STATE DURING SONG PLAYING. ALSO, ANDREI NEEDS TO ADD LEDS BACK TO THESE IF STATEMENTS.

      //Explanation of arrays:
      //int prev_button_state[5] = {0,0,0,0};  //Gives previous button state   -> gets set in main loop
      //int current_button_state[5] = {0,0,0,0}; //Gives current button state -> gets set in main loop
      //int can_transition[5] = {0,0,0,0}; //Indicates that it is within range to press down to start the note -> gets set to 0 at beginning of draw_notes, then gets set to 1 if front of note is within buffer
      //int could_count[5] = {0,0,0,0}; //Means that the line is inside the note and has the potential to be counted -> gets set to 0 at beginning of draw_notes, gets set to 1 if line is inside the note
      //int should_count[5] = {0,0,0,0}; // Means that we saw a transition at the beginning of the note -> gets set to 1 in main loop if there is a button transition and can transition is 1, gets set to 0 when a note moves beyond the line that was being counted before
      int transitions[5] = {0,0,0,0};

      //Gets currentState of button
      if (!digitalRead(BUTTON)) {
        current_button_state[0] = 1;
      } else {
        current_button_state[0] = 0;
      }
      if (!digitalRead(BUTTON2)) {
        current_button_state[1] = 1;
      } else {
        current_button_state[1] = 0;
      }
      if (!digitalRead(BUTTON3)) {
        current_button_state[2] = 1;
      } else {
        current_button_state[2] = 0;
      }
      if (!digitalRead(BUTTON4)) {
        current_button_state[3] = 1;
      } else {
        current_button_state[3] = 0;
      }

      //Detects a new transition
      if (current_button_state[0] == 1 && prev_button_state[0] == 0 && can_transition[0] == 1) {
        should_count[0] = 1;
      }
      if (current_button_state[1] == 1 && prev_button_state[1] == 0 && can_transition[1] == 1) {
        should_count[1] = 1;
      }
      if (current_button_state[2] == 1 && prev_button_state[2] == 0 && can_transition[2] == 1) {
        should_count[2] = 1;
      }
      if (current_button_state[3] == 1 && prev_button_state[3] == 0 && can_transition[3] == 1) {
        should_count[3] = 1;
      }

      //Adds points if it could and should be counted
      if (should_count[0] == 1 && could_count[0] == 1 && current_button_state[0] == 1) {
        points += 1;
      }
      if (should_count[1] == 1 && could_count[1] == 1 && current_button_state[1] == 1) {
        points += 1;
      }
      if (should_count[2] == 1 && could_count[2] == 1 && current_button_state[2] == 1) {
        points += 1;
      }
      if (should_count[3] == 1 && could_count[3] == 1 && current_button_state[3] == 1) {
        points += 1;
      }

      //Moves curr button state to previous
      prev_button_state[0] = current_button_state[0];
      prev_button_state[1] = current_button_state[1];
      prev_button_state[2] = current_button_state[2];
      prev_button_state[3] = current_button_state[3];

      //Draws notes
      draw_notes();

      //Adds point total at the top of the screen
      tft.setCursor(0, 0, 1);
      tft.println("Points: ");
      tft.println(points);
      tft.setTextColor(TFT_RED, TFT_BLACK); //set color for font

      if (millis() - start_time < (NOTE_OFFSET + SCROLL_TIME - 1000)){
        tft.setCursor(80, 0, 2);
        tft.println("READY!");
      }
      else if (millis() - start_time < NOTE_OFFSET + SCROLL_TIME){
        tft.setCursor(80, 0, 2);
        tft.println("SET!");
      }
      else if (millis() - start_time < (NOTE_OFFSET + SCROLL_TIME + 1000) && playing == 0){
        tft.setCursor(80, 0, 2);
        tft.println("GO!");
        myDFPlayer.play(mp3_song);
        playing = 1;
        tft.fillScreen(TFT_BLACK);
      }
      else if(playing ==1){
        
      }

   
      if (!digitalRead(BUTTON4)) {
        ledcWrite(PWM_CHANNEL, (4095) - (4095 * 50/100.0));
      }
      else{
        ledcWrite(PWM_CHANNEL, 0);
      }
      if(!digitalRead(BUTTON2)){
        ledcWrite(PWM_CHANNEL2, (4095) - (4095 * 50/100.0));
      }
      else{
        ledcWrite(PWM_CHANNEL2, 0);
      }
      if (!digitalRead(BUTTON3)){
        ledcWrite(PWM_CHANNEL3, (4095) - (4095 * 50/100.0));
      }
      else{
        ledcWrite(PWM_CHANNEL3, 0);
      }
      if (!digitalRead(BUTTON)){
        ledcWrite(PWM_CHANNEL4, (4095) - (4095 * 50/100.0));
        ledcWrite(PWM_CHANNEL5, (4095) - (4095 * 50/100.0));
      }
      else{
        ledcWrite(PWM_CHANNEL5, 0);
        ledcWrite(PWM_CHANNEL4, 0);
    
      }
      delay(15);
    }
  } else if (menu == Instrument_Menu) {
    choice = instrument_menu.update(change_button_val, select_button_val);
    if (change) {
      
      tft.fillScreen(TFT_BLACK);
      tft.setTextColor(TFT_GREEN, TFT_BLACK);
      tft.setCursor(60, 30, 2);
      tft.setTextSize(1.75);
      tft.println("Pick an Instrument:");
      tft.println("");
      tft.setTextSize(0.75);
      int y_index[6] = {60, 75, 90};
      for (int i = 0; i < 3; i++) {
        tft.setCursor(90, y_index[i], 2);
        if (i == choice) {
          tft.setTextColor(TFT_BLACK, TFT_GREEN);
          tft.println(instrument_choices[i]);
          tft.setTextColor(TFT_GREEN, TFT_BLACK);
        } else {
          tft.println(instrument_choices[i]);
        }
      }
      change = false;
    }
  } else if (menu == Difficulty_Menu) {
    choice = difficulty_menu.update(change_button_val, select_button_val);
    if (change) {
      tft.fillScreen(TFT_BLACK);
      tft.setTextColor(TFT_GREEN, TFT_BLACK);
      tft.setCursor(60, 30, 2);
      tft.setTextSize(1.75);
      tft.println("Pick a Difficulty:");
      tft.println("");
      tft.setTextSize(0.75);
      int y_index[6] = {60, 75, 90, 105};
      for (int i = 0; i < 4; i++) {
        tft.setCursor(70, y_index[i], 2);
        if (i == choice) {
          tft.setTextColor(TFT_WHITE, TFT_GREEN);
          tft.println(difficulty_choices[i]);
          tft.setTextColor(TFT_GREEN, TFT_BLACK);
        } else {
          tft.println(difficulty_choices[i]);
        }
      }
      change = false;
    }
  } else if (menu == Song_Menu) {
    choice = song_menu.update(change_button_val, select_button_val);
    if (change) {
      tft.fillScreen(TFT_BLACK);
      tft.setTextColor(TFT_GREEN, TFT_BLACK);
      tft.setCursor(70, 30, 2);
      tft.setTextSize(1.75);
      tft.println("Pick a Song:");
      tft.println("");
      tft.setTextSize(0.75);
      for (int i = 0; i < 9; i++) {
        tft.setCursor(0, 60 + 20 * i, 2);
        if (i == choice) {
          tft.setTextColor(TFT_BLACK, TFT_GREEN);
          tft.println(song_choices[i]);
          tft.setTextColor(TFT_GREEN, TFT_BLACK);
        } else {
          tft.println(song_choices[i]);
        }
      }
      change = false;
    }
  } else if (menu == UserNameInput) {
    int right_value = !digitalRead(CHANGE_BUTTON_PIN);
    int left_value = !digitalRead(REVERSE_BUTTON_PIN);
    ng.update(left_value, right_value, select_button_val, response); //input: angle and button, output String to display on this timestep
    if (change || strcmp(response, old_response) != 0) {//only draw if changed!
      tft.fillScreen(TFT_BLACK);
      tft.setTextColor(TFT_GREEN, TFT_BLACK);
      tft.setCursor(50, 30, 2);
      tft.setTextSize(1.75);
      tft.println("Enter your username");
      tft.setCursor(90, 48, 2);
      tft.println("below:");
      tft.println("");
      tft.setTextSize(0.75);
      int newIndex = 100 - (strlen(response)) * 2.5;
      tft.setCursor(newIndex, 70, 2);
      char temp[100] = "";
      if (strlen(response) != 1) {
        strncpy(temp, response, strlen(response) - 1);
        temp[strlen(response)-1] = '\0';
      }
      tft.print(temp);
      tft.setTextColor(TFT_BLACK, TFT_GREEN);
      tft.print(response[strlen(response)-1]);
      tft.setTextColor(TFT_GREEN, TFT_BLACK);
      change = false;
    }
    memset(old_response, 0, sizeof(old_response));
    strcat(old_response, response);
    while (millis() - primary_timer < LOOP_PERIOD); //wait for primary timer to increment
    primary_timer = millis();
    choice = last_choice;
  } else if (menu == Player_Choice) {
    choice = player_choice_menu.update(change_button_val, select_button_val);
    if (change) {
      tft.fillScreen(TFT_BLACK);
      tft.setCursor(60, 40, 2);
      tft.setTextSize(2);
      tft.setTextColor(TFT_RED, TFT_BLACK);
      tft.println("ARDUINO");
      tft.setTextSize(2);
      tft.setCursor(80, 75, 2);
      tft.println("HERO");
      tft.println("");
      tft.setTextSize(0.75);
      int y_index[3] = {120, 140};
      tft.setTextColor(TFT_GREEN, TFT_BLACK);
      for (int i = 0; i < 2; i++) {
        tft.setCursor(70, y_index[i], 2);
        if (i == choice) {
          tft.setTextColor(TFT_BLACK, TFT_GREEN);
          tft.println(player_choices[i]);
          tft.setTextColor(TFT_GREEN, TFT_BLACK);
        } else {
          tft.println(player_choices[i]);
        }
      }
      change = false;
    }
  } else if (menu == Waiting_Room) {
    if(ready){
        char requestO[500];
        char bodyO[200];
        sprintf(bodyO, "user=%s&song=%s&instrument=%s&difficulty=%s&action=startgame", username, song_choice, instrument, difficulty);
        sprintf(requestO, "POST /sandbox/sc/team49/server.py HTTP/1.1\r\n");
        sprintf(requestO + strlen(requestO), "Host: %s\r\n", host);
        strcat(requestO, "Content-Type: application/x-www-form-urlencoded\r\n");
        sprintf(requestO + strlen(requestO), "Content-Length: %d\r\n\r\n", strlen(bodyO));
        strcat(requestO, bodyO);
        Serial.println(response);
        do_http_request(host, requestO, response, OUT_BUFFER_SIZE, RESPONSE_TIMEOUT, true);
        Serial.println(response);
        parse_song_file(response);

        ready = false;
        char request[500];
        char body[200];
        sprintf(body, "user=%s&roomname=%s&action=play&password=PASSWORD", username, roomname);
        Serial.println("finishes thing");
        sprintf(request, "POST /sandbox/sc/team49/server.py HTTP/1.1\r\n");
        sprintf(request + strlen(request), "Host: %s\r\n", host);
        strcat(request, "Content-Type: application/x-www-form-urlencoded\r\n");
        sprintf(request + strlen(request), "Content-Length: %d\r\n\r\n", strlen(body));
        strcat(request, body);
        Serial.println("Finishes copying");
        do_http_request(host, request, response, OUT_BUFFER_SIZE, RESPONSE_TIMEOUT, true);
    }
    else if(millis() - primary_timer > 2000){
      tft.fillScreen(TFT_BLACK);
      tft.setCursor(0, 0, 2);
      tft.println("Waiting");
      tft.println(username);
      tft.println(instrument);
      choice = last_choice;
      primary_timer = millis();

      char request[500];
      char body[200];
      sprintf(body, "user=%s&roomname=%s&action=waiting&password=PASSWORD", username, roomname);
      sprintf(request, "POST /sandbox/sc/team49/server.py HTTP/1.1\r\n");
      sprintf(request + strlen(request), "Host: %s\r\n", host);
      strcat(request, "Content-Type: application/x-www-form-urlencoded\r\n");
      sprintf(request + strlen(request), "Content-Length: %d\r\n\r\n", strlen(body));
      strcat(request, body);
      do_http_request(host, request, response, OUT_BUFFER_SIZE, RESPONSE_TIMEOUT, true);
      if(strstr(response, "Waiting") == NULL){
        delay(atoi(response));
        menu = PlaySong;
        for (int i = 0; i < 2000; i++){
          times[i] += 4000;
        }
        global_index = 0;
        start_time = millis();
        start = 0;
      }
      tft.fillScreen(TFT_BLACK);
    }
  }

  if (choice != last_choice || menu != last_menu) {
    change = true;
  }

  last_menu = menu;
  last_choice = choice;
}


bool detect_note(){
  if(millis() + 70 >= times[score_index] && millis() - 150 <= times[score_index]){
    return true;
  }
  else if(millis() - 151 > times[score_index]){
    score_index += 1;
    return false;
  }
  else{
    return false;
  }
}

//NEEDS TO BE ADDED, JUST COPY PASTE OVER THE OLD ONE
void draw_notes(){
  

  if (millis() - start_time >= times[global_index]){
    int len[2];
    int length1;
    if (notes[global_index]/10 == 0){
      len[0] = notes[global_index];
      length1 = 1;
    }
    else{
      int temp1 = notes[global_index]/10;
      int temp2 = notes[global_index]%10;
      if (temp1 == temp2){
        len[0]= notes[global_index];
        length1 = 1;
      }
      else{
        len[0] = notes[global_index]/10;
        len[1] = notes[global_index]%10;
        length1 = 2;
      }
    }
    for (int j = 0; j < length1;j++){
      int temp_note = len[j];
      if(temp_note == 1){
        Note new_note;
        new_note.x_coordinate = 0; 
        new_note.y_coordinate= 0;
        new_note.rect_height = 0;
        new_note.start_time = times[global_index];
        new_note.duration = durations[global_index];
        new_note.lane = 1;
        note_sequence[start] = new_note;
      }
      else if(temp_note == 2){
        Note new_note;
        new_note.x_coordinate = 60; 
        new_note.y_coordinate= 0;
        new_note.rect_height = 0;
        new_note.start_time = times[global_index];
        new_note.duration = durations[global_index];
        new_note.lane = 2;
        note_sequence[start] = new_note;
      }
      else if(temp_note == 3){
        Note new_note;
        new_note.x_coordinate = 120; 
        new_note.y_coordinate= 0;
        new_note.rect_height = 0;
        new_note.start_time = times[global_index];
        new_note.duration = durations[global_index];
        new_note.lane = 3;
        note_sequence[start] = new_note;
      }
      else if(temp_note == 4){
        Note new_note;
        new_note.x_coordinate = 180; 
        new_note.y_coordinate= 0;
        new_note.rect_height = 0;
        new_note.start_time = times[global_index];
        new_note.duration = durations[global_index];
        new_note.lane = 4;
        note_sequence[start] = new_note;
      }
      start++;
      
    }
    global_index++;
    
      
  }

    // Create an object of MyClass

  // Access attributes and set values
  
  tft.fillRect(0, 280, 240, 1, TFT_GREEN);
  for(int l = 0; l < 4; l++) {
    can_transition[l] = 0;
    could_count[l] = 0;
  }
  for (int i = still_on_screen; i < start; i++){

      //Sets buffer depending on if it is easy or hard, in terms of pixels.
      int range;
      if (difficulty == "easy") {
        range=16;
      } else {
        range = 8;
      }

      //Gives notification that a user should be pressing around this time
      if (abs(note_sequence[i].y_coordinate + note_sequence[i].rect_height - 280 <= range)) {
        can_transition[note_sequence[i].lane - 1] = 1;
      }

      //Says that a point should be allocated, as the button is being held during the timing of the note.
      int note_state = 0;
      if (note_sequence[i].y_coordinate + note_sequence[i].rect_height - 280 >= 0 && note_sequence[i].y_coordinate - 280 <= 0) {
        could_count[note_sequence[i].lane - 1] = 1;
        //Node_state denotes that this specific note is one that should be played - is used to see when a note should no longer be played, as its movement
        // causes the note to move beyond the line
        note_state = 1;
      }

      
      if (note_sequence[i].finished ==0) {
        tft.drawRect(note_sequence[i].x_coordinate, note_sequence[i].y_coordinate - 4, 60, note_sequence[i].rect_height, TFT_BLACK);
        tft.drawRect(note_sequence[i].x_coordinate, note_sequence[i].y_coordinate, 60, note_sequence[i].rect_height, TFT_GREEN);
        if (millis() - start_time - note_sequence[i].start_time >= note_sequence[i].duration) {
          note_sequence[i].finished = 1;
        } else {
          note_sequence[i].rect_height += 4;
        }
      } else {
        tft.drawRect(note_sequence[i].x_coordinate, note_sequence[i].y_coordinate - 4, 60, note_sequence[i].rect_height, TFT_BLACK);
        tft.drawRect(note_sequence[i].x_coordinate, note_sequence[i].y_coordinate, 60, note_sequence[i].rect_height, TFT_GREEN);
        note_sequence[i].y_coordinate += 4;
        if (note_sequence[i].y_coordinate > 319) { 
          still_on_screen++;
        }
      }

      //This means that the note has moved beyond the line, and should thus no longer be counted.
      if (note_state == 1 &&  note_sequence[i].y_coordinate - 280 > 0) {
        should_count[note_sequence[i].lane - 1] = 0;
      }
  }
}

void parse_song_file(char* song_file){
  
  int t = 0;
  int time_index = 0;
  int note_index = 0;
  int duration_index = 0;
  char tim[10];
  char note[10];
  char duration[10];
  int t_index = 0;
  int n_index = 0;
  int d_index = 0;
  for (int i = 0; i< strlen(song_file); i++){
    
    if (t == 0){
      if (song_file[i] == '#'){
        t = t + 1;
        
      }
      else if (song_file[i] == '\n'){
        tim[t_index] = '\0';
        t_index = 0;
        times[time_index] = atoi(tim);
        ending_time = atoi(tim) + 10000;
        time_index++;
        
      }
      else{
        tim[t_index] = song_file[i];
        t_index++;
      }
    }
    else if (t == 1){
      if (song_file[i] == '#'){
        t = t + 1;
        
      }
      else if (song_file[i] == '\n'){
        note[n_index] = '\0';
        n_index = 0;
        notes[note_index] = atoi(note);
        note_index++;
        
      }
      else{
        note[n_index] = song_file[i];
        n_index++;
      }
    }
    else{
      if (song_file[i] == '\n'){
        duration[d_index] = '\0';
        d_index = 0;
        durations[duration_index] = atoi(duration);
        duration_index++;
        
      }
      else{
        duration[d_index] = song_file[i];
        d_index++;
      }
      
    }
  }
}

/*----------------------------------
  char_append Function:
  Arguments:
     char* buff: pointer to character array which we will append a
     char c:
     uint16_t buff_size: size of buffer buff

  Return value:
     boolean: True if character appended, False if not appended (indicating buffer full)
*/
uint8_t char_append(char* buff, char c, uint16_t buff_size) {
  int len = strlen(buff);
  if (len > buff_size) return false;
  buff[len] = c;
  buff[len + 1] = '\0';
  return true;
}

