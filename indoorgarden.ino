#include <Bridge.h>
#include <BridgeServer.h>
#include <BridgeClient.h>

#include <dht11.h>

//#include <FileIO.h>
#include <Process.h>

#define RELAY_ON 0
#define RELAY_OFF 1

#define pompe  3 // digital 1 = prise 220v #1
#define ventilateur  2 // digital 2 = prise 220v #2

int timelapse = 30; // 30 x 2 secondes
int curl_interval=0;

unsigned long previousMillis = 0;
const long interval = 60000; 


BridgeServer server;
dht11 DHT;
Process picture;

void setup() {

  pinMode(pompe, OUTPUT);
  digitalWrite(pompe, RELAY_OFF);
  
  pinMode(ventilateur, OUTPUT);
  digitalWrite(ventilateur, RELAY_OFF);
  
  Bridge.begin();
  server.listenOnLocalhost();
  server.begin();
  Serial.begin(9600);
  //while(!Serial);
  
  //picture.runShellCommand("mjpg_streamer -i \"input_uvc.so -d /dev/video0 -r 640x480\" -o \"output_http.so -p 8080 -w /mnt/sda1\"");
  //FileSystem.begin();

}

//Lux
double Light (int RawADC0) {
  float Vout0 = RawADC0 * 0.0048828125;      // calculate the voltage
  int lux = 500 / (1 * ((5 - Vout0) / Vout0));    // calculate the Lux
  return lux;
}


void loop() {
  unsigned long currentMillis = millis();
  // Get clients coming from server
  BridgeClient client = server.accept();

  // get the time from the server:
  Process time;
  time.runShellCommand("date");
  String timeString = "";
  while (time.available()) {
    char c = time.read();
    timeString += c;
  }
  timeString.trim();

  //float temperature = ( ( analogRead(0) * (5000.0 / 1024.0) - 500 ) / 10.0 );
  float light = Light(analogRead(4));

  int chk = DHT.read(4);

  
  float waterlevel = analogRead(1); 
  float moisture1 = analogRead(4);
  float moisture2 = analogRead(3);

  

  if (client) {
    // read the command
    String command = client.readString();
    command.trim();
    if (command == "temperature") {
      //Serial.print(DHT.humidity,1);
      //Serial.println(DHT.temperature,1);
  
      String gardendatas = "{\"temperature\":" + String(DHT.temperature) + ",\"light\":" + String(light) + ",\"humidity\":" + String(DHT.humidity) + ",\"waterlevel\":" + String(waterlevel) + ",\"moisture1\":" + String(moisture1) + ",\"moisture2\":" + String(moisture2) + ",\"timestamp\":\"" + String(timeString) + "\"}";
      client.print( gardendatas );
      
    } else if(command == "cam1"){
      //
      picture.runShellCommand("killall webcamDaemon mjpg_streamer");
      while(picture.running());
      
      picture.runShellCommand("mjpg_streamer -i \"input_uvc.so -d /dev/video1 -r 640x480\" -o \"output_http.so -p 8080 -w /mnt/sda1\" &");    
      while(picture.running());
      client.print( "camera2" );
      
      // Take picture
      //picture.runShellCommand("fswebcam /mnt/sda1/DCIM/cam1.jpg -d /dev/video0 -r 640x480 -i 0 -S 80 --no-banner");
      //while(picture.running());
      //client.print( "/DCIM/cam1.jpg" );
      
    } else if(command == "cam2"){
      
      picture.runShellCommand("killall webcamDaemon mjpg_streamer");
      while(picture.running());
      picture.runShellCommand("mjpg_streamer -i \"input_uvc.so -d /dev/video0 -y -r QVGA\" -o \"output_http.so -p 8080 -w /mnt/sda1\" &");
      while(picture.running());
      client.print( "camera2" );
      
      // Take picture
      //picture.runShellCommand("fswebcam /mnt/sda1/DCIM/cam2.jpg -d /dev/video1 -i 0 -S 80 --no-banner");
      //while(picture.running());
      //client.print( "/DCIM/cam2.jpg" );
      
    } else if(command == "pompe_on"){
      digitalWrite(pompe, RELAY_ON);
      client.print( "pompe on" );
    } else if(command == "pompe_off"){
      digitalWrite(pompe, RELAY_OFF);
      client.print( "pompe off" );
    } else if(command == "ventilateur_on"){
      digitalWrite(ventilateur, RELAY_ON);
      client.print( "vent on" );
    } else if(command == "ventilateur_off"){
      digitalWrite(ventilateur, RELAY_OFF);
      client.print( "vent off" );

    }
    
    // commands
    client.stop();
  }
  //client

/*
mjpg_streamer -i "input_uvc.so -d /dev/video1 -y -r QVGA" -o "output_http.so -p 8080 -w /mnt/sda1"
 mjpg_streamer -i "input_uvc.so -d /dev/video1 -r 640x480" -o "output_http.so -p 8080 -w /mnt/sda1"

 killall webcamDaemon mjpg_streamer

reset-mcu
 
 */

  //picture.runShellCommand("curl -d temperature=\"" + String(DHT.temperature) + "\" -d moisture=\"" + String(DHT.humidity) + "\" -d light=\"" + String(light) + "\" http://lesitevideo.net/jardin2000/publish/");
  //while(picture.running());
  /*
  if (currentMillis - previousMillis >= interval) {
    previousMillis = currentMillis;
    picture.runShellCommand("curl -d temperature=\"" + String(DHT.temperature) + "\" -d moisture=\"" + String(DHT.humidity) + "\" -d light=\"" + String(light) + "\" http://lesitevideo.net/jardin2000/publish/");
    while(picture.running());
    
  }
  */
  
  
  
  
  delay(2000); 
  



}



String getTimeStamp() {
  String result;
  Process time;
  time.begin("date");
  time.addParameter("+%D-%T");  
  time.run(); 

  while(time.available()>0) {
    char c = time.read();
    if(c != '\n')
      result += c;
  }

  return result;
}

