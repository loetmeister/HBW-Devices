#include "HmwKey.h"
#include "HmwDevice.h"

// Class HmwKey
HmwKey::HmwKey( PortPin _pin, Config* _config, HmwChannel* _feedbackChannel ) :
   unlocked( true ),
   pulldownSupported( true ),
   keyPressNum( 1 ),
   config( _config ),
   feedbackChannel( _feedbackChannel ),
   digitalIn( _pin )
{
   type = HmwChannel::HMW_KEY;
   resetChannel();
}

uint8_t HmwKey::get( uint8_t* data )
{
   *data++ = ( isPressed() ? MAX_LEVEL : 0 );
   *data++ = 0; // state flags not used   // TODO: why alway 0? fix?
   return 2;
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
      else if ( config->isMotionSensor() )
      {
         handleMotionSensorSignal();
      }
   }
}

void HmwKey::handleSwitchSignal()
{
   if ( !isPressed() )
   {
      if ( lastSentLong.isValid() )
      {
         uint8_t data[8];
         HmwDevice::sendInfoMessage( channelId, get( data ), data );
         if ( HmwDevice::sendKeyEvent( channelId, keyPressNum, false ) == IStream::SUCCESS )
         {
            keyPressNum++;
            lastSentLong.reset();
         }
      }
      keyPressedTimestamp.reset();
      if ( feedbackChannel && config->isFeedbackEnabled() )
      {
         uint8_t cmd = KEY_FEEDBACK_OFF;
         feedbackChannel->set( 1, &cmd );
      }
   }
   else
   {
      if ( !keyPressedTimestamp.isValid() )
      {
         // Taste war vorher nicht gedrueckt
         keyPressedTimestamp = Timestamp();
      }
      else if ( ( keyPressedTimestamp.since() >= DEBOUNCE_TIME ) && !lastSentLong.isValid() )
      {
         uint8_t data[8];
         HmwDevice::sendInfoMessage( channelId, get( data ), data );
         if ( HmwDevice::sendKeyEvent( channelId, keyPressNum, false ) == IStream::SUCCESS )
         {
            keyPressNum++;
            lastSentLong = Timestamp();
         }
      }
      if ( feedbackChannel && config->isFeedbackEnabled() )
      {
         uint8_t cmd = KEY_FEEDBACK_ON;
         feedbackChannel->set( 1, &cmd );
      }
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
            HmwDevice::sendKeyEvent( channelId, keyPressNum, lastSentLong.isValid() );
               keyPressNum++;
            }
            // auch beim loslassen nach einem langen Tastendruck ein weiteres Event senden
//            HmwDevice::sendKeyEvent( channelId, keyPressNum, lastSentLong.isValid() );
         }
         keyPressedTimestamp.reset();
         if ( feedbackChannel && config->isFeedbackEnabled() )
         {
            uint8_t cmd = KEY_FEEDBACK_OFF;
            feedbackChannel->set( 1, &cmd );
         }
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
               lastSentLong = Timestamp();
            }
         }
         else if ( keyPressedTimestamp.since() >= long(config->getLongPressTime() ) * 100 )
         {
            // erstes LONG
            HmwDevice::sendKeyEvent( channelId, keyPressNum, true, true );                    // long press
            keyPressNum++;
            lastSentLong = Timestamp();
         }
      }
      else
      {
         // Taste war vorher nicht gedrueckt
         keyPressedTimestamp = Timestamp();
         lastSentLong.reset();
         if ( feedbackChannel && config->isFeedbackEnabled() )
         {
            uint8_t cmd = KEY_FEEDBACK_ON;
            feedbackChannel->set( 1, &cmd );
         }
      }
   }
}

void HmwKey::handleMotionSensorSignal()	// TODO: Add brightness value to event message? (message id=0x41) - no HMW device will understand
{
   if ( !isPressed() )
   {
      if ( lastSentLong.isValid() )
      {
         lastSentLong.reset();
      }
      keyPressedTimestamp.reset();
      if ( feedbackChannel && config->isFeedbackEnabled() )
      {
         uint8_t cmd = KEY_FEEDBACK_OFF;
         feedbackChannel->set( 1, &cmd );
      }
   }
   else
   {
      if ( !keyPressedTimestamp.isValid() )
      {
         // Taste war vorher nicht gedrueckt
         keyPressedTimestamp = Timestamp();
      }
      else if ( ( keyPressedTimestamp.since() >= 100 ) && !lastSentLong.isValid() )
      {
         // if return value is 1, bus is not idle, retry next time
         if ( HmwDevice::sendKeyEvent( channelId, keyPressNum, false ) == IStream::SUCCESS )		// only send KeyEvent for raising or falling edge - not both
         {
            keyPressNum++;   // increment only on success
            lastSentLong = Timestamp();
         }
      }
      if ( feedbackChannel && config->isFeedbackEnabled() )
      {
         uint8_t cmd = KEY_FEEDBACK_ON;
         feedbackChannel->set( 1, &cmd );
      }
   }
}

void HmwKey::resetChannel()
{
   if ( pulldownSupported )
   {
      config->isPullUp() ? digitalIn.enablePullup() : digitalIn.enablePulldown();
   }
   else
   {
      digitalIn.enablePullup();
   }
   // backward compatibility to prevent endless key events
   if ( config->isPushButton() )
   {
      config->setInverted( !config->isPullUp() && pulldownSupported );
   }

   digitalIn.setInverted( config->isInverted() );

   keyPressedTimestamp.reset();
   lastSentLong.reset();
   keyPressNum = 1;
}

void HmwKey::checkConfig()
{
   if ( ( config->getLongPressTime() < 4 ) || ( config->getLongPressTime() > 50 ) )
   {
      config->setLongPressTime( Config::DEFAULT_LONG_PRESS_TIME );
   }
   if ( config->getInputType() >= Config::MAX_INPUTTYPE )
   {
      config->setInputType( Config::PUSHBUTTON );
   }
   resetChannel();
}