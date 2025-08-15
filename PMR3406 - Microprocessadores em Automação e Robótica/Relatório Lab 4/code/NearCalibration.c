/*
 * Arquivo:         main.c
 * Autores:         Antônio Augusto Carnevalli  13682909 
 *                  Thiago Lam Brawerman        10770502
 * Data:            14/03/2025
 * Descrição:       Programa de calibração do sensor de proximidade, ao apertar 
 *                  o botão o programa realiza 10 medidas com um intervalo de 
 *                  0.25 segundos entre elas, ao final o programa tira a média 
 *                  entre os valore e escreve ela no display LCD.       
 * 
 * Historico de revisões:   14/03/2025 v1.0 Criação do arquivo.
 *                          07/04/2025 v1.1 Simplificação do código e eliminação 
 *                          de comentários desnecessários.
 *                          22/04/2025 v1.2 Remover comentários adicionais
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
#include "key.h"        // botão da placa

// Saídas
#define BUZZER RB7   // bit para buzzer

// Variáveis Globais
volatile char nMeasures = 0;    // número de medidas realizada 
volatile int sumMeasures = 0;   // soma das medidas
volatile char readStatus = OFF; // status de leitura das medidas (ON) modo de leitura, (OFF) não realiza leitura

// Funções

// inicializa Buzzer
void buzzer_init(void) {
    TRISB7 = 0;            // RB7 é saída para BUZZER 
    BUZZER = 0; 
    #ifndef SIMULATOR
    sensor_power(ON);     // Buzzer precisa para funcionar
    #endif
}

// inicializa Timer 0
void t0_init(void) {
    // O Timer 0 é utilizado para interrupção periódica a cada 5 ms aproximadamente
    OPTION_REGbits.T0CS = 0;    // usa clock interno FOSC/4
    OPTION_REGbits.PSA = 0;     // pre-scaler é para Timer 0 e não para WDT
    OPTION_REGbits.PS = 6;      // ajusta Prescaler para 1:128
    TMR0 = 0xff - 195;          // valor inicial do Timer 0 p/ 5 ms
    TMR0IE = 1;                 // habilita interrupção do Timer 0
}

// Toca um beep
// Para usar o BUZZER o SENSOR POWER deve estar ligado
void beep() {
    BUZZER = ON;          // liga buzzer
    __delay_ms(300);      // por 0,3s
    BUZZER = OFF;         // desliga buzzer
}

// Display da menssagem inicial
void welcome_message() {
    lcd_goto(0);                          // vai para início da 1a. linha
    lcd_puts("Lab 4");                    // coloca no LCD a atividade       
    lcd_goto(64);                         // vai para o início da 2a. linha
    lcd_puts("Grupo 02");                 // coloca o grupo
    __delay_ms(1);                        // dá um tempo para o display mostrar a mensagem
}

// Função de tratamento de interrupções
void __interrupt() isr(void) {
    
    static int tick = 0;     // contador de vezes que o Timer 0 interrompe

    // Timer 0
    // Interrupção do Timer 0 a cada ~5 ms
    if (TMR0IE && TMR0IF) {    // se for interrupção do Timer 0
    
        // Conta quantas vezes é chamado
        if (++tick >= 50) {                     // se interromper 50x é ~250 ms
            tick = 0;                           // zera o contador 
        
            // Faz a medida do sensor de proximidade a cada 250 ms
            if( readStatus && nMeasures++ < 10){    // se está no modo de leitura e não realizou todas as medições
                sumMeasures += sensorNear_read();   // adiciona os dados do sensor de proximidade para a soma das medidas feitas 

            } // fim - contagem das medidas
        } // fim - contagem de vezes que ocorre interrupção
                
        // Debounce da chave. Deve ser incluído na interrupçãp periódica de Timer.
        key_debounce(2);    // 2 é o número de ciclos para dar 5 a 10 ms
        
        TMR0 = 0xff - 195;  // recarrega contagem no Timer 0 p/ 5,0176ms
        TMR0IF = 0;         // limpa interrupção
        
    }// fim - tratamento do Timer 0
    
    // Interrupt-on-change do PORT B usado pela chave
    if (RBIE && RBIF) { // se for mudança estado do Port B
        
        // Seseta as medidas sendo feitas ou já feitas
        char portB = PORTB; // faz a leitura do Port B, isso reseta a condição de interrupção junto com RBIF = 0
        key_read(portB);    // faz a leitura da chave
        RBIF = 0;           // reseta o flag de interrupção para poder receber outra interrupção
        nMeasures = 0;      // zera o contador do número das medidas
        sumMeasures = 0;    // zera a soma das medidas
        readStatus = ON;    // inicia o modo leitura caso ainda não esteja nele
        
    } // fim - tratamento I-O-C PORT B
}// fim - tratamento de todas as interruções
 
 
// Programa Principal
void main(void) {
     
    // Variáveis locais
    char sVar[9];       // string auxiliar para 8 caracteres
    char keyIn = FALSE; // tecla pressionada, TRUE = sim
    
    // Inicializa o robô
    lcd_init();         // inicializa lcd
    sensor_init();      // inicializa sensores

    // Inicializações da placa local
    t0_init();          // inicializa Timer 0 para interrupção periódica de ~5 ms
    key_init();         // inicializa chave
    buzzer_init();      // inicializa buzzer
    
    // Controle de interrupções
    ei();           // habilita interrupções, equivale a GIE = 1;
    
    #ifndef SIMULATOR
    sensor_power(ON);      // liga energia dos sensores, demora 40 ms para ligar o sensor de proximidade
    #endif

    // LCD
    __delay_ms(2000);      // espera para poder ler mensagem de reset
    lcd_clear();           // limpa LCD, não deve ser usado dentro de loops pois demora muito
    lcd_show_cursor(OFF);  // desliga cursor do LCD

    // mensagem inicial no LCD 
    welcome_message(); // coloca mensagem inicial no LCD 
    beep();            // toca beep para sinalizar que está pronto
    
    __delay_ms(2000);           // delay para ler a mensagem
    lcd_clear();                // limpa LCD
    lcd_goto(0);                // vai para o início da 1a linha do LCD
    lcd_puts("MediaADC");       // mostra no LCD a mensagem de espera
    
    while(1) { // Loop Principal
        
        keyIn = key_pressed();  // armazena status da chave
        
        if(keyIn){   // se a chave foi pressionada
            lcd_goto(64);               // vai para o início da 2a linha do LCD
            lcd_puts("          ");     // limpa a 2a linha do LCD

        }
        
        if(nMeasures == 10){                        // caso as 10 medidas tenham sido feitas
            readStatus = OFF;                       // sai do modo leitura do sensor de proximidade
            nMeasures = 0;                          // reseta as medidas
            beep();                                 // beep de aviso da nova tela
            sprintf(sVar, "%04d", sumMeasures/10);  // monta a string com 4 casas da média dos valores medidos
            lcd_goto(64);                           // vai para o início da 2a linha do LCD
            lcd_puts(sVar);                         // mostra no LCD a string
            __delay_ms(300);                        // delay para a atualização do LCD
        }
          
    } // Fim do Loop Principal
} // Fim do Programa Principal