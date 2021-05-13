#include <WiFi.h> //Connect to WiFi Network
#include <SPI.h>
#include <TFT_eSPI.h>
#include <mpu6050_esp32.h>
#include <math.h>
#include <string.h>

TFT_eSPI tft = TFT_eSPI();
const int SCREEN_HEIGHT = 160;
const int SCREEN_WIDTH = 128;
const int SELECT_BUTTON_PIN = 0;
const int CHANGE_BUTTON_PIN = 5;
const int REVERSE_BUTTON_PIN = 14;
const int LOOP_PERIOD = 40;

MPU6050 imu; //imu object called, appropriately, imu

char network[] = "MIT Secure";  //SSID for 6.08 Lab
char password[] = "12345678"; //Password for 6.08 Lab


//Some constants and some resources:
const int RESPONSE_TIMEOUT = 6000; //ms to wait for response from host
const uint16_t OUT_BUFFER_SIZE = 5000; //size of buffer to hold HTTP response
char old_response[OUT_BUFFER_SIZE]; //char array buffer to hold HTTP request
char response[OUT_BUFFER_SIZE]; //char array buffer to hold HTTP request
char host[] = "608dev-2.net";

class Note {       // The class
  public:             // Access specifier
    int x_coordinate;        // Attribute (int variable)
    int y_coordinate;  // Attribute (string variable)
};

char username[200];
char roomname[200];
char action[10];
char current_song[20000];
int times[2000];
int notes[2000];
int durations[2000];
Note squares[100];

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

int global_index = 0;
int start;
int start_time;
int score_index = 1;

int points = 0;
int ammount = 0;
int ammount1 = 0;
int ammount2 = 0;
int ammount3 = 0;

const int BUTTON = 0;
const int BUTTON2 = 5;
const int BUTTON3 = 15;
const int BUTTON4 = 14;

const uint32_t PWM_CHANNEL = 0; //hardware pwm channel used in secon part
const uint32_t PWM_CHANNEL2 = 1;
const uint32_t PWM_CHANNEL3 = 2;
const uint32_t PWM_CHANNEL4 = 4;
const uint32_t PWM_CHANNEL5 = 5;


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

PWM_608 backlight(PIN1, 120, 1); 

//used to get x,y values from IMU accelerometer!
void get_angle(float* x, float* y) {
  imu.readAccelData(imu.accelCount);
  *x = imu.accelCount[0] * imu.aRes;
  *y = imu.accelCount[1] * imu.aRes;
}

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
        } else if (choice == 1) {
          menu = JoinRoomNameInput;
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
          menu = Multiplayer_Menu;
          reset_menu();
          Serial.println(menu);
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
  Serial.begin(115200); //for debugging if needed.
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
  // if (imu.setupIMU(1)) {
  //   Serial.println("IMU Connected!");
  // } else {
  //   Serial.println("IMU Not Connected :/");
  //   Serial.println("Restarting");
  //   ESP.restart(); // restart the ESP (proper way)
  // }
  tft.init();
  tft.setRotation(2);
  tft.setTextSize(1);
  tft.fillScreen(TFT_BLACK);
  tft.setTextColor(TFT_GREEN, TFT_BLACK); //set color of font to green foreground, black background
  pinMode(SELECT_BUTTON_PIN, INPUT_PULLUP);
  pinMode(CHANGE_BUTTON_PIN, INPUT_PULLUP);
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
  tft.setCursor(0, 0, 1);

  global_index = 0;
  start_time = millis();
    backlight.set_duty_cycle(60); //initialize the software PWM to be at 30%
  parse_song_file(response);
  for (int i = 0; i < 2000; i++){
    times[i] += 4000;
  }
}


//FOR SONG
// action = “startgame”
// user = username
// song
// instrument
// difficulty


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
      tft.setCursor(0, 0, 1);
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
    Serial.println(choice);
    if (change) {
      tft.fillScreen(TFT_BLACK);
      tft.setTextColor(TFT_GREEN, TFT_BLACK);
      tft.setCursor(12, 30, 1);
      int y_index[6] = {50, 65, 80};
      int x_index[6] = {47, 20, 10};
      for (int i = 0; i < 3; i++) {
        tft.setCursor(x_index[i], y_index[i], 1);
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
    

  }else if (menu == HostRoomNameInput) {
    int right_value = !digitalRead(CHANGE_BUTTON_PIN);
    int left_value = !digitalRead(REVERSE_BUTTON_PIN);
    strcpy(action, "create");
    room_ng.update(left_value, right_value, select_button_val, response); //input: angle and button, output String to display on this timestep
    if (change || strcmp(response, old_response) != 0) {//only draw if changed!
      tft.fillScreen(TFT_BLACK);
      tft.setCursor(0, 0, 1);
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
      sprintf(request, "GET /sandbox/sc/team49/server.py?user=%s HTTP/1.1\r\n", username);
      sprintf(request + strlen(request), "Host: %s\r\n\r\n", host);
      do_http_request(host, request, response, OUT_BUFFER_SIZE, RESPONSE_TIMEOUT, true);
      Serial.println(response);
      change = true;
    }

    choice = hostWaitingRoomMenu.update(change_button_val, select_button_val);
    if (change) {
      tft.fillScreen(TFT_BLACK);
      tft.setCursor(0, 0, 1);
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
      tft.setCursor(0, 0, 1);
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
      sprintf(request, "GET /sandbox/sc/team49/server.py?user=%s HTTP/1.1\r\n", username);
      sprintf(request + strlen(request), "Host: %s\r\n\r\n", host);
      do_http_request(host, request, response, OUT_BUFFER_SIZE, RESPONSE_TIMEOUT, true);
      Serial.println(response);
      if(strstr(response, "Readying") != NULL){
        menu = Instrument_Menu;
      }
      change = true;
    }
    if (change) {
      tft.fillScreen(TFT_BLACK);
      tft.setCursor(0, 0, 1);
      tft.println(response);
      change = false;
    }
  } else if (menu == PlaySong) {
      if (millis() - start_time > 60000) {
          char request[500];
          char body[200];
          tft.fillScreen(TFT_BLACK);
          tft.setCursor(40, 0, 1);
          tft.println("WELL DONE!");
          tft.setCursor(25, 10, 1);
          tft.println("Your Score Is:");
          tft.setCursor(55, 20, 1);
          tft.println(points);
          sprintf(body, "user=%s&song=%s&instruments=%s&score=%i&action=leaderboard", username, song_choice, instrument, points);
          sprintf(request, "POST /sandbox/sc/nfaro/server.py HTTP/1.1\r\n");
          sprintf(request + strlen(request), "Host: %s\r\n", host);
          strcat(request, "Content-Type: application/x-www-form-urlencoded\r\n");
          sprintf(request + strlen(request), "Content-Length: %d\r\n\r\n", strlen(body));
          strcat(request, body);
          Serial.println("This is the request");
          Serial.println(request);
          do_http_request(host, request, response, OUT_BUFFER_SIZE, RESPONSE_TIMEOUT, true);
          delay(5000);
          tft.fillScreen(TFT_BLACK);
          tft.setCursor(0, 0, 1);
          tft.println(response);

          delay(5000);

          tft.fillScreen(TFT_BLACK);
          tft.setCursor(0, 0, 1);
          menu = Exit_Menu;

          
      }
      draw_notes();
      if(detect_note()){
        if (notes[score_index]%10 == 1){
          if (!digitalRead(BUTTON3)){
            ledcWrite(PWM_CHANNEL4, (4095) - (4095 * 50/100.0));
            ledcWrite(PWM_CHANNEL5, (4095) - (4095 * 50/100.0));
            score_index += 1;
            points += 1;
          }
          else{
            ledcWrite(PWM_CHANNEL5, 0);
            ledcWrite(PWM_CHANNEL4, 0);
          }
        }
        if (notes[score_index]%10 == 2){
          if (!digitalRead(BUTTON2)){
            ledcWrite(PWM_CHANNEL3, (4095) - (4095 * 50/100.0));
            score_index += 1;
            points += 1;
          }
          else{
            ledcWrite(PWM_CHANNEL3, 0);
          }
        }
        if (notes[score_index]%10 == 3){
          if(!digitalRead(BUTTON)){
            ledcWrite(PWM_CHANNEL, (4095) - (4095 * 50/100.0));
            score_index += 1;
            points += 1;
          }
          else{
            ledcWrite(PWM_CHANNEL, 0);
          }
        }
        if (notes[score_index]%10 == 4){
          if(!digitalRead(BUTTON4)){
            ledcWrite(PWM_CHANNEL2, (4095) - (4095 * 50/100.0));
            score_index += 1;
            points += 1;
          }
          else{
            ledcWrite(PWM_CHANNEL2, 0);
          }
        }
        
      }
      if (!digitalRead(BUTTON)) {
        ledcWrite(PWM_CHANNEL, (4095) - (4095 * 50/100.0));
      }
      else{
        ledcWrite(PWM_CHANNEL, 0);
      }
      if(!digitalRead(BUTTON4)){
        ledcWrite(PWM_CHANNEL2, (4095) - (4095 * 50/100.0));
      }
      else{
        ledcWrite(PWM_CHANNEL2, 0);
      }
      if (!digitalRead(BUTTON2)){
        ledcWrite(PWM_CHANNEL3, (4095) - (4095 * 50/100.0));
      }
      else{
        ledcWrite(PWM_CHANNEL3, 0);
      }
      if (!digitalRead(BUTTON3)){
        ledcWrite(PWM_CHANNEL4, (4095) - (4095 * 50/100.0));
        ledcWrite(PWM_CHANNEL5, (4095) - (4095 * 50/100.0));
      }
      else{
        ledcWrite(PWM_CHANNEL5, 0);
        ledcWrite(PWM_CHANNEL4, 0);
    
      }
      delay(15);
  } else if (menu == Instrument_Menu) {
    choice = instrument_menu.update(change_button_val, select_button_val);
    if (change) {
      
      tft.fillScreen(TFT_BLACK);
      tft.setTextColor(TFT_GREEN, TFT_BLACK);
      tft.setCursor(10, 30, 1);
      tft.setTextSize(1.75);
      tft.println("Pick an Instrument:");
      tft.println("");
      tft.setTextSize(0.75);
      int y_index[6] = {60, 75, 90};
      int x_index[6] = {45, 52, 50};
      for (int i = 0; i < 3; i++) {
        tft.setCursor(x_index[i], y_index[i], 1);
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
      tft.setCursor(12, 30, 1);
      tft.setTextSize(1.75);
      tft.println("Pick a Difficulty:");
      tft.println("");
      tft.setTextSize(0.75);
      int y_index[6] = {60, 75, 90, 105};
      int x_index[6] = {52, 45, 52, 46};
      for (int i = 0; i < 4; i++) {
        tft.setCursor(x_index[i], y_index[i], 1);
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
      tft.setCursor(30, 30, 1);
      tft.setTextSize(1.75);
      tft.println("Pick a Song:");
      tft.println("");
      tft.setTextSize(0.75);
      int y_index[6] = {60, 80, 100, 115, 130};
      int x_index[6] = {0, 0, 0, 0, 0};
      for (int i = 0; i < 5; i++) {
        tft.setCursor(x_index[i], y_index[i], 1);
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
      tft.setCursor(8, 30, 1);
      tft.setTextSize(1.75);
      tft.println("Enter your username");
      tft.setCursor(50, 42, 1);
      tft.println("below:");
      tft.println("");
      tft.setTextSize(0.75);
      int newIndex = 60 - (strlen(response)) * 2.5;
      tft.setCursor(newIndex, 70, 1);
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
      tft.setCursor(22, 30, 1);
      tft.setTextSize(2);
      tft.setTextColor(TFT_RED, TFT_BLACK);
      tft.println("ARDUINO");
      tft.setTextSize(0.5);
      tft.println("");
      tft.setTextSize(2);
      tft.setCursor(40, 50, 1);
      tft.println("HERO");
      tft.println("");
      tft.setTextSize(0.75);
      int y_index[3] = {90, 105};
      int x_index[3] = {27, 30};
      tft.setTextColor(TFT_GREEN, TFT_BLACK);
      for (int i = 0; i < 2; i++) {
        tft.setCursor(x_index[i], y_index[i], 1);
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
        do_http_request(host, requestO, current_song, OUT_BUFFER_SIZE, RESPONSE_TIMEOUT, true);
        Serial.println(current_song);

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
      tft.setCursor(0, 0, 1);
      tft.println("Waiting");
      tft.println(username);
      tft.println(instrument);
      choice = last_choice;
      primary_timer = millis();

      char request[500];
      char body[200];
      sprintf(body, "user=%s&roomname=%s&action=waiting&password=PASSWORD", username, roomname);
      Serial.println("finishes thing");
      sprintf(request, "POST /sandbox/sc/team49/server.py HTTP/1.1\r\n");
      sprintf(request + strlen(request), "Host: %s\r\n", host);
      strcat(request, "Content-Type: application/x-www-form-urlencoded\r\n");
      sprintf(request + strlen(request), "Content-Length: %d\r\n\r\n", strlen(body));
      strcat(request, body);
      Serial.println("Finishes copying");
      do_http_request(host, request, response, OUT_BUFFER_SIZE, RESPONSE_TIMEOUT, true);
      if(strstr(response, "Waiting") == NULL){
        Serial.println(response);
        Serial.println(millis());
        delay(atoi(response));
        Serial.println(millis());
        menu = PlaySong;
        backlight.set_duty_cycle(60); //initialize the software PWM to be at 30%
        parse_song_file(current_song);
        for (int i = 0; i < 2000; i++){
          times[i] += 4000;
        }
        global_index = 0;
        start_time = millis();
        start = 0;
      }
    }
  }

  if (choice != last_choice || menu != last_menu) {
    change = true;
  }

  last_menu = menu;
  last_choice = choice;
}
bool detect_note(){
  if(millis() - start_time + 70 >= times[score_index] && millis()- start_time - 150 <= times[score_index]){
    return true;
  }
  else if(millis() - start_time -151 > times[score_index]){
    score_index += 1;
    return false;
  }
  else{
    return false;
  }
}

void draw_notes(){
//  char array_note[2] = notes[global_index]
  if (millis()- start_time + 3670 >= times[global_index]){
      if(notes[global_index]%10 == 1){
        Note new_note;
        new_note.x_coordinate = 0; 
        new_note.y_coordinate= 0;
        squares[global_index] = new_note;
      }
      else if(notes[global_index]%10 == 2){
        Note new_note;
        new_note.x_coordinate = 31; 
        new_note.y_coordinate= 0;
        squares[global_index] = new_note;
      }
      else if(notes[global_index]%10 == 3){
        Note new_note;
        new_note.x_coordinate = 62; 
        new_note.y_coordinate= 0;
        squares[global_index] = new_note;
      }
      else if(notes[global_index]%10 == 4){
        Note new_note;
        new_note.x_coordinate = 93; 
        new_note.y_coordinate= 0;
        squares[global_index] = new_note;
      }
      global_index ++;
  }

    // Create an object of MyClass

  // Access attributes and set values
  
//  Serial.println("this is the start");
//  Serial.println(start);
  tft.fillScreen(TFT_BLACK);
  tft.fillRect(0, 140, 160, 1, TFT_GREEN);
  for (int i = 0; i < 100; i++){
      tft.drawRect(squares[i].x_coordinate, squares[i].y_coordinate, 30, 15, TFT_GREEN);
      squares[i].y_coordinate += 2;

  }
  
}

void random_delay() {
  int random_val = random(0, 10000); //get a random number between 0 and 100
  if (random_val > 9998) delay(random(0, 4));
}

void parse_song_file(char* song_file){
  char time_char[2000];
  char note_char[2000];
  char duration_char[2000];
  int t = 0;
  int time_index = 0;
  int note_index = 0;
  int duration_index = 0;
  for (int i = 0; i< strlen(song_file); i++){
    
    if (t == 0){
      if (song_file[i] == '#'){
        
        t = t + 1;
        time_char[time_index] ='\0';
        
      }
      else{
        time_char[time_index] = song_file[i];
        time_index++;
      }
    }
    else if (t == 1){
      if (song_file[i] == '#'){
        t = t + 1;
        note_char[note_index] ='\0';
        
      }
      else{
        note_char[note_index] = song_file[i];
        note_index++;
      }
    }
    else{
        
        duration_char[duration_index] = song_file[i];
        duration_index++;
      
    }
  }

  char tim[10];
  int tim_index = 0;
  int times_index = 0;
  for (int i = 0; i < strlen(time_char); i++){
      if (time_char[i] == '\n'){
          tim[tim_index] = '\0';
          int num = atoi(tim);
         
          times[times_index] = num;
          times_index++;
          tim_index = 0;
      }
      else{
        tim[tim_index] = time_char[i];
        tim_index++;
      }
      
  }

  
  times[times_index] = '\0';
  note_char[note_index] = '\0';
  char note[10];
  int n_index = 0;
  note_index = 0;
  for (int i = 0; i < strlen(note_char); i++){
    if (note_char[i] == '\n'){
        note[strlen(note)] = '\0';
        if (strlen(note) > 1){
          notes[note_index] = atoi(note);
        }
        else{
          notes[note_index] = (int) note;
        }
        n_index = 0;
        note_index++;
    }
    else{
      note[n_index] = note_char[i];
      n_index++;
      
    }
  }

  char duration[10];
  duration_char[duration_index] = '\0';
  n_index = 0;
  duration_index = 0;
  for (int i = 0; i < strlen(duration_char); i++){
    if (duration_char[i] == '\n'){
        duration[n_index] = '\0';
        durations[duration_index] = atoi(duration);
        duration[0] = '\0';
        n_index = 0;
        duration_index++;
    }
    else{
      
      duration[n_index] = duration_char[i];
      n_index++;
      
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
