#include <ESP8266WiFi.h>
#include <WiFiClientSecure.h>
#include <EEPROM.h>
#include <myonlinehome_tk.h>

#define moh_user "Your_MyOnlineHome.tk_User_Name_Here"
#define moh_password "Your_MyOnlineHome.tk_Password_Here"
#define node_secret "Your_MyOnlineHome.tk_Node_Secret_Code_Here"
#define node_SSID "Your_WiFi_SSID"
#define node_PWD "Your_WiFi_Password"
#define UPDATE_GAP 60000   // Gap Between Two Updated

myonlinehome_tk moh;
WiFiClientSecure client;

void setup() 
{
  moh.begin(client, moh_user, moh_password, node_secret, node_SSID, node_PWD);
  delay(1000);
}

void loop() 
{
  moh.test();
  moh.show_basic_info();
  String data="test";    // Command Here
  String response=moh.httpspost(data);
  Serial.println(response);
  delay(UPDATE_GAP);
}
