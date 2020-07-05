//==========================================================================
// A library for controlling a Microchip RN2903 LoRa radio.
//
// Author - David Souza - SmartMosaic - Brasil
// version 1.2 - out/19
//
// Based on JP Meijers and Nicolas Schteinschraber
//
//==========================================================================

#ifndef rn2903_h
#define	rn2903_h

// Define use the SingleChannel
// #define SINGLECHANNEL

#include "Arduino.h"

enum TX_RETURN_TYPE {
  TX_FAIL = 0,    		// The transmission failed.
						// If you sent a confirmed message and it is not acked,
						// this will be the returned value.

  TX_FAIL_TIMES = 1,    // The transmission failed per limit times.
						
  TX_FAIL_LEN = 2,		// The transmission failed per invalid data len.

  TX_FAIL_PARAM = 3,	// The transmission failed per invalid parameters.
				  
  TX_SUCCESS = 4, 		// The transmission was successful.
						// Also the case when a confirmed message was acked.

  TX_WITH_RX = 5  		// A downlink message was received after the transmission.
						// This also implies that a confirmed message is acked.
 
};

class rn2903
{
  public:

    // =================================================================================================
    // A simplified constructor taking only a Stream ({Software/Hardware}Serial) object.
    // The serial port should already be initialised when initialising this library.
    // =================================================================================================
    rn2903(Stream& serial, byte resetPin=0);

    // =================================================================================================
    // Transmit the correct sequence to the rn2903 to trigger its autobauding feature.
    // After this operation the rn2903 should communicate at the same baud rate than us.
    // =================================================================================================
    bool autobaud(void);

    // =================================================================================================
    // Setup parameter for module operation
	// sb = subband of 8 channels
	// dr = data rate
	// ch = single channel (0 - 63). 255 = subband
	// adr = adaptative data rate
	// retx = number for retransmissions tx
	// debug = debug serial port
    // =================================================================================================
    void setParams(byte sb=0, byte dr=2, byte ch=255, bool adr=false, byte retx=4, byte debug=0, bool ar=false, byte pw=6);

    // =================================================================================================
	// Config al parameters in the module 
    // =================================================================================================
    bool configParams(void);
	
    // =================================================================================================
    // Setup parameter for JOIN
	// AppEUI = AppEUI for OTAA or NwkSKey for ABP
	// AppEUI = AppKey for OTAA or AppSKey for ABP
	// DevEUI = DevEUI for OTAA or AppEUI  for ABP
	// otaa = True for OTAA or False for ABP
    // =================================================================================================
    void setJoin(String AppEUI, String v, String DevEUI="", bool otaa=true);

    // =================================================================================================
    // Initialise the rn2903 and join the LoRa network (if applicable).
    // This function can only be called after calling setJOIN()
    // The sole purpose of this function is to re-initialise the radio if it
    // is in an unknown state.
    // =================================================================================================
    void init(void);

    // =================================================================================================
    // Execute a Join (OTAA or ABP).
    // =================================================================================================
    bool join(void);

    // =================================================================================================
    // Get the rn2903 hardware and firmware version number. This is also used
    // to detect if the module is an RN2903.
    // =================================================================================================
    String sysver(void);

    // =================================================================================================
	// Return Network Conection Type: OTAA ou ABP
    // =================================================================================================
    String netType(void);	

    // =================================================================================================
    // Get the hardware EUI of the radio, so that we can register it on The Things Network
    // and obtain the correct AppKey.
    // You have to have a working serial connection to the radio before calling this function.
    // In other words you have to at least call autobaud() some time before this function.
    // =================================================================================================
    String hweui(void);

    // =================================================================================================
    // Returns the AppSKey or AppKey used when initializing the radio.
    // In the case of ABP this function will return the AppSKey.
    // In the case of OTAA this function will return the AppKey.
    // =================================================================================================
    String appkey(void);

    // =================================================================================================
    // Returns the AppSKey or AppKey used when initializing the radio.
    // In the case of ABP this function will return the NwkSKey.
    // In the case of OTAA this function will return the AppEUI.
    // =================================================================================================
    String appeui(void);

    // =================================================================================================
    // Returns the AppSKey or AppKey used when initializing the radio.
    // In the case of ABP this function will return the DevADDR.
    // In the case of OTAA this function will return the DevEUI.
    // =================================================================================================
    String deveui(void);

    // =================================================================================================
    // Put the rn2903 to sleep for a specified timeframe.
    // The rn2903 accepts values from 100 to 4294967296.
    // Rumour has it that you need to do a autobaud() after the module wakes up again.
    // =================================================================================================
    String sleep(long msec);

    // =================================================================================================
    // Reset the RN2903 for factory parameters
    // =================================================================================================
	void factoryReset(void);

    // =================================================================================================
    // Reset the rn2903 for MAC level 
    // =================================================================================================
    String macReset(void);

    // =================================================================================================
    // Reset the rn2903 by reset PIN 
    // =================================================================================================
    void pinReset(void);

   // =================================================================================================
    // Returns the last downlink message HEX string.
    // =================================================================================================
    String getRx(void);

    // =================================================================================================
    // Get the RN2903's RSSI value from the last received frame. Helpful to debug link quality.
    // =================================================================================================
    signed int getRSSI(void);
	
    // =================================================================================================
    // Get the RN2903's SNR of the last received packet. Helpful to debug link quality.
    // =================================================================================================
    signed int getSNR(void);
	
    // =================================================================================================
    // Get the VDD in the RN2903's (x1000)
    // =================================================================================================
	int getVDD(void);


    // =================================================================================================
    // Transmit the provided data. The data is hex-encoded by this library,
    // so plain text can be provided.
    // cfn=false use txUncnf() and cfn=true use txCfn()
    // Parameter is an ascii text string.
    // =================================================================================================
    TX_RETURN_TYPE tx(String Data, bool cfn=false);

    // =================================================================================================
    // Transmit raw byte encoded data via LoRa WAN.
    // This method expects a raw byte array as first parameter.
    // The second parameter is the count of the bytes to send.
	// cfn is used for confirm or noconfirm tx
    // =================================================================================================
    TX_RETURN_TYPE txBytes(const byte* data, uint8_t size, bool cfn=false);

    // =================================================================================================
    // Do a confirmed transmission via LoRa WAN.
    //
    // Parameter is an ascii text string.
    // =================================================================================================
    TX_RETURN_TYPE txCnf(String data);

    // =================================================================================================
    // Do an unconfirmed transmission via LoRa WAN.
    //
    // Parameter is an ascii text string.
    // =================================================================================================
    TX_RETURN_TYPE txUncnf(String data);

    // =================================================================================================
    // Transmit the provided data using the provided command.
    //
    // String - the tx command to send
    //           can only be one of "mac tx cnf 1 " or "mac tx uncnf 1 "
    // String - an ascii text string if bool is true. A HEX string if bool is false.
    // bool - should the data string be hex encoded or not
    // =================================================================================================
    TX_RETURN_TYPE txCommand(String command, String data, bool shouldEncode);

    // =================================================================================================
    // Send a raw command to the rn2903 module.
    // Returns the raw string as received back from the rn2903.
    // If the rn2903 replies with multiple line, only the first line will be returned.
    // =================================================================================================
    String sendRawCommand(String command);

    // =================================================================================================
    void sendEncoded(String);

    // =================================================================================================
 	String getRxMessenge(void);

    // =================================================================================================
	void rn2903::debug(String txt1, String txt2="");
	
  private:
  
	// Poiters to serial ports
    Stream& _serial;
	
	// setup the module
	byte _sb = 0;				// Sub-band
	byte _dr = 1;				// Data rate
	byte _ch = 0;				// Number of single channel
	byte _pw = 6;				// Power TX
	byte _ar = false;			// Automatic Reply
	byte _retx = 4;				// Number of retransmissions
	byte _port = 100;			// POrt number for TX
	byte _debug = true;		    // Debug is ON or OFF
	byte _resetPin;				// Pino de reset do RN2903
	bool _adr = false;			// Adaptative data rate


    //Flags to switch code paths. Default is to use OTAA.
    bool _otaa = true;
	
	// Parameters to OTAA / ABP
    String _deveui = "";		//OTAA = devEUI, ABP = devADDR
    String _appeui = "";		//OTAA = appeui, ABP = nwkSkey
    String _appkey = "";		//OTAA = appKey, ABP = appSKey
 
    // The downlink messenge
    String _rxMessenge = "";
	

};

#endif
