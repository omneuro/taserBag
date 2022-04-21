#include <SPI.h>
#include<nRF24L01.h>
#include<RF24.h>
#include<DFRobot_ID809.h>
#include<SoftwareSerial.h>
SoftwareSerial Serial1(2,3); //setting pin 2 and 3 as software serial RX and TX respectively
#define FPSerial Serial1
#define COLLECT_NUMBER 3     //Fingerprint sampling times, can be set to 1-3
#define IRQ 6               // Interrupt pin set as pin 6

DFRobot_ID809 fingerprint;

RF24 radio(7,8); //creating an RF24 object with arguments being CE, CSN respectively
const byte address[6] = "00001";   //setting address pipe for transmitting information
uint8_t enrollCount;              //Number of registered users
uint8_t list[80] = {0};           //array to hold ID list


void setup() {
  //Setting up and printing the module information of the finger print scanner.
  Serial.begin(9600);             //initializing the serial port printing to the monitor.
  FPSerial.begin(115200);         //initializing the FPSerial
  fingerprint.begin(FPSerial);    //Take FPSerial as communication port of finger print module
  while(!Serial);                 //wait for serial to open.
  while(fingerprint.isConnected() == false){
    Serial.println("Comunication with device failed, please check connection");
    delay(1000);
  }
  // Module information
  Serial.println("--------------Module Information----------------");
  Serial.print("Module ID: ");
  Serial.println(fingerprint.getDeviceID()); //Read module ID
  Serial.print("Module security level: ");
  Serial.println(fingerprint.getSecurityLevel()); //Read module security level
  Serial.print("Module baud rate: ");
  Serial.println(fingerprint.getBaudrate());
  Serial.print("Module self-learning function: ");
  Serial.println(fingerprint.getSelfLearn()?"ON":"OFF");
  Serial.print("Number of registered fingerprints in the module: ");
  Serial.println(enrollCount = fingerprint.getEnrollCount());
  fingerprint.getEnrolledIDList(list);
  Serial.print("ID list of registered users: ");
  for(uint8_t i=0; i < enrollCount; i++){
    Serial.print(list[i]);
    Serial.print(",");
  }
  Serial.print("\n Number of broken fingerprints: ");
  Serial.println(fingerprint.getBrokenQuantity());
  Serial.println("------------------------------------------------");
  //Setting up the transceiver module
  radio.begin();                  //initializing the radio object
  radio.openWritingPipe(address); //set the address of the reciever to which we will send data
  radio.setPALevel(RF24_PA_MIN);  //setting the pwer amplifier level to minimum because the modules will be tested at a close range (note: will set to max after the near range test phase)
  radio.stopListening();          //sets the module as a transmitter
}

//Blue LED Comparison mode   Yellow LED Registration mode   Red Deletion mode
void loop() {
  if(digitalRead(IRQ)){                           //if IRQ pin is high execute
    uint16_t j = 0;
    if((fingerprint.collectionFingerprint(5)) != ERR_ID809){        //Capture fingerprint image, 5s idle timeout, if timeout = 0, Disable the collection time out function. Return 0 if succeed, otherwise return ERR_ID809
      //Get the time finger pressed down; Set finger print LED ring mode, color, and number of blinks
      fingerprint.ctrlLED(fingerprint.eFastBlink, fingerprint.eLEDBlue, 3);       //Blue LED blinks quickly 3 times, means it's in comparison mode
      while(fingerprint.detectFinger()){          //Wait for finger to realse.
        delay(50);
        j++;
        if(j == 15){                              //Yellow LED blinks quickly 3 times, means it's in registration mode
          fingerprint.ctrlLED(fingerprint.eFastBlink, fingerprint.eLEDYellow, 3) //Set fingerprint LED ring to always ON in yellow
        }else if(j == 30){                        //Red LED blinks quickly 3 times, means it's in deletion mode
          fingerprint.ctrlLED(fingerprint.eFastBlink, fingerprint.eLEDRed, 3) //set fingerprint LED ring to always ON in red
        }
      }
    }
    if(j == 0){
      //Fingerprint capturing failed
    }else if(j > 0 && j < 15){
      Serial.println("Fingerprint comparison mode");
      fingerprintMatching();
    }else if(j >=15 && j < 30){
      Serial.println("Fingerprint registration mode");
      fingerprintRegistration();
    }else{
      Serial.println("Fingerprint deletion mode");
      fingerprintDeletion();
    }
  }
}

void fingerprintMatching(){
  uint8_t ret = fingerprint.search();         //returns zero if no  match found
  if(ret != 0){
    //Set fingerprint LED ring to always ON in green
    fingerprint.ctrlLED(fingerprint.eKeepsOn, fingerprint.eLEDGreen, 0);
    Serial.print("Successfully matched, ID=");
    Serial.println(ret);
    const char text[] = "Hello World";
    radio.write(&text, sizeof(text));
    delay(1000);
  }else{
    //Set fingerprint LED ring to always ON in red
    fingerprint.ctrlLED(fingerprint.eKeepsOn, fingerprint.eLEDRed, 0);
    Serial.println("Matching failed");
  }
  delay(1000);
  //Turn off fingerprint LED ring
  fingerprint.ctrlLED(fingerprint.eNormalClose, fingerprint.eLEDBlue, 0);
  Serial.println("-------------------------------------------------");
}

void fingerprintRegistration(){
  uint8_t ID, j;
  fingerprint.search();
  if((ID = fingerprint.getEmptyID()) == ERR_ID809){
    while(1){
      desc = fingerprint.getErrorDescription();
      Serial.println(desc);
      delay(1000);
    }
  }
  Serial.print("Unregistered ID, ID=");
  Serial.print(ID);
  j = 0;                                //clear sampling times
  while(j < COLLECT_NUMBER){
    fingerprint.ctrlLED(fingerprint.eBreathing, fingerprint.eLEDBlue, 0)
    Serial.print("The fingerprint sampling of the");
    Serial.print(j+1);
    Serial.println("(th) time is being taken");
    Serial.println("Please press down your finger");
    if((fingerprint.collectionFingerprint(10)) != ERR_ID809){
      fingerprint.ctrlLED(fingerprint.eFastBlink, fingerprint.eLEDYellow, 3);
      Serial.println("Capturing succeeds");
      j++;
    }else{
      Serial.println("Capturing fails");
    }
    Serial.println("Please release your finger");
    while(fingerprint.detectFinger());
  }

  //Save fingerprint information into an unregistered ID
  if(fingerprint.storeFingerprint(ID) != ERR_ID809){
    Serial.print("Saving succeed, ID=");
    Serial.println(ID);
    fingerprint.ctrlLED(fingerprint.eKeepsOn, fingerprint.eLEDGreen, 0);
    delay(1000);
    fingerprint.ctrlLED(fingerprint.eNormalClose, fingerprint.eLEDRed, 0);
  }else{
    Serial.println("Saving failed");
  }
  Serial.println("--------------------------------------------------------------");
}

void fingerprintDeletion(){
  uint8_t ret;
  ret = fingerprint.search();
  if(ret){
    fingerprint.ctrlLED(fingerprint.eKeepsOn, finger.eLEDGreen, 0);
    fingerprint.delFingerprint(ret);
    Serial.print("deleted fingerprint, ID=");
    Serial.println(ret);
  }else{
    fingerprint.ctrlLED(fingerprint.eKeepsOn, finger.eLEDRed, 0);
    Serial.println("Matching failed or the fingerprint hasn't been registered");
  }
  delay(1000);
  fingerprint.ctrlLED(fingerprint.eNormalClose, fingerprint.eLEDRed, 0);
  Serial.println("------------------------------------------------------------");
}
