// <-- In Arduino, the two slashes indicated comments. Anything after these on a line will be ignored when compiling the code.
#include <SparkFun_RHT03.h> //This is an additional .zip library that we imported for the temp/humidity sensor

//Digital Inputs
const int hallSensors[] = {34, 35, 36, 31, 32, 33, 28, 29, 30, 25, 26, 27, 22, 23, 24}; //Each integer represents a digital input on the arduino mega that we used.
int ultraTrig = 40;
int ultraEcho = 41;
int temp = 45;
RHT03 rht;


//Analog Input;
int photo = A0;

//Digital Outputs
int vca = 2; //5V (pin 2 on mega)
int vcf = 5; //5V (pin 5 on mega)
int frq = 4; //5V (pin 4 on mega)
int lfo = 6; //2.5V (pin 6 on mega)

// Below are various variables created and used in the code.
int waitTime = 70; //Delay time at the end of the loop() function

int marker = 255; //Marker used for testing with max. Disregard
int value; //unused

float note = (60/12.0); //Scale factor for an octave

int thresh = 5; //threshold value used in Play() function
int select = 0; //index marker for each of the array melodies below
/*
 * Arrays are melody snippets.
 * Ready variables equal 1 if hallsensor is activated
 * Play variables activated if the element should play a note of its melody when the play function is called
 */
int earth[] = {2, 5, 9, 14, 9, 5}; 
int earthReady = 0; 
int earthPlay = 1;
int wind[] = {7, 11, 7, 5, 0, 1};
int windReady = 0;
int windPlay = 0;
int water[] = {2, 9, 14, 12, 11, 12};
int waterReady = 0;
int waterPlay = 0;
int fire[] = {14, 9, 11, 7, 5, 4}; 
int fireReady = 0;
int firePlay = 0;

/*
 * One is the bottom hall sensor on each pole.
 * Two is the middle.
 * Com is the result of 2*Two + One
 */
 //Left pole
int earthOne = 0;
int earthTwo = 0;
int earthCom = 0;

//Middle left
int windOne = 0;
int windTwo = 0;
int windCom = 0;

// Middle pole
int soloOne = 0;
int soloTwo = 0;
int soloThree = 0;
int soloSwap = 0;

//Middle right
int waterOne = 0;
int waterTwo = 0;
int waterCom = 0;

//Right
int fireOne = 0;
int fireTwo = 0;
int fireCom = 0;

//Environment variables
int light = 0;
float latestHum;
float latestTempC;
float weather;

//Ultrasonic Sensor variables
long duration = 0;
long distance = 0;

//The number of elementReady variables that equal 1
int numReady = 0;

//other values
int lfoVal = 0;
int vcaVal = 0;
int frqShiftVal = 0;
int vcfVal = 0;
int panVal = 0;

//The void function declaration means that the function will not return anything. It is used to set values and perform a series of tasks. 
void setup() {
  // Initializing all pin inputs, pin outputs, and starting the serial bus.
  pinMode(frq, OUTPUT);
  pinMode(lfo, OUTPUT);
  pinMode(vca, OUTPUT);
  pinMode(vcf, OUTPUT);

  for (int i=0; i<15; i++)
  {
      pinMode(hallSensors[i], INPUT);
  }
  
  pinMode(ultraTrig, OUTPUT);
  pinMode(ultraEcho, INPUT);
  
  pinMode(photo, INPUT);
  rht.begin(temp);
  
  Serial.begin(115200);
}

void loop() {
    ReadInputs(); 

    Serial.println("SoloOne");
    Serial.println(soloOne);
    //Digital effect markers - 255: water, 254: fire, 253: panning, 252: reverb, 251: noise
    Serial.println(252); //Marker
    Serial.println(light); //Photosensor
    delay(30);
    Serial.println(253); //Marker
    Serial.println(calPanAmt(earthOne, earthTwo, fireOne, fireTwo));
    delay(30);

    weather = (((2*(latestTempC-15))+50)+latestHum)/2;
    Serial.println(251);
    Serial.println(weather);
    delay(30);
    
    if (soloOne == 0) //If the top bead on the middle pole is in place, do this.
    {
        //Digital effect markers - 255: water, 254: fire, 253: panning
        //Write to output pin. Which pin is determined by the first argument. How much by the second.
        analogWrite(vca, CheckCol(earthReady, earthOne, earthTwo, earthCom, true, 5)); //From Earth
        analogWrite(lfo, CheckCol(windReady, windOne, windTwo, windCom, true, 2.5)); //From Wind

        Serial.println(255); //Sending information to Max
        Serial.println(CheckCol(waterReady, waterOne, waterTwo, waterCom, false, 0)); //From Water (digital effect)
        delay(30); //Delay to allow max to separate data.

        Serial.println(254);
        Serial.println(CheckCol(fireReady, fireOne, fireTwo, fireCom, false, 0)); //From fire (digital effect)
        delay(30);

        analogWrite(vcf, int(distance*6.375));
        Play();
        delay(waitTime);
    }
    
    else //Otherwise, do this
    {
        analogWrite(vca, CheckCol(1, earthOne, earthTwo, earthCom, true, 5)); //From Earth
        analogWrite(lfo, CheckCol(1, windOne, windTwo, windCom, true, 2.5)); //From Wind

        Serial.println(255);
        Serial.println(CheckCol(1, waterOne, waterTwo, waterCom, false, 0)); //From Water (digital effect)
        delay(30);

        Serial.println(254);
        Serial.println(CheckCol(1, fireOne, fireTwo, fireCom, false, 0)); //From fire (digital effect)
        delay(30);

        numReady = CountReady(earthReady, windReady, waterReady, fireReady);

        analogWrite(vcf, CheckCol(1, waterOne, waterTwo, waterCom, true, 5));
        if ((soloTwo == 1) & (soloThree == 1))
        {
            // Tremolos
            analogWrite(frq, numReady*note+distance*3.125);
            delay(100);
            analogWrite(frq, numReady*note+2*distance*3.125);
            delay(100);
        }
        else if (soloTwo == 1)
        {
          
            analogWrite(frq, numReady*note+distance*3.125);
            delay(150);
            analogWrite(frq, numReady*note+2*distance*3.125);
            delay(150);
        }
        else if (soloThree == 1)
        {

            analogWrite(frq, numReady*note+distance*3.125);
            delay(200);
            analogWrite(frq, numReady*note+2*distance*3.125);
            delay(200);
        }
        else
        {
            analogWrite(frq, numReady*note+distance*3.125);
        }
    }
}

void ReadInputs()
{
    // The '!' means not. If the sensor reads 1, a 0 is stored. If the sensor reads 0, a 1 is stored.
    earthReady = !digitalRead(hallSensors[0]);
    earthTwo = !digitalRead(hallSensors[1]);
    earthOne = !digitalRead(hallSensors[2]);
    windReady = !digitalRead(hallSensors[3]);
    windTwo = !digitalRead(hallSensors[4]);
    windOne = !digitalRead(hallSensors[5]);
    soloOne = !digitalRead(hallSensors[6]);
    soloTwo = !digitalRead(hallSensors[7]);
    soloThree = !digitalRead(hallSensors[8]);
    waterReady = !digitalRead(hallSensors[9]);
    waterTwo = !digitalRead(hallSensors[10]);
    waterOne = !digitalRead(hallSensors[11]);
    fireReady = !digitalRead(hallSensors[12]);
    fireTwo = !digitalRead(hallSensors[13]);
    fireOne = !digitalRead(hallSensors[14]);

    //Ultrasonic sensor read
    digitalWrite(ultraTrig, LOW);
    delayMicroseconds(2);
    digitalWrite(ultraTrig, HIGH);
    delayMicroseconds(10);
    digitalWrite(ultraTrig, LOW);

    duration = pulseIn(ultraEcho, HIGH);
    if ((duration/2)/29.1 <= 40)
    {
        distance = (duration/2)/29.1;
    }

    light = analogRead(photo); //read photo sensor. Can be any value between 0 and 1023 inclusive.

    int updateRet = rht.update(); //temp and humidity
    if (updateRet == 1)
    {
        latestHum = rht.humidity();
        latestTempC = rht.tempC();
    }
}

//The int function declaration means the function will return an integer after performing its tasks.
int CheckCol(int go, int value1, int value2, int com, bool analog, float highVolt)
{
    if (go == 1)
    {
        if (value1 == 1)
        {
            com += 1; //This is equivalent to com = com + 1. Note the = sign does not mean "same". It is the assignment operator. "==" means same.
        }
        if (value2 == 1)
        {
            com += 2;
        }

        if (analog)
        {
            return int((50*highVolt/3)*com);
        }
        else
        {
            return com;
        }  
    }
    
    else
    {
        return 0;
    }
}

int calPanAmt(int valueL1, int valueL2, int valueR1, int valueR2)
{
    panVal = (2*valueR2 + valueR1) - (2*valueL2 + valueL1);

    return panVal;
}

int CountReady(int earth, int wind, int water, int fire)
{
    numReady = 0;
    if (earth == 1)
    {
        numReady += 1;
    }
    if (wind == 1)
    {
        numReady += 1;
    }
    if (water == 1)
    {
        numReady += 1;
    }
    if (fire == 1)
    {
        numReady += 1;
    }
    return numReady;
}

void Play()
{   
    if (earthReady == 1)
    {
        if (earthPlay == 1)
        {
            waterPlay = 0;
            windPlay = 0;
            firePlay = 0;
            analogWrite(frq, earth[select]*note);
            select += 1;
            if (select > thresh)
            {
                select = 0;
                earthPlay = 0;
                if (windReady == 1)
                {
                    windPlay = 1;
                }
                else if (waterReady == 1)
                {
                    waterPlay = 1;
                }
                else if (fireReady == 1)
                {
                    firePlay = 1;
                }
                else if (earthReady == 1)
                {
                    earthPlay = 1;
                }
            }
        }
    }
    else
    {
        firePlay = 1;
    }

    if (windReady == 1)
    {
        if (windPlay == 1)
        {
            waterPlay = 0;
            earthPlay = 0;
            firePlay = 0;
            analogWrite(frq, wind[select]*note);
            select += 1;
            if (select > thresh)
            {
                select = 0;
                windPlay = 0;

                if (waterReady == 1)
                {
                    waterPlay = 1;
                }
                else if (fireReady == 1)
                {
                    firePlay = 1;
                }
                else if (earthReady == 1)
                {
                    earthPlay = 1;
                }
                else if (windReady == 1)
                {
                    windPlay = 1;
                }
            }
        }
    }
    else
    {
        earthPlay = 1;
    }

    if (waterReady == 1)
    {
        if (waterPlay == 1)
        {
            earthPlay = 0;
            windPlay = 0;
            firePlay = 0;
            analogWrite(frq, water[select]*note);
            select += 1;
            if (select > thresh)
            {
                select = 0;
                waterPlay = 0;
                if (fireReady == 1)
                {
                    firePlay = 1;
                }
                else if (earthReady == 1)
                {
                    earthPlay = 1;
                }
                else if (windReady == 1)
                {
                    windPlay = 1;
                }
                else if (waterReady == 1)
                {
                    waterPlay = 1;
                }
            }
        } 
    }
    else
    {
        windPlay = 1;
    }

   if (fireReady == 1)
    {
        if (firePlay == 1)
        {
            waterPlay = 0;
            windPlay = 0;
            earthPlay = 0;
            analogWrite(frq, fire[select]*note);
            select += 1;
            if (select > thresh)
            {
                select = 0;
                firePlay = 0;
                if (earthReady == 1)
                {
                    earthPlay = 1;
                }
                else if (windReady == 1)
                {
                    windPlay = 1;
                }
                else if (waterReady == 1)
                {
                    waterPlay = 1;
                }
                else if (fireReady == 1)
                {
                    firePlay = 1;
                }
            }
        } 
    }
    else
    {
        firePlay = 1;
    }

    if ((earthReady == 0) && (windReady == 0) && (waterReady == 0) && (fireReady == 0))
    {
        earthPlay = 0;
        windPlay = 0;
        waterPlay = 0;
        firePlay = 0;
        analogWrite(frq, 0);
    }
    
    if ((earthPlay == 0) && (windPlay == 0) && (waterPlay == 0) && (firePlay == 0))
    {
        earthPlay = 1;
        windPlay = 1;
        waterPlay = 1;
        firePlay = 1;
    }
}

// Arpeggiate is an old function. Not used anymore. Some variables in the function no longer exist.

//void Arpeggiate(int quality, int seventh, int seventhQ)
//{   
//    //set note array length
//    if (seventh == 1) { thresh = 4; }
//    else { thresh = 3; }
//
//    // major or minor
//    if (quality == 0){
//        //7th?
//        if (seventh == 0){ analogWrite(frq, minorChord[select]*note); }
//        else { 
//            if (seventhQ = 0) {analogWrite(frq, minor7[select]*note); }
//            else { analogWrite(frq, minorMaj[select]*note); }
//        }
//    }
//    else{
//        //7th?
//        if (seventh == 0) { analogWrite(frq, majorChord[select]*note); }
//        else { 
//            if (seventhQ = 0)  {analogWrite(frq, dom7[select]*note); }
//            else { analogWrite(frq, major7[select]*note); }
//        }
//    }
//
//    if (select == thresh){
//        direct = 0;
//    }
//    else if (select == 0){
//        direct = 1;
//    }
//    
//    if (direct == 1){
//        select += 1;
//    }
//    
//    else{
//        select -= 1;
//    }
//}
