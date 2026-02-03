#include "HmwKey.h"
#include "HmwDevice.h"

// Class HmwKey
HmwKey::HmwKey( PortPin _pin, Config* _config, HmwChannel* _feedbackChannel ) :
   unlocked( true ),
   pulldownSupported( true ),
   isStartUp( true ),
   keyPressNum( 0 ),
   config( _config ),
   feedbackChannel( _feedbackChannel ),
   digitalIn( _pin )
{
   type = HmwChannel::HMW_KEY;
   resetChannel();
}

uint8_t HmwKey::get( uint8_t* data )
{
   *data = ( isPressed() ? MAX_LEVEL : 0 );
   return 1;
}

void HmwKey::loop()
{
   if ( isUnlocked() )
   {
      if ( config->isPushButton() )
      {
         handlePushButtonSignal();
      }
      else if ( config->isSwitch() )
      {
         handleSwitchSignal();
      }
      else if ( config->isMotionSensor() || config->isMotionSensorReTrigger() )
      {
         handleMotionSensorSignal();
      }
      else if ( config->isDoorSensor() )
      {
         handleDoorSensorSignal();
      }
   }
}

void HmwKey::handleSwitchSignal()
{
   // sends a short KeyEvent, each time the input (e.g. wall switch) changes the polarity
   if ( !isPressed() )
   {
      if ( lastSentLong.isValid() )
      {
		  /* use DoorSensor input type for sending InfoMessage */
         //if ( config->repeatOnLongPress() ) {
            //uint8_t data[1];
            //HmwDevice::sendInfoMessage( channelId, get( data ), data );
         //}
         if ( HmwDevice::sendKeyEvent( channelId, keyPressNum, false ) == IStream::SUCCESS )
         {
            keyPressNum++;
            lastSentLong.reset();
         }
      }
      keyPressedTimestamp.reset();
	  setFeedbackChannel( KEY_FEEDBACK_OFF );
   }
   else
   {
      if ( !keyPressedTimestamp.isValid() )
      {
         // Taste war vorher nicht gedrueckt
         keyPressedTimestamp.setNow();
      }
      else if ( ( keyPressedTimestamp.since() >= DEBOUNCE_TIME ) && !lastSentLong.isValid() )
      {
		  /* use DoorSensor input type for sending InfoMessage */
         //if ( config->repeatOnLongPress() ) {
	         //uint8_t data[1];
	         //HmwDevice::sendInfoMessage( channelId, get( data ), data );
         //}
         if ( HmwDevice::sendKeyEvent( channelId, keyPressNum, false ) == IStream::SUCCESS )
         {
            keyPressNum++;
            lastSentLong.setNow();
         }
      }
	  setFeedbackChannel( KEY_FEEDBACK_ON );
   }
}

void HmwKey::handlePushButtonSignal()
{
   if ( !isPressed() )
   {
      // d.h. Taste nicht gedrueckt
      // "Taste war auch vorher nicht gedrueckt" kann ignoriert werden
      // Taste war vorher gedrueckt?
      if ( keyPressedTimestamp.isValid() )
      {
         // entprellen, nur senden, wenn laenger als Entprellzeit gedrueckt
         if ( ( keyPressedTimestamp.since() >= DEBOUNCE_TIME ) )
         {
            if ( !lastSentLong.isValid() )
            {
               // noch kein "long" gesendet, fuer kurzes druecken keyPressNum erhoehen
               keyPressNum++;
            //HmwDevice::sendKeyEvent( channelId, keyPressNum, lastSentLong.isValid() );
            }
            // auch beim loslassen nach einem langen Tastendruck ein weiteres Event senden
            HmwDevice::sendKeyEvent( channelId, keyPressNum, lastSentLong.isValid() );
         }
         keyPressedTimestamp.reset();
		 setFeedbackChannel( KEY_FEEDBACK_OFF );
      }
   }
   else
   {
      // Taste gedrueckt
      // Taste war vorher schon gedrueckt
      if ( keyPressedTimestamp.isValid() )
      {
         // muessen wir ein "long" senden?
         if ( lastSentLong.isValid() )
         {
            // schon ein LONG gesendet
            if ( ( lastSentLong.since() >= REPEAT_LONG_PRESS_TIME ) && config->repeatOnLongPress() )
            {
               // alle 300ms wiederholen
               // keyPressNum nicht erhoehen
               HmwDevice::sendKeyEvent( channelId, keyPressNum, true, true );                  // long press
               lastSentLong.setNow();
            }
         }
         else if ( keyPressedTimestamp.since() >= (unsigned long)config->getLongPressTime() * 100 )
         {
            // erstes LONG
            keyPressNum++;
            HmwDevice::sendKeyEvent( channelId, keyPressNum, true, true );                    // long press
            lastSentLong.setNow();
         }
      }
      else
      {
         // Taste war vorher nicht gedrueckt
         keyPressedTimestamp.setNow();
         lastSentLong.reset();
		 setFeedbackChannel( KEY_FEEDBACK_ON );
      }
   }
}

void HmwKey::handleMotionSensorSignal()	// TODO: Add brightness value to event message? (message id=0x41) - no HMW device will understand
{
   // ignore active motion sensor at startup/power on. Wait until it becomes inactive, or LongPressTime*10 is over.
   // Can be disabled by channel config "repeat_on_long_press" = no
   if ( isStartUp && config->repeatOnLongPress() && ( (SystemTime::now() < (unsigned long)config->getLongPressTime() * 1000) || isPressed() ) ) {
      return;
   }
   else {
      isStartUp = false;
   }
   
   /* special "MotionSensorReTrigger": use this input type for sensors that are set to re-trigger mode, which will keep their output active as long as they sense motion.
    * This config would repeat keyEvents every LongPressTime (4...50 seconds), but also block it for the same duration if the sensor would become inactive quicker. */
   if (config->isMotionSensorReTrigger() && lastSentLong.isValid() && (lastSentLong.since() >= (unsigned long)config->getLongPressTime() * 1000) )  // "MotionSensorReTrigger": repeat keyEvent after LongPressTime
   {
	   lastSentLong.reset();
   }

   
   if ( !isPressed() )
   {
      if ( lastSentLong.isValid() )
      {
         if ( !config->isMotionSensorReTrigger() )
		 {
			 lastSentLong.reset();
		 }
      }
      keyPressedTimestamp.reset();
	  setFeedbackChannel( KEY_FEEDBACK_OFF );
   }
   else
   {
      if ( !keyPressedTimestamp.isValid() )
      {
         // Taste war vorher nicht gedrueckt
         keyPressedTimestamp.setNow();
      }
      else if ( ( keyPressedTimestamp.since() >= DEBOUNCE_TIME ) && !lastSentLong.isValid() )
      {
         // if bus is not idle, retry next time
         if ( HmwDevice::sendKeyEvent( channelId, keyPressNum, false ) == IStream::SUCCESS )		// only send short KeyEvent for raising or falling edge - not both
         {
            keyPressNum++;   // increment only on success
            lastSentLong.setNow();
         }
         else
         {
            keyPressedTimestamp.setNow();   // delay next sendKeyEvent() by DEBOUNCE_TIME
         }
      }
	  setFeedbackChannel( KEY_FEEDBACK_ON );
   }
}

void HmwKey::handleDoorSensorSignal()
{
	// sends a short KeyEvent on HIGH and long KeyEvent on LOW input level changes
	// sends also notify/info message, if channel config "repeat_on_long_press" = yes (default)
	bool currentInputState = isPressed();
	
	if ( currentInputState != oldInputState )
	{
	   setFeedbackChannel( currentInputState ? KEY_FEEDBACK_ON : KEY_FEEDBACK_OFF);
  
		if ( !keyPressedTimestamp.isValid() )
		{
			keyPressedTimestamp.setNow();
		}
		else if ( keyPressedTimestamp.since() >= (unsigned long)config->getLongPressTime() * 100 )  // use long_press_time for debounce (400 ms default)
		{
			keyPressedTimestamp.setNow();
			
			if ( config->repeatOnLongPress() ) {
				uint8_t data[1];
				HmwDevice::sendInfoMessage( channelId, get( data ), data );
			}
			
			// if bus is not idle, retry next time
			if ( HmwDevice::sendKeyEvent( channelId, keyPressNum, !currentInputState ) == IStream::SUCCESS )
			{
				keyPressNum++;   // increment only on success
				oldInputState = currentInputState;
			}
		}
	}
	else
	{
		keyPressedTimestamp.reset();
	}
}

void HmwKey::resetChannel()
{
   keyPressedTimestamp.reset();
   lastSentLong.reset();
   setFeedbackChannel( isPressed() ? KEY_FEEDBACK_ON : KEY_FEEDBACK_OFF );
}

void HmwKey::checkConfig()
{
   if ( ( config->getLongPressTime() < 3 ) || ( config->getLongPressTime() > 50 ) )
   {
      config->setLongPressTime( Config::DEFAULT_LONG_PRESS_TIME );
   }
   if ( config->getInputType() >= Config::MAX_INPUTTYPE )
   {
      config->setInputType( Config::PUSHBUTTON );
   }
   
   // check if pulldown is supported
   if ( !pulldownSupported )
   {
	   config->setIsPullUp(true);
   }
   config->isPullUp() ? digitalIn.enablePullup() : digitalIn.enablePulldown();
   
   digitalIn.setInverted( config->isInverted() );

   resetChannel();
}