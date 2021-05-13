
#include <SPI.h>
#include <TFT_eSPI.h>

TFT_eSPI tft = TFT_eSPI();

class Note {       // The class
  public:             // Access specifier
    int x_coordinate;        // Attribute (int variable)
    int y_coordinate;  // Attribute (string variable)
};

char input_file[] = "ESP_song_files/less_i_know_the_better";
char response[] = "897\n1282\n1666\n1794\n2051\n2564\n3076\n3589\n4102\n4615\n5128\n5384\n5641\n6153\n6666\n7179\n7692\n8205\n8717\n9230\n9487\n9743\n10000\n10256\n10769\n11282\n11794\n12307\n12820\n13333\n13589\n13846\n14358\n14871\n15384\n15897\n16410\n16923\n17435\n17692\n17948\n18205\n18461\n18974\n19487\n20000\n20512\n21025\n21538\n21794\n22051\n22564\n23076\n23589\n24102\n24615\n25128\n25641\n25897\n26153\n26410\n26666\n29743\n30769\n33846\n34871\n37948\n38974\n42051\n43076\n46153\n47179\n50256\n51282\n54359\n55384\n58461\n59487\n60000\n60512\n61025\n61538\n62051\n62564\n62820\n63076\n63589\n64102\n64615\n65128\n#4\n3\n3\n2\n1\n1\n3\n2\n1\n4\n4\n3\n3\n2\n2\n4\n3\n2\n4\n4\n3\n2\n2\n1\n1\n3\n2\n1\n4\n4\n3\n3\n2\n2\n4\n3\n2\n4\n4\n3\n2\n2\n1\n1\n3\n2\n1\n4\n4\n3\n3\n2\n2\n4\n3\n2\n4\n4\n3\n2\n2\n13\n23\n24\n23\n13\n23\n24\n23\n13\n23\n24\n23\n13\n23\n24\n23\n1\n1\n3\n2\n1\n4\n4\n3\n3\n2\n2\n4\n3\n";

int times[2000];
int notes[2000];
Note squares[100];

int global_index = 0;
int start;


const int RESPONSE_TIMEOUT = 6000; //ms to wait for response from host
const uint16_t OUT_BUFFER_SIZE = 1000; //size of buffer to hold HTTP response


//we'll use a different pin for each PWM so we can run them side-by-side
const uint8_t PIN1 = 26; //pin we use for software PWM
const uint8_t PIN2 = 25; //pin we use for hardware PWM
const uint8_t PIN3 = 27;
const uint8_t PIN4 = 12;
const uint8_t PIN5 = 13;

const int BUTTON = 5;
const int BUTTON2 = 14;
const int BUTTON3 = 15;
const int BUTTON4 = 0;
int points = 0;
int ammount = 0;
int ammount1 = 0;
int ammount2 = 0;
int ammount3 = 0;

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

PWM_608 backlight(PIN1, 120, 1); //create instance of PWM to control backlight on pin 1, operating at 50 Hz

void setup() {
  Serial.begin(115200); // Set up serial port

  //For Lab05a second part, you will uncomment the next two lines
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
  
  


  tft.init();  //init screen
  tft.setRotation(2); //adjust rotation
  tft.setTextSize(1); //default font size
  tft.fillScreen(TFT_BLACK); //fill background
  tft.setTextColor(TFT_GREEN, TFT_BLACK); //set color for font
  
  backlight.set_duty_cycle(60); //initialize the software PWM to be at 30%
  parse_song_file(response);
  for (int i = 0; i < 2000; i++){
    times[i] += 4000;
    
  }
  global_index = 0;
  start = 0;

  tft.setCursor(0, 0, 2); //set cursor, font size 1
  
}

void draw_notes(){
 
  if (millis() + 3670 >= times[global_index]){
      if(notes[global_index] == 1){
        Note new_note;
        new_note.x_coordinate = 0; 
        new_note.y_coordinate= 0;
        squares[global_index] = new_note;
      }
      else if(notes[global_index] == 2){
        Note new_note;
        new_note.x_coordinate = 31; 
        new_note.y_coordinate= 0;
        squares[global_index] = new_note;
      }
      else if(notes[global_index] == 3){
        Note new_note;
        new_note.x_coordinate = 62; 
        new_note.y_coordinate= 0;
        squares[global_index] = new_note;
      }
      else if(notes[global_index] == 4){
        Note new_note;
        new_note.x_coordinate = 93; 
        new_note.y_coordinate= 0;
        squares[global_index] = new_note;
      }
      global_index ++;
  }

    // Create an object of MyClass

  // Access attributes and set values
  
  Serial.println("this is the start");
  Serial.println(start);
  tft.fillScreen(TFT_BLACK);
  tft.fillRect(0, 140, 160, 1, TFT_GREEN);
  for (int i = 0; i < 100; i++){
      tft.drawRect(squares[i].x_coordinate, squares[i].y_coordinate, 30, 15, TFT_GREEN);
      squares[i].y_coordinate += 2 ;

  }
  
}



void loop() {
  draw_notes();
//  points+= ammount+ammount1+ammount2+ammount3;
  if (!digitalRead(BUTTON)) {
    ledcWrite(PWM_CHANNEL, (4095) - (4095 * 50/100.0));

//    tft.fillScreen(TFT_BLACK);
    tft.setCursor(0,0,1);
    tft.setTextColor(TFT_GREEN, TFT_BLACK);
//    tft.print(points);
//    ammount = 1;
  }
  else{
    ledcWrite(PWM_CHANNEL, 0);
//    ammount = 0;
  }
  if(!digitalRead(BUTTON4)){
    
    ledcWrite(PWM_CHANNEL2, (4095) - (4095 * 50/100.0));
//
//    tft.fillScreen(TFT_BLACK);
//    tft.setCursor(0,0,1);
//    tft.setTextColor(TFT_GREEN, TFT_BLACK);
//    tft.print(points);
//    ammount1 = 1;
  }
  else{
    ledcWrite(PWM_CHANNEL2, 0);
    ammount1 = 0;
  }
  if (!digitalRead(BUTTON2)){
    ledcWrite(PWM_CHANNEL3, (4095) - (4095 * 50/100.0));

//    tft.fillScreen(TFT_BLACK);
//    tft.setCursor(0,0,1);
//    tft.setTextColor(TFT_GREEN, TFT_BLACK);
//    tft.print(points);
//    ammount2 = 1;
  }
  else{
    ledcWrite(PWM_CHANNEL3, 0);
//    ammount2 = 0;
  }
  if (!digitalRead(BUTTON3)){
    ledcWrite(PWM_CHANNEL4, (4095) - (4095 * 50/100.0));
    ledcWrite(PWM_CHANNEL5, (4095) - (4095 * 50/100.0));

//    tft.fillScreen(TFT_BLACK);
//    tft.setCursor(0,0,1);
//    tft.setTextColor(TFT_GREEN, TFT_BLACK);
//    tft.print(points);
//    ammount3 = 1;
  }
  else{
    ledcWrite(PWM_CHANNEL5, 0);
    ledcWrite(PWM_CHANNEL4, 0);
//    ammount3 = 0;
  }
  delay(15);

  
}

//This function is used to simulate some other task with an unpredictable delay
//it can stand in for a sensor measurement, or an http GET or http POST
//we could do this for real, but since we're not really using WiFi today we're avoiding it so
//things compile faster :)
void random_delay() {
  int random_val = random(0, 10000); //get a random number between 0 and 100
  if (random_val > 9998) delay(random(0, 4));
}



void parse_song_file(char* song_file){

  char time_char[5000];
  char note_char[2000];
  bool t = 0;
  int time_index = 0;
  int note_index = 0;
  for (int i = 0; i< strlen(song_file); i++){
    if (!t){
      if (song_file[i] == '#'){
        t = 1;
        
        time_char[time_index] ='\0';
        
      }
      else{
        time_char[time_index] = song_file[i];
        time_index++;
      }
    }
    else{
      
      note_char[note_index] = song_file[i];
      note_index++;
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
//  Serial.println("NOTES: ");
//  Serial.println(note_char);
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
  
}
