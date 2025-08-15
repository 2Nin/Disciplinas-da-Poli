/*
 * Arquivo:         main.c
 * Autores:         Antônio Augusto Carnevalli  13682909 
 *                  Thiago Lam Brawerman        10770502
 * Data:            25/05/2025
 * Descrição:       calcular o incremento dos encoders   
 * Historico de revisões:   ...
 */

//#define SIMULATOR       // comentar quando for no robô

#include <xc.h>         // definições dos microcontroladores
#include <stdio.h>      // I/O básico (printf, sprintf, ...)
#include <stdint.h>     // tipos de variáveis padrão (uint8_t, int16_t, ...)
#include <stdlib.h>     //funções adicionais (itoa, ltoa, ultoa, ...)

// Defines

// Todas as definições usadas no programa
#define DIST_THRESHOLD 528 // distância mínima de 40mm (baseado nos dados do Lab 4)

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
#include "sensor.h"
#include "lcd8x2.h"     // LCD, no robô é necessário o SPI.
#include "led_rgb.h"  // LED RGB do robô
#include "pwm.h"        // PWM 

// Saídas
#define BUZZER RB7      // bit para buzzer
#define LED RB5      // bit de saída para o LED


// Grava a matriz de transição na EEPROM
__EEPROM_DATA(  0,  1, -1,  0,  // linha 0
               -1,  0,  0,  1); // linha 1
__EEPROM_DATA(  1,  0,  0, -1,  // linha 2  
                0, -1,  1,  0); // linha 3


// Variáveis Globais

// variável do caráter que vai ser enviavel

volatile signed int encoder_counter_r = 0;  // contador de pulsos do encoder direito
volatile signed short diff_r = 0;           // valor da diferença do número de pulsos do encoder direito por medida

#ifndef SIMULATOR
volatile signed int encoder_counter_l = 0;  // contador de pulsos do encoder esquerdo
volatile signed short diff_l = 0;           // valor da diferença do número de pulsos do encoder esquerdo por medida
#endif

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

// inicializa o Encoder
void encoder_init(){
    nRBPU = 0;      // permite usar pull-ups no Port B

    // Encoder do lado direito
    TRISB3 = 1; // Pino B3 é entrada B1
    TRISB4 = 1; // Pino B4 é entrada A1
    
    WPUB3 = 0;  // weak pull-up disabled para chave
    WPUB4 = 0;  // weak pull-up disabled para chave
    
    IOCB3 = 1;  // interrupt on change enabled
    IOCB4 = 1;  // interrupt on change enabled
    
    ANS9 = 0;   // entrada digital
    ANS11 = 0;  // entrada digital
    
#ifndef SIMULATOR
    // Encoder do lado esquerdo
    TRISB1 = 1; // Pino B3 é entrada B2
    TRISB2 = 1; // Pino B4 é entrada A2
    
    WPUB1 = 1;  // weak pull-up enabled para chave
    WPUB2 = 1;  // weak pull-up enabled para chave
    
    IOCB1 = 1;  // interrupt on change enabled
    IOCB2 = 1;  // interrupt on change enabled
    
    ANS8 = 0;   // entrada digital
    ANS10 = 0;  // entrada digital
#endif
    RBIE = 1;    // habilita interrupção da Porta B
    
}

// inicializa LED
void led_init(void) {
  TRISB5 = 0;  // RB5 é saída para LED
  ANS13 = 0;   // RB5/AN13 é digital  
  LED = 0;
}

// Toca um beep
// Para usar o BUZZER o SENSOR POWER deve estar ligado
void beep() {
    BUZZER = ON;          // liga buzzer
    __delay_ms(200);      // por 0,2s
    BUZZER = OFF;         // desliga buzzer
}

// Display da menssagem inicial
void welcome_message() {
    lcd_goto(0);                          // vai para início da 1a. linha
    lcd_puts("Lab 6");                    // coloca no LCD a atividade       
    lcd_goto(64);                         // vai para o início da 2a. linha
    lcd_puts("Grupo 02");                 // coloca o grupo
    __delay_ms(1);                        // dá um tempo para o display mostrar a mensagem
}
// Função de conversão para mm/s

// Função de tratamento de interrupções
void __interrupt() isr(void) {
    static int tick = 0;     // contador de vezes que o Timer 0 interrompe
    static char new_state_r = 0;            // novo estado da saida do encoder direito
    static char old_state_r = 0;            // antigo estado da saida do encoder direito
    static signed int old_counter_r = 0;    // medida da última contagem de pulsos do encoder direito

#ifndef SIMULATOR
    static char new_state_l = 0;            // novo estado da saida do encoder esquerdo
    static char old_state_l = 0;            // antigo estado da saida do encoder esquerdo
    static signed int old_counter_l = 0;    // medida da última contagem de pulsos do encoder esquerdo
#endif
    
    // Timer 0
    // Interrupção do Timer 0 a cada ~5 ms
    if (TMR0IE && TMR0IF) {    // se for interrupção do Timer 0
    
        // Conta quantas vezes é chamado
        if (++tick >= 20) {    // se interromper 20x é ~100ms
            tick = 0;          // zera o contador
            
            diff_r = encoder_counter_r - old_counter_r; // diferença das medidas do encoder direito em 100ms
            old_counter_r = encoder_counter_r;          // medida antiga se torna medida nova

#ifndef SIMULATOR
            diff_l = encoder_counter_l - old_counter_l; // diferença das medidas do encoder esquerdo em 100ms
            old_counter_l = encoder_counter_l;          // medida antiga se torna medida nova
#endif     
        } // fim - contagem de vezes que ocorre interrupção
        
        TMR0 = 0xff - 195;      // recarrega contagem no Timer 0 p/ 5,0176ms
        TMR0IF = 0;             // limpa interrupção
        
    }// fim - tratamento do Timer 0

    // Interrupt-on-change do PORT B usado pela chave
    if (RBIE && RBIF) { // se for mudança estado do Port B
        
        char portB = PORTB;                 // faz a leitura do Port B, isso reseta a condição de interrupção junto com RBIF = 0
        new_state_r = (portB>>3) & 0x03;    // atualiza o estado do encoder direito
        encoder_counter_r +=  EEPROM_READ(4 * old_state_r + new_state_r);   // atualiza o contador do encoder com o dado da EEPROM()
        old_state_r = new_state_r;          // atualiza o estado antigo com o valor do novo estado do encoder direito
        
#ifndef SIMULATOR
        new_state_l = (portB>>1) & 0x03;    // atualiza o estado do encoder esquerdo
        encoder_counter_l +=  EEPROM_READ(4 * old_state_l + new_state_l);   // atualiza o contador do encoder com o dado da EEPROM()
        old_state_l = new_state_l;          // atualiza o estado antigo com o valor do novo estado do encoder esquerdo
#endif
        RBIF = 0;           // reseta o flag de interrupção para poder receber outra interrupção 
    } // fim - tratamento I-O-C PORT B
} // fim - tratamento de todas as interruções
 
 
// Programa Principal
void main(void) {
    
    int nearSensorData;         // valor do sensor de proximidade
    int dutyCycle;              // valor do Duty cycle do PWM
    char sVar[9];       // string auxiliar para 8 caracteres
    
    // Inicializações o robô
    lcd_init();         // inicializa lcd
    sensor_init();
    
    // Inicializações da placa local
    led_init();     // inicializa LED para debug
    encoder_init();
    pwm_init();         // inicializa PWM
    t0_init();          // inicializa Timer 0 para interrupção periódica de ~5 ms
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
    welcome_message();  // coloca mensagem inicial no LCD 
    beep();             // toca beep para sinalizar que está pronto
    __delay_ms(2000);   // delay de 2s para ler a mensagem
    lcd_clear();        // limpa LCD
    lcd_goto(0);        // vai para o início da 1a linha do LCD
    
    pwm_direction(0);       // Ambos motores indo para frente
    __delay_ms(50);         // tem que esperar antes de religar motores pois o pico de corrente reseta o PIC interno

    while(1) { // Loop Principal
        
        // Sensor de proxímidade
        nearSensorData = (int)sensorNear_read(); // salva o valor do sensor de proximidade

        // ajuste do dutyCycle pela distância
        dutyCycle = nearSensorData >= DIST_THRESHOLD ? 0 : 600 - nearSensorData; 
        
        // aplica o dutyCycle nos motores
        pwm_set(1,dutyCycle);   // ajusta o PWM do motor esquerdo
        pwm_set(2,dutyCycle);   // ajusta o PWM do motor direito
        
        // conversão de diff_r/100ms para mm/s
        // vel = 10 * diff_r * 42/48 * pi
        // vel = 27.5 * diff
        // vel = 27 * diff
        sprintf(sVar, "%03d mm/s", 27 * diff_r); // calcula e coloca em uma string a velocidade do motor direito
        lcd_goto(0);        // volta para a 1a linha do LCD
        lcd_puts(sVar);     // coloca no LCD o valor
#ifndef SIMULATOR
        sprintf(sVar, "%03d mm/s", 27 * diff_l); // calcula e coloca em uma string a velocidade do motor esquerdo
        lcd_goto(64);          
        lcd_puts(sVar);
#endif
        __delay_ms(300);

    } // Fim do Loop Principal
} // Fim do Programa Principal