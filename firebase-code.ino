
#include <Arduino.h>
#include <WiFi.h>
#include <FirebaseESP32.h>
#include <addons/TokenHelper.h>
#include <Firebase_ESP_Client.h>
// Provide the RTDB payload printing info and other helper functions.
#include <addons/RTDBHelper.h>

int FinalResult=0;
int old_FinalResult =0;

/* 1. Define the WiFi credentials */
#define WIFI_SSID "Ross"
#define WIFI_PASSWORD "dm3j9f@35"
// For the following credentials, see examples/Authentications/SignInAsUser/EmailPassword/EmailPassword.ino

/* 2. Define the API Key */
#define API_KEY "AIzaSyBgpUQqMd8SQj7ZsB33GVh8WJcD0jlD0pk"

/* 3. Define the RTDB URL */
#define DATABASE_URL "https://app2-d200f-default-rtdb.firebaseio.com/" //<databaseName>.firebaseio.com or <databaseName>.<region>.firebasedatabase.app
#define FIREBASE_PROJECT_ID "app2-d200f" //<databaseName>.firebaseio.com or <databaseName>.<region>.firebasedatabase.app

/* 4. Define the user Email and password that already registered or added in your project */
#define USER_EMAIL "lamamahmoud1510@gmail.com"
#define USER_PASSWORD "123456789lama"

// Define Firebase Data object
FirebaseData fbdo;
FirebaseAuth auth;
FirebaseConfig config;

unsigned long sendDataPrevMillis = 0;

unsigned long count = 0;

void setup() {
  
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
  Serial.print("Connecting to Wi-Fi");
  while (WiFi.status() != WL_CONNECTED)
  {
    Serial.print(".");
    delay(300);
  }
  Serial.println();
  Serial.print("Connected with IP: ");
  Serial.println(WiFi.localIP());
  Serial.println();

  Serial.printf("Firebase Client v%s\n\n", FIREBASE_CLIENT_VERSION);

  /* Assign the api key (required) */
  config.api_key = API_KEY;

  /* Assign the user sign in credentials */
  auth.user.email = USER_EMAIL;
  auth.user.password = USER_PASSWORD;

  /* Assign the RTDB URL (required) */
  config.database_url = DATABASE_URL;

  /* Assign the callback function for the long running token generation task */
  config.token_status_callback = tokenStatusCallback; // see addons/TokenHelper.h

  Firebase.begin(&config, &auth);
  // Comment or pass false value when WiFi reconnection will control by your code or third party library e.g. WiFiManager
  Firebase.reconnectNetwork(true);

  //
//   fbdo.setBSSLBufferSize(4096 /* Rx buffer size in bytes from 512 - 16384 */, 1024 /* Tx buffer size in bytes from 512 - 16384 */);

//   Firebase.begin(&config, &auth);

//   Firebase.setDoubleDigits(5);

}

void loop() {

  
    String documentPath = '/node_1/PeNEvH48hHkSe3PTkpRv';
    FirebaseJson content;
    
   
   // if alert
    FinalResult = 1;

   if(FinalResult != old_FinalResult){

       Serial.printf("Set FinalResult ... %s\n", Firebase.setInt(fbdo, F("/FinalResult1"), FinalResult) ? "ok" : fbdo.errorReason().c_str());
       old_FinalResult = FinalResult; 
   }
//  else - not alert case 
     FinalResult = 0;

   if(FinalResult != old_FinalResult){

       Serial.printf("Set FinalResult ... %s\n", Firebase.setInt(fbdo, F("/FinalResult1"), FinalResult) ? "ok" : fbdo.errorReason().c_str());
       old_FinalResult = FinalResult; 



  }
}