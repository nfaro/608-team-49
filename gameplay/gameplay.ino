#include <WiFi.h> //Connect to WiFi Network
#include <SPI.h>
#include <TFT_eSPI.h>
#include <cmath>
#include <DFRobotDFPlayerMini.h>


HardwareSerial mySoftwareSerial(2);
DFRobotDFPlayerMini myDFPlayer;

TFT_eSPI tft = TFT_eSPI();
int playing = 0;

//NEEDS TO BE ADDED AS THERE ARE A LOT OF NEW FIELDS
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
//---------------------------------------------

const int RESPONSE_TIMEOUT = 6000; //ms to wait for response from host

//NEEDS TO BE ADDED WITH NEW OUT BUFFER SIZE
const uint16_t OUT_BUFFER_SIZE = 10000; //size of buffer to hold HTTP response
const int MAX_INT_ARRAY_SIZE = 800; //longest int section in library
const int NOTE_OFFSET = 3000;
const int SCROLL_TIME = 2100;
//---------------------------------------------

char host[] = "608dev-2.net";
char leader_board_response[2000];
char response[OUT_BUFFER_SIZE];

//char response[] = "0\n272\n545\n818\n1090\n1363\n1909\n2181\n2454\n2727\n3000\n3545\n3818\n4090\n4909\n5181\n5454\n5727\n6545\n7363\n7500\n7636\n8727\n9000\n9272\n9545\n9818\n10090\n10636\n10909\n11181\n11454\n11727\n12272\n12545\n12818\n13090\n13636\n13909\n14181\n14454\n15272\n16090\n16227\n16363\n17454\n18000\n18818\n19363\n19636\n20181\n20454\n21000\n21272\n21818\n22363\n22636\n23181\n23727\n24000\n24545\n24818\n25363\n25636\n26181\n26727\n27000\n27545\n28090\n28363\n28909\n29181\n29727\n30000\n30545\n31090\n31363\n31909\n32454\n32727\n33272\n33545\n34090\n34363\n34909\n36272\n36681\n37090\n38454\n38863\n39272\n40636\n41045\n41454\n43363\n43636\n45000\n45409\n45818\n47181\n48000\n49363\n49772\n50181\n52363\n52909\n53181\n53727\n54272\n54545\n55090\n55363\n55909\n56181\n56727\n57272\n57545\n58090\n58636\n58909\n59454\n59727\n60272\n60545\n61090\n61363\n61636\n62454\n63000\n63272\n63545\n64636\n64909\n65454\n65727\n66000\n66272\n66545\n66818\n67636\n68454\n68590\n68727\n76363\n77181\n77318\n77454\n85090\n85909\n86045\n86181\n87272\n87545\n87818\n88090\n88363\n88636\n89181\n89454\n89727\n90000\n90272\n90818\n91090\n91363\n92181\n92454\n92727\n93000\n93818\n94636\n94772\n94909\n96000\n96545\n97363\n97909\n98181\n98727\n99000\n99545\n99818\n100363\n100909\n101181\n101727\n102272\n102545\n102818\n103090\n103363\n103909\n104181\n104727\n105272\n105545\n106090\n106636\n106909\n107454\n107727\n108272\n108545\n109090\n109636\n109909\n110454\n111000\n111272\n111818\n112091\n112636\n112909\n113454\n113727\n114000\n114818\n115363\n115636\n115909\n117000\n117272\n117818\n118091\n118363\n119181\n119727\n120000\n120272\n121363\n121636\n121909\n122181\n122454\n122727\n123545\n124091\n124363\n124636\n125727\n126000\n126545\n126818\n127091\n127363\n127636\n127909\n128727\n129545\n129681\n129818\n137454\n138272\n138409\n138545\n146181\n147000\n147136\n147272\n148363\n148636\n148909\n149181\n149454\n149727\n150000\n150272\n150545\n150818\n151091\n151363\n151636\n151909\n152181\n152454\n152727\n153000\n153272\n153545\n153818\n154091\n154363\n154636\n154909\n157091\n157363\n157636\n157909\n158181\n158454\n158727\n159000\n159272\n159545\n159818\n160091\n160363\n160636\n160909\n161181\n161454\n162000\n162272\n162818\n163363\n163636\n164181\n164454\n165000\n165272\n165818\n166363\n166636\n167181\n167727\n168000\n168272\n168545\n168818\n169363\n169636\n170181\n170727\n171000\n171545\n172091\n172363\n172909\n173181\n173727\n174000\n174545\n175091\n175363\n175909\n176454\n176727\n177272\n177545\n178091\n178363\n178909\n179454\n179727\n180272\n180818\n181091\n181636\n181909\n182454\n182727\n183272\n183818\n184091\n184636\n185181\n185454\n185727\n186000\n186272\n186818\n187091\n187636\n188181\n188454\n189000\n189545\n189818\n190363\n190636\n191181\n191454\n192000\n192545\n192818\n193363\n193909\n194181\n194727\n195000\n195545\n195818\n196363\n196636\n196909\n197727\n198272\n198545\n198818\n199909\n200181\n200727\n201000\n201272\n202091\n202636\n202909\n203181\n203454\n204272\n204545\n205091\n205363\n205636\n206454\n207000\n207272\n207545\n208636\n208909\n209454\n209727\n210000\n210818\n211363\n211636\n211909\n212181\n213000\n213272\n#1\n1\n4\n3\n4\n3\n12\n3\n2\n3\n2\n1\n2\n1\n13\n13\n13\n13\n4\n3\n2\n1\n1\n1\n4\n3\n4\n3\n12\n3\n2\n3\n2\n1\n2\n1\n12\n24\n24\n24\n24\n4\n3\n2\n1\n12\n12\n23\n1\n13\n13\n13\n23\n23\n12\n12\n12\n23\n1\n13\n13\n13\n23\n23\n12\n12\n12\n23\n1\n13\n13\n13\n23\n23\n12\n12\n12\n23\n1\n13\n13\n13\n23\n23\n4\n4\n23\n1\n12\n3\n4\n4\n23\n1\n2\n4\n4\n23\n1\n12\n4\n4\n23\n1\n12\n12\n12\n23\n1\n13\n13\n13\n23\n23\n12\n12\n12\n23\n1\n13\n13\n13\n23\n23\n12\n12\n12\n23\n1\n13\n13\n23\n23\n12\n12\n24\n24\n24\n24\n4\n3\n2\n1\n4\n3\n2\n1\n4\n3\n2\n1\n1\n1\n4\n3\n4\n3\n12\n3\n2\n3\n2\n1\n2\n1\n13\n13\n13\n13\n4\n3\n2\n1\n12\n12\n23\n1\n13\n13\n13\n23\n23\n12\n12\n12\n23\n1\n13\n13\n13\n13\n23\n23\n12\n12\n12\n23\n1\n13\n13\n13\n23\n23\n12\n12\n12\n23\n1\n13\n13\n13\n23\n23\n12\n12\n12\n23\n1\n13\n13\n23\n23\n12\n12\n12\n23\n1\n13\n13\n23\n23\n23\n12\n12\n12\n23\n1\n13\n13\n23\n23\n12\n12\n24\n24\n24\n24\n4\n3\n2\n1\n4\n3\n2\n1\n4\n3\n2\n1\n12\n1\n12\n1\n12\n12\n12\n13\n13\n13\n13\n13\n13\n13\n13\n1\n23\n23\n23\n23\n23\n23\n23\n23\n14\n12\n1\n12\n1\n12\n12\n12\n13\n13\n13\n13\n13\n13\n13\n13\n1\n12\n12\n12\n23\n1\n13\n13\n13\n23\n23\n12\n12\n12\n23\n1\n13\n13\n13\n13\n23\n23\n12\n12\n12\n23\n1\n13\n13\n13\n23\n23\n12\n12\n12\n23\n1\n13\n13\n13\n23\n23\n12\n12\n12\n23\n1\n13\n13\n13\n23\n23\n12\n12\n12\n23\n1\n13\n13\n13\n13\n23\n23\n12\n12\n12\n23\n1\n13\n13\n13\n23\n23\n12\n12\n12\n23\n1\n13\n13\n13\n23\n23\n12\n12\n12\n23\n1\n13\n13\n23\n23\n12\n12\n12\n23\n1\n13\n13\n13\n23\n23\n12\n12\n12\n23\n1\n13\n13\n23\n23\n12\n12\n12\n23\n1\n13\n13\n13\n23\n23\n#136\n136\n136\n136\n136\n136\n136\n136\n136\n136\n136\n136\n136\n136\n272\n272\n272\n272\n272\n136\n136\n1090\n136\n136\n136\n136\n136\n136\n136\n136\n136\n136\n136\n136\n136\n136\n545\n272\n272\n272\n272\n272\n136\n136\n1090\n545\n272\n272\n272\n545\n272\n272\n272\n272\n545\n272\n272\n272\n272\n545\n272\n272\n272\n272\n545\n272\n272\n272\n272\n545\n272\n272\n272\n272\n545\n272\n272\n272\n272\n545\n272\n272\n272\n272\n1363\n272\n409\n1363\n409\n409\n1363\n272\n409\n1363\n272\n1363\n272\n409\n1363\n409\n1363\n272\n409\n1363\n545\n272\n272\n272\n272\n545\n272\n272\n272\n272\n545\n272\n272\n272\n272\n545\n272\n272\n272\n272\n272\n272\n272\n272\n272\n272\n272\n272\n272\n272\n272\n272\n272\n272\n272\n272\n136\n136\n1090\n272\n136\n136\n1090\n272\n136\n136\n1090\n136\n136\n136\n136\n136\n136\n136\n136\n136\n136\n136\n136\n136\n136\n272\n272\n272\n272\n272\n136\n136\n1090\n545\n272\n272\n272\n545\n272\n272\n272\n272\n545\n272\n272\n272\n272\n272\n272\n272\n272\n272\n272\n545\n272\n272\n272\n272\n545\n272\n272\n272\n272\n545\n272\n272\n272\n272\n545\n272\n272\n272\n272\n272\n272\n272\n272\n272\n272\n272\n272\n272\n272\n272\n272\n272\n272\n272\n272\n272\n272\n272\n272\n272\n272\n272\n272\n272\n272\n272\n272\n272\n272\n272\n272\n272\n272\n272\n136\n136\n1090\n272\n136\n136\n1090\n272\n136\n136\n1090\n272\n136\n136\n136\n136\n136\n136\n136\n136\n136\n136\n136\n136\n136\n136\n136\n136\n136\n136\n136\n136\n136\n136\n136\n2181\n272\n136\n136\n136\n136\n136\n136\n272\n136\n136\n136\n136\n136\n136\n136\n136\n545\n272\n272\n272\n272\n545\n272\n272\n272\n272\n545\n272\n272\n272\n272\n272\n272\n272\n272\n272\n272\n545\n272\n272\n272\n272\n545\n272\n272\n272\n272\n545\n272\n272\n272\n272\n545\n272\n272\n272\n272\n545\n272\n272\n272\n272\n545\n272\n272\n272\n272\n545\n272\n272\n272\n272\n272\n272\n272\n272\n272\n272\n545\n272\n272\n272\n272\n545\n272\n272\n272\n272\n545\n272\n272\n272\n272\n545\n272\n272\n272\n272\n272\n272\n272\n272\n272\n272\n272\n272\n272\n272\n272\n272\n272\n272\n272\n272\n272\n272\n272\n272\n272\n272\n272\n272\n272\n272\n272\n272\n272\n272\n272\n272\n272\n272\n272\n272\n272\n272\n";


//THIS NEEDS BLOCK NEEDS TO BE ADDED TO GLOBAL VARIABLES
int times[MAX_INT_ARRAY_SIZE];
int notes[MAX_INT_ARRAY_SIZE];
int durations[MAX_INT_ARRAY_SIZE];

//char time_char[MAX_CHAR_ARRAY_SIZE];
//char note_char[MAX_CHAR_ARRAY_SIZE];
//char duration_char[MAX_CHAR_ARRAY_SIZE];
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
//---------------------------------------------


char username[] = "Andrei";
char instrument[] = "Guitar"; 

char difficulty[] = "medium";

const char *song_choices[12] = { "Every Morning", "Fluorescent Adolescent", "My Own Worst Enemy", "Photograph", "Still Into You", "The Less I Know the Better", "Under Cover of Darkness", "What You Know", "When I Come Around"};

char song[] = "photograph_guitar_medium";
int mp3_song = 4;
//MP3 MAPPING FOR SONG S AT INDEX I IN song_choices IS I+1

const char *instrument_choices[3] = { "Guitar", "Drums" };
const char *difficulty_choices[5] = {"Easy", "Medium", "Hard","Expert"};


char network[] = "MIT Uncensored";  //SSID for 6.08 Lab
char password[] = "E?3QjXep>&gy"; //Password for 6.08 Lab



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
  while (WiFi.status() != WL_CONNECTED && count < 3) {
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
  char request[500];
  
//  
  sprintf(request, "GET /sandbox/sc/gagordon/final_project/song_file_handler.py?song_file_name=%s HTTP/1.1\r\n", song);
  strcat(request, "Host: 608dev-2.net\r\n"); //add more to the end
  strcat(request, "Content-Type: application/x-www-form-urlencoded\r\n");
  strcat(request, "\r\n"); //add blank line!
//  submit to function that performs GET.  It will return output using response_buffer char array
  do_http_GET("608dev-2.net", request, response, OUT_BUFFER_SIZE, RESPONSE_TIMEOUT, true);
//  Serial.println(response); //print to serial monitor

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

  //NEW VARS HERE IN SETUP NEED TO BE ADDED
  for (int i = 0; i < 2000; i++){
    times[i] += NOTE_OFFSET;
  }
  global_index = 0;
  start = 0;
  tft.setCursor(0, 0, 2); //set cursor, font size 1
  start_time = millis();
  //---------------------------------------------
  //myDFPlayer.play(1);
}
bool detect_note(){
  if(millis() + 70 >= times[score_index] && millis() - 150 <= times[score_index]){
    return true;
  }
  else if(millis() -151 > times[score_index]){
    score_index += 1;
    return false;
  }
  else{
    return false;
  }
}

//NEEDS TO BE ADDED, JUST COPY PASTE OVER THE OLD ONE
void draw_notes(){
  

  if (millis() -start_time >= times[global_index]){
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
        new_note.x_coordinate = 31; 
        new_note.y_coordinate= 0;
        new_note.rect_height = 0;
        new_note.start_time = times[global_index];
        new_note.duration = durations[global_index];
        new_note.lane = 2;
        note_sequence[start] = new_note;
      }
      else if(temp_note == 3){
        Note new_note;
        new_note.x_coordinate = 62; 
        new_note.y_coordinate= 0;
        new_note.rect_height = 0;
        new_note.start_time = times[global_index];
        new_note.duration = durations[global_index];
        new_note.lane = 3;
        note_sequence[start] = new_note;
      }
      else if(temp_note == 4){
        Note new_note;
        new_note.x_coordinate = 93; 
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
  
//  Serial.println("this is the start");
//  Serial.println(start);
  tft.fillScreen(TFT_BLACK);
  tft.fillRect(0, 140, 160, 1, TFT_GREEN);
  for(int l = 0; l < 4; l++) {
    can_transition[l] = 0;
    could_count[l] = 0;
  }
  for (int i = still_on_screen; i < start; i++){

      //Sets buffer depending on if it is easy or hard, in terms of pixels.
      int range;
      if (difficulty == "easy") {
        range=8;
      } else {
        range = 4;
      }

      //Gives notification that a user should be pressing around this time
      if (abs(note_sequence[i].y_coordinate + note_sequence[i].rect_height - 140 <= range)) {
        can_transition[note_sequence[i].lane - 1] = 1;
      }

      //Says that a point should be allocated, as the button is being held during the timing of the note.
      int note_state = 0;
      if (note_sequence[i].y_coordinate + note_sequence[i].rect_height - 140 >= 0 && note_sequence[i].y_coordinate - 140 <= 0) {
        could_count[note_sequence[i].lane - 1] = 1;
        //Node_state denotes that this specific note is one that should be played - is used to see when a note should no longer be played, as its movement
        // causes the note to move beyond the line
        note_state = 1;
      }

      
      if (note_sequence[i].finished ==0) {
        tft.drawRect(note_sequence[i].x_coordinate, note_sequence[i].y_coordinate, 30, note_sequence[i].rect_height, TFT_GREEN);
        if (millis() - start_time - note_sequence[i].start_time >= note_sequence[i].duration) {
          note_sequence[i].finished = 1;
        } else {
          note_sequence[i].rect_height += 2;
        }
      } else {
        tft.drawRect(note_sequence[i].x_coordinate, note_sequence[i].y_coordinate, 30, note_sequence[i].rect_height, TFT_GREEN);
        note_sequence[i].y_coordinate += 2;
        if (note_sequence[i].y_coordinate > 159) {
          still_on_screen++;
        }
      }

      //This means that the note has moved beyond the line, and should thus no longer be counted.
      if (note_state == 1 &&  note_sequence[i].y_coordinate - 140 > 0) {
        should_count[note_sequence[i].lane - 1] = 0;
      }
  }
}
//---------------------------------------------


void loop() {
  if(millis() - start_time >= ending_time){
    char request[500];
    char body[200];
    tft.fillScreen(TFT_BLACK);
    tft.setCursor(40, 0, 1);
    tft.println("WELL DONE!");
    tft.setCursor(25, 10, 1);
    tft.println("Your Score Is:");
    tft.setCursor(55, 20, 1);
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
    tft.setCursor(0, 0, 1);
    tft.println(leader_board_response);
    done = 1;
  }
  else if (done ==1){
    
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
      }
      else if(playing ==1){
        
      }

    //---------------------------------------------
    
    
//    if(detect_note()){
//      int len[2];
//      int length1;
//      if (notes[global_index]/10 == 0){
//        len[0] = notes[global_index];
//        length1 = 1;
//      }
//      else{
//        int temp1 = notes[global_index]/10;
//        int temp2 = notes[global_index]%10;
//        if (temp1 == temp2){
//          len[0]= notes[global_index];
//          length1 = 1;
//        }
//        else{
//          len[0] = notes[global_index]/10;
//          len[1] = notes[global_index]%10;
//          length1 = 2;
//        }
//      }
//      for (int j = 0; j < length1;j++){
//        int temp_note = len[j];
//        
//        if (temp_note == 1){
//          if (!digitalRead(BUTTON3)){
//            ledcWrite(PWM_CHANNEL4, (4095) - (4095 * 50/100.0));
//            ledcWrite(PWM_CHANNEL5, (4095) - (4095 * 50/100.0));
//            score_index += 1;
//            points += 1;
//          }
//          else{
//            ledcWrite(PWM_CHANNEL5, 0);
//            ledcWrite(PWM_CHANNEL4, 0);
//          }
//        }
//        if (temp_note == 2){
//          if (!digitalRead(BUTTON2)){
//            ledcWrite(PWM_CHANNEL3, (4095) - (4095 * 50/100.0));
//            score_index += 1;
//            points += 1;
//          }
//          else{
//            ledcWrite(PWM_CHANNEL3, 0);
//          }
//        }
//        if (temp_note == 3){
//          if(!digitalRead(BUTTON)){
//            ledcWrite(PWM_CHANNEL, (4095) - (4095 * 50/100.0));
//            score_index += 1;
//            points += 1;
//          }
//          else{
//            ledcWrite(PWM_CHANNEL, 0);
//          }
//        }
//        if (temp_note == 4){
//          if(!digitalRead(BUTTON4)){
//            ledcWrite(PWM_CHANNEL2, (4095) - (4095 * 50/100.0));
//            score_index += 1;
//            points += 1;
//          }
//          else{
//            ledcWrite(PWM_CHANNEL2, 0);
//          }
//        }
//      }
//      
//      
//    }
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
  }
  

  
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
        ending_time = atoi(tim) + 5000;
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
//  Serial.println("TIMES");
//  for (int i = 0; i < MAX_INT_ARRAY_SIZE; i++){
//    Serial.println(times[i]); 
//  }
//
//  Serial.println("NOTES");
//  for (int i = 0; i < MAX_INT_ARRAY_SIZE; i++){
//    Serial.println(notes[i]); 
//  }
//
//  Serial.println("DURATIONS");
//  for (int i = 0; i < MAX_INT_ARRAY_SIZE; i++){
//    Serial.println(durations[i]); 
//  }
  
}


uint8_t char_append(char* buff, char c, uint16_t buff_size) {
  int len = strlen(buff);
  if (len > buff_size) return false;
  buff[len] = c;
  buff[len + 1] = '\0';
  return true;
}

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

void do_http_GET(char* host, char* request, char* response, uint16_t response_size, uint16_t response_timeout, uint8_t serial){
  WiFiClient client; //instantiate a client object
  if (client.connect(host, 80)) { //try to connect to host on port 80
    if (serial) Serial.print(request);//Can do one-line if statements in C without curly braces
    client.print(request);
    memset(response, 0, response_size); //Null out (0 is the value of the null terminator '\0') entire buffer
    uint32_t count = millis();
    while (client.connected()) { //while we remain connected read out data coming back
      client.readBytesUntil('\n',response,response_size);
      if (serial) Serial.println(response);
      if (strcmp(response,"\r")==0) { //found a blank line!
        break;
      }
      memset(response, 0, response_size);
      if (millis()-count>response_timeout) break;
    }
    memset(response, 0, response_size);  
    count = millis();
    while (client.available()) { //read out remaining text (body of response)
      char_append(response,client.read(),OUT_BUFFER_SIZE);
    }
    if (serial) Serial.println(response);
    client.stop();
    if (serial) Serial.println("-----------");  
  }else{
    if (serial) Serial.println("connection failed :/");
    if (serial) Serial.println("wait 0.5 sec...");
    client.stop();
  }
}      
