#include "Arduino.h"
#include "ESP8266WiFi.h"
#include "WiFiClientSecure.h"
#include "EEPROM.h"
#include "ArduinoJson.h"
#include "myonlinehome_tk.h"


myonlinehome_tk::myonlinehome_tk()
{
  Serial.begin(9600);
  for(int i=1;i<=50;i++)
  {
    print_data_ln("");
  }
  print_data("Starting Serial Monitor.......");
  print_data_ln("OK");
  pinMode(LED_BUILTIN, OUTPUT);
  delay(100);
  digitalWrite(LED_BUILTIN, HIGH);
  delay(100);
}
void myonlinehome_tk::getVersion()
{
  char* ver = MOH_VER;
  print_data_ln(ver);
}
void myonlinehome_tk::begin(WiFiClientSecure client, const char* node_user, const char* node_password, const char* node_secret, char* node_ssid, char* node_passkey)
{
  this->_ssid_reserve = node_ssid;
  this->_pwd_reserve = node_passkey;
  setclient(client);
  set_node_secret(node_secret);
  printArtWork();
  print_data_ln("Starting MyOnlineHome Client ..........OK");
  if(strlen(node_user) >= 5)
  {
    this->_node_username = node_user;
  }
  else
  {
    print_data_ln("Minimum 5 Character Username is Required.");
    return;
  }
  if(strlen(node_password) >= 5)
  {
    this->_node_password = node_password;
  }
  else
  {
    print_data_ln("Minimum 5 Character Password is Required.");
    return;
  }
  String ssid_buffer = this->read_EEPROM_SSID().c_str();
  this->_ssid = (char*) ssid_buffer.c_str();
  String ssid_pwd = this->read_EEPROM_PWD().c_str();
  this->_pwd = (char*) ssid_pwd.c_str();
  delay(100);
  print_data_ln("Note: Please Use moh.custom_set(node_SSID, node_PWD); to connect to custom ssid.");
  custom_set(node_ssid, node_passkey);
  delay(100);
  if (WiFi.status() != WL_CONNECTED)
  {
    WIFI_Connect();
  }
  test_node();
}
myonlinehome_tk::~myonlinehome_tk()
{
  this->_ssid="";
  this->_pwd="";
  this->_node_username="";
  this->_node_password="";
}
void myonlinehome_tk::printArtWork(void)
{
    print_data_ln("");
    print_data_ln("  MOH MOH                MOH MOH                                               ");
    print_data_ln("MOH      MOH         MOH      MOH                                              ");
    print_data_ln("            MOH   MOH                                    MOH             MOH   ");
    print_data_ln("         MOH   MOH   MOH             MOH MOH MOH MOH     MOH             MOH   ");
    print_data_ln("         MOH   MOH   MOH           MOH             MOH   MOH             MOH   ");
    print_data_ln("         MOH   MOH   MOH           MOH             MOH     MOH MOH MOH MOH     ");
    print_data_ln("         MOH   MOH   MOH           MOH             MOH   MOH             MOH   ");
    print_data_ln("         MOH         MOH           MOH             MOH   MOH             MOH   ");
    print_data_ln("         MOH         MOH             MOH MOH MOH MOH     MOH             MOH   ");
    print_data_ln("                                                                               ");
    print_data("                                                                               ");
    getVersion();
    WiFi.disconnect();
}

void myonlinehome_tk::custom_set(char* node_ssid, char* node_passkey)
{
  String ssid_buffer = this->read_EEPROM_SSID().c_str();
  char* this_ssid = (char*) ssid_buffer.c_str();
  String pwd_buffer = this->read_EEPROM_PWD().c_str();
  char* this_pwd = (char*) pwd_buffer.c_str();
  if(ssid_buffer == node_ssid)
  {
    print_data_ln("Same SSID is stored. No need to Overwrite.");
  }
  else
  {
    write_EEPROM_SSID(node_ssid);
    print_data_ln("SSID Overwritten.");
  }
  if(pwd_buffer == node_passkey)
  {
    print_data_ln("Same Password is stored. No need to Overwrite.");
  }
  else
  {
    write_EEPROM_PWD(node_passkey);
    print_data_ln("Passkey Overwritten.");
  }
  this->_ssid = node_ssid;
  this->_pwd = node_passkey;
}


// Basic Functions of NodeMCU
String myonlinehome_tk::_read_EEPROM(int min, int max)
{
    EEPROM.begin(1024);
    delay(10);
    String buffer;
    for (int L = min; L < max; ++L)
    {
      if (EEPROM.read(L) > 0)
      {
        buffer += char(EEPROM.read(L));
      }
    }
    //print_data_ln("Read From EEPROM ");
    //print_data_ln(buffer);
    return buffer;
}

bool myonlinehome_tk::_write_EEPROM(String buffer, int start)
{
  int size = strlen(buffer.c_str());
  if(size > MAX_SSID_LEN)
  {
    size > MAX_SSID_LEN;
  }
  EEPROM.begin(1024);
  delay(10);
  for (int L = 0; L < 32; ++L)
  {
    EEPROM.write(start + L, 0);
  }
  for (int L = 0; L < size; ++L)
  {
    EEPROM.write(start + L, buffer[L]);
  }
  EEPROM.commit();
  //print_data_ln("Written To EEPROM ");
  //print_data_ln(buffer);
}


// Functions Related to WiFi
void myonlinehome_tk::WIFI_Connect()
{
  while(WiFi.status() != WL_CONNECTED)
  {
    WiFi.softAPdisconnect();
    WiFi.disconnect();
    WiFi.mode(WIFI_STA);
    delay(1000);
    this->scanAndSort();
    delay(1000);
    if(strlen(ssid) > 0)
    {
      print_data("Connecting to ...");
      print_data_ln(ssid);

     WiFi.begin(ssid);
     unsigned short try_cnt = 0;
     while (WiFi.status() != WL_CONNECTED && try_cnt < 60)
     {
       delay(1000);
       try_cnt++;
     }
     if(WiFi.status() == WL_CONNECTED)
     {
       print_data("Connected To WiFi !");
       print_data(" (Local IP : ");
       print_data((String)WiFi.localIP().toString());
       print_data_ln(")");
       test_network();
       String internal_ip = WiFi.localIP().toString();
       this->_node_internal_ip = internal_ip ;
       this->_ssid = ssid;
     }
     else
     {
       print_data_ln("Connection FAILED");
       connect_private();
     }
    }
    else
    {
     print_data_ln("No open networks available. :-(");
     connect_private();
    }
    digitalWrite(2,0);
  }
}
void myonlinehome_tk::connect_private()
{
  print_data_ln("Trying To Connect To Private WiFi Network ...");
  WiFi.softAPdisconnect();
  WiFi.disconnect();
  WiFi.mode(WIFI_STA);
  delay(1000);
  String ssid_buffer = this->read_EEPROM_SSID().c_str();
  this->_ssid = (char*) ssid_buffer.c_str();
  String ssid_pwd = this->read_EEPROM_PWD().c_str();
  this->_pwd = (char*) ssid_pwd.c_str();

  print_data("Private ssid is - ");
  print_data_ln(this->_ssid);
  //print_data("Passkey is - ");
  //print_data_ln(this->_pwd);
  WiFi.begin(this->_ssid, this->_pwd);
   // Wait for connection
  for (int i = 0; i < 50; i++)
  {
   if ( WiFi.status() != WL_CONNECTED )
   {
     delay (500);
   }
  }
  if(WiFi.status() == WL_CONNECTED)
  {
    print_data("Connected To WiFi !");
    print_data(" (Local IP : ");
    print_data((String)WiFi.localIP().toString());
    print_data_ln(")");
    String internal_ip = WiFi.localIP().toString();
    this->_node_internal_ip = internal_ip ;
  }
  else
  {
    print_data_ln("Unable To Connect. Please Use moh.WiFi_connect();");
  }
}

void myonlinehome_tk::scanAndSort() {
  memset(ssid, 0, MAX_SSID_LEN);
  int n = WiFi.scanNetworks();
  if (n == 0)
  {
    print_data_ln("No Networks Found.");
  }
  else
  {
    print_data((String)n);
    print_data_ln(" Networks Found.");
    int indices[n];
    print_data_ln("-----------------------");
    print_data("Network Name    (Encryption) - ");
    print_data_ln("Signal Strength: ");
    for (int i = 0; i < n; i++)
    {
      indices[i] = i;
      if(WiFi.SSID(i).length() > 14)
      {
        print_data(WiFi.SSID(i));
        print_data("(");
      }
      else if(WiFi.SSID(i).length() > 10)
      {
        print_data(WiFi.SSID(i));
        print_data("\t (");
      }
      else
      {
        print_data(WiFi.SSID(i));
        print_data("\t \t (");
      }

      byte encryption = WiFi.encryptionType(i);
      String Encryption_This;
      switch (encryption)
      {
        case 2:
        Encryption_This = "TKIP (WPA)";
        break;
        case 4:
        Encryption_This = "CCMP (WPA) ";
        break;
        case 5:
        Encryption_This = "WEP";
        break;
        case 7:
        Encryption_This = "Open";
        break;
        case 8:
        Encryption_This = "Auto";
        break;

        default:

        break;
      }
      print_data(Encryption_This);
      print_data(")");
      print_data_ln((String)WiFi.RSSI(i));

    }
    print_data_ln("-----------------------");


    for (int i = 0; i < n; i++)
    {
      for (int j = i + 1; j < n; j++)
      {
        if (WiFi.RSSI(indices[j]) > WiFi.RSSI(indices[i]))
        {
          std::swap(indices[i], indices[j]);
        }
      }
    }
    for (int i = 0; i < n; ++i)
    {
      if(WiFi.encryptionType(indices[i]) == ENC_TYPE_NONE)
      {
        memset(ssid, 0, MAX_SSID_LEN);
        strncpy(ssid, WiFi.SSID(indices[i]).c_str(), MAX_SSID_LEN);
        break;
      }
    }
  }
}
void myonlinehome_tk::restart_node()
{
 print_data_ln("Restarting Node.....");
 delay(1000);
 String ssid_buffer = this->read_EEPROM_SSID().c_str();
 char* this_ssid = (char*) ssid_buffer.c_str();
 String pwd_buffer = this->read_EEPROM_PWD().c_str();
 char* this_pwd = (char*) pwd_buffer.c_str();
 begin(this->client, this->_node_username, this->_node_password, this->_node_secret, this_ssid, this_pwd);
}

String myonlinehome_tk::read_EEPROM_SSID()
{
  String buffer=this->_read_EEPROM(96,127);
  return buffer;
}

bool myonlinehome_tk::write_EEPROM_SSID(String SSID)
{
  return this->_write_EEPROM(SSID, 96);
}

String myonlinehome_tk::read_EEPROM_PWD()
{
  return this->_read_EEPROM(128,159);
}

bool myonlinehome_tk::write_EEPROM_PWD(String PWD)
{
  return this->_write_EEPROM(PWD, 128);
}



//Functions Related to Update/Receive Sensor Values
void myonlinehome_tk::setclient(WiFiClientSecure client)
{
  this->client = client;
}
void myonlinehome_tk::set_node_secret(const char* node_secret)
{
  this->_node_secret = node_secret;
}
void myonlinehome_tk::test_network()
{
  String data="testNetwork";
  String result = httpspost(data);
  DynamicJsonBuffer  jsonBuffer(2000);
  JsonObject& root = jsonBuffer.parseObject(result);

  // Test if parsing succeeds.
  if (!root.success()) {
    print_data_ln("Connection Failed");
    delay(2000);
    restart_node();
    return;
  }

  String status = root["Network"].as<String>();
  if(status == "Success")
  {
    print_data_ln("Network OK");
  }
  else
  {
    connect_private();
  }
}
void myonlinehome_tk::test_node()
{
  String data="testNode";
  data += "&node_secret=";
  data += (String) this->_node_secret;
  String result = httpspost(data);
  //print_data_ln(result);
  DynamicJsonBuffer  jsonBuffer(2000);
  JsonObject& root = jsonBuffer.parseObject(result);

  // Test if parsing succeeds.
  if (!root.success()) {
    print_data_ln("parseObject() failed");
    delay(2000);
    restart_node();
    return;
  }

  // Fetch values.
  //
  // Most of the time, you can rely on the implicit casts.
  // In other case, you can do root["time"].as<long>();
  int node_id = root["Node Id"];
  this->_node_id = node_id;
  if((int) node_id > 0)
  {
    this->_node_valid = 1;
  }
  String IP = root["IP"];
  this->_node_external_ip =IP;
  String time = root["time"];
  this->_node_last_update = time;
  String node_location ="{";
  node_location += root["data"]["loc"].as<String>();
  node_location +=  "} - ";
  node_location += root["data"]["city"].as<String>();
  node_location += ", ";
  node_location += root["data"]["region"].as<String>();
  node_location += ", ";
  node_location += root["data"]["country"].as<String>();
  node_location += " - ";
  node_location += root["data"]["postal"].as<String>();
  this->_node_location = node_location;
  get_sensors();
  print_data_ln("Node Updated");
}


void myonlinehome_tk::get_sensors()
{
  print_data("Checking Node Status .......");
  if(this->_node_valid > 0)
  {
    print_data_ln(" Valid");
  }
  else
  {
    print_data_ln(" Invalid");
  }
  String data="getSensors";
  data += "&node_id=";
  data += (String) this->_node_id;
  String result = httpspost(data);
  //print_data_ln(result);
  DynamicJsonBuffer  jsonBuffer(2000);
  JsonObject& root = jsonBuffer.parseObject(result);

  // Test if parsing succeeds.
  if (!root.success()) {
    print_data_ln("parseObject() failed");
    delay(2000);
    restart_node();
    return;
  }

  String details = root["data"].as<String>();
  int sensors_attached = root["Sensors_Attached"].as<int>();
  print_data("Node Have ");
  print_data((String)sensors_attached);
  this->sensors_attached = sensors_attached;
  print_data_ln(" Sensors Attached With It");
  for(int i=0; i <= sensors_attached; i++)
  {
    String this_sensor= root["data"]["Sensor"][i].as<String>();
    this->sensor[i]= this_sensor;
  }
  show_basic_info();

  /*print_data_ln("Node Have Following Details");
  print_data_ln(details);*/
}


void myonlinehome_tk::update_sensors()
{
  if(this->_node_valid > 0)
  {
   String data="getSensors";
   data += "&node_id=";
   data += (String) this->_node_id;
   String result = httpspost(data);
   //print_data_ln(result);
   DynamicJsonBuffer  jsonBuffer(2000);
   JsonObject& root = jsonBuffer.parseObject(result);

   // Test if parsing succeeds.
   if (!root.success()) {
     print_data_ln("parseObject() failed");
     delay(2000);
     restart_node();
     return;
   }

   String details = root["data"].as<String>();
   int sensors_attached = root["Sensors_Attached"].as<int>();
   print_data("Node Have ");
   print_data((String)sensors_attached);
   print_data_ln(" Sensors Attached With It");
   if(sensors_attached != this->sensors_attached)
   {
    print_data_ln("No of Sensors in this Node has been changed. Restarting NodeMCU in 5 Seconds");
   }
   this->sensors_attached = sensors_attached;
   for(int i=0; i <= sensors_attached; i++)
   {
    String this_sensor= root["data"]["Sensor"][i].as<String>();
    this->sensor[i]= this_sensor;
   }
  }
  else
  {
    print_data_ln("Invalid Node. Restarting NodeMCU in 5 Seconds");
    delay(5000);
    restart_node();
  }
}



String myonlinehome_tk::httpspost(String data)
{
  String url = this->url;
  WiFiClientSecure client= this->client;
  int statuss = WiFi.status();
  if (WiFi.status() != WL_CONNECTED)
  {
    WIFI_Connect();
  }
  else
  {
    data = "action="+ data + "&userName=" +(String)this->_node_username + "&userPassword=" +(String)this->_node_password;
    //print_data_ln(data);
    delay(10);
    //client.setFingerprint(this->fingerprint);
    print_data("Connecting to server..... ");
    if (!client.connect("myonlinehome.tk", 443))
    {
      WiFi.disconnect();
      print_data_ln(" Failed.");
      delay(5000);
      print_data_ln("Restarting Node in 5 Seconds");
      delay(5000);
      restart_node();
      return "Connection Failed";
    }
    else
    {
      print_data_ln(" Connected.");
    }
    if (client.verify(this->fingerprint, this->server))
    {
        print_data("[Sending request]");
        client.println("POST " + url + " HTTP/1.1");
        client.println("Host: " + (String)server);
        client.println("User-Agent: ESP8266/1.0");
        client.println("Connection: close");
        client.println("Content-Type: application/x-www-form-urlencoded;");
        client.print("Content-Length: ");
        client.println(data.length());
        client.println();
        client.println(data);
        print_data_ln("...... OK");
        delay(10);

        print_data("[Recieving Data:]");
        String response = client.readString();
        //print_data_ln(response);
        int bodypos =  response.indexOf("\r\n\r\n") + 4;
        response=response.substring(bodypos);
        bodypos =  response.indexOf("\n");
        print_data_ln("...... OK");
        return response.substring(bodypos);
      }
      else
      {
        WiFi.disconnect();
        return "ERROR";
      }
    }
}
void myonlinehome_tk::print_data(String data)
{
 Serial.print(data);
}
void myonlinehome_tk::print_data_ln(String data)
{
 Serial.println(data);
}
void myonlinehome_tk::show_basic_info(void)
{
  print_data_ln("Basic Information Related to This Node are as : ");
  print_data("Node ID     : ");
  print_data((String)this->_node_id);
  if(this->_node_valid == 1)
  {
    print_data_ln("  [Valid]");
  }
  else
  {
    print_data_ln("  [Invalid]");
  }
  print_data("Node Secret : ");
  print_data_ln(this->_node_secret);
  print_data("Node User   : ");
  print_data_ln(this->_node_username);
  print_data("Last Update : ");
  print_data_ln(this->_node_last_update);
  print_data("IP Address  : Int. (");
  print_data(this->_node_internal_ip);
  print_data_ln(")");
  print_data("\t      Ext. (");
  print_data(this->_node_external_ip);
  print_data_ln(")");
  print_data("Location    : ");
  print_data_ln(this->_node_location);
  print_data("WiFi SSID [EEPROM] : ");
  String ssid_eeprom = this->read_EEPROM_SSID().c_str();
  print_data_ln(ssid_eeprom);
}
void myonlinehome_tk::show_sensor_info(void)
{
  update_sensors();
  print_data("Various Sensor Values For This Node (Node: ");
  print_data((String)this->_node_id);
  print_data_ln(")");
  DynamicJsonBuffer  jsonBuffer(2000);
  for(int i=0; i < this->sensors_attached; i++)
  {
      JsonObject& root = jsonBuffer.parseObject(this->sensor[i]);
      // Test if parsing succeeds.
      if (!root.success())
      {
       print_data_ln("Unable TO Decode Sensor Data. Please Call Support.");
       delay(10000);
       restart_node();
       return;
      }
      //print_data_ln(this->sensor[i]);
      int sensor_id = root["Sensor_ID"].as<int>();
      String sensor_name = root["Sensor_Name"].as<String>();
      int sensor_type = root["Sensor_Type"].as<int>();
      bool switch_value = 0;
      if(sensor_type == 0 || sensor_type == 12)
      {
        switch_value = root["Sensor_Value"].as<bool>();
      }
      int sensor_pin = root["Sensor_Pin"].as<int>();
      String sensor_value = root["Sensor_Value"].as<String>();
      String sensor_type_text = root["Sensor_Type_Text"].as<String>();
      String sensor_pin_mode = root["Sensor_Pin_Mode"].as<String>();
      if(i==0)
      {
        print_data("Sensor ID [Name] \t ");
        print_data("Sensor Type   \t \t");
        print_data("Sensor Value \t ");
        print_data("Sensor Pin \t ");
        print_data_ln("Sensor Last Updated");
      }

      print_data((String)sensor_id);
      print_data(" [");
      print_data(sensor_name);
      //print_data((String)sensor_name.length());
      if(sensor_name.length() < 10)
      {
        print_data("]  \t\t");
      }
      else if(sensor_name.length() < 18)
      {
        print_data("]  \t");
      }
      else
      {
        print_data("] ");
      }
      print_data((String)sensor_type);
      print_data(" [");
      print_data(sensor_type_text);
      if(sensor_type_text.length() < 10)
      {
        print_data("]  \t\t");
      }
      else if(sensor_type_text.length() < 18)
      {
        print_data("]  \t");
      }
      else
      {
        print_data("]  ");
      }
      print_data(sensor_value);
      if(sensor_value.length() < 10)
      {
        print_data(" \t\t ");
      }
      else
      {
        print_data(" \t ");
      }
      print_data((String)sensor_pin);
      print_data(" [");
      print_data(sensor_pin_mode);
      print_data("] \t");
      print_data_ln(" Coming Soon");
      if(sensor_pin_mode == "OUTPUT" || sensor_pin_mode == "INPUT")
      {
       if(sensor_pin_mode == "OUTPUT")
       {
         pinMode(sensor_pin, OUTPUT);
         delay(2000);
         if(sensor_type == 0)
         {
           digitalWrite(sensor_pin, switch_value);
          }
          if(sensor_type == 12)
          {
            digitalWrite(sensor_pin, !switch_value);
           }
       }
       else
       {
         delay(1000);
         pinMode(sensor_pin, INPUT);
         delay(2000);
       }
      }
      else
      {
        print_data_ln("Invalid Pin Mode ");
        print_data_ln("Resetting Node in 5 Seconds");
      }
   }
}







//test functions
void myonlinehome_tk::test()
{
  digitalWrite(LED_BUILTIN, LOW);
  delay(1000);
  digitalWrite(LED_BUILTIN, HIGH);
  delay(2000);
}
