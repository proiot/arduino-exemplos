// ***************************************************************************************************
// *  Chaves de configuração dos sensores utilizados                                                 *
// *                                                                                                 *
// *  Desenvolvido por David Souza - SmartMosaic - smartmosaic.com.br                                *
// *  Versão 1.0 - Junho/2020                                                                        *
// *                                                                                                 *
// ***************************************************************************************************

// ***************************************************************************************************
// *  Sensores que estão sendo utilizados                                                            *
// ***************************************************************************************************

#define USE_LM35      ON       // Ativa o uso do sensor LM35
#define USE_DHT11     OFF       // Ativa o uso do sensor DHT11
#define USE_DHT22     ON       // Ativa o uso do sensor DHT12

// ***************************************************************************************************
// *  Bibliotecas                                                                                    *
// ***************************************************************************************************

#if (USE_DHT11 == ON || USE_DHT22 == ON)
  #include <SimpleDHT.h>
#endif

// ***************************************************************************************************
// *  Definição da pinos utilizados nos sensores                                                     *
// ***************************************************************************************************

#if (USE_LM35 == ON)
  const int PIN_LM35 = A2;    //PINO ANALÓGICO UTILIZADO PELO SENSOR LM35
#endif

#if (USE_DHT11 == ON || USE_DHT22 == ON)
  #define PIN_DHT     6         // Pino de comunicação com o DHT
#endif
#if (USE_DHT11 == ON)
  SimpleDHT11 dht11(PIN_DHT); 
#elif (USE_DHT22 == ON)
  SimpleDHT22 dht22(PIN_DHT); 
#endif

// ***************************************************************************************************
// *  Função de leitura de sensores                                                                  *
// ***************************************************************************************************
void read_sensors(void)
{
    // Definição da Umidade
    #if (HUMI == RAND)
      humidity = random(VAL_MIN, VAL_MAX) * VAL_FAC;
    #elif (HUMI == DHT11 && USE_DHT11 == ON)
      dht11.read(&temperature, &humidity, NULL);
    #elif (HUMI == DHT22 && USE_DHT22 == ON)
      dht22.read2(&temperature, &humidity, NULL);
    #else
      humidity = 0;
    #endif

    // Definição da Temperatura
    #if (TEMP == RAND)
      temperature = random(VAL_MIN, VAL_MAX) * VAL_FAC;
    #elif (TEMP == DHT11 && USE_DHT11 == ON)
      dht11.read(&temperature, &humidity, NULL);
    #elif (TEMP == DHT22 && USE_DHT22 == ON)
      dht22.read2(&temperature, &humidity, NULL);
    #elif (TEMP == LM35 && USE_LM35 == ON)
      temperature = (float(analogRead(PIN_LM35)) * 5 / (1023)) / 0.01;
    #else
      temperature = 0;
    #endif

        // Definição da Temperatura 2
    #if (TEMP2 == RAND)
      temperature2 = random(VAL_MIN, VAL_MAX) * VAL_FAC;
    #elif (TEMP2 == DHT11 && USE_DHT11 == ON)
      dht11.read(&temperature2, &humidity, NULL);
    #elif (TEMP2 == DHT22 && USE_DHT22 == ON)
      dht22.read2(&temperature2, &humidity, NULL);
    #elif (TEMP2 == LM35 && USE_LM35 == ON)
      temperature2 = (float(analogRead(PIN_LM35)) * 5 / (1023)) / 0.01;
    #else
      temperature2 = 0;
    #endif
 
}
