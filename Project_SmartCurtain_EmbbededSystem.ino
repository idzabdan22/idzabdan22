//Library Define
#include <ESP8266WiFi.h>
#include "Adafruit_MQTT.h"
#include "Adafruit_MQTT_Client.h"
#include <WiFiClientSecure.h>
#include <NTPClient.h>
#include <WiFiUdp.h>
#include <UniversalTelegramBot.h>
#include <Servo.h>
//end
//System Define
#define WIFI_SSID "Adam@AP"
#define WIFI_PASSWORD "kindaprivate2345"
#define BOT_TOKEN "1892472769:AAFLwUGSKHhKPfY5OSipvEiETyvdmztKOMc"
#define AIO_SERVER "io.adafruit.com"
#define AIO_SERVERPORT 1883
#define AIO_USERNAME "smartcurtain2345"
#define AIO_KEY "aio_pCsA92U6c5QslhZgRcviKiZ9NPCS"
//end
//User Define
#define MAX_DATA 20
#define MAX_USER 5
//end
//System Declaration
WiFiClient client;
WiFiClientSecure secured_client;
WiFiUDP ntpUDP;
NTPClient timeClient(ntpUDP, "pool.ntp.org");
X509List cert(TELEGRAM_CERTIFICATE_ROOT);
Adafruit_MQTT_Client mqtt(&client, AIO_SERVER, AIO_SERVERPORT, AIO_USERNAME, AIO_KEY);
Adafruit_MQTT_Subscribe smartCurtain = Adafruit_MQTT_Subscribe(&mqtt, AIO_USERNAME "/feeds/SmartCurtain");
const unsigned long BOT_MTBS = 1000; // mean time between scan messages
unsigned long bot_lasttime; // last time messages' scan has been done
UniversalTelegramBot bot(BOT_TOKEN, secured_client);
Servo myServo;
//end

//User Declaration
const byte LDR_PIN = A0, servoPin = D2;
String User[MAX_USER], Data[MAX_DATA], recentTime, lastMessage[2], lastId, timeOpen, timeClose;
static byte numUser = 0, value = 0, stat, degree = 0, mesCount = 0;
byte countGlo;
byte userInputState;
bool state, command, curtStat;
uint16_t lastSignal, lastSignalAuto, lastSignalAutoLdr;
bool firstOpen = 0, restAuto = 0, newCom;
String dayGlobal, timeGlobal, statGlobal, keyboard, data, getData;
String weekDays[7]={"Sunday", "Monday", "Tuesday", "Wednesday", "Thursday", "Friday", "Saturday"};
String months[12]={"January", "February", "March", "April", "May", "June", "July", "August", "September", "October", "November", "December"};

void servOpen(){
  myServo.write(0);
  delay(100);
}

void servClose(){
  myServo.write(100);
  delay(100);
}

bool userTelebot(String usr){
  byte i = 0; 
  bool state = true;
  if (numUser == MAX_USER){
    while(i<MAX_USER){
      if(User[i] == usr){
        state = true;
        break;
      }
      else
        state = false;
      i++;
    }
  }
  while(usr != User[i] && i<MAX_USER){
    if(User[i] == NULL){
      User[i] = usr;
      numUser++;
      state = true;
      break;
    }
    i++;
  }  
  return state;
}

void brodcastUser(String usr_id, bool statCurt){
  byte i = 0;
  if(statCurt == 1){
     while(i<numUser){
        if(User[i] == usr_id);
        else
          bot.sendMessage(User[i], "Curtain Open!");
        i++;
      }
    }
    else{
      while(i<numUser){
         if(User[i] == usr_id);
         else
          bot.sendMessage(User[i], "Curtain Close!");
         i++;
         }
      }
   }

void componentSetup(){
  pinMode(LDR_PIN, INPUT);
  myServo.attach(servoPin);
}

String welcomeTxt(String from_name){
   String welcome = "Hai, " + from_name + " My name is Alexa, nice to meet you <3 !.\n";
   welcome += "Here it is, the command/mode is down below for " + from_name + " guide to Alexa!.\n";
   welcome += "Choose Mode:\n";
   welcome += "1./mode1 = User Input Control(for Open/Close The Curtain via user input.)\n";
   welcome += "2./mode2 = Autonomous (for Open/Close The Curtain based on LDR status, Full Auto.)\n";
   welcome += "3./mode3 = Semi Autonomous (for Open/Close The Curtain based on LDR status, via user input.)\n";
   welcome += "4./mode4 = Timer Set Control(for Open/Close The Curtain based on Real Time, via user input.)\n";
   
   welcome += "Universal Command:\n";
   welcome += "5./dataLog (to view all the Curtain status data history.)\n";
   welcome += "LETS GET STARTED WITH ALEXA!";  
   return welcome;
}

String sorryTxt(String from_name){
  String sorry = "Sorry " + from_name + " your command doesn't exist:(\n";
  sorry += "Try again with the list of the command/mode below:\n";
  sorry += "Choose Mode:\n";
  sorry += "1./mode1 = User Input Control(for Open/Close The Curtain via user input.)\n";
  sorry += "2./mode2 = Autonomous (for Open/Close The Curtain based on LDR status, Full Auto.)\n";
  sorry += "3./mode3 = Semi Autonomous (for Open/Close The Curtain based on LDR status, via user input.)\n";
  sorry += "4./mode4 = Timer Set Control(for Open/Close The Curtain based on Real Time, via user input.)\n";
   
  sorry += "Universal Command:\n";
  sorry += "5./dataLog (to view all the Curtain status data history.)";
  sorry += "LETS GET STARTED WITH ALEXA!";  
  return sorry;
}

void ldrAutonomus(String lastid, byte inputstate){
  if(inputstate == 2 && lastid != NULL){
    short int ldrSignal = analogRead(A0);
    if ((lastSignalAuto <= 39  && ldrSignal >= 40) && firstOpen != 1){
       servOpen();
       Serial.println("First open");
       firstOpen = 1;
       restAuto = 0;
       degree = 10;
    }
    if (restAuto == 1){
      if(lastSignalAutoLdr >= ldrSignal && lastSignalAutoLdr >= 20){
        lastSignalAutoLdr -= 50;
        myServo.write(0 + degree); 
        delay(100);
        degree+=10;
      }
      else;
      Serial.print("Last Comparation LDR:");
      Serial.println(lastSignalAutoLdr);
      firstOpen = 0;
    }
    if (ldrSignal > 499 && restAuto != 1){
         restAuto = 1;
         lastSignalAutoLdr = ldrSignal;
         Serial.println("LDR autonomous Started!");
    }
    lastSignalAuto = ldrSignal;
    Serial.print("Current Light:");
    Serial.println(ldrSignal);  
  }
    Serial.println(userInputState);
    Serial.println(lastid); 
}
    
void ldrOutput(String lastid){
  byte i = 0;
  short int ldrSignal = analogRead(A0);
  if(ldrSignal >= lastSignal - 35 && ldrSignal <= lastSignal + 35);
  else{
    if(ldrSignal < 39){
        keyboard = "[[\"/yes\", \"/no\"]]";
        bot.sendMessageWithReplyKeyboard(lastid, "Would you like to close the Curtains?", "", keyboard, true);     
      }
    else{
        keyboard = "[[\"/yes\", \"/no\"]]";
        bot.sendMessageWithReplyKeyboard(lastid, "Would you like to open the Curtains?", "", keyboard, true);     
     }
   }
  lastSignal = ldrSignal;
}

void openCurtain(String id){
  servOpen();
  bot.sendMessage(id, "Curtain Open!");
}

void closeCurtain(String id){
  servClose();
  bot.sendMessage(id, "Curtain Close!");
}

void setupCurrentTime(){
  timeClient.begin();
  timeClient.setTimeOffset(25200);
}

void dataLog(String currentTime, bool stateCurt){
  stat = 1;
  (stateCurt == 1) ? (data = (currentTime + " Curtain Open")) : (data = (currentTime + " Curtain Close"));  
  if(value == MAX_DATA ){
    for (int i = 0;i < value; i++){
      (i==value-1) ? (Data[i]=data) : (Data[i] = Data[i+1]);
    }
  }
  else{
    Data[value]=data;
    value++;
  }
}

void userTimer(String timex, String dayx, String statx, String lastId){
  String realTime = timeClient.getFormattedTime().substring(0,5);
  String realDay = weekDays[timeClient.getDay()];
  
  if(dayx == "/everyday"){
    if(statx == "/open" && realTime == timex){
      openCurtain(lastId);
      brodcastUser(lastId, 1);
    }
    else if(statx == "/close" && realTime == timex){
      closeCurtain(lastId);
      brodcastUser(lastId, 0);
    }  
  }
  else{
    if(statx == "/open" && realTime == timex){
       openCurtain(lastId);
       brodcastUser(lastId, 1);
    }
    else if(statx == "/close" && realTime == timex){
       closeCurtain(lastId);
       brodcastUser(lastId, 0);
    }
  }
}

void viDataLog(String usr){
 if(stat != 0){
  getData = "";
  for (int i=0;i < value;i++) 
    getData += (Data[i] + "\n");
  bot.sendMessage(usr, getData);
 }
 else
  bot.sendMessage(usr, "Data Log still Empty :(");
}

String showCurrentTime(){
  timeClient.update();
  unsigned long epochTime = timeClient.getEpochTime();
  String formattedTime = timeClient.getFormattedTime();
  String weekDay = weekDays[timeClient.getDay()];
  //Get a time structure
  struct tm *ptm = gmtime ((time_t *)&epochTime); 
  int monthDay = ptm->tm_mday;
  int currentMonth = ptm->tm_mon+1;
  String currentMonthName = months[currentMonth-1];
  int currentYear = ptm->tm_year+1900;
  String date = (weekDay + ", " + monthDay + " " + currentMonthName + " " + formattedTime + " " + currentYear);
  return date;
}

void MQTT_connect() {
  int8_t ret;
  if (mqtt.connected()) {
    return;
  }
  Serial.print(F("Connecting to MQTT... "));
  uint8_t retries = 3;
  while ((ret = mqtt.connect()) != 0) { // connect will return 0 for connected
       Serial.println(mqtt.connectErrorString(ret));
       Serial.println(F("Retrying MQTT connection in 5 seconds..."));
       mqtt.disconnect();
       delay(5000);  // wait 5 seconds
       retries--;
       if (retries == 0) {// basically die and wait for WDT to reset me
         while (1);
       }
  }
  Serial.println(F("MQTT Connected!"));
}

void handleNewMessages(int numNewMessages){
  Serial.println(F("handleNewMessages"));
  for (int i = 0; i < numNewMessages; i++){
    String chat_id = bot.messages[i].chat_id;
    String text = bot.messages[i].text;
    String from_name = bot.messages[i].from_name;

    recentTime = showCurrentTime();
    bool userState = userTelebot(chat_id);
    command = 0;
   
    if (userState == false){
      bot.sendMessage(chat_id, "Sorry you are not permitted!");
      command = 1;
    }

    if((text.charAt(0) == '0' || text.charAt(0) == '1' || text.charAt(0) == '2') && (lastMessage[1] == "/everyday" || lastMessage[1] == "/today") && (lastMessage[0] == "/open" || lastMessage[0] == "/close")  && userState == true){
      dayGlobal = lastMessage[1];
      timeGlobal = text;
      statGlobal = lastMessage[0];
      userTimer(timeGlobal, dayGlobal, statGlobal, chat_id);
      String keyboard = "[[\"/setupTimer\", \"/exitMode\"]]";
      bot.sendMessageWithReplyKeyboard(chat_id, "Timer Set, and ready to GO!", "", keyboard, true);
      newCom = true;     
      command = 1;
    }

    if((text == "/openCurtain" || text == "/yes")  && userState == true){
      bool stat = 1;
      openCurtain(chat_id);
      brodcastUser(chat_id, stat);
      dataLog(recentTime, stat);
      command = 1;
    }

    if((text == "/open" || text == "/close") && userState == true){
      String keyboard = "[[\"/everyday\", \"/today\"]]";
      bot.sendMessageWithReplyKeyboard(chat_id, "For When?", "", keyboard, true);    
      command = 1;
    }

    if((text == "/everyday" || text == "/today") && userState == true){
      bot.sendMessageWithReplyKeyboard(chat_id, "What Time? Ex: 12:30 (24 hour format)", "", "", true);       
      command = 1;
    }

    if((text == "/closeCurtain" || text == "/no") && userState == true){
      bool stat = 0;
      closeCurtain(chat_id);
      brodcastUser(chat_id, stat);
      dataLog(recentTime, stat);
      command = 1;
    }

    if(text == "/setupTimer" && userState == true){
      String keyboard = "[[\"/open\", \"/close\"]]";
      bot.sendMessageWithReplyKeyboard(chat_id, "Would you like set your open/close?", "", keyboard, true);    
      command = 1;
    }

    if (text == "/dataLog" && userState == true){
      viDataLog(chat_id);
      command = 1;
    }

    if (text == "/start" || text == "/exitMode"&& userState == true){
      String welTxt = welcomeTxt(from_name);
      bot.sendMessage(chat_id, welTxt);
      newCom = false;
      command = 1;
    }

    if(text == "/mode4" && userState == true){
      String keyboard = "[[\"/setupTimer\", \"/exitMode\"]]";
      bot.sendMessageWithReplyKeyboard(chat_id,"Timer Mode Enabled", "", keyboard, true);    
      command = 1; 
      newCom = true;
      userInputState = 4;
    }

     if(text == "/mode3" && userState == true){
      String keyboard = "[[\"/exitMode\"]]";
      bot.sendMessageWithReplyKeyboard(chat_id, "Semi Autonomous Enabled", "", keyboard, true);     
      command = 1;
      newCom = true;
      userInputState = 3;
    }

     if(text == "/mode2" && userState == true){
      String keyboard = "[[\"/exitMode\"]]";
      bot.sendMessageWithReplyKeyboard(chat_id, "Autonomous Enabled", "", keyboard, true);     
      userInputState = 2;   
      newCom = true;
      command = 1;
    }

     if(text == "/mode1" && userState == true){
      String keyboard = "[[\"/openCurtain\", \"/closeCurtain\"],[\"/exitMode\"]]";
      bot.sendMessageWithReplyKeyboard(chat_id, "User Input Control Enabled", "", keyboard, true);     
      command = 1;
      newCom = true;
      userInputState = 1;
    }
    
    if (userState == true){
      String keyboard = "[[\"/mode1\", \"/mode2\"],[\"/mode3\", \"/mode4\"],[\"/dataLog\"]]";
      if (text ==  "/start" || text == "/exitMode")
        bot.sendMessageWithReplyKeyboard(chat_id, "Choose Command/Mode:", "", keyboard, true);     
    }

    if (command != 1 && userState == true) {
      String sorText = sorryTxt(from_name);
      bot.sendMessage(chat_id, sorText);
    }
    
     if(mesCount == 2){
      lastMessage[0] = lastMessage[1];
      lastMessage[1] = text;
     }
     else{
      lastMessage[mesCount] = text;
      mesCount++;
     }
     lastId = chat_id;
  }
}  


void setup() {
  Serial.begin(115200);
  Serial.println();
  Serial.print(F("Connecting to Wifi SSID "));
  Serial.print(WIFI_SSID);
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD); 
  secured_client.setTrustAnchors(&cert); // Add root certificate for api.telegram.org
  Serial.print(F("\nConnecting..."));
  while (WiFi.status() != WL_CONNECTED){
    delay(1000);
  }
  Serial.print(F("\nWiFi connected. IP address: "));
  Serial.println(WiFi.localIP());
  Serial.print(F("Retrieving time: "));
  configTime(0, 0, "pool.ntp.org"); // get UTC time via NTP
  time_t now = time(nullptr);
  while (now < 24 * 3600){
    Serial.print(".");
    delay(1000);
    now = time(nullptr);
  }
  Serial.println(F("Finish Retrieve Time"));
  //end of wifi setup
  //Arduino setup
  componentSetup();
  setupCurrentTime();
  userInputState = 0;
  newCom = false;
  // Setup MQTT subscription for onoff feed.
  mqtt.subscribe(&smartCurtain);
}

void loop() {
  if ((millis() - bot_lasttime) > BOT_MTBS){
    int numNewMessages = bot.getUpdates(bot.last_message_received + 1);
    while (numNewMessages){
      Serial.println(F("got response"));
      handleNewMessages(numNewMessages);
      numNewMessages = bot.getUpdates(bot.last_message_received + 1);
    }
    bot_lasttime = millis();
  }
  MQTT_connect();
  Adafruit_MQTT_Subscribe *subscription;
  while ((subscription = mqtt.readSubscription(5000))) {
    if (subscription == &smartCurtain){
      recentTime=showCurrentTime();
      Serial.print(F("Got: "));
      Serial.println((char *)smartCurtain.lastread);
      int curtainState = atoi((char *)smartCurtain.lastread);
      if(curtainState == 1){
        openCurtain("1139051246");
      }
      else if (curtainState == 0){
        closeCurtain("1139051246");
      }
     }
 }
 if(newCom == true){
   if(userInputState == 2){
    ldrAutonomus(lastId, userInputState);
    delay(250);
   }
   if(userInputState == 3){
    ldrOutput(lastId);
    delay(250);
   }
   if(userInputState == 4){
    userTimer(timeGlobal, dayGlobal, statGlobal, lastId);
    delay(250);
   }
 }
 else;
 if (!mqtt.ping()){ 
    mqtt.disconnect();
  }
}
