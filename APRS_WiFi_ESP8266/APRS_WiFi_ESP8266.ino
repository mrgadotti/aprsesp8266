/* Library used by ESP8266 */
#include <ESP8266WiFi.h>

/* Wifi parameters */
const char* ssid     = "Wifi_SSID";
const char* password = "WiFiPassword";

/* APRS server */
char SVR_NAME[] = "brazil.d2g.com";
#define SVR_PORT 14579

/***************************************************************/
/*      Station parameters                                     */
/***************************************************************/

/* Define your callsign, passcode*/
#define callsign "PP5MGT-1"
#define passcode "00000"

/*
    Usar o exemplo do link:
    Obter pelo Google Maps:
    Latitude -27.590348 Longitude -48.519487
    Converter para o formato reconhecido pelo APRS:
    http://digined.pe1mew.nl/?How_to:Convert_coordinates
    Latitude 2735.42S Longitude 048.31.17W
*/
/* Define your location */
#define location "2735.42S/04831.17W"

/*
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
*/
#define sta_symbol "`"

/* Define your comment */
#define comment "Arduino APRS-IS - pp5mgt@qsl.net"

/* Update interval in minutes */
int REPORT_INTERVAL = 10;

/***************************************************************/

/* Don not change anything bellow */

#define VER "1.0"
#define SVR_VERIFIED "verified"

#define TO_LINE  10000

// Use WiFiClient class to create TCP connections
WiFiClient client;

void setup() {
  Serial.begin(9600);
  delay(10);

  // We start by connecting to a WiFi network

  Serial.println();
  Serial.println();
  Serial.print("Connecting to ");
  Serial.println(ssid);

  WiFi.begin(ssid, password);

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("");
  Serial.println("WiFi connected");
  Serial.print("IP address: ");
  Serial.println(WiFi.localIP());
  Serial.println("");
}



void loop() {

  if ( client.connect(SVR_NAME, SVR_PORT) )
  {
    Serial.println("Server connected");
    client.print("user ");
    client.print(callsign);
    client.print(" pass ");
    client.print(passcode);
    client.print(" vers ESPAPRS ");
    client.println(VER);
    if ( wait4content(&client, SVR_VERIFIED, 8) )
    {
      Serial.println("Login OK");
      client.print(callsign);
      client.print(">APRS,TCPIP*,qAC,WIDE1-1,WIDE2-1,BRASIL:@000000h");
      client.print(location);
      client.print(sta_symbol);
      client.print(comment);
      Serial.println("Data sent OK");
      delay(5000);
      client.stop();
      Serial.println("Server disconnected\n");
      delay((long)REPORT_INTERVAL * 60L * 1000L);
    }
    else
    {
      Serial.println("Login failed.");
    }
  }
  else
  {
    Serial.println("Can not connect to the server.");
  }

}

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

