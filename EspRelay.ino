
//WiFi relay controllers
//Copyright: Owen Duffy 2022/03/20

#define VERSION "0.1"

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
//char chipid[]=Serial.println(wifiManager.getInfoData("chipid"));

//Need global visibility of the config stuff
//StaticJsonDocument<2000> doc; //on stack  arduinojson.org/assistant
DynamicJsonDocument doc(1024);//arduinojson.org/assistant
JsonObject json;
JsonArray relays;
IPAddress ipaddress(0,0,0,0);
IPAddress ipmask(0,0,0,0);
IPAddress ipgateway(0,0,0,0);

unsigned char* relstat;

char name[21]="dev name";
PageElement  elm;
PageBuilder  page;
String currentUri;
char hostname[20] = "esp8266-relay01";
//----------------------------------------------------------------------------------
int config(const char* cfgfile){
  int i,n;
  Serial.print(F("config file: "));
  Serial.println(cfgfile);
  if (LittleFS.exists(cfgfile)){
    //file exists, reading and loading
    Serial.println(F("Reading config file..."));
    delay(1000);
    File configFile=LittleFS.open(cfgfile,"r");
    if (configFile){
      Serial.println(F("Opened config file..."));
      size_t size=configFile.size();
      // Allocate a buffer to store contents of the file.
      static char* buf=new char[size];
      configFile.readBytes(buf,size);
      DeserializationError error = deserializeJson(doc, buf);
      if (error) {
          Serial.println(F("Failed to load JSON config."));
          while(1);
      }
      json = doc.as<JsonObject>();
      Serial.println(F("\nParsed json."));
      strncpy(hostname,json[F("hostname")],sizeof(hostname));
      hostname[sizeof(hostname)-1]='\0';

      JsonArray ip4;
      ip4=json["staticip"]["address"];
      ipaddress=IPAddress(ip4[0],ip4[1],ip4[2],ip4[3]);
      ip4=json["staticip"]["mask"];
      ipmask=IPAddress(ip4[0],ip4[1],ip4[2],ip4[3]);
      ip4=json["staticip"]["gateway"];
      ipgateway=IPAddress(ip4[0],ip4[1],ip4[2],ip4[3]);

      Serial.print(F("Hostname: "));
      Serial.println(hostname);

      relays=json["relays"];
      int n=relays.size();
      relstat=new unsigned char[size];
      for(i=0;i<n;i++){
        relstat[i]=relays[i][2].as<int>();
      }
      return 0;
    }
  }
  Serial.println(F("Error: config file not found."));
  return 1;
}
//----------------------------------------------------------------------------------
String rootPage(PageArgument& args) {
  int i,n;
  String buf1="";
  char line[300];
  buf1.reserve(PAGEBUFRESSIZE);
  n=relays.size();
  if (server.hasArg("update")){
    for(i=0;i<n;i++){
      sprintf(line,"n%02d",i);
      if (server.hasArg(line)){
        relstat[i]=1;
      }
      else
        relstat[i]=0;
    }
  }
  sprintf(line,"<h1>ESP Relay controller (v%s)</h1><h2>%s</h2>\n",VERSION,hostname);
  buf1+=String(line);
  buf1+="<form method=\"get\" action=\"/\">\n";

  if(n>1){
    buf1+=F("<input type='button' value='Check all' onClick=\"javascript:f=this.form;for(x=0;x<f.elements.length;x++){if (f.elements[x].type=='checkbox'){f.elements[x].checked=true;}}\">\n");
    buf1+=F("<input type='button' value='Uncheck all' onClick=\"javascript:f=this.form;for(x=0;x<f.elements.length;x++){if (f.elements[x].type=='checkbox'){f.elements[x].checked=false;}}\">\n");
  }
  buf1+=F("<hr>\n");
  for(i=0;i<n;i++){
    sprintf(line,"<input type=\"checkbox\" id=\"r%02d\" name=\"n%02d\"%s><label for=\"r%02d\">%s</label><br>\n",i,i, relstat[i]?" checked":""  ,i,relays[i][0].as<const char*>());
    buf1+=String(line);
    pinMode(relays[i][1],OUTPUT);
    digitalWrite(relays[i][1],relstat[i]);
  }
  buf1+=F("<hr>\n<input type=\"hidden\" id=\"update\" name=\"update\" value=\"\">\n<input type=\"button\" value=\"Read\" onclick=\"location.href='/';\">\n<input type=\"submit\" value=\"Write\">\n</form>\n");
  return  buf1;
}
//----------------------------------------------------------------------------------
// This function creates dynamic web page by each request.
// It is called twice at one time URI request that caused by the structure
// of ESP8266WebServer class.
bool handleAcs(HTTPMethod method, String uri) {
//  Serial.println(F("in handleACS\n"));
//  Serial.println(uri);

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
  Serial.print("\n\nStarting... \n\n");
  if (LittleFS.begin()){
    Serial.println(F("Mounted file system"));
    config("config.cfg");
  }
  else{
    Serial.println(F("Failed to mount FS"));
    while(1);
  }
  WiFi.hostname(hostname);
  // Configures static IP address
  if(ipaddress){
    Serial.println("Configure static IP");
    if(!WiFi.config(ipaddress,ipgateway,ipmask)){
      Serial.println("STA Failed to configure");
    }
  }
  wifiManager.setDebugOutput(true);
  wifiManager.setHostname(hostname);
  wifiManager.setConfigPortalTimeout(120);
  Serial.print(F("Connecting to ... "));
  Serial.println(WiFi.SSID());
  wifiManager.autoConnect(hostname);
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
//----------------------------------------------------------------------------------
void loop(){
  server.handleClient();
}
