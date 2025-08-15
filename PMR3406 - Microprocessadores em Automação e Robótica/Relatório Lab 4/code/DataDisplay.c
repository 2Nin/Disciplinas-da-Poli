/*
 * Arquivo:         main.c
 * Autores:         Antônio Augusto Carnevalli  13682909 
 *                  Thiago Lam Brawerman        10770502
 * Data:            14/03/2025
 * Descrição:       O programa realiza coleta de dados do sensor de proximidade,
 *                  converte os dados analógicos para digitais e converte 
 *                  novamente para a unidade correta (milímetros).
 * Historico de revisões:   14/03/2025 v1.0 criação do arquivo
 *                          07/04/2025 v1.1 Altera as "variáveis" para constantes
 *                          22/04/2025 v1.2 Remove comentários desnecessários 
 *                          24/04/2025 v2.0 Correções durante o lab
 *                          13/05/2025 v2.1 Detalhes finais antes da entrega
 */

//#define SIMULATOR       // comentar quando for no robô

#include <xc.h>         // definições dos microcontroladores
#include <stdio.h>      // I/O básico (printf, sprintf, ...)
#include <stdint.h>     // tipos de variáveis padrão (uint8_t, int16_t, ...)
#include <stdlib.h>     //funções adicionais (itoa, ltoa, ultoa, ...)


// PIC16F886 ajustes do bit de configuração
// CONFIG1
#pragma config FOSC = EC    // bits de configuração do ocilador (EC: I/O function on RA6/OSC2...
#pragma config WDTE = OFF   // bit de habilitação do Watchdog Timer (WDT disabled and can be enab...
#pragma config PWRTE = ON   // bit de habilitação do Power-up Timer (PWRT enabled)
#pragma config MCLRE = ON   // bit de seleção da função do pino RE3/MCLR (RE3/MCLR pin function...
#pragma config CP = OFF     // bit de proteção do código (Program memory code protection is ...
#pragma config CPD = OFF    // bit de proteção do código dos Dados (Data memory code protection i...
#pragma config BOREN = ON   // bit de seleção do Brown Out Reset (BOR enabled)
#pragma config IESO = OFF   // bit de Internal External Switchover (Internal/External Swi...
#pragma config FCMEN = OFF  // bit de habilitação do Fail-Safe Clock Monitor (Fail-Safe Clock Mo...
#pragma config LVP = OFF    // bit de habilitação de Low Voltage Programming (RB3 pin has digital...
// CONFIG2
#pragma config BOR4V = BOR40V   // bit de seleção do Brown-out Reset (Brown-out Reset set ...
#pragma config WRT = OFF        // bits de habilitação do Flash Program Memory Self Write (Write ...

// Includes do projeto
#include "always.h"

#include "adc.h"        // conversor A/D
#include "lcd8x2.h"     // LCD, no robô é necessário o SPI.
#include "sensor.h"     // sensores de linha, de proximidade e buzzer

// Saídas
#define BUZZER RB7   // bit para buzzer

// Funções

//Conversão para milímetros dos dados coletados pelo sensor de proximidade 
int conversion_equation(int adcResult){

    // Constantes obtidas experimentalmente pela calibração do robô
    const long mLine = 32124;
    const int bLine = 27;
    const int k = 18;
    
    // Checagem do resultado do conversor A/D para garantir que não exista divisão por 0.
    if (-bLine == adcResult){
        return 0;
    };
    
    // Retorna o resultado da equação m'/(V+b')-k 
    return mLine/(adcResult + bLine)-k;    
}

// inicializa Buzzer
void buzzer_init(void) {
    TRISB7 = 0;            // RB7 é saída para BUZZER 
    BUZZER = 0; 
    #ifndef SIMULATOR
    sensor_power(ON);     // Buzzer precisa para funcionar
    #endif
}

// Toca um beep
// Para usar o BUZZER o SENSOR POWER deve estar ligado.
void beep() {
    BUZZER = ON;          // liga buzzer
    __delay_ms(200);      // por 0,2s
    BUZZER = OFF;         // desliga buzzer
}

void welcome_message() {
    lcd_goto(0);                          // vai para início da 1a. linha
    lcd_puts("Lab 4");                    // coloca no LCD a atividade atual      
    lcd_goto(64);                         // vai para o início da 2a. linha
    lcd_puts("Grupo 02");                 // coloca o grupo
    __delay_ms(1);                        // dá um tempo para o display mostrar a mensagem
}
// Programa Principal
void main(void) {
     
    // Variáveis locais
    char sVar[9];       // string auxiliar para 8 caracteres
    int nearSensorData; // valor do sensor de proximidade

    // Inicializações
    lcd_init();         // inicializa lcd
    buzzer_init();      // inicializa o buzzer 
    sensor_init();      // inicializa sensores
    
    #ifndef SIMULATOR
    sensor_power(ON);       // liga energia dos sensores, demora 40 ms para ligar o sensor de proximidade
    #endif

    // LCD
    __delay_ms(2000);       // espera para poder ler mensagem de reset
    lcd_clear();            // limpa LCD, não deve ser usado dentro de loops pois demora muito
    lcd_show_cursor(OFF);   // desliga cursor do LCD
    
    welcome_message();      // recoloca mensagem inicial 
    beep();                 // toca beep para sinalizar que está pronto

     __delay_ms(2000); //espera para a leitura da welcome_message()
    
    lcd_clear();        // limpa LCD
    lcd_goto(0);        // vai para linha 1, coluna 1 do LCD
    lcd_puts("Dist:");  // coloca a string "Dist:" no LCD


    while(1) { // Loop Principal
        nearSensorData = conversion_equation((int)sensorNear_read());   // leitura e conversão do sensor de proximidade
        sprintf(sVar,"%04d mm",nearSensorData);                         // monta a string com o valor medido
        lcd_goto(64);                                                   // vai para linha 2, coluna 1 do LCD
        lcd_puts(sVar);                                                 // mostra no LCD a string
        __delay_ms(300);                                                // delay para a atualização do LCD
    
    } // Fim do Loop Principal
} // Fim do Programa Principal