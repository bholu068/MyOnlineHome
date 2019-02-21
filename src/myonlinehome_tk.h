/*
 * myonlinehome_tk.h - A basic class to setup MyOnlineHome.tk settings on ESP8266 NodeMCU v3
 * Created by Electrical Engineering Students and Staff @ CDL Govt. Polytechnic, Nathusari Chopta on 07/10/2018
 * Version 0.0.1 - See readMe
 */

#ifndef moh_h
#define moh_h
#define MOH_VER	"v0.0.4"
#define MAX_SSID_LEN 32

#include "Arduino.h"
#include "ESP8266WiFi.h"
#include "WiFiClientSecure.h"
#include "EEPROM.h"
#include "ArduinoJson.h"

class myonlinehome_tk
{
	public:
		// Declare Constructor and Destructor  Here
		myonlinehome_tk();
		~myonlinehome_tk();

		// Declare Public Variables
		const char* server = "myonlinehome.tk";
		String url="/sensors/nodemcu_handler.php";
		const int port = 443;
		const char* fingerprint = "00:EF:6F:7B:0A:DE:1E:1C:7E:22:A3:BD:A7:8B:49:12:35:2C:25:27";


		// Declare Public Functions Here - Final
		void begin(WiFiClientSecure client, const char* node_user, const char* node_password, const char* node_secret, char* node_ssid, char* node_passkey);
		void custom_set(char* node_ssid, char* node_passkey);
		String httpspost(String data);
		void show_basic_info(void);



		// Test Functions
		void test();





	private:
		// Declare Private Variables - Final
		char* _ssid;
		char* _pwd;
		char ssid[MAX_SSID_LEN] = {};
		int _node_id = 0;
		const char* _node_secret = "";
		const char* _node_username = "";
		const char* _node_password = "";
		const char* _node_SSID = "";
		const char* _node_PWD = "";
		String _node_last_update = "";
		String _node_internal_ip = "";
		String _node_external_ip = "";
		String _node_location = "";
		bool _node_valid = false;
		WiFiClientSecure client;


		// Declare Private Functions - Final
		void printArtWork(void);
		void connect_private(void);
		void scanAndSort(void);
		String _read_EEPROM(int min, int max);
		bool _write_EEPROM(String buffer, int start);
		void getVersion(void);
		void WIFI_Connect(void);
		String read_EEPROM_SSID();
		String read_EEPROM_PWD();
		bool write_EEPROM_SSID(String buffer);
		bool write_EEPROM_PWD(String buffer);
		void setclient(WiFiClientSecure client);
		void test_node();
		void set_node_secret(const char* node_secret);
		void get_sensors();



		// Declare Test Private Functions





};

#endif
