#include <WiFi.h> //Connect to WiFi Network
#include <SPI.h>
#include <TFT_eSPI.h>
#include <mpu6050_esp32.h>
#include<math.h>
#include<string.h> using namespace std;
#undef min

char input_file[] = "ESP_song_files/less_i_know_the_better";
char response[] = "897\n1282\n1666\n1794\n2051\n2564\n3076\n3589\n4102\n4615\n5128\n5384\n5641\n6153\n6666\n7179\n7692\n8205\n8717\n9230\n9487\n9743\n10000\n10256\n10769\n11282\n11794\n12307\n12820\n13333\n13589\n13846\n14358\n14871\n15384\n15897\n16410\n16923\n17435\n17692\n17948\n18205\n18461\n18974\n19487\n20000\n20512\n21025\n21538\n21794\n22051\n22564\n23076\n23589\n24102\n24615\n25128\n25641\n25897\n26153\n26410\n26666\n29743\n30769\n33846\n34871\n37948\n38974\n42051\n43076\n46153\n47179\n50256\n51282\n54359\n55384\n58461\n59487\n60000\n60512\n61025\n61538\n62051\n62564\n62820\n63076\n63589\n64102\n64615\n65128\n#4\n3\n3\n2\n1\n1\n3\n2\n1\n4\n4\n3\n3\n2\n2\n4\n3\n2\n4\n4\n3\n2\n2\n1\n1\n3\n2\n1\n4\n4\n3\n3\n2\n2\n4\n3\n2\n4\n4\n3\n2\n2\n1\n1\n3\n2\n1\n4\n4\n3\n3\n2\n2\n4\n3\n2\n4\n4\n3\n2\n2\n13\n23\n24\n23\n13\n23\n24\n23\n13\n23\n24\n23\n13\n23\n24\n23\n1\n1\n3\n2\n1\n4\n4\n3\n3\n2\n2\n4\n3\n";


int times[2000];
int notes[2000];

int global_index = 0;
int start;

char network[] = "Garrett's iPhone";  //SSID for 6.08 Lab
char password[] = "diz9wk1dh4fi8"; //Password for 6.08 Lab

const int RESPONSE_TIMEOUT = 6000; //ms to wait for response from host
const uint16_t OUT_BUFFER_SIZE = 1000; //size of buffer to hold HTTP response


void lookup(char* query, char* response, int response_size) {
  char request_buffer[200];
  //CHANGE WHERE THIS IS TARGETED! IT SHOULD TARGET YOUR SERVER SCRIPT
  //CHANGE WHERE THIS IS TARGETED! IT SHOULD TARGET YOUR SERVER SCRIPT
  //CHANGE WHERE THIS IS TARGETED! IT SHOULD TARGET YOUR SERVER SCRIPT
  sprintf(request_buffer, "GET /sandbox/sc/gagordon/final_project/song_file_handler.py?song_file_name=%s HTTP/1.1\r\n", query);
  strcat(request_buffer, "Host: 608dev-2.net\r\n");
  strcat(request_buffer, "\r\n"); //new line from header to body

  do_http_request("608dev-2.net", request_buffer, response, response_size, RESPONSE_TIMEOUT, true);
}
void print_int_array(int* arr){
  Serial.printf("[");
  for (int i = 0; i < 1000; i++){
    Serial.printf("%d, ",arr[i]);
  }
  Serial.printf("]");
  
}
void setup() {
  // put your setup code here, to run once:
  Serial.begin(115200); //for debugging if needed.
  parse_song_file(response);
  Serial.printf("\n");
  start = millis();
}

void loop() {
  // put your main code here, to run repeatedly:
  if (millis() - start > times[global_index]){
      Serial.printf("Time (millis) %d\n", times[global_index]);
      Serial.printf("Note:");
      Serial.println(notes[global_index]);
      global_index++;
  }
  

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
          Serial.println(num);
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
  Serial.println("NOTES: ");
  Serial.println(note_char);
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
