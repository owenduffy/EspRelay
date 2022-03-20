
//WiFi relay controller
//Copyright: Owen Duffy 2022/03/20

// Import required libraries
#include <LittleFS.h>

#if defined(ARDUINO_ARCH_ESP8266)
#include <ESP8266WiFi.h>
#include <ESP8266WebServer.h>
ESP8266WebServer  server;
#elif defined(ARDUINO_ARCH_ESP32)
#include <WiFi.h>
#include <WebServer.h>
WebServer  server;
#endif
#define ARDUINOJSON_USE_DOUBLE 1
#include <ArduinoJson.h>
#include <PageBuilder.h>
#define PAGEBUFRESSIZE 3000

#include <WiFiManager.h>
WiFiManager wifiManager;

//Need global visibility of the config stuff
//StaticJsonDocument<2000> doc; //on stack  arduinojson.org/assistant
DynamicJsonDocument doc(1024);//arduinojson.org/assistant
JsonObject json;
JsonArray relays;
unsigned char* relstat;

char name[21]="dev name";
PageElement  elm;
PageBuilder  page;
String currentUri;
char hostname[20] = "esp8266-relay01";
//----------------------------------------------------------------------------------
int config(const char* cfgfile){
  int i,n;
//  StaticJsonDocument<2000> doc; //on stack  arduinojson.org/assistant
//  DynamicJsonDocument doc(1024);//arduinojson.org/assistant
  Serial.println(F("config file"));
  Serial.println(cfgfile);
  if (LittleFS.exists(cfgfile)){
    //file exists, reading and loading
    Serial.println(F("Reading config file"));
    delay(1000);
    File configFile=LittleFS.open(cfgfile,"r");
    if (configFile){
      Serial.println(F("Opened config file"));
      size_t size=configFile.size();
      // Allocate a buffer to store contents of the file.
      static char* buf=new char[size];
      configFile.readBytes(buf,size);
      DeserializationError error = deserializeJson(doc, buf);
      if (error) {
          Serial.println(F("Failed to load JSON config"));
          while(1);
      }
      json = doc.as<JsonObject>();
      Serial.println(F("\nParsed json"));
      strncpy(hostname,json[F("hostname")],sizeof(hostname));
      hostname[sizeof(hostname)-1]='\0';
      Serial.println(hostname);

      // extract the values
      relays=json["relays"];
      int n=relays.size();
      relstat=new unsigned char[size];

      Serial.print(" size: ");
      Serial.println(relays.size());

      //for (String label : labels) {
      for(i=0;i<n;i++){
        relstat[i]=relays[i][2].as<int>();
        Serial.print(i);
        Serial.print(":");
        Serial.print(relstat[i]);
        Serial.print(":");
        Serial.print(relays[i][1].as<int>());
        Serial.print(":");
        Serial.print(relays[i][2].as<int>());
        Serial.print(":");

        Serial.println(relays[i][0].as<String>());
      }
      return 0;
    }
  }
  Serial.println(F("No config file"));
  return 1;
}

//----------------------------------------------------------------------------------
String rootPage(PageArgument& args) {
  int i,n;
//  String buf((char *)0);
  String buf1="";
  char line[300];
  buf1.reserve(PAGEBUFRESSIZE);
  Serial.println(F("in rootpage\n"));
  Serial.print(" size: ");
  Serial.println(relays.size());
  n=relays.size();

  if (server.hasArg("update")){
    for(i=0;i<n;i++){
      sprintf(line,"n%02d",i);
      if (server.hasArg(line)){
        Serial.println(line);
        relstat[i]=1;
      }
      else
        relstat[i]=0;
    }
  }

  sprintf(line,"<h1>ESP Relay controller</h1><h2>%s</h2><hr>\n",hostname);
  buf1+=String(line);
  buf1+="<form method=\"get\" action=\"/\">\n";

//  for(String label : labels) {
  for(i=0;i<n;i++){
    Serial.print(i);
    Serial.print(":");
    Serial.print(relstat[i]);
    Serial.print(":");
    Serial.print(relays[i][1].as<int>());
    Serial.print(":");
    Serial.print(relays[i][2].as<int>());
    Serial.print(":");
    Serial.println(relays[i][0].as<String>());

    sprintf(line,"<input type=\"checkbox\" id=\"r%02d\" name=\"n%02d\" %s> <label for=\"r%02d\">%s</label><br>\n",i,i, relstat[i]?"checked":""  ,i,relays[i][0].as<const char*>());
    buf1+=String(line);

    Serial.println("pins:");
    pinMode(relays[i][1],OUTPUT);
    digitalWrite(relays[i][1],relstat[i]);
    Serial.println(relays[i][1].as<int>());
    Serial.println(relstat[i]);
  }
  sprintf(line,"<hr>\n<input type=\"hidden\" id=\"update\" name=\"update\" value=\"\">\n<input type=\"button\" value=\"Read\" onclick=\"location.href='/';\">\n<input type=\"submit\" value=\"Write\">\n</form>\n");
  buf1+=String(line);
  return  buf1;
}
//----------------------------------------------------------------------------------
// This function creates dynamic web page by each request.
// It is called twice at one time URI request that caused by the structure
// of ESP8266WebServer class.
bool handleAcs(HTTPMethod method, String uri) {
  Serial.println(F("in handleACS\n"));
  Serial.println(uri);

  if (uri==currentUri){
    // Page is already prepared.
    return true;
  }
  else{
    currentUri=uri;
    page.clearElements();          // Discards the remains of PageElement.
    page.addElement(elm);         // Register PageElement for current access.

    Serial.println("Request:" + uri);

    if(uri=="/"){
      page.setUri(uri.c_str());
      elm.setMold(F(
        "<html>"
        "<body>"
        "{{ROOT}}"
        "</body>"
        "</html>"));
      elm.addToken("ROOT", rootPage);
      return true;
    }
  else{
    return false;    // Not found to accessing exception URI.
    }
  }
}
//----------------------------------------------------------------------------------
void setup(){
  WiFi.mode(WIFI_OFF);
  // Serial port for debugging purposes
  Serial.begin(115200);
  if (LittleFS.begin()){
    Serial.println(F("Mounted file system"));
    config("config.cfg");
  }
  else{
    Serial.println(F("Failed to mount FS"));
    while(1);
  }

  WiFi.hostname(hostname);
  wifiManager.setDebugOutput(true);
  wifiManager.setHostname(hostname);
  wifiManager.setConfigPortalTimeout(120);
  Serial.println(F("Connecting..."));
//  Serial.print(WiFi.hostname());
  Serial.print(F(" connecting to "));
  Serial.println(WiFi.SSID());
  wifiManager.autoConnect("ESP8266-Relay01");
  Serial.println(WiFi.localIP());

  // Prepare dynamic web page
  page.exitCanHandle(handleAcs);    // Handles for all requests.
  page.insert(server);

  // Print local IP address and start web server
  Serial.println(F(""));
  Serial.println(F("WiFi connected."));
  Serial.println(F("IP address: "));
  Serial.println(WiFi.localIP());
  Serial.println(F("Hostname: "));
#if defined(ARDUINO_ARCH_ESP8266)
  Serial.println(WiFi.hostname());
#elif defined(ARDUINO_ARCH_ESP32)
  Serial.println(WiFi.getHostname());
#endif
  server.begin();
}

void loop(){
//  Serial.println(F("in loop\n"));
  server.handleClient();
}
