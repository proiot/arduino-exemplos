# Arduino - Exemplos

# MQTT

## Materiais necessários

* [Arduino IDE](https://www.arduino.cc/en/Main/Software)
* [Biblioteca PubSubClient](https://pubsubclient.knolleary.net/)
* Arduino UNO
* Ethernet Shield

## Contexto

* 1: Envio de temperatura

### MQTT

* Ex 1 - Conexão insegura: [link](./firmware/mqtt/mqtt.ino)

## Referências

* [PubSubClient](https://pubsubclient.knolleary.net/)


# HTTP

## Materiais necessários

* [Arduino IDE](https://www.arduino.cc/en/Main/Software)
* [Biblioteca WeeESP8266](https://github.com/itead/ITEADLIB_Arduino_WeeESP8266)
* Arduino UNO
* Módulo ESP8266

## Contexto 1 (Teste-Modulo-ESP8266)

* 0: Utiliza somente comandos AT do módulo
* 1: Somente testa a comunicação com o módulo ESP8266 através de uma
     porta serial via software (UNO)

## Contexto 2 (TCP-Echo-WeeESP8266)

* 0: Utiliza biblioteca WeeESP8266
* 1: O node é ativado como um TCPServe que fica aguardando recever algum dado
     e repete esse mesmo dado recebido de volta ao endereço de envio
* 2: Para enviar um dado pode ser utilizado Putty na porta correta (tipo RAW)	 

## Contexto 3 (HTTP-POST-WeeESP8266)

* 0: Utiliza biblioteca WeeESP8266
* 1: Envio de 1 varável para a plataforma ProIoT
* 2: Parametrização dos dados da rede WiFi (dados podem estar no arquivo chaves.h)
* 3: Parametrização dos dados da plataforma ProIoT (dados podem estar no arquivo chaves.h)
* 4: Parametrização do tempo de envio automático
* OBS: Está com o código do ECHO implementado também, mas ainda não estão
       funcionando os 2 recursos ao mesmo tempo

## Contexto 4 (Sensores-HTTP-WeeESP8266)

* 0: Utiliza biblioteca WeeESP8266
* 1: Origem/sensores: LM35 (temperatura), DHT11 ou 22 (Temperatura/Umidade) ou Randômico
* 2: 3 variáveis (2x temperatura e umidade)com parametrização da origem e alias
* 3: Parametrização dos dados da rede WiFi (dados podem estar no arquivo chaves.h)
* 4: Parametrização dos dados da plataforma ProIoT (dados podem estar no arquivo chaves.h)
* 5: Parametrização do tempo de leitura e envio automático
* 6: Reset automático do módulo ESP8266 (exige pino extra, se ativado)
* 7: DEBUG (porta serial) pode ser ativado ou desativado
* 8: Pode ser ativado WDT
	   
## Referências

* [WeeESP8266](https://github.com/itead/ITEADLIB_Arduino_WeeESP8266)
* [SimpleDHT] (https://github.com/winlinvip/SimpleDHT)
