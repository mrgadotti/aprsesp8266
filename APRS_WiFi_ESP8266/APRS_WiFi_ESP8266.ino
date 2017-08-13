/*
  APRS Spot using ESP8266
  PP5MGT - Marcelo
  pp5mgt@qsl.net
  Version 1.10

  Define your station symbol
  "-" Home
  "$" Phone
  "'" Plane
  "`" Antenna
  ">" Car
  "Z" Windows 95
  "," Scout
  "?" Desktop Computer
  Can you find others symbols on APRS documentation

  Get location on Google Maps:
  Latitude -27.590348 Longitude -48.519487
  APRS coordinate converter:
  http://digined.pe1mew.nl/?How_to:Convert_coordinates
  Latitude 2735.42S Longitude 048.31.17W

*/

/* Library used by ESP8266 */
#include <ESP8266WiFi.h>

/* Wifi parameters */
const char* ssid     = "WiFi SSID";
const char* password = "WiFi Password";

/* APRS server */
char SVR_NAME[] = "brazil.d2g.com";
#define SVR_PORT 14579

/* Update interval in minutes */
int REPORT_INTERVAL = 30;

/***************************************************************/

#define VER "1.10"
#define SVR_VERIFIED "verified"

#define TO_LINE  10000

#define BT_BEACON  2

// Use WiFiClient class to create TCP connections
WiFiClient client;

boolean sent;
long interval = 0;
long time_elapsed;

void setup() {
  delay(2000);

  Serial.begin(115200);
  delay(5);

  pinMode(BT_BEACON, INPUT);
  Serial.println("");
  Serial.print("APRS8266 ");
  Serial.println(VER);

  // We start by connecting to a WiFi network
  init_wifi();
  Serial.print("Beacon interval in minutes: ");
  Serial.println(REPORT_INTERVAL);
  Serial.println("");
}

void loop() {

  /*
  * Set your tation info here. You can replicate to other stations.
  */
  send_beacon("CALLSIGN", "PASSWORD", "0000.00S/00000.00W", "SYMBOL", "COMMENT");

  /*********************************************************************************
  * Do not change anything below, used for beacon interval
  */
  interval = (long)REPORT_INTERVAL * 60L * 1000L;
  while (time_elapsed < interval)
  {
    delay(1);
    if (!digitalRead(BT_BEACON))
    {
      // wait 2 seconds after press the beacon button, time to release
      Serial.println("Send Beacon\n");
      delay(2000);
      break;
    }
    time_elapsed++;

  }
  time_elapsed = 0;
}

/*********************************************************************************
* Do not change anything below.
*
*/

/* Connect to WiFi */
void init_wifi()
{
  Serial.println();
  Serial.print("Connecting to ");
  Serial.println(ssid);

  WiFi.disconnect(); // Solve bug from reconect after reset button
  WiFi.begin(ssid, password);

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println();
  Serial.println("WiFi connected");
  Serial.print("IP address: ");
  Serial.println(WiFi.localIP());
}

/* Wait response from server*/
boolean wait4content(Stream* stream, char *target, int targetLen)
{
  size_t index = 0;  // maximum target string length is 64k bytes!
  int c;
  boolean ret = false;
  unsigned long timeBegin;
  delay(50);
  timeBegin = millis();

  while ( true )
  {
    //  wait and read one byte
    while ( !stream->available() )
    {
      if ( millis() - timeBegin > TO_LINE )
      {
        break;
      }
      delay(2);
    }
    if ( stream->available() ) {
      c = stream->read();
      //  judge the byte
      if ( c == target[index] )
      {
        index ++;
        if ( !target[index] )
          // return true if all chars in the target match
        {
          ret = true;
          break;
        }
      }
      else if ( c >= 0 )
      {
        index = 0;  // reset index if any char does not match
      } else //  timed-out for one byte
      {
        break;
      }
    }
    else  //  timed-out
    {
      break;
    }
  }
  return ret;
}

/*
* Send station beacon
*/
unsigned char send_beacon(String user, String passwd, String location, String symbol, String comment)
{
  Serial.println("-----------------------------------------");
  if ( client.connect(SVR_NAME, SVR_PORT) )
  {
    Serial.print("Conected on server: ");
    Serial.println(SVR_NAME);

    client.print("user ");

    client.print(user);
    client.print(" pass ");

    client.print(passwd);
    client.print(" vers APRS8266 ");
    client.println(VER);
    if ( wait4content(&client, SVR_VERIFIED, 8) )
    {
      // Insert your callsign
      Serial.print("Login: ");
      Serial.println(user);
      client.print(user);

      client.print(">APE001,TCPIP*,qAC,WIDE1-1,WIDE2-1,BRASIL:!");
      // Insert your location
      Serial.print("Location: ");
      Serial.println(location);
      client.print(location);
      // Insert your station symbol
      client.print(symbol);
      // Insert your comment
      Serial.print("Comment: ");
      Serial.println(comment);
      client.print(location);
      // Do not change below
      Serial.println("Data sent");
      delay(2000);
      client.stop();
      Serial.println("Server disconnected");
      Serial.println("-----------------------------------------\n");
      delay(500);
      return 1;
    }
    else
    {
      Serial.println("Login failed.");
      Serial.println("-----------------------------------------\n");
      init_wifi(); // Reconnect WiFi if occur any connection error
      return 0;
    }
  }
  else
  {
    Serial.println("Can not connect to the server.");
    Serial.println("-----------------------------------------\n");
    init_wifi(); // Reconnect WiFi if occur any connection error
    return 0;
  }
  Serial.println("-----------------------------------------\n");
}

