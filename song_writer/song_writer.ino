
#include <TFT_eSPI.h>


TFT_eSPI tft = TFT_eSPI();
const int SCREEN_HEIGHT = 160;
const int SCREEN_WIDTH = 128;
const int BPM = 100;
const int MILLIS_PER_MINUTE = 60000;
const int NOTE_SPEED = 16; //sixteenth notes
const float MINUTE_PER_BEAT = 1.0/(float)BPM;
const float BEAT_PER_NOTE = 4.0/ (float)NOTE_SPEED;
const float MILLIS_PER_NOTE = MINUTE_PER_BEAT*BEAT_PER_NOTE*MILLIS_PER_MINUTE;
const int BUTTON_PIN_1 = 15;
const int BUTTON_PIN_2 = 17;
const int BUTTON_PIN_3 = 5;
const int BUTTON_PIN_4 = 22;

class Button{
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
  
//feel free to use case instead of if-else-if!!
  int update() {
    read();
    flag = 0;
    if (state==0) {
      if (button_pressed) {
        state = 1;
        button_change_time =millis();
        
      }
    } else if (state==1) {
      // CODE HERE
      if(button_pressed){
        if (millis() - button_change_time >= debounce_duration){
          state = 2;
          state_2_start_time = millis();
        }
      }
      else{
        button_change_time = millis();
        state = 0;
      }
      
    } else if (state==2) {
      // CODE HERE
        if (button_pressed){
          if(millis() - state_2_start_time >= long_press_duration){
            state = 3;
          }
        }
        else{
          state = 4;
        }
      
    } else if (state==3) {
      // CODE HERE
        if(!button_pressed){
          state = 4;
          button_change_time = millis();
        }
    } else if (state==4) {        
      // CODE HERE
      if(button_pressed){
        button_change_time = millis();
        if(millis() - state_2_start_time >= long_press_duration){
          state = 3;
          
        }
        else{
          state = 2;
        }
      }
      else{
        
        if(millis() - button_change_time >= debounce_duration){
          state = 0;
          if (millis() - state_2_start_time >= long_press_duration){
            flag = 2;
          }
          else{
            flag = 1;
          }
        }
      }
    }
    return flag;
  }
};

Button b1(BUTTON_PIN_1);
Button b2(BUTTON_PIN_2);
Button b3(BUTTON_PIN_3);
Button b4(BUTTON_PIN_4);

int note;
int reading_period;
char song_string[2400];
int last_read;


void setup() {
  // put your setup code here, to run once:
  Serial.begin(115200);
  note = 0;
  song_string[note] = '\0';
  last_read = millis();
  tft.init();
  tft.setRotation(2);
  tft.setTextSize(1);
  tft.fillScreen(TFT_BLACK);
  
  
  reading_period = (int) MILLIS_PER_NOTE;
}

void loop() {
  // put your main code here, to run repeatedly:
  if (millis() - last_read >= reading_period){
    last_read = millis();
    int bv1 = b1.update();
    int bv2 = b2.update();
    int bv3 = b3.update();
    int bv4 = b4.update();
    Serial.printf("NOTE %d\n", note);
    note++;
    
  }
    

}
