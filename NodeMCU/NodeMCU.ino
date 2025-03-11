//   ArduTalk for ESP12E, by Jyneda (Dr. Yun-Wei Lin)
///// Original Version by Jyneda (Dr. Yun-Wei Lin) :  Jyneda@Gmail.com
/// See  https://github.com/IoTtalk/ArduTalk-for-NodeMCU
///
////////// This is a modified version, t5; by tsaiwn@cs.nctu.edu.tw
/// === This CODE file can be found here:  https://goo.gl/6jtP41
/// === File Path is  ArduTalk/ESP12E_modified_tsaiwn/ArduTalk_ESP12e_1_t5.zip 
///// Suggest buy NodeMCU 1.0 (ESP12E) with CP210x chip (smaller SIZE), V2 or V3 all OK

#include <Arduino.h>
const String version = "ESP12e_1_t5";

#define DefaultIoTtalkServerIP  "140.113.199.200"
#define DM_name  "NodeMCU" 
#define DF_list  {"D0~","D1~","D2~","D5","D6","D7","D8","A0"}
#define nODF     10  // The max number of ODFs which the DA can pull.
#define LENTH_INFO 64    // 100;  64;  length of SSID, PASS, IP
int obLED = 2;       // NodeMCU by LoLin, GPIO 02 is on board LED 
#define clr_ROM_pin 0
  ///  use 5  for clr_ROM_pin  if using ESP12F

//#define DEBUGESP 1  //  enable ESP Debuggin
//#define DEBUG 1     // Serial print some DEBUG info.
//#define DEBUG22 1   // dummy register/push/pull
//#define DEBUG33 1   // dummy STA connected
//#define DEBUG55 1  // show LOOP count in loop( )
//#define DEBUG88 1   // Report Free Heap Memory

////////// This is a modified version. current version number is  t5
///   Version t5, modified by tsaiwn@cs.nctu.edu.tw
///(0)When Power On and/or Reset
///   LED on for 2 seconds and then quickly falsh around one second
///(1)Read the network Info (SSID, PASS, IoTtalkServerIP) from EEPROM
///   If there is NO data, goto (3) to enter AP server mode
///   If it Got netword info, the on board LED will Quickly Flash twice.
///(2)Try to connect to the AP specifined in the network Info.
///   Enter STA (station) mode, will try to connect the AP for 25 seconds
///   During this time, It will flash the on board LED quickly 6 times every 3 seconds
///   After timeout (25 seconds), enter AP server mode with IP 192.168.0.1
/// ** try 25 秒如無法連上, 會切換到 AP server mode 等待連線 (如果EEPROM沒網路資料就不會 try 25秒)
///(3)Enter AP server mode, Server IP address is 192.168.0.1
///   LED will flash twice slowly every 5 seconds.
///  ** 此時 on board LED 燈會每隔約五秒連續慢閃燈2下, 表示在 AP mode 等待連入做設定
///   User can use a smart Phone to connect to the ESP WiFi AP which SSID begins with "MCU"
///   And then open a Browser to connect to 192.168.0.1 to do wifi setting/Configuration
///  *** AP 的 SSID 為 MCU- 開頭, 無需密碼; 用你手機開啟 WiFi 選該 MCU- 開頭的即可連上網路
///(4)ESP MCU will DO SCAN all available WiFi SSID and add them into the web Page
///   You can choose One of them; You may have to enter the Password for that AP.
///   Or you can even Enter a SSID name of an AP you prefered (This will be used if any)
///  ** 也可以自己手動指定 AP 與密碼 (這有填寫的話就會優先用這!!), 
///   Also enter its Password if required
///   Then click the Submit Button to save the network Information
///(5)ESP MCU will respond with a sucessful Page and then enter WiFi Station mode
///   It will Goto (2) to try to connect to the specified AP (for 25 seconds)
///(*)More about (4)
///  ****** 例如可填你手機分享的 AP, 或你實驗室自己的 AP, 甚或學校的 AP (這通常不能用! 理由如下);
///  ***      注意很多學校的 AP 連入之後還要用網頁做帳號密碼驗證, 這樣不能給 MCU 用!!
///  ***      所以, 不要填連上後還要網頁認證一次的 AP, 否則無法使用 ! (誰能從 MCU 內去開網頁認證?!)
///  ***      還有, 當然也可從網頁內修改要連的 IoTtalk Server IP (也可填 FQDN 網址)
///  ***** *** 不論是填入指定或選擇 AP, 好了之後點按 Submit 按鈕即可存起來並透過 AP 連出去 Internet
///(6)About RESET and How to clear the network Info in EEPROM ?
///   Press the RESET (RST) button will restart the MCU, 相當於拔電源重新插入 (廢話 :-)
///   Flash 按鈕 (USB 接頭旁邊有兩個小按鈕, 左邊 Reset, 右邊 Flash, 有寫字)
///// Long Press the Flash button (more than 5 seconds) will cause the network Info in EEPROM been erased !
///// ESP will restart( ) after clearing the EEPROM network Info.
/// ** 按住不放(LED燈會亮)超過五秒, 會把 EEPROM 內網路連線資料刪除, 重新開機,
///    (* Note that the data in EEPROM are  AP SSID, PassWord, IoTtalkServer IP ! *)
/// ---(** 如果後悔可在燈亮著還沒五秒之前放掉Flash 按鈕 ! **)---
///    這時自動重開後, 因為沒之前網路連線資料, 會立即進入 AP server 模式等你用手機連入做網路設定!
////// 自動重開 LED 應該會先亮 3 秒左右, 如果沒有亮則可能 Reboot 失敗, 請手動按 Reset 按鈕!!
////// (那是 ESP8266 ESP-12E 以及各版本的 Bug)
////// 關於 NodeMCU ESP8266 各版本請看 https://en.wikipedia.org/wiki/ESP8266 
////// NodeMCU 另外有 ESP32 (比 ESP8266 多了藍芽), 請看  https://en.wikipedia.org/wiki/ESP32
//////============== ========================================================================
///// Original Version by Jyneda (Dr. Yun-Wei Lin) :  Jyneda@Gmail.com
/// 用 Google 搜尋  iottalk + ardutalk + nodemcu 可找到以下網址
// See  https://github.com/IoTtalk/ArduTalk-for-NodeMCU
// Doc in  https://github.com/IoTtalk/ArduTalk-for-NodeMCU/tree/master/Documents
// === Installing Drivers with Boards Manager ===
//(0)Start Arduino and open Preferences window(偏好設定).
//     https://github.com/esp8266/Arduino
//(1)Enter the following URL into "Additional Board Manager URLs" field.
//     http://arduino.esp8266.com/stable/package_esp8266com_index.json 
//(2)Open Boards Manager from Tools
//   Tools 工具  >  Board  開發板 >  Board Manager  開發板管理員
//(3)在 開發板管理員 找到 或搜尋 ESP8266, 安裝最新版 (目前 2.5.0)
///// Documentation for the ESP8266: 
///   https://arduino-esp8266.readthedocs.io/en/2.5.0/
///===== Or use git clone to install the Library
// Clone git hub repository into hardware/esp8266com/esp8266 directory
/////  cd hardware;  mkdir esp8266com ; cd esp8266com
/////  git clone https://github.com/esp8266/Arduino.git esp8266
///// cd esp8266
///// git submodule update --init
//////////////////////////////////////////////////////////////////
//(4) USB to TTL Driver : CH34x or CP210x depends on your Development Board
/// You will also need CH34x Driver  OR  CP210x  Driver !
/// See https://github.com/IoTtalk/ArduTalk-for-NodeMCU/tree/master/Documents 
/// CH34x and Cp210x Drivers can be found there
///  CLick ArduTalk安裝教學(NodeMCU).pdf
///// https://sparks.gogo.co.nz/ch340.html
///// http://www.wch.cn/download/CH341SER_EXE.html
///Cp2102 Usb-to-Serial Driver Installation 
/////  https://exploreembedded.com/wiki/Cp2102_Usb-to-Serial_Driver_Installation
////////
//(5)Choose correct Board and COM port .. Before you starting to Burn the program ...
/// Board:  Tools 工具 > 開發板 Board  >  選  NodeMCU 1.0(ESP-12E Module)
/// COM:   Tools  工具 > Serial Port 序列埠  >  Choose CORRECT port 
//////////////////////////////////////////////////////////////////////////

#include <ESP8266WiFi.h>
//#include <WiFiClient.h>
#include <ESP8266WebServer.h>
#include <ESP8266WiFiMulti.h>
#include "ESP8266HTTPClient2.h"
#include <EEPROM.h>

char IoTtalkServerIP[LENTH_INFO] = "";    // LENTH_INFO  is  64
String result;
String url = "";
String passwordkey ="";

char DefaultMyAP_SSID_[LENTH_INFO]= "tsaiwnAP";   // connect to Internet through this AP
char DefaultMyAP_PASS_[LENTH_INFO]= "12345678";
int wifiTimeout = 25000;   //  25 seconds ;  //   10000; 

HTTPClient http;

#ifdef DEBUG88
/******  NOT WORK  ! ?
int freeRam () {
  extern int __heap_start; 
  extern void *__brkval;
  int v; 
  return (int) &v - (__brkval == 0 ? (int) &__heap_start : (int) __brkval); 
}
******/
void reportMemory( ) {
  Serial.print("Free Heap Memory: ");  Serial.println(ESP.getFreeHeap()); 
  // Serial.println(freeRam()); 
  Serial.flush( );
}
#endif

String remove_ws(const String& str )   // remove white space
{
    String str_no_ws ;
    for( char c : str ) if( !std::isspace(c) ) str_no_ws += c ;
    return str_no_ws ;
}

void clr_eeprom(int sw=0){    
    /// Usage:  call  clr_eeprom(1);  if you want to do CLEAR via software on purpose
    if (sw == 0){   // Not enforce via software(不是軟體強制), 若要強制CLEAR可傳不是 0 進來
        Serial.println("Count down 5 seconds to clear EEPROM.");Serial.flush( );
        delay(168); digitalWrite(obLED, LOW);  // on board LED ON
        for(int i=0; i < 3; ++i) {   // divided into serval period to check button Released
          delay(888);
          if( digitalRead(clr_ROM_pin) == HIGH)  {  // pin released (放掉了?)
             digitalWrite(obLED, HIGH);  delay(123); // turn OFF LED
             return;   // abort if button released (已經放掉按鈕就不做 Clear EEPROM)
          } // if( button released GPIO 0 is HIGH ( Flash button Released)
         } // for( int i
         delay(1968);   // Total around 5 seconds = 3*(888+123) ms + 1968 ms
    }
    /// check clr_ROM_pin (GPIO 0) after 5 sec. to confirm (做確認) again
    if( (digitalRead(clr_ROM_pin) == LOW) || (sw == 1) ) {  //  sw==1 means on purpose in Software
        Serial.println("DO clear EEPROM NOW . . . will Reboot !"); Serial.flush( );
        digitalWrite(obLED, HIGH);   // GPIO 2 is on board LED
        delay(168);
        for(int addr=0; addr<50; addr++) EEPROM.write(addr,0);   // clear eeprom
        EEPROM.commit(); delay(168);
        Serial.println("Clear EEPROM and reboot."); Serial.flush( );
        delay(58);
        digitalWrite(0, HIGH); 
        digitalWrite(15, LOW);    // on Boot/Reset we must keep GPIO 15 LOW
        pinMode(0, INPUT_PULLUP);  digitalWrite(0, HIGH);
        digitalWrite(2, HIGH);    // GPIO 2 is on board LED
        /// make sure GPIO 0 : HIGH, GPIO 2: HIGH, GPIO 15: :LOW  when ESP.restart( )
        delay(168);    // NodeMCU 手冊建議說改用 ESP.restart( ); 因 .reset( ) 會殘留暫存器 !
        ESP.restart();   // ESP.reset();    // can ONLY use ESP.restart( );  if ESP32
    }
} // clr_eeprom(int

void save_netInfo(char *wifiSSID, char *wifiPASS, char *ServerIP){  //stoage format: [SSID,PASS,ServerIP]
    char *netInfo[3] = {wifiSSID, wifiPASS, ServerIP};
    int addr=0,i=0,j=0;
    EEPROM.write (addr++,'[');  // the code is equal to (EEPROM.write (addr,'[');  addr=addr+1;)
    for (j=0;j<3;j++){
        i=0;
        while(netInfo[j][i] != '\0') EEPROM.write(addr++,netInfo[j][i++]);
        if(j<2) EEPROM.write(addr++,',');
    }
    EEPROM.write (addr++,']');
    EEPROM.commit();
}

int read_netInfo(char *wifiSSID, char *wifiPASS, char *ServerIP){   // storage format: [SSID,PASS,ServerIP]
    char *netInfo[3] = {wifiSSID, wifiPASS, ServerIP};
    String readdata="";
    int addr=0;
  
    char temp = EEPROM.read(addr++);
    if(temp == '['){
        for (int i=0; i<3; i++){
            readdata ="";
            while(1){
                temp = EEPROM.read(addr++);
                if (temp == ',' || temp == ']') break;
                readdata += temp;
            }
            readdata.toCharArray(netInfo[i], LENTH_INFO);
        }
 
        if (String(ServerIP).length () < 7){
            Serial.println("ServerIP loading failed.");
            return 2;
        }
        else{ 
            Serial.println("Load setting successfully.");
            return 0;
        }
    }
    else{
        Serial.println("no data in eeprom");
        return 1;
    }
}


String scan_network(void){
    int AP_N,i;  //AP_N: AP number 
    // /// ...  color:blue; \" required>" ;// make ap_name
    String AP_List="<select name=\"SSID\" style=\"width: 280px; font-size:16px; color:blue; \" required>" ;// make ap_name in a string
    AP_List += "<option value=\"NCTU-Wireless\" selected>Select AP or Use Specified AP below</option>";
    
    WiFi.disconnect();
    delay(100);
    AP_N = WiFi.scanNetworks();

    if(AP_N>0) for (i=0;i<AP_N;i++) AP_List += "<option value=\""+WiFi.SSID(i)+"\">" + WiFi.SSID(i) + "</option>";
    else AP_List = "<option value=\"\">NO AP</option>";
    AP_List +="</select><br><br>";
    return(AP_List); 
}

ESP8266WebServer server ( 80 );
void handleRoot(int retry){
  String temp = "<html><title>Wi-Fi Setting</title>";
  temp += "<head><meta http-equiv=\"Content-Type\" content=\"text/html; charset=utf-8\"/>";
  temp += "<meta name=\"viewport\" content=\"width=device-width, initial-scale=1.0\">"\
     "<script> function myFun() {"\
     "var x = document.getElementById(\"MyPASS\");"\
     "if (x.type === \"password\") { x.type = \"text\";"\
     "} else { x.type = \"password\";} } "\
     "function myCLR() {"\
     "var x = document.getElementById(\"MySSIDCHK\");"\
     "if(x.checked == true) {document.getElementById(\"MySSID\").value=\"\";"\
     "x.checked = true;} }"\
     "</script></head><body bgcolor=\"#F2F299\">";
  if (retry) temp += "<font color=\"#FF0000\">Please fill all fields.</font>";
  temp += "<form action=\"setup\"><div>";
  temp += "<center>SSID:<br>";
  temp += scan_network(); 
  temp += "Password:<br>";
  temp += "<input type=\"password\" name=\"Password\" vplaceholder=\"Password\" style=\"width: 150px; font-size:16px; color:blue; \">";
  temp += "<br><br>IoTtalk Server IP <br>";  
  temp += "<input type=\"serverIP\" name=\"serverIP\" value=\"";
  temp += DefaultIoTtalkServerIP; 
  temp += "\" style=\"width: 180px; font-size:16px; color:blue;\" required>";

  temp += "<hr style=\"border: 1px solid red;\">";
  temp += "<P align=left><font color=red>Specify SSID if you have one</font>:<br>"; 
  temp += "<span><form id=\"MySSIDFORM\"><input type=\"Text\" name=\"MySSID\" id=\"MySSID\" value=\"";
  temp += String(DefaultMyAP_SSID_); 
  temp += "\" style=\"width: 180px; font-size:18px; color:blue; \">";
  temp += "<input type=\"checkbox\"  style=\"color:red;\" id=\"MySSIDCHK\""\
  " name=\"MySSIDCHK\" onclick=\"myCLR();\"><font color=red>CLEAR</font>"\
  "</input></form></span>";

  temp += "<br><b style=\"font-size:13px;\">!! Leave it BLANK if you are NOT sure</b> !";

  temp += "<br><font color=darkred>Specify its PASSword</font>:<br>"; 
  temp += "<span><input type=\"password\" name=\"MyPASS\" id=\"MyPASS\" value=\"";
  temp += String(DefaultMyAP_PASS_); 
  temp += "\" style=\"width: 160px; font-size:16px; color:darkred; \"> &nbsp;";
  temp += "<input type=\"checkbox\" style=\"font-size:12px;\" onclick=\"myFun();\">Show Password</input></span>";
  temp += "</p>";
  temp += "<br><input style=\"-webkit-border-radius: 11; -moz-border-radius: 11";
  temp += "border-radius: 8px;";
  temp += "text-shadow: 1px 1px 3px #996666;";
  temp += "font-family: Arial; font-weight:bold;";
  temp += "color: #2233ff;";
  temp += "font-size: 24px;";
  temp += "background: #33AAAA;";
  temp += "padding: 10px 20px 7px 20px;";
  temp += "text-decoration: none;\"" ;
  temp += "type=\"submit\" value=\"Submit\" on_click=\"javascript:alert('TEST');\"></center>";
  temp += "</div></form><br>";
  temp += "</body></html>";
  server.send ( 200, "text/html", temp );
}

void handleNotFound() {
  server.send( 404, "text/html", "Page not found.");
}


//////  //////  ////// ====== ======  192.168.0.1/setup      =============================
//    URL ... /setup   comes here
void saveInfoAndConnectToWiFi() {
  #ifdef DEBUG
    Serial.println("Get network information from server page and save them into EEPROM.");
  #endif
    char _SSID_[LENTH_INFO]="";
    char _PASS_[LENTH_INFO]="";
    int gotAP = 0;  // Did we get the AP
    String tmpStr = "";
    /// check arg(3) FIRST  -- Specified AP
    if(server.arg(3) != ""){  // Specified AP not empty
        server.arg(3).toCharArray(_SSID_, LENTH_INFO);
        server.arg(4).toCharArray(_PASS_, LENTH_INFO);
        gotAP = 1;        
    }
    if(!gotAP) // no Specified AP field and thus try arg(0)
    if (server.arg(0) != "" && server.arg(2) != ""){//arg[0]-> SSID, arg[1]-> password (both string)
        server.arg(0).toCharArray(_SSID_, LENTH_INFO);  // AP selected
        server.arg(1).toCharArray(_PASS_, LENTH_INFO);
        gotAP = 1;
    } // 2nd choice
    if(gotAP) {
        server.arg(2).toCharArray(IoTtalkServerIP, LENTH_INFO);
        //  server.send(200, "text/html", tmpStr);
        tmpStr += 
        "<html><body><center><span style=\" font-size:58px; color:blue; margin:88px; \"> "
        "Setup successfully. </span><br>"
        "<p align=Left><span style=\" font-size:50px; color:blue; margin:12px; \"> " ;
        tmpStr += String( _SSID_ );
        tmpStr += " <font color=red> &nbsp; &nbsp;  PassWord: </font> ";
        tmpStr += String( _PASS_  );
        tmpStr += "<br> &nbsp;";
        tmpStr += String( IoTtalkServerIP );
        tmpStr += "<br></p>"\
        "<br><hr style=\"border: 3px solid blue;\">"\
        "<form action=\"/\"><div><center>"\
        "<br><hr style=\"border: 1px solid green;\">"\
        "<br><b style=\" font-size:58px; color:blue; margin:24px; \"> "\
        "<br><input style=\"-webkit-border-radius: 11; -moz-border-radius: 11"\
        "border-radius: 8px;"\
        "text-shadow: 1px 1px 3px #66BB66;"\
        "font-family: Arial; font-size: 58px;"\
        "color: #ff2233; background: #FEFE55;"\
        "padding: 8px 12px 7px 12px;"\
        "text-decoration: none;\"" \
        "type=\"submit\" value=\"Re-Scan/Config Network\" on_click=\"javascript:alert('TEST');\">"\
        "</b></center></div></form><br></center>"\
        "</body></html>";
        server.send(200, "text/html", tmpStr);
        delay(1234); server.stop();
        save_netInfo(_SSID_, _PASS_, IoTtalkServerIP);
        delay(200);
        connect_to_wifi(_SSID_, _PASS_);      
    }
    else {
        handleRoot(1);
    }
} // saveInfoAndConnectToWiFi( 

void start_web_server(void){
    server.on ( "/", [](){handleRoot(0);} );
    server.on ( "/setup", saveInfoAndConnectToWiFi);
    server.onNotFound ( handleNotFound );
    server.begin();  
}

/// ===  Switch to AP mode and start web server at    192.168.0.1
void wifi_setting(void){  
    String softapname = "MCU-";
    uint8_t MAC_array[6];
    WiFi.macAddress(MAC_array);
    for (int i=0;i<6;i++){
        if( MAC_array[i]<0x10 ) softapname+="0";
        softapname+= String(MAC_array[i],HEX);      //Append the mac address to url string
    }
    softapname.toUpperCase();   //  to Upper Case
    Serial.print ( " AP name =  " );  Serial.println (softapname);
 
    IPAddress ip(192,168,0,1);
    IPAddress gateway(192,168,0,1);
    IPAddress subnet(255,255,255,0);  
    WiFi.mode(WIFI_AP_STA);
    WiFi.disconnect();
    WiFi.softAPConfig(ip,gateway,subnet);
    WiFi.softAP(&softapname[0]);
    start_web_server(); delay(200);
    Serial.println("In wifi_setting( ) ... ");
    Serial.println ( "===== Switch to AP mode and start web server." );
    Serial.print ( "  Server IP Address: " ); Serial.println( WiFi.softAPIP( ) );
#ifdef DEBUG
    Serial.print ( "WiFi.macAddress() =  " ); Serial.println(WiFi.macAddress());
    Serial.print("  Subnet Mask: ");  Serial.println(WiFi.subnetMask());
    Serial.print("  Gateway IP: ");  Serial.println(WiFi.gatewayIP());
    Serial.print("  DNS: ");    Serial.println(WiFi.dnsIP());
#endif
}

extern void onOffLED(int onTime=15, int offTime=60, int count=7, int finVal=1);

uint8_t wifimode = 1; //1:AP , 0: STA 
long connectFlash = 0;
void connect_to_wifi(char *wifiSSID, char *wifiPASS){
#ifdef DEBUG33
   Serial.print("Dummy connected to "); Serial.println(String(wifiSSID));
   wifimode = 0;   // STA
   if(38 == 38) return;
#endif
  Serial.print("Try to connect to "); Serial.println(String(wifiSSID));
  long connecttimeout = millis();

  WiFi.softAPdisconnect(true);
  Serial.println("====== ----- Do WiFi.begin() to Connect to Wi-Fi----- ======");
  WiFi.begin(wifiSSID, wifiPASS);

/// About WiFi status code
/// See  https://www.arduino.cc/en/Reference/WiFiStatus
  onOffLED(13, 57, 7, 1);
  long connectFlash = millis() + 3000;   // every 3 sec.
  while (WiFi.status() != WL_CONNECTED && (millis() - connecttimeout < wifiTimeout) ) {  // 25000
      delay(333);
      if (digitalRead(clr_ROM_pin) == LOW) clr_eeprom(); 
      delay(335);
      if (digitalRead(clr_ROM_pin) == LOW) clr_eeprom(); 
      delay(333);
      if (digitalRead(clr_ROM_pin) == LOW) clr_eeprom();       
      #ifdef DEBUG
      Serial.print(". "); Serial.flush( ); delay(168);
      #endif
      if( millis( ) > connectFlash) {
          onOffLED(13, 57, 7, 1);
          connectFlash = millis() + 3000; 
      } // if( 已超過 3 秒
  } // while ( not timeOut 

  if(WiFi.status() == WL_CONNECTED){
    Serial.print ( "Connected to WiFi SSID ");
    Serial.println ( WiFi.SSID( ) );    Serial.flush( );
    digitalWrite(obLED, LOW);  // turn ON on board LED, GPIO 2
#ifdef DEBUG
  Serial.print("Local IP: ");
  Serial.println(WiFi.localIP());   //IP address assigned to ESP12E
      //////
    Serial.print("  Subnet Mask: ");   Serial.println(WiFi.subnetMask());
    Serial.print("  Gateway IP: ");   Serial.println(WiFi.gatewayIP());
    Serial.print("  DNS: ");   Serial.println(WiFi.dnsIP());
#endif
    wifimode = 0;  // STA  , station mode
  }
  else if (millis() - connecttimeout > wifiTimeout){   // 25000  ;  25 sec.
    Serial.println("Connect fail, try to run wifi_setting();");
    wifi_setting();
  }
}  // connect_to_wifi( 

int iottalk_register(void){
#ifdef DEBUG22
   Serial.print("Dummy iottalk_register to "); Serial.println(String(IoTtalkServerIP));
   if(38 == 38) return 200;    //  assume Sucessful 
#endif
    url = "http://" + String(IoTtalkServerIP) + ":9999/";  
    
    String df_list[] = DF_list;
    int n_of_DF = sizeof(df_list)/sizeof(df_list[0]); // the number of DFs in the DF_list
    String DFlist = ""; 
    for (int i=0; i<n_of_DF; i++){
        DFlist += "\"" + df_list[i] + "\"";  
        if (i<n_of_DF-1) DFlist += ",";
    }
  
    uint8_t MAC_array[6];
    WiFi.macAddress(MAC_array);//get esp12f mac address
    for (int i=0;i<6;i++){
        if( MAC_array[i]<0x10 ) url+="0";
        url+= String(MAC_array[i],HEX);      //Append the mac address to url string
    }
 
    //send the register packet
    Serial.println("[HTTP] POST..." + url);
    String profile="{\"profile\": {\"d_name\": \"";
    //profile += "MCU.";
    for (int i=3;i<6;i++){
        if( MAC_array[i]<0x10 ) profile+="0";
        profile += String(MAC_array[i],HEX);
    }
    profile += "\", \"dm_name\": \"";
    profile += DM_name;
    profile += "\", \"is_sim\": false, \"df_list\": [";
    profile +=  DFlist;
    profile += "]}}";

    http.begin(url);
    http.addHeader("Content-Type","application/json");
    int httpCode = http.POST(profile);

    Serial.println("[HTTP] Register... code: " + (String)httpCode );
    Serial.println(http.getString());
    //http.end();
    url +="/";  
    return httpCode;
}

String df_name_list[nODF];
String df_timestamp[nODF];
void init_ODFtimestamp(){
  for (int i=0; i<nODF; i++) df_timestamp[i] = "";
  for (int i=0; i<nODF; i++) df_name_list[i] = "";  
}

int DFindex(char *df_name){
    for (int i=0; i<nODF; i++){
        if (String(df_name) ==  df_name_list[i]) return i;
        else if (df_name_list[i] == ""){
            df_name_list[i] = String(df_name);
            return i;
        }
    }
    return nODF+1;  // df_timestamp is full
}

int push(char *df_name, String value){
#ifdef DEBUG22
   Serial.print("Push "); Serial.println(df_name);
   Serial.print("  value: "); Serial.println(value);
   if(38 == 38) return 38;
#endif
    http.begin( url + String(df_name));
    http.addHeader("Content-Type","application/json");
    String data = "{\"data\":[" + value + "]}";
    int httpCode = http.PUT(data);
    if (httpCode != 200) Serial.println("[HTTP] PUSH \"" + String(df_name) + "\"... code: " + (String)httpCode + ", retry to register.");
    while (httpCode != 200){
        digitalWrite(4, LOW);
        digitalWrite(2, HIGH);
        httpCode = iottalk_register();
        if (httpCode == 200){
            http.PUT(data);
           // if (switchState) digitalWrite(4,HIGH);
        }
        else delay(3000);
    }
    http.end();
    return httpCode;
}

String pull(char *df_name){
#ifdef DEBUG22
   Serial.print("Pull "); Serial.println(df_name);
   if(38 == 38) return "-38-";
#endif
    http.begin( url + String(df_name) );
   Serial.println(url + String(df_name));
    http.addHeader("Content-Type","application/json");
    int httpCode = http.GET(); //http state code
    
    if (httpCode != 200) Serial.println("[HTTP] "+url + String(df_name)+" PULL \"" + String(df_name) + "\"... code: " + (String)httpCode + ", retry to register.");
    while (httpCode != 200){
        digitalWrite(4, LOW);
        digitalWrite(obLED, HIGH);   // turn Off on board LED
        httpCode = iottalk_register();
        if (httpCode == 200){
            http.GET();
            //if (switchState) digitalWrite(4,HIGH);  
        }
        else delay(3000);
    }
    String get_ret_str = http.getString();  //After send GET request , store the return string
//    Serial.println
    
    Serial.println("output "+String(df_name)+": \n"+get_ret_str);
    http.end();

    get_ret_str = remove_ws(get_ret_str);
    int string_index = 0;
    string_index = get_ret_str.indexOf("[",string_index);
    String portion = "";  //This portion is used to fetch the timestamp.
    if (get_ret_str[string_index+1] == '[' &&  get_ret_str[string_index+2] == '\"'){
        string_index += 3;
        while (get_ret_str[string_index] != '\"'){
          portion += get_ret_str[string_index];
          string_index+=1;
        }
        
        if (df_timestamp[DFindex(df_name)] != portion){
            df_timestamp[DFindex(df_name)] = portion;
            string_index = get_ret_str.indexOf("[",string_index);
            string_index += 1;
            portion = ""; //This portion is used to fetch the data.
            while (get_ret_str[string_index] != ']'){
                portion += get_ret_str[string_index];
                string_index+=1;
            }
            return portion;   // return the data.
         }
         else return "___NULL_DATA___";
    }
    else return "___NULL_DATA___";
}
////////////////////////     ////////////////////////////  ////////////////////
#ifdef DEBUG
void showInfo( ) {
    uint64_t  chipid=ESP.getChipId(); //The chip ID is essentially its MAC address(length: 6 bytes).
    Serial.printf("ESP Chip ID = %04X",(uint16_t)(chipid>>32));//print High 2 bytes
    Serial.printf("%08X\n",(uint32_t)chipid);//print Low 4bytes.
    Serial.printf(" Chip ID in Decimal int: %s\n", String(ESP.getChipId()).c_str() );
    ///  https://www.binaryhexconverter.com/decimal-to-hex-converter
    Serial.print("See: "); 
    Serial.println("https://www.binaryhexconverter.com/decimal-to-hex-converter");
    if(wifimode == 0) return;  // STA
    Serial.print("MAC address: ");  Serial.println(WiFi.macAddress() );
    Serial.print("AP IP address: ");  Serial.println(WiFi.softAPIP() );
}  //  showInfo( 
#endif

void onOffLED(int onTime, int offTime, int count, int finVal){
  digitalWrite(obLED, HIGH); delay(15);
  for(int i=0; i < count; ++i) {
    digitalWrite(obLED, LOW); delay(onTime);
    digitalWrite(obLED, HIGH); delay(offTime);
  }
  digitalWrite(obLED, finVal);   // 1 = Off,  0== On
} // onOffLED(

long sensorValue, suspend = 0;
long cycleTimestamp = millis();

// for Arduino transmit data(UART)
//SoftwareSerial arduino_uart(D2, D1);          // PIN腳(RX, TX)

void setup() {
    Serial.begin(115200);
    //while (!Serial) continue;
    
    // Setup Boardrate arduino -> nodemcu UART
    //arduino_uart.begin(9600);
    
#ifdef DEBUGESP 
    delay(23);
    Serial.setDebugOutput(true);
#endif
#ifdef DEBUG88
  reportMemory( ); delay(168);
#endif
    //pinMode(obLED, OUTPUT);  // 2
    pinMode(obLED, OUTPUT); // D4 : on board led, GPIO 2
    digitalWrite(obLED, LOW);   // turn On the on board LED
    pinMode(clr_ROM_pin, INPUT_PULLUP); // D3, GPIO 0: clear eeprom button
    /// if ESP12F : the pin is GPIO 5
    //  pinMode(5, INPUT_PULLUP);//GPIO5: clear eeprom button for ESP12F
///////////////
    pinMode(16, OUTPUT);// D0~       
    pinMode(5, OUTPUT); // D1~       
    pinMode(4, OUTPUT); // D2~
    pinMode(14, OUTPUT);// D5
    pinMode(12, OUTPUT);// D6    
    pinMode(13, OUTPUT);// D7        
    pinMode(15, OUTPUT);// D8        

    EEPROM.begin(512);

    char wifissid[LENTH_INFO]="";
    char wifipass[LENTH_INFO]="";
    
    int statesCode = read_netInfo(wifissid, wifipass, IoTtalkServerIP); // read EEPROM
    delay(168);     
    digitalWrite(obLED, LOW);   // turn ON the on board LED
    Serial.print("Firmware version: "); Serial.println( version ); Serial.flush( );
#ifdef DEBUG
     Serial.println( "DEBUG On "); Serial.flush( );
#endif
#ifdef DEBUG22
     Serial.println( "DEBUG22 On -- dummy register/pull/push "); Serial.flush( );
#endif
#ifdef DEBUG33
     Serial.println( "DEBUG33  On -- dummy WiFi connected "); Serial.flush( );
#endif
    delay(568);   // LED ON 0.6 sec.
    onOffLED(15, 60, 7, 1);  // around 0.56 sec
    delay(333);
    // delay(2345);
    /////////////
    digitalWrite(obLED, HIGH);   // turn Off on board LED
    if(statesCode == 0)  { // got correct EEPROM
     if(wifissid[0] != 0  && wifissid[0] != ' ') {
       strncpy(DefaultMyAP_SSID_, wifissid, sizeof(DefaultMyAP_SSID_) );
       strncpy(DefaultMyAP_PASS_, wifipass, sizeof(DefaultMyAP_PASS_) );
     }
     onOffLED(15, 60, 2, 1); // around 0.56 sec
     delay(1968);  // 2 sec.
     digitalWrite(obLED, HIGH);   // turn Off on board LED
    }  //  if(statesCode
    Serial.print("In setup( ), DefaultMyAP_SSID_ :");
    Serial.println(String(DefaultMyAP_SSID_)); Serial.flush( );
 #ifdef DEBUG
    Serial.println("EEPROM 50 bytes:");
    for (int k=0; k<50; k++) Serial.printf("%c", EEPROM.read(k) );  //inspect EEPROM data for the debug purpose.
    Serial.println( ); Serial.flush( );
 #endif 
    if (statesCode == 0)  connect_to_wifi(wifissid, wifipass);
    else{
        Serial.println("Load setting failed! statesCode: " + String(statesCode)); // StatesCode 1=No data, 2=ServerIP with wrong format
        Serial.println("Goto wifi_setting() for user to choose AP ..");
        Serial.flush( );
        onOffLED(30, 150, 3, 1);
        delay(168); 
        digitalWrite(obLED, LOW);   // turn ON the  on board LED
        delay(1968);  
        digitalWrite(obLED, HIGH);   // Off LED
        delay(168);
        wifi_setting();   // LINE  323 ; switch to AP server and waitting for config Net Info.
    }
#ifdef DEBUG
   showInfo( ); Serial.flush( );
#endif

    static int ggyy=1;   // ggyy * 10 is the elasp time ; because delay(10) per Loop in while
    static int yyggg = 1; static int xxx = 0;
    while(wifimode){   // AP 
       if (digitalRead(clr_ROM_pin) == LOW) clr_eeprom();
#ifdef DEBUG33
       if(++xxx > 2000) wifimode = 0;   // STA, Dummy connected to Internet after 20 seconds
#endif
        if(ggyy < 2 )digitalWrite(obLED, 1);
        server.handleClient();  //waitting for connecting to AP ;
        delay(10); 
        if(++ggyy ==  290 )digitalWrite(obLED, 0);    //  290*10 = 2900 = 2.9 sec, LED ON 
        if(ggyy == 300)digitalWrite(obLED, HIGH);       // 3.0 sec turn OFF LED
        if(ggyy == 350)digitalWrite(obLED, LOW);       // 3.5 sec turn ON LED
        if(ggyy == 355){
          digitalWrite(obLED, HIGH);    // 3.55 sec turn OFF LED
         #ifdef DEBUG
          Serial.print(".");  Serial.flush();
         #endif
        }
        if(ggyy > 568  )  { ggyy = 1; ++yyggg; }      // Recycle after 5.68 Sec.
        if(yyggg > 78) {  // 78 dot char / Line
           yyggg=1;  
         #ifdef DEBUG
           Serial.println("X" );  Serial.flush();
         #endif
         #ifdef DEBUG88
           reportMemory( );
          #endif
        } // yyggg > 78
    } // while(wifimode !=0   ---  AP server mode waiting for Browser

/// Now wifimode == 0  which means the NodeMCU connected to some AP now
/// try to register to the iottalk server
    statesCode = 0;
    long tryRegTime = millis( );
    while (statesCode != 200) {
        statesCode = iottalk_register();
        if (statesCode != 200){
            Serial.println("Retry to register to the IoTtalk server. Suspend 3 seconds.");
            if (digitalRead(clr_ROM_pin) == LOW) clr_eeprom();
            delay(1000);
            if (digitalRead(clr_ROM_pin) == LOW) clr_eeprom();
            delay(1000);
            if (digitalRead(clr_ROM_pin) == LOW) clr_eeprom();
            delay(1000);
        } // if (
#ifdef DEBUG88
  reportMemory( ); delay(168);
#endif  
        if( millis( ) - tryRegTime > 15012) {   // 15 sec.
           onOffLED(2, 198, 3, 1);   // Flash 3 times and than turn Off
           tryRegTime = millis( );
        } // if(
    } // while( != 200
    
    init_ODFtimestamp();

    digitalWrite(16,LOW);   // D0~      
    digitalWrite(5,LOW);    // D1~      
    digitalWrite(4,LOW);    // D2~
    digitalWrite(14,LOW);    /// D5
    digitalWrite(12,LOW);    /// D6
    digitalWrite(13,LOW);    /// D7
    digitalWrite(15,LOW);    /// D8
    /// Note that when use GPIO 15 as output pin,
    ///  an external RELAY connected to GPIO15 must be connected between GND and the pin ..
    ///  so that is does not interfere with the action of the pull down resistor.
} // setup( 
////////////////////////////////////////////////////////////////////////
/// See  https://github.com/IoTtalk/ArduTalk-for-NodeMCU

int pinA0; 
long LEDflashCycle = millis();
long LEDonCycle = millis();
int LEDhadFlashed = 0;
int LEDisON = 0;

#ifdef DEBUG
int ggcount = 0;
#endif
#ifdef DEBUG55
long loopCount = 1;   // counting the Loop times in loop( )
#endif
void loop() {
    // Read Json From Arduino (TinPhone)
    //StaticJsonDocument<1024> Data;      // 建立JsonDocument by ArduinoJson.h

    // Deserialize the JSON document
    //DeserializationError error = deserializeJson(Data, arduino_uart);

    // Test if parsing succeeds.
    /*while (error) {
      Serial.println("Invalid JSON Object");
      delay(0.5*1000);
      DeserializationError error = deserializeJson(Data, arduino_uart);
    }*/

    /* Fetch values.
    int light_analogVal = Data["brightness"];
    float soilMois_percentage = Data["soilMoisture"];
    float air_t = Data["airTemperature"];
    float air_h = Data["airHumidity"];
    */
    if (digitalRead(clr_ROM_pin) == LOW) clr_eeprom();   // 隨時檢查看是否要 CLEAR EEPROM

    if (millis() - cycleTimestamp > 200) {   // at least after 0.2 second
#ifdef DEBUG
    Serial.print(++ggcount);
    Serial.println("::");
#endif
        pinA0 = analogRead(A0);          // Tinphoen Edit           
        push("A0", String(pinA0));
// pull("D0~")  -->  analogWrite to GPIO 16  
        result = pull("D0~");
        if (result != "___NULL_DATA___"){
            Serial.println ("D0~: "+result);
            if (result.toInt() >= 0 && result.toInt() <= 255) analogWrite(16, result.toInt());
        }
// pull("D1~")  -->  analogWrite to GPIO 5
        result = pull("D1~");
        if (result != "___NULL_DATA___"){
            Serial.println ("D1~: "+result);
            if (result.toInt() >= 0 && result.toInt() <= 255) analogWrite(5, result.toInt());
        }  
// pull("D2~")  -->  analogWrite to GPIO 4
        result = pull("D2~");
        if (result != "___NULL_DATA___"){
            Serial.println ("D2~: "+result);
            if (result.toInt() >= 0 && result.toInt() <= 255) analogWrite(4, result.toInt());
        }
// pull("D5")  -->  digitalWrite to GPIO 14
        result = pull("D5");
        if (result != "___NULL_DATA___"){
            Serial.println ("D5: "+result);
            if (result.toInt() > 0 ) digitalWrite(14, 1);
            else digitalWrite(14, 0);
        }
// pull("D6")  -->  digitalWrite to GPIO 12
        result = pull("D6");
        if (result != "___NULL_DATA___"){
            Serial.println ("D6: "+result);
            if (result.toInt() > 0 ) digitalWrite(12, 1);
            else digitalWrite(12, 0);
        }
// pull("D7")  -->  digitalWrite to GPIO 13      
        result = pull("D7");
        if (result != "___NULL_DATA___"){
            Serial.println ("D7: "+result);
            if (result.toInt() > 0 ) digitalWrite(13, 1);
            else digitalWrite(13, 0);
        }
////////
    /// Note that when use GPIO 15 as output pin,
    ///  an external RELAY connected to GPIO15 must be connected between GND and the pin ..
    ///  so that is does not interfere with the action of the pull down resistor.
// pull("D8")  -->  digitalWrite to GPIO 15
        result = pull("D8");
        if (result != "___NULL_DATA___"){
            Serial.println ("D8: "+result);
            if (result.toInt() > 0 ) digitalWrite(15, 1);
            else digitalWrite(15, 0);
        }
        cycleTimestamp = millis();
    }  // if cycleTimestamp after  200 msec
////// 
/// Note that GPIO6 to GPIO11 are Flash Memory GPIO Pins and thus can NOT be used !!!
/// See: https://www.instructables.com/id/ESP8266-Using-GPIO0-GPIO2-as-inputs/

    if (millis() - LEDflashCycle > 2000){   // at least after 2 seconds
        LEDhadFlashed = 0;      // mark as NOT flash the LED yet, BUT must to
        LEDflashCycle = millis();   // next flash time is 2 sec. + NOW 
#ifdef DEBUG88
  reportMemory( );
#endif
    }

    if (!LEDhadFlashed){   //  if need to flash the LED
      digitalWrite(obLED, 0);  // turn On on board LED  , GPIO 2
      LEDhadFlashed = 1;   // Flag to indicate Flash job done 
      LEDonCycle = millis();  // Time that the LED ON
      LEDisON = 1;
    }
/////// Turn OFF LED after at least 5 ms.
    if ( LEDisON && (millis()-LEDonCycle > 5) ) {
#ifdef DEBUG
    long yyTime = millis()-LEDonCycle;
#endif       
        digitalWrite(obLED, 1);   // turn OFF after at least on 5 msec
        LEDisON = 0;
#ifdef DEBUG
        Serial.print("\n=== LED already ON for ");
        Serial.println( yyTime );
#endif    
    }  //  if ( LEDisON && ..
#ifdef DEBUG55
    if(++loopCount %10000 == 0){
        Serial.print("\n=== Loop count in loop( ) :  ");
        Serial.println( loopCount );       
    }
#endif    
} // loop( 
