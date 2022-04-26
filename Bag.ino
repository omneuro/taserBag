#include <SPI.h>
#include <nRF24L01.h>
#include <RF24.h>


RF24 radio(7,8);//creating an RF24 object with arguments being CE, CSN respectively

const byte address[6] = "00001";    //setting address pipe for recieving information

void setup() {
Serial.begin(9600);                 //initializing the serial monitor with baud rate 9600
radio.begin();                      //initializing radio object
radio.openReadingPipe(0, address);  //setting the address of the transmitter from which we will be recieving data
radio.setPALevel(RF24_PA_MIN);      //setting the pwer amplifier level to minimum because the modules will be tested at a close range (note: will set to max after the near range test phase)
radio.startListening();             //ses the modules as a reciever
}

void loop() {
  if(radio.available()){            //Checking if there is data in the pipe to be recieved
    Serial.println("The radio is available");
    char text[32] = "";             //Creaing an array of 32 elements called text to save incoming data.
    radio.read(&text, sizeof(text));//Reading and storing the incoming data in a variable called text
    Serial.println(text);           //Printing the incoming data to the screen
  }
}
