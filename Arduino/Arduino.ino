#include <Arduino.h>
//#include <SoftwareSerial.h>
//#include <ArduinoJson.h>
#include <Wire.h>
#include <Adafruit_SHT31.h>
#include <U8g2lib.h>

// for OLED Display
U8G2_SSD1306_128X64_NONAME_1_HW_I2C u8g2(U8G2_R0, U8X8_PIN_NONE);

// for澆水按鈕
const int waterBtn_pin = 2;                  // PIN腳
//int waterBtnState = 0;                       // 按鈕狀態
//int before_waterBtnState=0;                  // 前一刻按鈕狀態

// for光線檢測模組的類比輸出
const int lightSensor_pin = A0;              // PIN腳
int light_analogVal = 0;                     // 光線的類比輸入值:0~1023

// for土壤溼度檢測模組:YL-69
const int yl69_pin = A1;                     // PIN腳
int soilMois_analogVal = 0;                  // 土壤濕度的類比輸入值:0~1023
float soilMois_percentage = 0.0;             // 土壤濕度百分比值(由類比值轉換)

// for空氣溫濕度感測模組:SHT31 + Adafruit_SHT31程式庫
Adafruit_SHT31 sht31 = Adafruit_SHT31();     // 使用Adafruit_SHT31()宣告方法
bool enableHeater = false;                   // SHT31狀態
float air_h;                                 // 空氣濕度
float air_t;                                 // 空氣溫度

// for LED燈條
const int led_pin = 4;                       // PIN腳

// for 5V抽水馬達
const int pumpMotor_pin = 7;                 // PIN腳

// for NodeMCU(IoTtalk)端數位訊號接收
const int IoTtalkControl_pin = 8;           // IoTtalk控制權
const int IoTtalkWater_pin = 9;             // IoTtalk澆水訊號
const int IoTtalkTurnLight_pin = 10;        // IoTtalk開燈訊號 

// for NodeMCU端訊號發送(UART發送JSON訊息)
//SoftwareSerial nodemcu_uart(5, 6);          // PIN腳(RX, TX)

void setup() 
{
  // 基礎設定
  Serial.begin(9600);                       // Serial Boardrate
  //nodemcu_uart.begin(9600);                 // arduino -> nodemce UART Boardrate
  pinMode(waterBtn_pin, INPUT);             // 澆水按鈕
  pinMode(lightSensor_pin, INPUT);          // 光線檢測模組
  pinMode(yl69_pin, INPUT);                 // YL-69
  pinMode(led_pin, OUTPUT);                 // 植物燈條
  pinMode(pumpMotor_pin, OUTPUT);           // 抽水馬達
  u8g2.begin();                             // OLED
  u8g2.enableUTF8Print();                   // OLED
  u8g2.setFont(u8g2_font_ncenB08_tr);       // OLED
  pinMode(IoTtalkControl_pin, INPUT);       // NodeMCE(IoTtalk)
  pinMode(IoTtalkWater_pin, INPUT);         // NodeMCE(IoTtalk)
  pinMode(IoTtalkTurnLight_pin, INPUT);     // NodeMCE(IoTtalk)
  
  // 預設關閉
  digitalWrite(led_pin, LOW);                // 植物燈條
  digitalWrite(pumpMotor_pin, HIGH);         // 抽水馬達

  // 確認SHT31 i2c address
  Serial.println("SHT31 test");
  if (! sht31.begin(0x44)) {   // Set to 0x45 for alternate i2c addr
    Serial.println("Couldn't find SHT31");
    while (1) delay(1);
  }
  // 確認SHT31狀態
  if (sht31.isHeaterEnabled())
    Serial.println("SHT31 ENABLED");
  else
    Serial.println("SHT31 DISABLED");

  Serial.println("Initialization OK ...");
  delay(1*1000);
}

int programSecond = 0;                                // loop中的執行秒數
void loop() 
{
    // IoTtalk控制迴圈: 偵測NodeMCU(IoTtalk)是否請求控制權
    if (digitalRead(IoTtalkControl_pin)){
      do{
      IoTtalk_Control_Loop();   // 讓IoTtalk控制
      }while(digitalRead(IoTtalkControl_pin));
    }

    // 讀取澆水按鈕
    if(digitalRead(waterBtn_pin)){                    // 判定按下去的瞬間
      while(digitalRead(waterBtn_pin))
        digitalWrite(pumpMotor_pin, LOW);
      digitalWrite(pumpMotor_pin, HIGH);
    }
    else{
      digitalWrite(pumpMotor_pin, HIGH);
    }
  
    // 讀取各數據
    Read_Bright_Data();                       // 讀取光亮度的類比值
    Read_YL69_Data();                         // 讀取土壤濕度的類比值並轉換成百分比
    Read_SHT31_Data();                        // 讀取空氣溫度、濕度
  
    // OLED顯示即時資訊
    OLED_Display_Data();

    // Send Data to NodeMCU
    //Arduino_UART_NodeMCU(4, 1);

    // 開燈
    if (light_analogVal >= 800)                  // 如果光線類比值>800，開啟LED燈條，否則關閉
      digitalWrite(led_pin, HIGH);
    else
      digitalWrite(led_pin, LOW);
  
    // 澆水
    if (soilMois_percentage < 20){          // 如果土壤濕度<20%，即需要澆水
      digitalWrite(pumpMotor_pin, LOW);  
      programSecond++;
      delay(2*1000);
    }
    else
      digitalWrite(pumpMotor_pin, HIGH);                    
  
    // 若沒有任何動作就是1秒偵測1次狀態
    programSecond++;
    delay(1*1000);
}

// 當IoTtalk取得控制權，進入此loop
void IoTtalk_Control_Loop()
{
  // 偵測NodeMCU(IoTtalk)是否要澆水
  if (digitalRead(IoTtalkWater_pin))
    digitalWrite(pumpMotor_pin, LOW);
  else
    digitalWrite(pumpMotor_pin, HIGH);
  
  // 偵測NodeMCU(IoTtalk)是否要開燈
  if (digitalRead(IoTtalkTurnLight_pin))
    digitalWrite(led_pin, HIGH);
  else
    digitalWrite(led_pin, LOW);

  // 讀取亮度、土壤濕度、空氣溫度、空氣濕度等即時資訊
  Read_Bright_Data();
  Read_YL69_Data();
  Read_SHT31_Data();

  // OLED顯示即時資訊
  OLED_Display_Data();
  programSecond++;
  delay(1*1000);

  // Send Data to NodeMCU
  //Arduino_UART_NodeMCU(4, 1);
}

// 讀取光敏感測的類比數值
void Read_Bright_Data()
{
  light_analogVal = analogRead(lightSensor_pin);
  Serial.print("Light Value = "); Serial.println(light_analogVal);
  //Arduino_UART_NodeMCU(0, 0);
}

// 讀取土壤濕度
void Read_YL69_Data()
{
  // 讀取YL-69的土壤濕度類比值(濕0~乾1023)
  soilMois_analogVal = analogRead(yl69_pin); 

  // 轉換成百分比 
  soilMois_percentage = (1023-soilMois_analogVal) / 1023.0;
  soilMois_percentage *= 100;
  Serial.print("Ground Humidity. % = "); Serial.println(soilMois_percentage);  
  //Arduino_UART_NodeMCU(1, 0);           
}

// 讀取空氣溫度，空氣濕度
void Read_SHT31_Data()
{
  // SHT31讀取空氣中的溫度&濕度
  air_t = sht31.readTemperature();
  air_h = sht31.readHumidity();
  
  if (! isnan(air_t)) {  // check if 'is not a number'
    Serial.print("Temp *C = "); Serial.print(air_t); Serial.print("\t\t");
    //Arduino_UART_NodeMCU(2, 0);
  } else { 
    Serial.println("Failed to read temperature");
  } 
  if (! isnan(air_h)) {  // check if 'is not a number'
    Serial.print("Hum. % = "); Serial.println(air_h); 
    //Arduino_UART_NodeMCU(3, 0);                 
  } else { 
    Serial.println("Failed to read humidity");
  }

  // Toggle heater enabled state every 30 seconds
  // An ~3.0 degC temperature increase can be noted when heater is enabled
  if (programSecond >= 30) {
    enableHeater = !enableHeater;
    sht31.heater(enableHeater);
    Serial.print("Heater Enabled State: ");
    if (sht31.isHeaterEnabled())
      Serial.println("ENABLED");
    else
      Serial.println("DISABLED");

    programSecond = 0;
  }
}

// OLED顯示即時資訊
void OLED_Display_Data()
{
  u8g2.firstPage();
  do {
    u8g2.setCursor(16, 14);
    u8g2.print("Real-Time Data");
    u8g2.setCursor(0, 30);
    u8g2.print("Temparature: "); u8g2.print(air_t); u8g2.print("*C");                 // 空氣溫度
    u8g2.setCursor(0, 46);
    u8g2.print("Humidity: "); u8g2.print(air_h);  u8g2.print("%");                    // 空氣濕度
    u8g2.setCursor(0, 61);   
    u8g2.print("SoilMoisture: "); u8g2.print(soilMois_percentage);  u8g2.print("%");  // 土壤濕度
  }while (u8g2.nextPage());
}

/*// UART傳輸JSON Document to NodeMCU
void Arduino_UART_NodeMCU(byte data, byte transmit)
{ 
  // 建立JsonDocument by ArduinoJson.h
  StaticJsonDocument<1024> Data;                

  // 依照data執行對應指令: 建立JSON Data for 溫溼度/土壤濕度/光照類比值
  switch(data)
  {
    case 0:     // 0: 光照類比值
      Data["brightness"] = light_analogVal;
      break;
      
    case 1:     // 1: 土壤濕度%
      Data["soilMoisture"] = soilMois_percentage;
      break;
      
    case 2:     // 2: 空氣溫度*C
      Data["airTemperature"] = air_t; 
      break;
      
    case 3:     // 3: 空氣溼度%
      Data["airHumidity"] = air_h;
      break;
      
    case 4:     // 4: Nothing
      break;
  } 

  if (transmit)   // Send Data to NodeMCU
    serializeJson(Data, nodemcu_uart);
}*/
