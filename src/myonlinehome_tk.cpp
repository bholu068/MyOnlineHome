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
    Serial.println();
  }
  Serial.print("Starting Serial Monitor.......");
  Serial.println("OK");
  pinMode(LED_BUILTIN, OUTPUT);
  delay(100);
  digitalWrite(LED_BUILTIN, HIGH);
  delay(100);
}
void myonlinehome_tk::getVersion()
{
  char* ver = MOH_VER;
  Serial.println(ver);
}
void myonlinehome_tk::begin(WiFiClientSecure client, const char* node_user, const char* node_password, const char* node_secret, char* node_ssid, char* node_passkey)
{
  setclient(client);
  set_node_secret(node_secret);
  printArtWork();
  Serial.println("Starting MyOnlineHome Client ..........OK");
  if(strlen(node_user) >= 5)
  {
    this->_node_username = node_user;
  }
  else
  {
    Serial.println("Minimum 5 Character Username is Required.");
    return;
  }
  if(strlen(node_password) >= 5)
  {
    this->_node_password = node_password;
  }
  else
  {
    Serial.println("Minimum 5 Character Password is Required.");
    return;
  }
  String ssid_buffer = this->read_EEPROM_SSID().c_str();
  this->_ssid = (char*) ssid_buffer.c_str();
  String ssid_pwd = this->read_EEPROM_PWD().c_str();
  this->_pwd = (char*) ssid_pwd.c_str();
  delay(100);
  Serial.println("Note: Please Use moh.custom_set(node_SSID, node_PWD); to connect to custom ssid.");
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
    Serial.println();
    Serial.println("  MOH MOH                MOH MOH                                               ");
    Serial.println("MOH      MOH         MOH      MOH                                              ");
    Serial.println("            MOH   MOH                                    MOH             MOH   ");
    Serial.println("         MOH   MOH   MOH             MOH MOH MOH MOH     MOH             MOH   ");
    Serial.println("         MOH   MOH   MOH           MOH             MOH   MOH             MOH   ");
    Serial.println("         MOH   MOH   MOH           MOH             MOH     MOH MOH MOH MOH     ");
    Serial.println("         MOH   MOH   MOH           MOH             MOH   MOH             MOH   ");
    Serial.println("         MOH         MOH           MOH             MOH   MOH             MOH   ");
    Serial.println("         MOH         MOH             MOH MOH MOH MOH     MOH             MOH   ");
    Serial.println("                                                                               ");
    Serial.print("                                                                               ");
    getVersion();
    WiFi.disconnect();
}

void myonlinehome_tk::custom_set(char* node_ssid, char* node_passkey)
{
  String ssid_buffer = this->read_EEPROM_SSID().c_str();
  char* this_ssid = (char*) ssid_buffer.c_str();
  String pwd_buffer = this->read_EEPROM_PWD().c_str();
  char* this_pwd = (char*) ssid_buffer.c_str();
  if(ssid_buffer == node_ssid)
  {
    Serial.println("Same SSID is stored. No need to Overwrite.");
  }
  else
  {
    write_EEPROM_SSID(node_ssid);
    Serial.println("SSID Overwritten.");
  }
  if(pwd_buffer == node_passkey)
  {
    Serial.println("Same Password is stored. No need to Overwrite.");
  }
  else
  {
    write_EEPROM_PWD(node_passkey);
    Serial.println("Passkey Overwritten.");
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
      Serial.print("Connecting to ...");
      Serial.println(ssid);

     WiFi.begin(ssid);
     unsigned short try_cnt = 0;
     while (WiFi.status() != WL_CONNECTED && try_cnt < 60)
     {
       delay(1000);
       try_cnt++;
     }
     if(WiFi.status() == WL_CONNECTED)
     {
       Serial.print("Connected To WiFi !");
       Serial.print(" (Local IP : ");
       Serial.print(WiFi.localIP());
       Serial.println(")");
       test_network();
       String internal_ip = WiFi.localIP().toString();
       this->_node_internal_ip = internal_ip ;
       this->_ssid = ssid;
     }
     else
     {
       Serial.println("Connection FAILED");
       connect_private();
     }
    }
    else
    {
     Serial.println("No open networks available. :-(");
     connect_private();
    }
    digitalWrite(2,0);
  }
}
void myonlinehome_tk::connect_private()
{
  Serial.println("Trying To Connect To Private WiFi Network ...");
  WiFi.softAPdisconnect();
  WiFi.disconnect();
  WiFi.mode(WIFI_STA);
  delay(1000);
  String ssid_buffer = this->read_EEPROM_SSID().c_str();
  this->_ssid = (char*) ssid_buffer.c_str();
  String ssid_pwd = this->read_EEPROM_PWD().c_str();
  this->_pwd = (char*) ssid_pwd.c_str();

  Serial.print("Private ssid is - ");
  Serial.println(this->_ssid);
  //Serial.print("Passkey is - ");
  //Serial.println(this->_pwd);
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
    Serial.print("Connected To WiFi !");
    Serial.print(" (Local IP : ");
    Serial.print(WiFi.localIP());
    Serial.println(")");
    String internal_ip = WiFi.localIP().toString();
    this->_node_internal_ip = internal_ip ;
  }
  else
  {
    Serial.println("Unable To Connect. Please Use moh.WiFi_connect();");
  }
}

void myonlinehome_tk::scanAndSort() {
  memset(ssid, 0, MAX_SSID_LEN);
  int n = WiFi.scanNetworks();
  if (n == 0)
  {
    Serial.println("No Networks Found.");
  }
  else
  {
    Serial.print(n);
    Serial.println(" Networks Found.");
    int indices[n];
    Serial.println("-----------------------");
    Serial.print("Network Name    (Encryption) - ");
    Serial.println("Signal Strength: ");
    for (int i = 0; i < n; i++)
    {
      indices[i] = i;
      if(WiFi.SSID(i).length() > 14)
      {
        Serial.print(WiFi.SSID(i));
        Serial.print("(");
      }
      else if(WiFi.SSID(i).length() > 10)
      {
        Serial.print(WiFi.SSID(i));
        Serial.print("\t (");
      }
      else
      {
        Serial.print(WiFi.SSID(i));
        Serial.print("\t \t (");
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
      Serial.print(Encryption_This);
      Serial.print(")");
      Serial.println(WiFi.RSSI(i));

    }
    Serial.println("-----------------------");


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
    Serial.println("Connection Failed");
    connect_private();
    return;
  }

  String status = root["Network"].as<String>();
  if(status == "Success")
  {
    Serial.println("Network OK");
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
  //Serial.println(result);
  DynamicJsonBuffer  jsonBuffer(2000);
  JsonObject& root = jsonBuffer.parseObject(result);

  // Test if parsing succeeds.
  if (!root.success()) {
    Serial.println("parseObject() failed");
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
  Serial.println("Node Updated");
}


void myonlinehome_tk::get_sensors()
{
  if(this->_node_valid > 0)
  {
    Serial.println("This is valid Node. Checking Next Detail.");
  }
  else
  {
    Serial.println("This is Invalid Node. Get Valid Node ID First.");
  }
  String data="getSensors";
  data += "&node_id=";
  data += (String) this->_node_id;
  String result = httpspost(data);
  Serial.println(result);
  DynamicJsonBuffer  jsonBuffer(2000);
  JsonObject& root = jsonBuffer.parseObject(result);

  // Test if parsing succeeds.
  if (!root.success()) {
    Serial.println("parseObject() failed");
    return;
  }

  String details = root["data"].as<String>();
  int sensors_attached = root["Sensors_Attached"].as<int>();
  Serial.print("Node Have ");
  Serial.print(sensors_attached);
  Serial.println(" Sensors Attached With It");
  for(int i=0; i <= sensors_attached; i++)
  {
    Serial.print("Sensor ");
    Serial.print(i);
    Serial.print(": ");
    Serial.println(root["data"]["Sensor"][i].as<String>());
  }
  /*Serial.println("Node Have Following Details");
  Serial.println(details);*/
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
    /*Serial.print("WiFi is Connected! (Local IP: ");
    Serial.print(WiFi.localIP());
    Serial.println(")");*/
    data = "action="+ data + "&userName=" +(String)this->_node_username + "&userPassword=" +(String)this->_node_password;
    //Serial.println(data);
    Serial.print("Connecting to server..... ");
    if (!client.connect(this->server, this->port))
    {
      WiFi.disconnect();
      Serial.println(" Failed.");
      return "Connection Failed";
    }
    else
    {
      Serial.println(" Connected.");
    }
    if (client.verify(this->fingerprint, this->server))
    {
        Serial.println("Authenticated");
        Serial.print("[Sending request]");
        client.println("POST " + url + " HTTP/1.1");
        client.println("Host: " + (String)server);
        client.println("User-Agent: ESP8266/1.0");
        client.println("Connection: close");
        client.println("Content-Type: application/x-www-form-urlencoded;");
        client.print("Content-Length: ");
        client.println(data.length());
        client.println();
        client.println(data);
        Serial.println("...... OK");
        delay(10);

        Serial.print("[Recieving Response:]");
        String response = client.readString();
        //Serial.println(response);
        int bodypos =  response.indexOf("\r\n\r\n") + 4;
        response=response.substring(bodypos);
        bodypos =  response.indexOf("\n");
        Serial.println("...... OK");
        return response.substring(bodypos);
      }
      else
      {
        WiFi.disconnect();
        return "ERROR";
      }
    }
}

void myonlinehome_tk::show_basic_info(void)
{
  Serial.println("Basic Information Related to This Node are as : ");
  Serial.print("Node ID     : ");
  Serial.print(this->_node_id);
  if(this->_node_valid == 1)
  {
    Serial.println("  [Valid]");
  }
  else
  {
    Serial.println("  [Invalid]");
  }
  Serial.print("Node Secret : ");
  Serial.println(this->_node_secret);
  Serial.print("Node User   : ");
  Serial.println(this->_node_username);
  Serial.print("Last Update : ");
  Serial.println(this->_node_last_update);
  Serial.print("IP Address  : Int. (");
  Serial.print(this->_node_internal_ip);
  Serial.println(")");
  Serial.print("\t      Ext. (");
  Serial.print(this->_node_external_ip);
  Serial.println(")");
  Serial.print("Location    : ");
  Serial.println(this->_node_location);
  Serial.print("WiFi SSID [EEPROM] : ");
  String ssid_eeprom = this->read_EEPROM_SSID().c_str();
  Serial.println(ssid_eeprom);
}








//test functions
void myonlinehome_tk::test()
{
  digitalWrite(LED_BUILTIN, LOW);
  delay(1000);
  digitalWrite(LED_BUILTIN, HIGH);
  delay(2000);
}
