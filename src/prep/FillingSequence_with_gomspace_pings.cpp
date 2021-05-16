
#include <Arduino.h>
#include <i2c_t3.h>

// valves
uint8_t const p1 = 3;
uint8_t const p2 = 4;
uint8_t const p3 = 5;
uint8_t const p4 = 6;
uint8_t const p5 = 27; 
uint8_t const p6 = 28;

// DCDC enable
uint8_t const dcdc1 = 25;
uint8_t const dcdc2 = 24;

//assuming that it's normal [n n n n tt tt]; tt-n are the thrusters
uint8_t const valve_array[6] = {p1, p2, p3, p4, p5, p6};

/** @fn ping_gomspace
 *  Resets the GOMSpace's watchdog timer with a simple read of the Who
 *  Am I register. */
void ping_gomspace() {
  // Set the Who Am I read address
  Wire.beginTransmission(0x02);
  Wire.write(0x01);
  Wire.write(0x09);
  if (Wire.endTransmission(I2C_NOSTOP, 100000))
    Serial.println("GS Transmission Failed!");
  // Read from the address
  if (Wire.requestFrom(0x02, 3, I2C_STOP, 100000) < 3)
    Serial.println("GS Request Failed!");
  uint8_t buf[3];
  Wire.read(buf, 3);
}

void fire(int valve) {
  //loop through each valve in valve_subarray
  digitalWrite(13, HIGH);
  digitalWrite(valve, HIGH);
  delay(3); //there is 3ms of delay between each valve turning on
  
}

void stop_fire(int valve) {
  //loop through each valve in valve_subarray
  digitalWrite(valve, LOW);
  digitalWrite(13, LOW);
  delay(3); //there is 3ms of delay between each valve turning off
  }


void setup() {
  // put your setup code here, to run once:
    
    Serial.begin(9600);
    Wire.begin(I2C_MASTER, 0x00, I2C_PINS_18_19, I2C_PULLUP_EXT, 400000, I2C_OP_MODE_IMM);
  
    pinMode(13, OUTPUT); //13 is led
    digitalWrite(13, LOW); // LED off
    
    //valves
    pinMode(p1, OUTPUT);
    pinMode(p2, OUTPUT);
    pinMode(p3, OUTPUT);
    pinMode(p4, OUTPUT);
    pinMode(p5, OUTPUT);
    pinMode(p6, OUTPUT);
    
    // dcdc enable
    pinMode(dcdc1, OUTPUT);
    pinMode(dcdc2, OUTPUT);
    
    digitalWrite(dcdc1, HIGH); // DCDC 1 enabled
    digitalWrite(dcdc2, HIGH); // DCDC 2 enabled

    delay(2500);
}

void loop() {
  // put your main code here, to run repeatedly:

////////////////////////////Filling normal valve allocation array
  Serial.println("Phase 1: started filling normal valve allocation array \n ");
  for (int numCycle = 0; numCycle < 10; numCycle++){
    Serial.printf("starting filling cycle %d of 10 \n ", numCycle+1); 
    for(int fill = 0; fill < 7; fill++){
      Serial.printf("started fill %d  of 7 \n ", fill+1);
      
      //Filling normal valve allocation array
      delay(5000); //padding of 5000 ms before firing sequence    
      //turn on all thrusters
        for (int valve = 0; valve < 6; valve++){
          fire(valve_array[valve]);
          Serial.printf("fire valve %d of 6  \n", valve+1);
        }
      delay(1000); //the thrusters are each turned on for defined firing time
      
      //turn off all thrusters
        for (int valve = 0; valve < 6; valve++){
          stop_fire(valve_array[valve]);
          Serial.printf("stop fire valve %d of 6  \n", valve+1);
        }
        
       delay(5000); //padding of 5000 ms after firing sequence
       ping_gomspace(); // keep GomSpace on////////////////////////////////////////////////// 
       for (int countR = 0; countR < 5; countR++){
            Serial.printf("started cycle venting %d of 5  \n", countR+1);
            
            delay(5000); //padding of 5000 ms before firing sequence    
            //turn on all thrusters
              for (int valve = 0; valve < 4; valve++){
                fire(valve_array[valve]);
                Serial.printf("fire valve %d of 4  \n", valve+1);
              }
            delay(1000); //the thrusters are each turned on for defined firing time
        
            //turn off all thrusters
              for (int valve = 0; valve < 4; valve++){
                stop_fire(valve_array[valve]);
                Serial.printf("stop fire valve %d of 4  \n", valve+1);
              }
             delay(5000); //padding of 5000 ms after firing sequence

             Serial.printf("finished cycle venting %d of 5  \n", countR+1);
       }  
       ping_gomspace(); // keep GomSpace on//////////////////////////////////////////////////   
       Serial.printf("finished fill %d  of 7 \n ", fill+1);
      }
    
    // Filling test: Nozzle Valve Venting
    Serial.println("now venting tank 2 \n");
    
    for(int k = 0; k < 4; k++){ 
      Serial.printf("started nozzle valve venting %d of 4 \n", k+1);
            delay(5000); //padding of 5000 ms before firing sequence    
            //turn on all thrusters
              for (int valve = 0; valve < 4; valve++){
                fire(valve_array[valve]);
                Serial.printf("fire valve %d of 4  \n", valve+1);
              }
            delay(1000); //the thrusters are each turned on for defined firing time
        
            //turn off all thrusters
              for (int valve = 0; valve < 4; valve++){
                stop_fire(valve_array[valve]);
                Serial.printf("stop fire valve %d of 4  \n", valve+1);
              }
             delay(5000); //padding of 5000 ms after firing sequence
             ping_gomspace(); // keep GomSpace on/////////////////////////////////////////////////////////
      Serial.printf("finished nozzle valve venting %d of 4 \n", k+1);
    }
     Serial.printf("finished filling cycle %d of 10 \n ", numCycle+1);
  }

 ////////////////Critical venting to remove excess compressed air in tank 1
  Serial.println("Phase 2: started critical venting to remove excess compressed air in tank 1 \n ");
  for(int i= 0; i < 2; i++){
    Serial.printf("started critical venting cycle %d of 2 \n", i+1);
    for(int k = 0; k < 10; k++){
      Serial.printf("started T-2-T Venting cycle %d of 10 \n", k+1);
      ////////Venting T-2-T
      delay(5000); //padding of 5000 ms before firing sequence    
      //turn on only the tt nozzles
      for (int valve = 4;valve < 6; valve++){
          fire(valve_array[valve]);
          Serial.printf("fire valve %d of 6  \n", valve+1);
        }
      delay(400); //the thrusters are each turned on for defined firing time
  
      //turn off all thrusters
      for (int valve = 4; valve < 6; valve++){
          stop_fire(valve_array[valve]);
          Serial.printf("stop fire valve %d of 6  \n", valve+1);
        }
      delay(5000); //padding of 5000 ms after firing sequence
      ping_gomspace(); // keep GomSpace on//////////////////////////////////////////////////////////////////
      Serial.printf("finished T-2-T venting cycle %d of 10 \n", k+1);
    }
    /////////Venting T-2-N
    for(int k = 0; k < 10; k++){
      Serial.printf("started T-2-N venting cycle %d of 10 \n", k+1);
      delay(5000); //padding of 5000 ms before firing sequence    
      //turn on only the tt nozzles
      for (int valve = 0; valve < 4; valve++){
          fire(valve_array[valve]);
          Serial.printf("fire valve %d of 4  \n", valve+1);
        }
      delay(1000); //the thrusters are each turned on for defined firing time
  
      //turn off all thrusters
      for (int valve = 0;valve < 4; valve++){
          stop_fire(valve_array[valve]);
          Serial.printf("stop fire valve %d of 4  \n", valve+1);
        }
      delay(5000); //padding of 5000 ms after firing sequence
      ping_gomspace(); // keep GomSpace on//////////////////////////////////////////////////////////////////
      Serial.printf("finished T-2-N venting cycle %d of 10 \n", k+1);
    }
    Serial.printf("finished critical venting cycle %d of 2 \n", i+1);
    }
    
    ///////top up losses due to venting
    Serial.println("Phase 3: begin top up losses due to venting \n");
    for(int numCycle = 0; numCycle < 3; numCycle++){
      Serial.printf("started top-up filling cycle %d  of 3 \n", numCycle+1);
      
      for(int k = 0; k < 6; k++){
        Serial.printf("started fill %d  of 6 \n", k+1);
        
        delay(5000); //padding of 5000 ms before firing sequence    
 
        for (int valve = 0; valve < 6; valve++){
            fire(valve_array[valve]);
            Serial.printf("fire valve %d of 6  \n", valve+1);
          }
        delay(1000); //the thrusters are each turned on for defined firing time
    
        //turn off all thrusters
        for (int valve = 0; valve < 6; valve++){
            stop_fire(valve_array[valve]);
            Serial.printf("stop fire valve %d of 6  \n", valve+1);
          }
        ping_gomspace(); // keep GomSpace on//////////////////////////////////////////////////////////////////
        delay(5000); //padding of 5000 ms after firing sequence
        
        for(int countR = 0; countR < 5; countR++){
            Serial.printf("started countR %d of 5 \n ", countR+1);
            
            delay(5000); //padding of 5000 ms before firing sequence    
            //turn on only the tt nozzles
            for (int valve = 0; valve < 4; valve++){
                fire(valve_array[valve]);
                Serial.printf("fire valve %d of 6  \n", valve+1);
              }
            delay(1000); //the thrusters are each turned on for defined firing time
        
            //turn off all thrusters
            for (int valve = 0; valve < 4; valve++){
                stop_fire(valve_array[valve]);
                Serial.printf("stop fire valve %d of 6  \n", valve+1);
              }
            delay(5000); //padding of 5000 ms after firing sequence
            ping_gomspace(); // keep GomSpace on//////////////////////////////////////////////////////////////////
            Serial.printf("finished countR %d of 5   \n", countR+1);
        }
        Serial.printf("finished fill %d  of 6 \n", k+1);
      }

      ///Filling test: Nozzle valve venting
      for(int k = 0; k < 13; k++){
            Serial.printf("started nozzle valve venting cycle %d of 13 \n", k+1);
            delay(5000); //padding of 5000 ms before firing sequence    
            //turn on only the tt nozzles
            for (int valve = 0; valve < 4; valve++){
                fire(valve_array[valve]);
                Serial.printf("fire valve %d of 6  \n", valve+1);
              }
            delay(1000); //the thrusters are each turned on for defined firing time
        
            //turn off all thrusters
            for (int valve = 0; valve < 4; valve++){
                stop_fire(valve_array[valve]);
                Serial.printf("stop fire valve %d of 6  \n", valve+1);
              }
            delay(5000); //padding of 5000 ms after firing sequence
            ping_gomspace(); // keep GomSpace on//////////////////////////////////////////////////////////////////
            Serial.printf("finished nozzle valve venting cycle %d of 13 \n", k+1);
       }
      Serial.printf("finished top-up filling cycle %d  of 3 \n", numCycle+1);
    }

   Serial.println("al fin \n");

   while (true){
     ping_gomspace();
     delay(1000);
   } 
}
