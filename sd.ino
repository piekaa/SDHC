class SDCard
{

  int MISO,SCK,MOSI,CS;

  int clk = LOW;

  long frequency;
  long delayTime;

  byte test;
  unsigned long longTest;

  int blockSize;

  int responses[50];
  
    void toggleClock()
    {
      if( clk == LOW )
        clk = HIGH;
      else
        clk = LOW;

        digitalWrite( SCK , clk );
        
    }


    void clockTick()
    {
      delayAndToggle();
      delayAndToggle();
    }

    void delayAndToggle()
    {
      if( delayTime > 1 )
      delayMicroseconds( delayTime );
      toggleClock();
    }

    void lowClock()
    {
      clk = LOW;
      digitalWrite( SCK , clk );
    }


    void highClock()
    {
      clk = HIGH;
      digitalWrite( SCK, clk);
    }


    void setResponses()
    {
        for(int i = 0 ; i < 50 ; i++)
         responses[i] = 1;

        responses[8]  =  7;
        responses[58] =  3;
    }



    void sendFastBytes()
    {
      byte b;

  //    digitalWrite( SCK, LOW);
      
      
      for(int i = 0 ; i < 512 ; i ++)
      {
        b = data[i];
        for(int j = 0 ; j < 8 ; j ++)
        {


          digitalWrite( MOSI,  ( (b & test) == test) );

          digitalWrite( SCK, HIGH);
          digitalWrite( SCK, LOW);


          b <<= 1;
            
        }

        
      }
    }



    void sendByte(byte b)
    {
      
      


    //  Serial.println( b );

      for(int i = 0 ; i < 8 ; i ++) 
      {

                    if( (b & test) == test )
                    {
                      digitalWrite( MOSI, HIGH );
                    }
                    else
                    {
                      digitalWrite( MOSI, LOW );
                    }

                    clockTick();
                    b <<= 1;
                
        }

      
    }

    void sendLong(long l)
    {



      for(int i = 0 ; i < 32 ; i++)
      {
                if( (l & longTest) == longTest )
                    {
                      digitalWrite( MOSI, HIGH );

                    }
                    else
                    {
                      digitalWrite( MOSI, LOW );

                    }

                    clockTick();
                    l <<= 1;
       }

    
      
    }
    


    int handleResponse(int command)
    {
      int resp = 0 ;
      if( responses[ command ] == 7 )
      {
          for(int j = 0 ; j < 5 ; j++)
              {
                for(int i = 0 ; i < 8 ; i ++ )
                {


                    if( j == 0 )
                    {

                            resp <<= 1;
                      
                           if( digitalRead( MISO ) == HIGH )
                           {
                              resp += 1;
                           }
        
                           
                    }
              //    Serial.print( digitalRead( MISO) );
                  clockTick();
                }
            //    Serial.println();
              }
      }
      else
      if( responses[ command ] == 1 )
      {
          for(int i = 0 ; i < 8 ; i ++ )
                    {
                               resp <<= 1;
                               if( digitalRead( MISO ) == HIGH )
                               {
                                  resp += 1;
                               }
                       //     Serial.print( digitalRead( MISO) );
                            clockTick();
                    }
    
          
        }
        else
        {
          Serial.println("Unknown response");
          for(int i = 0 ; i < 8 * 20 ; i ++ )
          {
              clockTick();
          }
        }

        return resp;
        
    
    }

    public:



          byte data[512];
          
          void setFrequency(int f)
          {
            frequency = f;
            delayTime = 1000000 / f  / 2;
          }

          void powerUP()
          {

          delay(2);
            
            digitalWrite( CS, HIGH );
            digitalWrite( MOSI, HIGH );

             delay(2);
           for(int i = 0 ; i < 100 ; i ++)
                clockTick();

              
          }


          void setBlockSize(int blockSize)
          {
            this->blockSize = blockSize;
            sendCommand(16, blockSize, 1 );
            
          }


          bool prepareCard()
          {
            delay(100);
            
            powerUP();

            delay(100);
            reset();
            delay(100);
            if( init() == true )
            {
              Serial.println("Card Ready");
              setFrequency(1000000);

              delayTime = 1;
            }
            else
            {
              Serial.println("Card Error");
            }
            sendCommand(16, 512, 1 );
          }
          
          
//card = new SDCard(7,6,8,9);
          SDCard(int MISO, int SCK, int MOSI, int CS)
          {
            this->MISO = MISO;
            this->SCK = SCK;
            this->MOSI = MOSI;
            this->CS = CS;

         

            pinMode( MISO, INPUT);
            pinMode( MOSI, OUTPUT);
            pinMode( CS, OUTPUT);
            pinMode( SCK, OUTPUT);


            test = B10000000;
            longTest = 2147483648L;

 
            setResponses();

            
            setFrequency(200000);
            
            prepareCard();
          }



          void reset()
          {
            sendCommand(0,0,145);
          }

          bool init()
          {
            

              int resp;
              
              resp = sendCommand( 8 , 0x000001AA , 135);
         

              while( ( resp = sendCommand( 41 ,1073741824L , 135) ) ==  1 )
              {
                
              }

              if( resp == 0 )
                return true;


              while( ( resp = sendCommand( 41 , 0 , 135) ) == 1 )
              {
                
              }

              
              if( resp == 0 )
                return true;

              while( ( resp = sendCommand(1  ,0, 135) ) == 1 )
              {
                
              }


              if( resp == 0 )
                return true;
              return false;


            
          }
     
         bool isReady()
         {
          if( digitalRead(MISO ) == LOW  )
                {
                  clockTick();
                  return false;
                }
                return true;
         }


         void waitForCard()
         {
              while( isReady() == false );
         }


          int sendCommand( byte command ,unsigned long argument, byte crc )
          {

              int resp = 0 ;
              int cmd = command;

              digitalWrite( CS, HIGH );
              clockTick();

     
              digitalWrite( CS, LOW );
              clockTick();


              if( digitalRead( MISO ) == LOW )
              {
                Serial.println("Card error: MISO low" );
                return -1;
              }


              int counter = 0;

              // Sending Command
              sendByte( (B01000000 | command ) );

              // Sending Argument
              sendLong( argument );

              // Sending CRC 
              sendByte(crc);

               counter = 0;
            
        //      Serial.println("Czekam na odpowiedz...");
              while( digitalRead( MISO ) == HIGH )
              {
                  clockTick();
  
                  counter ++;

                  if( counter > 10000 )
                  {
                    Serial.println("Card Error Timeout");
                    return -1;
                  }
                  
                
              }
          
              
        //      Serial.println("Jest odpowiedz, czas!");
       //       Serial.println(counter);


              resp = handleResponse( cmd );
            
                Serial.print("Command ");
                Serial.print( cmd );
                Serial.print(" Response ");
                Serial.print( resp );
                Serial.print(" (");
                int tmp = resp;
                for(int i = 0 ; i < 8 ; i++)
                {
                  if( (tmp & test) == test  )
                    Serial.print( 1 );
                  else
                    Serial.print( 0 );
                    tmp<<=1;
                }
                Serial.print(")");
                Serial.println();
              return resp;

              
          }






          
          bool saveData( unsigned long address )
          {
    //        int t = millis();



             

            
            sendCommand(24, address, 1 );


            
      
            for(int i = 0 ; i < 8 ; i++ )
            {
              clockTick();
            }
      

            
      
            byte dataToken = B11111110;
      
            
            sendByte(dataToken);


   //         for(int i = 0 ; i < 512  ; i ++ )
    //            sendByte( data[i] );
            sendFastBytes();  
      
            /// crc 
            sendByte(1);
            sendByte(1);


            digitalWrite( MOSI, HIGH);

      
      
            for(int i = 0 ; i < 3 ; i ++ )
            {
              clockTick();
            }

            if( digitalRead( MISO ) == HIGH )
            {
              Serial.println("Save error");
            }
            clockTick();

            int resp = 0;
            for(int i = 0 ; i < 3 ; i++)
            {
                resp <<= 1;
                if( digitalRead(MISO) == HIGH )
                  resp++;
                clockTick();  
            }

            clockTick();

         
           // Serial.print("Save time: ");
     //       Serial.println(millis()-t);

            if( resp == 2 )
            {
                Serial.println("Data accepted");      
                return true;
            }
            else
            {
              Serial.println("Save error");
              Serial.println( resp );
              return false;
            }
   
            
          }


          void printMISO()
          {
            if( digitalRead(MISO) == HIGH )
                Serial.print( 1 );
            else
              Serial.print(0);
          }


          void readData(unsigned long address)
          {
            sendCommand(17,address,1);


            for(int i = 0 ; i < 300 ; i++)
            {
              for(int j = 0 ; j < 8 ; j++)
              {

                printMISO();
                digitalWrite( SCK, HIGH);
                digitalWrite(SCK, LOW);
                
               // clockTick();
              }
              Serial.println();
              delayMicroseconds(100);
            }



            for(int i = 0 ;i < 512 * 8 ; i++)
            {
              clockTick();
            }
          
            
          }



          

  
}*card;





void setup() {
  // put your setup code here, to run once:
 Serial.begin(9600);
// SDCard(int MISO, int SCK, int MOSI, int CS)
card = new SDCard(12,13,11,4);

delay(10);

//card->prepareCard();

for(int i = 0 ; i < 512 ; i++)
card->data[i] = i;


  
 card->saveData( 0 );
 card->waitForCard();
// card->saveData( 1 );
card->waitForCard();
 card->readData(0);


 
}

void loop() {
  // put your main code here, to run repeatedly:

 

}
