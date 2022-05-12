
//WiFi relay controllers
//Copyright: Owen Duffy 2022/03/20

#define VERSION "0.02"

// Import required libraries
#include <string>
#include <LittleFS.h>

#if defined(ARDUINO_ARCH_ESP8266)
#include <ESP8266WiFi.h>
#include <ESP8266WebServer.h>
ESP8266WebServer  server;
#include <ESP8266mDNS.h>
#elif defined(ARDUINO_ARCH_ESP32)
#include <WiFi.h>
#include <WebServer.h>
WebServer  server;
#include <ESPmDNS.h>
#endif
#define ARDUINOJSON_USE_DOUBLE 1
#define ARDUINOJSON_USE_LONG_LONG 0
#include <ArduinoJson.h>
#include <PageBuilder.h>
#define PAGEBUFRESSIZE 3000

#include <WiFiManager.h>
WiFiManager wifiManager;

//Need global visibility of the config stuff
DynamicJsonDocument doc(4096);//arduinojson.org/assistant
JsonObject json;
int cfgver=0;
JsonArray outputs,inputs;
IPAddress ipaddress(0,0,0,0);
IPAddress ipmask(0,0,0,0);
IPAddress ipgateway(0,0,0,0);
char username[21]="",password[21]="",ssid[21]="",wifipwd[21]="";
int wificfgpin=-1;

unsigned char* outstate;
char name[21]="";
PageElement  elm;
PageBuilder  page;
String currentUri;
char hostname[21] = "EspRelay";
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

      if(json[F("cfgver")]){
        cfgver=json[F("cfgver")];
      }
      if(cfgver!=1){
        Serial.print(F("Incompatible config.json version: "));
        Serial.println(cfgver);
        return 1;
      }
      if(json[F("login")][F("user")]){
        strncpy(username,json[F("login")][F("user")],sizeof(username));
        username[sizeof(username)-1]='\0';
      }
      if(json[F("login")][F("pwd")]){
        strncpy(password,json[F("login")][F("pwd")],sizeof(password));
        password[sizeof(password)-1]='\0';
      }
      if(json[F("hostname")]){
        strncpy(hostname,json[F("hostname")],sizeof(hostname));
        hostname[sizeof(hostname)-1]='\0';
      }
      if(json[F("wifi")][F("ssid")]){
        strncpy(ssid,json[F("wifi")][F("ssid")],sizeof(ssid));
        ssid[sizeof(ssid)-1]='\0';
      }
      if(json[F("wifi")][F("pwd")]){
        strncpy(wifipwd,json[F("wifi")][F("pwd")],sizeof(wifipwd));
        wifipwd[sizeof(wifipwd)-1]='\0';
      }
      if(json[F("wificfgpin")]){
        wificfgpin=json[F("wificfgpin")];
      }

      JsonArray ip4;
      ip4=json["staticip"]["address"];
      ipaddress=IPAddress(ip4[0],ip4[1],ip4[2],ip4[3]);
      ip4=json["staticip"]["mask"];
      ipmask=IPAddress(ip4[0],ip4[1],ip4[2],ip4[3]);
      ip4=json["staticip"]["gateway"];
      ipgateway=IPAddress(ip4[0],ip4[1],ip4[2],ip4[3]);

      Serial.print(F("Hostname: "));
      Serial.println(hostname);

      outputs=json["outputs"];
      int n=outputs.size();
      outstate=new unsigned char[size];
      for(i=0;i<n;i++){
        outstate[i]=outputs[i][3];
        digitalWrite(outputs[i][1],outstate[i]!=outputs[i][2].as<int>());
        pinMode(outstate[i],OUTPUT);
      }
      inputs=json["inputs"];
      for(i=0;i<inputs.size();i++) if(inputs[i][3]) pinMode(inputs[i][1],INPUT_PULLUP);
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
  n=outputs.size();
  if (server.hasArg("update")){
    for(i=0;i<n;i++){
      sprintf(line,"n%02d",i);
      if(server.hasArg(line))
        outstate[i]=server.arg(line)=="on";
      else
        outstate[i]=0;
      digitalWrite(outputs[i][1],outstate[i]!=outputs[i][2].as<int>());
      pinMode(outputs[i][1],OUTPUT);
    }
  }

  sprintf(line,"<h1>ESP Relay controller (v%s)</h1><h2>%s (LocalIP: %s)</h2>\n",VERSION,hostname,WiFi.localIP().toString().c_str());
  buf1+=String(line);
  buf1+="<!-- status={\"outputs\":[";
  for(i=0;i<outputs.size();i++){
    buf1+=outstate[i]?"1":"0";
    if(i<outputs.size()-1)buf1+=",";
}
  buf1+="],";
  buf1+="\"inputs\":[";
  for(i=0;i<inputs.size();i++){
    buf1+=digitalRead(inputs[i][1])!=inputs[i][2].as<int>()?"1":"0";
    if(i<inputs.size()-1)buf1+=",";
  }
  buf1+="]} -->\n";

  buf1+="<form method=\"get\" action=\"/\">\n";

  n=outputs.size();
  if(n>0){
    buf1+=F("<hr>\n");
    if(n>1){
      buf1+=F("<input type='button' value='Check all' onClick=\"javascript:f=this.form;for(x=0;x<f.elements.length;x++){if (f.elements[x].type=='checkbox' && !f.elements[x].disabled){f.elements[x].checked=true;}}\">\n");
      buf1+=F("<input type='button' value='Uncheck all' onClick=\"javascript:f=this.form;for(x=0;x<f.elements.length;x++){if (f.elements[x].type=='checkbox' && !f.elements[x].disabled){f.elements[x].checked=false;}}\">\n");
      buf1+=F("<br>\n");
    }
    for(i=0;i<n;i++){
      sprintf(line,"<input type=\"checkbox\" id=\"r%02d\" name=\"n%02d\"%s><label for=\"r%02d\">%s</label><br>\n",i,i,outstate[i]?" checked":""  ,i,outputs[i][0].as<const char*>());
      buf1+=String(line);
   }
  }

  n=inputs.size();
  if(n>0){
    buf1+=F("<hr>\n");
    for(i=0;i<n;i++){
      sprintf(line,"<input type=\"checkbox\" id=\"i%02d\" name=\"ni%02d\"%s disabled><label for=\"i%02d\">%s</label><br>\n",i,i,digitalRead(inputs[i][1])!=inputs[i][2].as<int>()?" checked":""  ,i,inputs[i][0].as<const char*>());
      buf1+=String(line);
    }
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
  Serial.print("\n\nStarting EspRelay v");
  Serial.println(VERSION);
  if (LittleFS.begin()){
    Serial.println(F("Mounted file system"));
    config("/config.json");
  }
  else{
    Serial.println(F("Failed to mount FS"));
    while(1);
  }
  WiFi.hostname(hostname);
  if(ipaddress){
    Serial.println("Configure static IP");
    if(!WiFi.config(ipaddress,ipgateway,ipmask)){
      Serial.println("STA Failed to configure");
    }
  }
  if(ssid[0]!='\0'){
    Serial.println(F("Configure credentials from config file"));
    Serial.println(ssid);
    Serial.println(wifipwd);
    WiFi.begin(ssid, wifipwd);
    while (WiFi.status() != WL_CONNECTED)
    {
      delay(1000);
      Serial.print(".");
    }
  Serial.println();
  Serial.print("Connected, IP address: ");
  Serial.println(WiFi.localIP());
  }
  else{
    wifiManager.setDebugOutput(true);
    wifiManager.setHostname(hostname);
    wifiManager.setConfigPortalTimeout(120);
    if(wificfgpin>=0){
      pinMode(wificfgpin,INPUT_PULLUP);
      if(digitalRead(wificfgpin)==LOW){
        Serial.println(F("Start on demand config portal."));
        wifiManager.startConfigPortal(hostname);
      }
      else{
        wifiManager.autoConnect(hostname);
        Serial.println(F("Autoconnect, start config portal.")       );
      }
    }
    if(WiFi.status()!=WL_CONNECTED)
    {
      Serial.println("WiFi autoconnect failed, resetting...");
      ESP.restart(); //soft reboot
      delay(1000);
    }
  }
  Serial.print(F("Connecting to "));
  Serial.println(WiFi.SSID());
  Serial.println(WiFi.localIP());

  // Prepare dynamic web page
  if(username[0]!='\0') page.authentication(username,password,DIGEST_AUTH,"EspRelay");
  page.exitCanHandle(handleAcs);    // Handles for all requests.
  page.insert(server);

  // Print local IP address and start web server
  Serial.println(F(""));
  Serial.println(F("WiFi connected."));
  Serial.println(F("IP address: "));
  Serial.println(WiFi.localIP());
  Serial.print(F("Hostname: "));
#if defined(ARDUINO_ARCH_ESP8266)
  Serial.println(WiFi.hostname());
#elif defined(ARDUINO_ARCH_ESP32)
  Serial.println(WiFi.getHostname());
#endif
  if (!MDNS.begin(hostname)) {             // Start the mDNS responder for esp8266.local
    Serial.println("Error setting up MDNS responder!");
  }
  Serial.println("mDNS responder started");
  server.begin();
  MDNS.addService("_http", "_tcp", 80);
}
//----------------------------------------------------------------------------------
void loop(){
#if defined(ARDUINO_ARCH_ESP8266)
  MDNS.update();
#endif
  server.handleClient();
}
