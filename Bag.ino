#include <SPI.h>
#include <nRF24L01.h>
#include <RF24.h>


RF24 radio(7,8);//creating an RF24 object with arguments being CE, CSN respectively

const byte addresses[][6] = {"00001","00002"};    //setting address pipe for recieving information
int relay = 8;                                  //relay on pin D8
volatile byte relayState = LOW;

void setup() {
  Serial.begin(115200);                 //initializing the serial monitor with baud rate 9600
  radio.begin();                        //initializing radio object
  radio.openWritingPipe(addresses[0]); //00001
  radio.openReadingPipe(1, addresses[1]);  //00002
  radio.setPALevel(RF24_PA_MIN);      //setting the pwer amplifier level to minimum because the modules will be tested at a close range (note: will set to max after the near range test phase)

  pinMode(relay, OUTPUT);
}

void loop() {
  radio.startListening();           //sets the module as reciever
  if(radio.available()){            //Checking if there is data in the pipe to be recieved
    Serial.println("The radio is available");
    radio.read(&relayState, sizeof(relayState))
    if(relayState == HIGH){
      digitalWrite(relay,HIGH);
      delay(60000);
      digitalWrite(relay,LOW);
    }
    else{
      digitalWrite(relay,LOW);
    }
  }
  radio.stopListening();
  //Check the level of the battery here, trigger the buzzer if critical and send data to the remote to light the Red led.

  //if charging. Turn off buzzer and red light. Once full send data to the remote lighting the Green LED.

  //if full and charger is removed, Send data to the remote to turn off the green light.

}
