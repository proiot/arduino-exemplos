//==========================================================================
// A library for controlling a Microchip RN2903 LoRa radio.
//
// Author - David Souza - SmartMosaic - Brasil
// version 1.2 - out/19
//
// Based on JP Meijers and Nicolas Schteinschraber
//
//==========================================================================

#include "Arduino.h"
#include "rn2903.h"

extern "C" {
#include <string.h>
#include <stdlib.h>
}

//==========================================================================
rn2903::rn2903(Stream& serial, byte resetPin):
_serial(serial)
{
  _serial.setTimeout(2000);
  _resetPin = resetPin;
}


//==========================================================================
bool rn2903::autobaud(void)  
{
  String response;
  //clear serial buffer
  while(_serial.available())
    _serial.read();  
  
  // Try a maximum of 5 times with a 500 ms delay
  for (uint8_t i=0; i<5 && response.length()==0; i++)
  {
    delay(500);
	// break condition and de autobaud char (0x55)
    _serial.write((byte)0x00);
    _serial.write((byte)0x55);
    _serial.println();

	response = sysver();

  }
  
  // Check ccomunication with the module
  if (response.length() > 0){
  	return true;
  }
  else{
	return false;
  }

}

//==========================================================================
void rn2903::setParams(byte sb, byte dr, byte ch, bool adr, byte retx, byte debug, bool ar, byte pw)
{
	// Check parameters
	if (sb>7) sb = 0;
	if (dr>4) dr = 2;
	if (ch>63 && ch!=255) ch = 255;
	
	// save variables
	_sb = sb;
	_dr = dr;
	_ch = ch;
	_pw = pw;
	_ar = ar;
	_adr = adr;
	_retx = retx;
	_debug = debug;
}

//==========================================================================
bool rn2903::configParams(void)
{
	bool chOn;
    String receivedData;
	String command;
    _serial.setTimeout(5000);

	for(byte channel = 0; channel < 72; channel++)
	{
		chOn = false;
		if(_ch == 255)  // Sub Band
		{
			if(channel >= (_sb*8) && channel < ((_sb*8)+8))
			{
				chOn = true;
			}
		}
		else			// Single Channel
		{
			if(channel == _ch)
			{
				chOn = true;
			}
		}
		
		command = "mac set ch status ";
		command += channel;			
		if(chOn)	
		{
			sendRawCommand(command+F(" on"));
		}
		else
		{
			sendRawCommand(command+F(" off"));
		}
	}  

	// Canal 65
	// sendRawCommand(F("mac set ch status 65 on"));
	
	// Set Adaptive Data Rate.
	if(_adr)	
	{
		sendRawCommand(F("mac set adr on"));
	}
	else
	{
		sendRawCommand(F("mac set adr off"));
	}

	// Set Automatic Reply.
	if(_ar)	
	{
		sendRawCommand(F("mac set ar on"));
	}
	else
	{
		sendRawCommand(F("mac set ar off"));
	}

	// Set DR and freq for RX2
	sendRawCommand(F("mac set rx2 8 923300000"));
	sendRawCommand(F("mac set rxdelay1 1000"));
	
	
	// Set Data Rate
	command = "mac set dr ";
	command += _dr;
	sendRawCommand(command);
	
	// Set Number of Retransmissions
	command = F("mac set retx ");
	command += _retx;
	sendRawCommand(command);

    // Set the power TX
 	command = F("mac set pwridx ");
	command += _pw;
	sendRawCommand(command);

	// set join parameters
	if (_otaa)
	{
		command = "mac set deveui ";
		command += _deveui;
		receivedData = sendRawCommand(command);
		command = "mac set appeui ";
		command += _appeui;
		receivedData = sendRawCommand(command);
		command = "mac set appkey ";
		command += _appkey;
		receivedData = sendRawCommand(command);

	} else {
		command = "mac set devaddr ";
		command += _deveui;
		receivedData = sendRawCommand(command);
		command = "mac set nwkskey ";
		command += _appeui;
		receivedData = sendRawCommand(command);
		command = "mac set appskey ";
		command += _appkey;
		receivedData = sendRawCommand(command);
	}

	return true;
}


//==========================================================================
void rn2903::setJoin(String AppEUI, String AppKey, String DevEUI="", bool otaa=true)
{
  _otaa = otaa;
  _appeui = AppEUI;
  _appkey = AppKey;
  _deveui = DevEUI;
 
  if (_otaa){
	  if (DevEUI.length() != 16)
	  {
		_deveui = sendRawCommand(F("sys get hweui"));
	  }
  }
}


//==========================================================================
void rn2903::init(void)
{
  String receivedData;
  debug("Configure Parameters");

  //clear serial buffer
  while(_serial.available())
    _serial.read();

  // Config all parameters 
  configParams();
  
  _serial.setTimeout(5000);
  receivedData = sendRawCommand(F("mac save"));
  debug("Init Save: ",receivedData);
  
  _serial.setTimeout(2000);
}

//==========================================================================
bool rn2903::join()
{
  bool joined = false;
  String receivedData;

  _serial.setTimeout(15000);
  
  // Only try twice to join, then return and let the user handle it.
  for(int i=0; i<2 && !joined; i++)
  {
	// Reset with autobaud
	pinReset();
	  
	//clear serial buffer
    while(_serial.available())
	  _serial.read();

	// Execjute the JOIN
	// Parse 1st response

	if (_otaa){
      receivedData = sendRawCommand(F("mac join otaa"));
	} else {
	  receivedData = sendRawCommand(F("mac join abp"));
	}
	
	debug("Join cmd: ",receivedData);

    // Comand JOIN is ok
	if(receivedData.startsWith("ok"))
	{
		// Parse 2nd response
		receivedData = _serial.readStringUntil('\n');

		if(receivedData.startsWith(F("accepted")))
		{	
		  joined=true;
		} else {
			debug("Join Error: ",receivedData);
		}
		
	} else {
		debug("Join Error: ",receivedData);
	}
	// wait and retray
    delay(1000);
  }

  _serial.setTimeout(2000);
  return joined;
}

//==========================================================================
String rn2903::sysver(void)
{
  return sendRawCommand(F("sys get ver"));
}

//==========================================================================
String rn2903::netType(void)
{
  if(_otaa)
  {
	  return F("OTAA");
  }
  else
  {
	  return F("APB");
  }  
}

//==========================================================================
String rn2903::hweui(void)
{
  return sendRawCommand(F("sys get hweui"));
}

//==========================================================================
String rn2903::appeui(void)
{
  return _appeui;
}

//==========================================================================
String rn2903::appkey(void)
{
  return _appkey;
}

//==========================================================================
String rn2903::deveui(void)
{
  return _deveui;
}

//==========================================================================
String rn2903::sleep(long msec)
{
  return sendRawCommand("sys sleep " + msec);
}

//==========================================================================
void  rn2903::factoryReset(void)
{
	pinReset();
    debug(F("Reset de Fábrica do RN2903 com Autobaud"));
 	// reset the module - this will clear all keys set previously
	_serial.println(F("sys factoryRESET"));
  {
  } while(autobaud()==false);  
}

//==========================================================================
String rn2903::macReset(void)
{
	pinReset();
	debug(F("Reset de MAC do RN2903"));
	_serial.println(F("mac reset"));
  {
  } while(autobaud()==false);  
}

//==========================================================================
void rn2903::pinReset(void)
{
  {
	digitalWrite(_resetPin, LOW);      // Pino de RESET = 0
	delay(500);                        // Aguarda 500ms
	digitalWrite(_resetPin, HIGH);     // Pino de RESET = 1
	delay(500);                        // Aguarda 500ms
	debug(F("Reset de Pino do RN2903 com Autobaud"));
  } while(autobaud()==false);  
}

//==========================================================================
String rn2903::getRx(void)
{
  return _rxMessenge;
}

//==========================================================================
signed int rn2903::getRSSI(void)
{
  String str = sendRawCommand(F("radio get rssi"));
  str.trim();
  return str.toInt();
}

//==========================================================================
signed int rn2903::getSNR(void)
{
  String str = sendRawCommand(F("radio get snr"));
  str.trim();
  return str.toInt();
}

//==========================================================================
int rn2903::getVDD(void)
{
  String str = sendRawCommand(F("sys get vdd"));
  str.trim();
  return str.toInt();
}

//==========================================================================
TX_RETURN_TYPE rn2903::tx(String data, bool cfn)
{
  if (cfn){
    debug("TX type: Confirmed");
	return txCnf(data);
  } else {
    debug("TX type: Unconfirmed");
    return txUncnf(data);
  }
}

//==========================================================================
TX_RETURN_TYPE rn2903::txBytes(const byte* data, uint8_t size, bool cfn)
{
  char msgBuffer[size*2 + 1];

  char buffer[3];
  for (unsigned i=0; i<size; i++)
  {
    sprintf(buffer, "%02X", data[i]);
    memcpy(&msgBuffer[i*2], &buffer, sizeof(buffer));
  }
  
  String command;
  String dataToTx(msgBuffer);
  if (cfn){
	  command = "mac tx cnf ";
  }else{
	  command = "mac tx uncnf ";
  }
  command += _port;
  command += " ";
  return txCommand(command, dataToTx, false);
}

//==========================================================================
TX_RETURN_TYPE rn2903::txCnf(String data)
{
  String command;
  command = "mac tx cnf ";
  command += _port;
  command += " ";
  return txCommand(command, data, true);
}

//==========================================================================
TX_RETURN_TYPE rn2903::txUncnf(String data)
{
  String command;
  command = "mac tx uncnf ";
  command += _port;
  command += " ";
  return txCommand(command, data, true);
}


//==========================================================================
TX_RETURN_TYPE rn2903::txCommand(String command, String data, bool shouldEncode)
{
  uint8_t busy_count = 3;
  uint8_t retry_count = 3;
  String receivedData;
  
  while(retry_count!=0)
  {
    //clear serial buffer
    while(_serial.available())
      _serial.read();

    //retransmit a maximum of X times
    retry_count--;

    debug("UpCtr: ",sendRawCommand(F("mac get upctr")));

	// Send TX command for RN2903
    _serial.print(command);

    if(shouldEncode)
    {
      sendEncoded(data);
    }
    else
    {
      _serial.print(data);
    }
    _serial.println();
	
	// Comando TX recebe 2 respostas
    // 1ª Resposta do RN2903
	receivedData = _serial.readStringUntil('\n');
	
    debug("TX Resp 1: ",receivedData);

    // Resposta POSITIVA - Comando TX aceito
	if(receivedData.startsWith("ok"))
    {
	  // 2ª Resposta do RN2903, com timeout bem maior por causa do rádio
	  _serial.setTimeout(8000);
	  receivedData = _serial.readStringUntil('\n');

      debug("TX Resp 2: ",receivedData);

	  // Return timeout for default
	  _serial.setTimeout(2000);
	  
	  // Transmissão com sucesso
      if(receivedData.startsWith(F("mac_tx_ok")))
      {
        return TX_SUCCESS;
      }

	  // Transmissão com sucesso e dado recebido
      else if(receivedData.startsWith(F("mac_rx")))
      {
        //example: mac_rx 1 54657374696E6720313233
        _rxMessenge = receivedData.substring(receivedData.indexOf(' ', 7)+1);
        return TX_WITH_RX;
      }

	  // Erro na transmissão - Payload muito grande
      else if(receivedData.startsWith(F("invalid_data_len")))
      {
        return TX_FAIL_LEN;
      }

	  // Erro na transmissão - Não recebido ACK
	  // Reinicializa e tenta novamente
      else if(receivedData.startsWith(F("mac_err")))
      {
        debug(F("Erro: TX_MAC_ERR"));
		join();
      }

	  // Erro na transmissão - Resposta desconhecida / Timeout
	  // Reinicializa e tenta novamente
      else
      {
        debug(F("Erro: TX_TIME_OUT_1"));
		join();
		}
    }

    // Resposta NEGATIVA - Comando TX falhou por parametro invalido
	// Finaliza
	else if(receivedData.startsWith(F("invalid_param")))
    {
      //should not happen if we typed the commands correctly
      debug(F("Erro: TX_FAIL_PARAM"));
      return TX_FAIL_PARAM;
    }

    // Resposta NEGATIVA - Comando TX falhou por tamanho excessivo do PAYLOAD
	// Finaliza
    else if(receivedData.startsWith(F("invalid_data_len")))
    {
      //should not happen if the prototype worked
      debug(F("Erro: TX_FAIL_LEN"));
      return TX_FAIL_LEN;
    }

    // Resposta NEGATIVA - Comando TX falhou por falta de canal disponível
	// Aguarda um pouco e tenta novamente
	else if(receivedData.startsWith(F("no_free_ch")))
    {
      delay(1000);
      debug(F("Erro: TX_FREE_CH"));
    }

    // Resposta NEGATIVA - Comando TX falhou por falta de conexão
	// Reinicializa e tenta novamente
	else if(receivedData.startsWith(F("not_joined")))
    {
      debug(F("Erro: TX_NOT_JOINED"));
      join();
    }
 
    // Resposta NEGATIVA - Comando TX falhou pelo módulo estar em modo silêncio
	// Reinicializa e tenta novamente
    else if(receivedData.startsWith(F("silent")))
    {
      debug(F("Erro: TX_SILENT"));
	  pinReset();
    }

    // Resposta NEGATIVA - Comando TX falhou por erro no contador
	// Reinicializa e tenta novamente
    else if(receivedData.startsWith(F("frame_counter")))
    {
      debug(F("Erro: TX_FRAME_ERR"));
	  join();
    }

    // Resposta NEGATIVA - Comando TX falhou por MAC pausado
	// Reinicializa e tenta novamente
    else if(receivedData.startsWith(F("mac_paused")))
    {
      debug(F("Erro: TX_MAC_PAUSED"));
	  join();
    }

    // Resposta NEGATIVA - Comando TX falhou por MAC ocupado
	// Aguarda e tenta novamente. Se não conseguir por X vezes reinicia
    else if(receivedData.startsWith(F("busy")))
    {
      debug(F("Erro: TX_BUSY"));
      if(busy_count==0)
      {
        join();
      }
      else
      {
        busy_count--;
        delay(1000);
      }
    }

    // Sem Resposta conhecida ou timeout
    else
    {
      //unknown response after mac tx command
      debug(F("Erro: TX_TIME_OUT_2"));
	  join();
    }
  }

  // Terminou todas as tentativas
  return TX_FAIL_TIMES;
}

//==========================================================================
void rn2903::sendEncoded(String input)
{
  char working;
  char buffer[3];
  for (unsigned i=0; i<input.length(); i++)
  {
    working = input.charAt(i);
    sprintf(buffer,"%02x", int(working));
    _serial.print(buffer);
  }
}


//==========================================================================
String rn2903::sendRawCommand(String command)
{
  delay(10);
  // Limpa dados recebidos
  while(_serial.available())
    _serial.read();

  // Envia comando para o RN2903'
  _serial.println(command);
  
  // Aguarda resposta do módulo
  String ret = _serial.readStringUntil('\n');
  ret.trim();

  return ret;
}

//==========================================================================
String rn2903::getRxMessenge(void)
{
  return _rxMessenge;
}

//==========================================================================
void rn2903::debug(String txt1, String txt2)
{
	if (_debug){
		Serial.print(F("-> "));
		Serial.print(txt1);
		Serial.println(txt2);
	}
}