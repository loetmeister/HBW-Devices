/*
 * HmWStream.cpp
 *
 * Created: 18.11.2017 17:32:07
 * Author: viktor.pankraz
 */


#include "HmwStream.h"
#include "HmwDevice.h"

HmwStream::MessageQueue HmwStream::inMessageQueue;

HmwStream::MessageVector HmwStream::outMessageVector;

IStream::Status HmwStream::sendMessage( HmwMessageBase& msg )
{
	// do not send any messages if zeroCommunication is active
	// Building a queue is not useful, as this state might take long (e.g. other devices on the bus get firmware updates)
	// TODO: check if it's ok to return with LOCKED state here
	if ( HmwDevice::pendingActions.zeroCommunicationActive ) return IStream::LOCKED;
	
   // if ACK is pending wait additional MIN_IDLE_TIME before sending next message
   additionalMinIdleTime = ( rand() & 0xF ) + ( isAwaitingAck() ? MIN_IDLE_TIME : 0 );

   IStream::Status status = HmwStreamBase::sendMessage( msg );

   // make sure the message is back in LittleEndian format after sending
   msg.convertToLittleEndian();
   msg.notifySending();

   // for INFO messages that are not sent broadcast we expect an ACK
   // if ACK is not received, message should be sent again after approx. 100ms
   if ( msg.isInfo() && !msg.isBroadcast() )
   {
      if ( msg.getSendingTries() == 1 )
      {
         // it was the first try to send, put message into outVector to check for ACK later or to resend
         if ( !outMessageVector.isFull() )
         {
            HmwMessageBase* outMsg = msg.copy();
            if ( outMsg )
            {
               outMessageVector.add( outMsg );
            }
            else
            {
               LOG_ERROR( FSTR( "HmwStream::sendMessage() out of memory" ) );
            }
         }
         else
         {
            LOG_ERROR( FSTR( "HmwStream::sendMessage() outMessageVector.isFull()" ) );
         }
      }
   }
   return status;
}
