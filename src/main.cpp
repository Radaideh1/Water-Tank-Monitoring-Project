#include <arduino.h>
#include <LiquidCrystal_I2C.h>
#include "secrets.h"


// Wifi : 
#include <WiFi.h>
#include <HTTPClient.h>



const char* ssid = WIFI_SSID;
const char* password = WIFI_PASSWORD;

const char* server = "https://api.thingspeak.com/update";
const char* writeAPIKey = THINGSPEAK_WRITE_KEY;

void Connect_Wifi() {

    WiFi.begin(ssid, password);

    Serial.print("Connecting to WiFi");

    while (WiFi.status() != WL_CONNECTED) {
        delay(500);
        Serial.print(".");
    }

    Serial.println("\nWiFi Connected!");
    Serial.println(WiFi.localIP());
}





// Object : 

class HCSR04 {

public:

  HCSR04(int Echc, int Trig):Pin_Echc(Echc),pin_Trig(Trig)
  {
    pinMode(Pin_Echc, INPUT);
    pinMode(pin_Trig, OUTPUT);

  }

  

  double Calclate_Distance (){

    double distance = (_Send_Signals() * 0.0343) / 2;
    return distance;

    
  }

  
  private:
  const int Pin_Echc;
  const int pin_Trig;

  double _Send_Signals(){
    digitalWrite(pin_Trig, LOW);
    delayMicroseconds(2);
    digitalWrite(pin_Trig, HIGH);
    delayMicroseconds(10);
    digitalWrite(pin_Trig, LOW);

    return pulseIn(Pin_Echc,HIGH,30000);
  }

};

class Water_Tank{ 
  

  public:

  Water_Tank(int E , int T):hcs(E,T){

    _total_internal_tank_Height = 0 ;

    _internal_Tank_Diameter = 0 ;

  
  }

  void setup_total_internal_tank_Height_Automatically() {

    _total_internal_tank_Height = _get_Average();

  }

  void setup_total_internal_tank_Height_Manually(float value){
    _total_internal_tank_Height = value;

  }


  float get_Current_Water_Level_cm() {

    if(_get_Average() >_total_internal_tank_Height ){
      _CurrentWaterLevel = 0.0 ;
    }

    else {
    _CurrentWaterLevel = _total_internal_tank_Height - _get_Average();
      
    }


    return _CurrentWaterLevel;
}

  float get_water_percentage() {

    if (_total_internal_tank_Height <= 0) {
        return 0;
    }

    float percentage =
        (_CurrentWaterLevel / _total_internal_tank_Height) * 100;

    return constrain(percentage, 0, 100);
}


  private:
  
  HCSR04 hcs;
  

  // H :  طول الخزان كامل
  float _total_internal_tank_Height;
 
  // ارتفاع الماء الحالي  
  float _CurrentWaterLevel;

  // D
  float _internal_Tank_Diameter;


  double _get_Average(int average_calculation_count = 10) {

    double tempH = 0;

    for (int i = 0; i < average_calculation_count; i++) {

        tempH += hcs.Calclate_Distance();

        delay(50);
    }

    return tempH / average_calculation_count;
}



};


// init Object : 

LiquidCrystal_I2C lcd(0x27,16,2);
Water_Tank wa_tank(12,13);

// function : 

void Send_Data_To_ThingSpeak() {

    if (WiFi.status() == WL_CONNECTED) {

        HTTPClient http;

        float waterLevel = wa_tank.get_Current_Water_Level_cm();
        float waterPercentage = wa_tank.get_water_percentage();

        String url = String(server) +
                     "?api_key=" + writeAPIKey +
                     "&field1=" + String(waterLevel) +
                     "&field2=" + String(waterPercentage);

        http.begin(url);

        int httpCode = http.GET();

        Serial.print("HTTP Response Code: ");
        Serial.println(httpCode);

        http.end();
    }
}


//  setup Function :  : 

void Lcd_Setup(){

  lcd.init();
  lcd.backlight();

  lcd.print("Radaideh");
  delay(5000);
  lcd.clear();
  


}

void setup(){
  Serial.begin(115200);

  Lcd_Setup();
  
  Connect_Wifi();
  
  wa_tank.setup_total_internal_tank_Height_Automatically();
  
}



// Main function : 

void loop(){

  

  
 
  lcd.setCursor(0,0);
  lcd.print("Water L||Water P");
  lcd.setCursor(0,1);
  lcd.print(wa_tank.get_Current_Water_Level_cm());
  lcd.setCursor(4,1);
  lcd.print(" cm||");
  lcd.setCursor(9,1);
  lcd.print(wa_tank.get_water_percentage());
  lcd.setCursor(13,1);
  lcd.print(" %");

  Send_Data_To_ThingSpeak();


  delay(15000);
  

}