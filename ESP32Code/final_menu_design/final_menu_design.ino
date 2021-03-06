#include <WiFi.h> //Connect to WiFi Network
#include <SPI.h>
#include <TFT_eSPI.h>
#include <mpu6050_esp32.h>
#include<math.h>
#include<string.h>

TFT_eSPI tft = TFT_eSPI();
const int SCREEN_HEIGHT = 160;
const int SCREEN_WIDTH = 128;
const int SELECT_BUTTON_PIN = 5;
const int CHANGE_BUTTON_PIN = 19;
const int REVERSE_BUTTON_PIN = 21;
const int LOOP_PERIOD = 40;
const int points = 150;
int done =0;

MPU6050 imu; //imu object called, appropriately, imu

char network[] = "MIT Uncensored";  //SSID for 6.08 Lab
char password[] = "E?3QjXep>&gy"; //Password for 6.08 Lab

//Some constants and some resources:
const int RESPONSE_TIMEOUT = 6000; //ms to wait for response from host
const uint16_t OUT_BUFFER_SIZE = 1000; //size of buffer to hold HTTP response
char old_response[OUT_BUFFER_SIZE]; //char array buffer to hold HTTP request
char response[OUT_BUFFER_SIZE]; //char array buffer to hold HTTP request
char leader_board_response[OUT_BUFFER_SIZE];
char host[] = "608dev-2.net";
char username[200];
char roomname[200];
char action[10];
int currently_in_game = 0;

uint32_t primary_timer;

int old_val;

enum MENU {UserNameInput, Player_Choice, Multiplayer_Menu, HostRoomNameInput, 
  Host_Waiting_Room, JoinRoomNameInput, Join_Waiting_Room, 
  Song_Menu, Instrument_Menu, Difficulty_Menu, PlaySong} menu = Player_Choice;

const char *song_choices[6] = { "\"Livin on a Prayer\": Bon Jovi", "\"September\": Earth, Wind, and Fire", "\"Barracuda\": Heart", "\"Slow Ride\": Foghat", "\"Dammit\": Blink-182"};
const char *instrument_choices[3] = { "Guitar", "Bass", "Drums" };
const char *multiplayer_choices[4] = { "Host Room", "Join Room", "View Rooms" };
const char *host_waiting_choices[3] = { "Start", "Cancel" };
const char *player_choices[3] = { "Singleplayer", "Multiplayer" };
const char *difficulty_choices[5] = {"Easy", "Medium", "Hard","Expert"};

char song_choice[200];
char instrument[200];
char difficulty[200];

//used to get x,y values from IMU accelerometer!
void get_angle(float* x, float* y) {
  imu.readAccelData(imu.accelCount);
  *x = imu.accelCount[0] * imu.aRes;
  *y = imu.accelCount[1] * imu.aRes;
}

//-------------------------------------------------------------General Classes -----------------------------------------------------------

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
    char alphabet[50] = " ABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789_";
    char msg[400] = {0}; //contains previous query response
    char query_string[50] = {0};
    int char_index;
    int state;
    uint32_t scrolling_timer;
    const int scrolling_threshold = 150;
    const float angle_threshold = 0.3;
    char* string_to_save_to;
    bool do_http_request_at_end;
  public:
    MENU menu_choice;

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
//        if (button == 1) {
          char_index = 0;
          strcpy(query_string, "");
          scrolling_timer = millis();
          state = 1;
//        }
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
          sprintf(request, "POST /sandbox/sc/nfaro/server.py HTTP/1.1\r\n");
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

//--------------------------------------Initialization of Generics----------------------------------

NameGetter ng("Input Username after long press.", Player_Choice, username, false);
NameGetter room_ng("Input Room name after long press.", Host_Waiting_Room, roomname, true);
Button button(SELECT_BUTTON_PIN);
Button change_button(CHANGE_BUTTON_PIN);
Button reverse_button(REVERSE_BUTTON_PIN);

//---------------------------------------------------Different Menu Types -----------------------------------------------------------------

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
          ng.menu_choice = Song_Menu;
        } else if (choice == 1) {
          ng.menu_choice = Multiplayer_Menu;
        }
        menu = UserNameInput;
      }
      return choice;
    }
};

class SongMenu: public Menu {
  public:
    int update(int change_button, int select_button) {
      if (change_button == 1) {
        choice = (choice + 1) % 5;

      }
      if (select_button == 1) {
        strcpy(song_choice, song_choices[choice]);
        menu = Instrument_Menu;
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
        menu = PlaySong;
      }

      return choice;
    }
};

class InstrumentMenu: public Menu {
  public:
    int update(int change_button, int select_button) {
      if (change_button == 1) {
        choice = (choice + 1) % 3;
        Serial.println(choice);
      }
      if (select_button == 1) {
        strcpy(instrument, instrument_choices[choice]);
        menu = Difficulty_Menu;
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
          sprintf(request, "POST /sandbox/sc/nfaro/server.py HTTP/1.1\r\n");
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
          Serial.println(menu);
        }
      }
      return choice;
    }
};

//---------------------------------------------------Initialization -----------------------------------------------------------------

InstrumentMenu instrument_menu;
SongMenu song_menu;
MultiplayerMenu multiplayer_menu;
HostWaitingRoomMenu hostWaitingRoomMenu;
PlayerChoiceMenu player_choice_menu;
DifficultyMenu difficulty_menu;

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
 
  tft.init();
  tft.setRotation(2);
  tft.setTextSize(1);
  tft.fillScreen(TFT_BLACK);
  tft.setTextColor(TFT_GREEN, TFT_BLACK); //set color of font to green foreground, black background
  pinMode(SELECT_BUTTON_PIN, INPUT_PULLUP);
  pinMode(CHANGE_BUTTON_PIN, INPUT_PULLUP);
  strcpy(instrument, instrument_choices[0]);
  primary_timer = millis();

  tft.fillScreen(TFT_BLACK);
  tft.setCursor(0, 0, 1);
}


MENU last_menu = Player_Choice;
int last_choice = 0;
bool change = true;

//---------------------------------------------------------------------Loop--------------------------------------------------------------

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
  } else if (menu == HostRoomNameInput) {
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
      sprintf(request, "GET /sandbox/sc/nfaro/server.py?user=%s?action=lobby HTTP/1.1\r\n", username);
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
      sprintf(request, "GET /sandbox/sc/nfaro/server.py?user=%s HTTP/1.1\r\n", username);
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
      tft.setCursor(0, 0, 1);
      tft.println(response);
      change = false;
    }
  } else if (menu == PlaySong) {
    if (currently_in_game == 0) {
      currently_in_game = 1;
      char request[500];
      char body[200];
      sprintf(body, "user=%s&song=%s&instrument=%s&difficulty=%s&action=startgame", username, song_choice, instrument, difficulty);
      sprintf(request, "POST /sandbox/sc/nfaro/server.py HTTP/1.1\r\n");
      sprintf(request + strlen(request), "Host: %s\r\n", host);
      strcat(request, "Content-Type: application/x-www-form-urlencoded\r\n");
      sprintf(request + strlen(request), "Content-Length: %d\r\n\r\n", strlen(body));
      strcat(request, body);
      Serial.println(response);
      do_http_request(host, request, response, OUT_BUFFER_SIZE, RESPONSE_TIMEOUT, true);
      Serial.println(response);
    }
    if (change) {
      char request[500];
      char body[200];
      if(done){
        Serial.println(leader_board_response);
        tft.fillScreen(TFT_BLACK);
        tft.setCursor(0, 0, 1);
        tft.println(leader_board_response);
        change = false;
      }
      else{
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
        do_http_request(host, request, leader_board_response, OUT_BUFFER_SIZE, RESPONSE_TIMEOUT, true);
        delay(5000);
        done = 1;
      }
      
      
    }
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
  }

  if (choice != last_choice || menu != last_menu) {
    change = true;
  }

  last_menu = menu;
  last_choice = choice;

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
