# Qt4_ModemModule
Class for sending sms from mobile 2G/3G modem.


Module for sending SMS from local GSM/3G modem

[SMS SENDING]
 For send SMS you need call function [sendSMSList] with pointers to sms objects
 or standalone sms function [sendSMS] with pointer to sms and bool value [isCyrillic]

 functions marked sms param [isSended] if sending complete.

 after all sms sends class emit signal [operationSendSuccesfull]
 and error if modem not valid

 [SMS OBJECT INITIALIZE]
 For create SMS object call constructor with params:
 message - text message
 cellphone - number to sending
