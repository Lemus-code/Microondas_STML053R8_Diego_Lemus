#include <stdint.h> // Librería estándar para tipos enteros
#include <string.h>

// Direcciones Relevantes (GPIOS, RCC)
#define PERIPHERAL_BASE_ADDRESS     0x40000000U
#define AHB_BASE_ADDRESS            (PERIPHERAL_BASE_ADDRESS + 0x00020000U)
#define RCC_BASE_ADDRESS            (AHB_BASE_ADDRESS + 0x00001000U)
#define IOPORT_BASE_ADDRESS         (PERIPHERAL_BASE_ADDRESS + 0x10000000U)
#define GPIOA_BASE_ADDRESS          (IOPORT_BASE_ADDRESS + 0x00000000U)
#define GPIOB_BASE_ADDRESS          (IOPORT_BASE_ADDRESS + 0x00000400U)
#define GPIOC_BASE_ADDRESS          (IOPORT_BASE_ADDRESS + 0x00000800U)

// Estructura de GPIO
typedef struct {
    uint32_t MODER;
    uint32_t OTYPER;
    uint32_t OSPEEDR;
    uint32_t PUPDR;
    uint32_t IDR;
    uint32_t ODR;
    uint32_t BSRR;
    uint32_t LCKR;
    uint32_t AFRL;
    uint32_t AFRH;
    uint32_t BRR;
} GPIO_STRUCT;

// Estructura de RCC
typedef struct {
    uint32_t CR;
    uint32_t ICSCR;
    uint32_t CRRCR;
    uint32_t CFGR;
    uint32_t CIER;
    uint32_t CIFR;
    uint32_t CICR;
    uint32_t IOPRSTR;
    uint32_t AHBRSTR;
    uint32_t APB2RSTR;
    uint32_t APB1RSTR;
    uint32_t IOPENR;
    uint32_t AHBENR;
    uint32_t APB2ENR;
    uint32_t APB1ENR;
    uint32_t IOPSMENR;
    uint32_t AHBSMENR;
    uint32_t APB1SMENR;
    uint32_t APB2SMENR;
    uint32_t CCIPR;
    uint32_t CSR;
} RCC_STRUCT;

struct reloj{
	uint8_t min_unidades;
	uint8_t min_decenas;
	uint8_t h_unidades;
	uint8_t h_decenas;
};

// Asignar estructuras
#define GPIOA   ((GPIO_STRUCT*)GPIOA_BASE_ADDRESS)
#define GPIOB   ((GPIO_STRUCT*)GPIOB_BASE_ADDRESS)
#define GPIOC   ((GPIO_STRUCT*)GPIOC_BASE_ADDRESS)
#define RCC     ((RCC_STRUCT*)RCC_BASE_ADDRESS)

//Variables fijas reloj
#define ALL_DISPLAY_OFF 0x33
#define D0_ctrl 		(1<<5)
#define D1_ctrl 		(1<<4)
#define D2_ctrl 		(1<<1)
#define D3_ctrl 		(1<<0)

#define cc_0 0b00111111
#define cc_1 0b00000110
#define cc_2 0b01011011
#define cc_3 0b01001111
#define cc_4 0b01100110
#define cc_5 0b01101101
#define cc_6 0b01111101
#define cc_7 0b00000111
#define cc_8 0b01111111
#define cc_9 0b01101111
#define cc_all_off 0b00000000

#define ca_cc_bits 0xFF //sirve más adelante para limpiar segmentos



// Variables keypad
int8_t tecla_presionada = -1;
int8_t tecla_anterior = 0x0;
uint8_t gnd_control = 0x00;

//Variables reloj
uint16_t time_control_24 = 0x00;
uint16_t time_control_micro = 0x00;
uint8_t en_control = 0x00;
struct reloj clk_24 = {0,0,0,0}; // 00:00
struct reloj microwave_clk = {0,0,0,0};
struct reloj display_clk = {0,0,0,0};


//Variables Varias
uint8_t microwave_insert = 0x00;
uint8_t microwave_start = 0x00;
uint8_t delay_lcd = 0;
uint8_t motor_change = 0;
uint8_t motor_delay = 0;
const uint32_t time_micro = 120;
const uint32_t time_24 = 60 * time_micro;

// Función delay
void delay_ms(uint16_t n) {
    volatile uint16_t i;
    for (; n > 0; n--) {
        for (i = 0; i < 40; i++);
    }
}


//<-----FUNCIONES CLOCK----->
uint8_t parser(uint8_t decode){
    switch(decode){
        case 0: return cc_0;
        case 1: return cc_1;
        case 2: return cc_2;
        case 3: return cc_3;
        case 4: return cc_4;
        case 5: return cc_5;
        case 6: return cc_6;
        case 7: return cc_7;
        case 8: return cc_8;
        case 9: return cc_9;
        default: return cc_all_off;
    }
}

void microwave_time_move (struct reloj *time, uint8_t first_key){
    time->h_decenas    = time->h_unidades;
    time->h_unidades   = time->min_decenas;
    time->min_decenas  = time->min_unidades;
    time->min_unidades = first_key;
}

//<-----FUNCIONES Keypad----->
void active_key(){

	tecla_presionada = -1; //reset tecla

	GPIOB->ODR |= 0x0F; //reset

	GPIOB->ODR &= ~(1<<0);
	delay_ms(3);

	if((GPIOB->IDR & (1<<7))== 0){
		tecla_presionada = 0x0A;
	}
	else if((GPIOB->IDR & (1<<6)) == 0){
		tecla_presionada = 0x0B;
	}
	else if((GPIOB->IDR & (1<<5)) == 0) {
		tecla_presionada = 0x0C;
	}
	else if((GPIOB->IDR & (1<<4)) == 0){
		tecla_presionada = 0x0D;
	}

	GPIOB->ODR |= (1<<0);
	GPIOB->ODR &= ~(1<<1);
	delay_ms(3);

	if((GPIOB->IDR & (1<<7))== 0){
		tecla_presionada = 0x03;
	}
	else if((GPIOB->IDR & (1<<6)) == 0){
		tecla_presionada = 0x06;
	}
	else if((GPIOB->IDR & (1<<5)) == 0) {
		tecla_presionada = 0x09;
	}
	else if((GPIOB->IDR & (1<<4)) == 0){
		tecla_presionada = 0x0E;
	}

	GPIOB->ODR |= (1<<1);
	GPIOB->ODR &= ~(1<<2);
	delay_ms(3);

	if((GPIOB->IDR & (1<<7))== 0){
		tecla_presionada = 0x02;
	}
	else if((GPIOB->IDR & (1<<6)) == 0){
		tecla_presionada = 0x05;
	}
	else if((GPIOB->IDR & (1<<5)) == 0) {
		tecla_presionada = 0x08;
	}
	else if((GPIOB->IDR & (1<<4)) == 0){
		tecla_presionada = 0x00;
	}

	GPIOB->ODR |= (1<<2);
	GPIOB->ODR &= ~(1<<3);
	delay_ms(3);

	if((GPIOB->IDR & (1<<7))== 0){
		tecla_presionada = 0x01;
	}
	else if((GPIOB->IDR & (1<<6)) == 0){
		tecla_presionada = 0x04;
	}
	else if((GPIOB->IDR & (1<<5)) == 0) {
		tecla_presionada = 0x07;
	}
	else if((GPIOB->IDR & (1<<4)) == 0){
		tecla_presionada = 0x0F;
	}

	GPIOB->ODR |= (1<<3);
}

//<-----FUNCIONES LCD----->
void lcd_pulso(){
    // Enabled
    GPIOA->ODR |= (1 << 7);
    delay_ms(1);
    GPIOA->ODR &= ~(1 << 7);
    delay_ms(2);
}

void lcd_4bcommand(uint16_t comando) {
    uint32_t bus_alto = (comando & 0xF0) << 4;
    uint32_t bus_bajo = ((comando << 4) & 0xF0) << 4;

    // RS = 0 → modo comando
    GPIOA->ODR &= ~(1 << 6);

    // Enviar nibble alto
    GPIOA->ODR &= ~((1 << 8) | (1 << 9) | (1 << 10) | (1 << 11));
    GPIOA->ODR |= bus_alto;
    lcd_pulso();

    // Enviar nibble bajo
    GPIOA->ODR &= ~((1 << 8) | (1 << 9) | (1 << 10) | (1 << 11));
    GPIOA->ODR |= bus_bajo;
    lcd_pulso();

    delay_ms(2); // espera final
}


void lcd_init() {
    int contador = 0;

    // Despertar LCD
    while (contador < 3) {
        // RS = 0 para comando y RW a GND
        GPIOA->ODR &= ~(1 << 6);

        // Limpiar pines D4-D7
        GPIOA->ODR &= ~((1 << 8) | (1 << 9) | (1 << 10) | (1 << 11));

        // D4-D5
        GPIOA->ODR |= (1 << 8) | (1 << 9); //comado 0b0011

        lcd_pulso();
        contador++;
    }

    GPIOA->ODR &= ~((1 << 8) | (1 << 9) | (1 << 10) | (1 << 11));
    GPIOA->ODR |= (1 << 9);  //0b0010 para indicarle que usamos 4 bits
    lcd_pulso();

    delay_ms(2);

    lcd_4bcommand(0x28);

    // Comando para encender pantalla
    lcd_4bcommand(0x0C);

    // Comando para limpiar display
    lcd_4bcommand(0x01);

    // Cursor avanza a la derecha
    lcd_4bcommand(0x06);
}

//Enviar data
void lcd_data(char* informacion){
	for(uint8_t i = 0; i < strlen(informacion); i++){
		uint32_t bus_alto = (informacion[i] & 0xF0) << 4;
		uint32_t bus_bajo = ((informacion[i] << 4) & 0xF0)<< 4;
		// RS = 1 → modo comando
		GPIOA->ODR |= (1 << 6);

		// Enviar nibble alto
		GPIOA->ODR &= ~((1 << 8) | (1 << 9) | (1 << 10) | (1 << 11));
		GPIOA->ODR |= bus_alto;
		lcd_pulso();

		// Enviar nibble bajo
		GPIOA->ODR &= ~((1 << 8) | (1 << 9) | (1 << 10) | (1 << 11));
		GPIOA->ODR |= bus_bajo;
		lcd_pulso();

		delay_ms(2); // espera final
	}
}


void activar_motor(uint8_t *motor_change){
    // Apagar todas las fases antes de activar la siguiente
    GPIOC->ODR &= ~((1<<8) | (1<<9));  // PC8, PC9
    GPIOB->ODR &= ~((1<<8) | (1<<9));  // PB8, PB9

    // Secuencia correcta: IN1 → IN2 → IN3 → IN4
    switch (*motor_change) {
        case 0:
            GPIOC->ODR |= (1<<8);   // IN1 (PC8)
            *motor_change = 1;
            break;
        case 1:
            GPIOC->ODR |= (1<<9);   // IN2 (PC9)
            *motor_change = 2;
            break;
        case 2:
            GPIOB->ODR |= (1<<8);   // IN3 (PB8)
            *motor_change = 3;
            break;
        case 3:
            GPIOB->ODR |= (1<<9);   // IN4 (PB9)
            *motor_change = 0;
            break;
        default:
            *motor_change = 0;
            break;
    }
}






int main(void) {
    // Habilitar Clk en GPIOA, B, C
    RCC->IOPENR |= (1 << 0) | (1 << 1) | (1 << 2);


    //Configurar salidas de los PC0, PC1, PC2, PC3, PC4, PC5, PC6, PC7
    GPIOC->MODER &= ~((1<<1)|(1<<3)|(1<<5)|(1<<7)|(1<<9)|(1<<11)|(1<<13)|(1<<15));

    //Enabled , PA0, PA1 , PA5
    GPIOA->MODER &= ~((1<<1)|(1<<3)|(1<<11));

    //Enabled PA4 (Viene en 0x00 por default, saber porque)
    GPIOA->MODER &= ~((3<<(4*2)));
    GPIOA->MODER |= ((1<<(4*2)));

    //Outputs Keypad PB0-PB3
    GPIOB->MODER &=  ~((3<<(0*2)) | (3<<(1*2)) | (3<<(2*2)) | (3<<(3*2)));
    GPIOB->MODER |=  ((1<<(0*2)) | (1<<(1*2)) | (1<<(2*2)) | (1<<(3*2)));

    //Inputs Keypad PB4-PB7
    GPIOB->MODER &=  ~((3<<(4*2)) | (3<<(5*2)) | (3<<(6*2)) | (3<<(7*2)));
    GPIOB->PUPDR |=  ((1<<(4*2)) | (1<<(5*2)) | (1<<(6*2)) | (1<<(7*2)));

    //Pines de LCD como salidas PA6 - PB11
    GPIOA->MODER &= ~((3<<(6*2)) | (3<<(7*2)) | (3<<(8*2)) | (3<<(9*2)) | (3<<(10*2)) | (3<<(11*2)));
    GPIOA->MODER |= ((1<<(6*2)) | (1<<(7*2)) | (1<<(8*2)) | (1<<(9*2)) | (1<<(10*2)) | (1<<(11*2)));

    //PC8, PC9, PB8, PB9
    GPIOC->MODER &= ~((3<<(8*2)) | (3<<(9*2)));
    GPIOC->MODER |= ((1<<(8*2)) | (1<<(9*2)));

    GPIOB->MODER &= ~((3<<(8*2)) | (3<<(9*2)));
    GPIOB->MODER |= ((1<<(8*2)) | (1<<(9*2)));

    lcd_init();

    while (1) {

    	active_key();

    	if(tecla_presionada != tecla_anterior){
    		tecla_anterior = tecla_presionada;
    	}else{
    		tecla_presionada = -1;
    	}

    	if(tecla_presionada != -1 && (tecla_presionada >= 0 && tecla_presionada <= 9)){
    		lcd_4bcommand(0x01); //limpiar
    		lcd_data("Esperando *");
    		microwave_time_move(&microwave_clk, tecla_presionada);
    		microwave_insert = 1;
    	}

    	switch(tecla_presionada){
    	case 0x0A :{ //Cafecito 2:00
    		lcd_4bcommand(0x01); //limpiar
    		lcd_data("Cafe 2:00 M");

    		microwave_clk.min_unidades = 0;
    		microwave_clk.min_decenas = 0;
    		microwave_clk.h_unidades = 2;
    		microwave_clk.h_decenas = 0;
    		microwave_insert = 1;
    		break;
    	}
    	case 0x0B :{ //Descongelar pollo
    		lcd_4bcommand(0x01); //limpiar
    		lcd_data("Pollo 3:00 M");
    		microwave_clk.min_unidades = 0;
    		microwave_clk.min_decenas = 0;
    		microwave_clk.h_unidades = 3;
    		microwave_clk.h_decenas = 0;
    		microwave_insert = 1;
    		break;
    	}
    	case 0x0C :{//Sopa
    		lcd_4bcommand(0x01); //limpiar
    		lcd_data("Sopa 2:30 M");
    		microwave_clk.min_unidades = 0;
    		microwave_clk.min_decenas = 3;
    		microwave_clk.h_unidades = 2;
    		microwave_clk.h_decenas = 0;
    		microwave_insert = 1;
    		break;
    	}
    	case 0x0D :{//Poporopos
    		lcd_4bcommand(0x01); //limpiar
    		lcd_data("Poporopos 1:30 M");
    		microwave_clk.min_unidades = 0;
    		microwave_clk.min_decenas = 3;
    		microwave_clk.h_unidades = 1;
    		microwave_clk.h_decenas = 0;
    		microwave_insert = 1;
    		break;
    	}
    	case 0x0E :{
    		lcd_4bcommand(0x01); //limpiar
    		lcd_data("Cancelado");
    		microwave_clk.min_unidades = 0;
    		microwave_clk.min_decenas = 0;
    		microwave_clk.h_unidades = 0;
    		microwave_clk.h_decenas = 0;
    		microwave_insert = 0;
    		microwave_start = 0;
    		break;
    	}
    	case 0x0F:
    	{
    		if(microwave_clk.min_unidades == 0 && microwave_clk.min_decenas == 0 && microwave_clk.h_unidades == 0 && microwave_clk.h_decenas == 0){
    			lcd_4bcommand(0x01); //limpiar
    			lcd_data("Tiempo invalido");
    			break;
    		}else if(microwave_clk.min_decenas >= 6 || microwave_clk.h_decenas >= 6){
    			lcd_4bcommand(0x01); //limpiar
    			lcd_data("Tiempo invalido");
    			microwave_clk.min_unidades = 0;
    			microwave_clk.min_decenas = 0;
    			microwave_clk.h_unidades = 0;
    			microwave_clk.h_decenas = 0;
    			microwave_insert = 0;
    			microwave_start = 0;

    			break;
    		}

    		//Iniciar micro
    		lcd_4bcommand(0x01); //limpiar
    		lcd_data("Calentando");
    		microwave_start = 1;
    		break;
    	}

    	default:{
    		break;
    	}

    	}


    	if(microwave_insert == 1){
    		display_clk.min_unidades = microwave_clk.min_unidades;
    		display_clk.min_decenas = microwave_clk.min_decenas;
    		display_clk.h_unidades = microwave_clk.h_unidades;
    		display_clk.h_decenas = microwave_clk.h_decenas;
    	}else{
    		display_clk.min_unidades = clk_24.min_unidades;
    		display_clk.min_decenas = clk_24.min_decenas;
    		display_clk.h_unidades = clk_24.h_unidades;
    		display_clk.h_decenas = clk_24.h_decenas;
    	}

    	//Para que no parpadeen los displays
    	delay_lcd++;

    	if(delay_lcd >= 1){
    		delay_lcd = 0;
    		switch(en_control){
    		    		case 0:
    		    		{
    		    			GPIOA->BSRR = (D3_ctrl << 16); //apago el D3
    		    			GPIOC->BSRR = (ca_cc_bits); //limpio displays
    		    			GPIOA->BSRR = D0_ctrl;
    		    			GPIOC->ODR  = parser(display_clk.min_unidades);
    		    			en_control++;
    		    			break;
    		    		}
    		    		case 1:
    		    		{
    		    			GPIOA->BSRR = (D0_ctrl << 16); //apago el D3
    		    			GPIOC->BSRR = (ca_cc_bits); //limpio displays
    		    			GPIOA->BSRR = D1_ctrl;
    		    			GPIOC->ODR  = parser(display_clk.min_decenas);
    		    			en_control++;
    		    			break;
    		    		}
    		    		case 2:
    		    		{
    		    			GPIOA->BSRR = (D1_ctrl << 16); //apago el D3
    		    			GPIOC->BSRR = (ca_cc_bits); //limpio displays
    		    			GPIOA->BSRR = D2_ctrl;
    		    			GPIOC->ODR  = parser(display_clk.h_unidades);
    		    			en_control++;
    		    			break;
    		    		}
    		    		case 3:
    		    		{

    		    			GPIOA->BSRR = (D2_ctrl << 16); //apago el D3
    		    			GPIOC->BSRR = (ca_cc_bits); //limpio displays
    		    			GPIOA->BSRR = D3_ctrl;
    		    			GPIOC->ODR  = parser(display_clk.h_decenas);
    		    			en_control = 0x00;
    		    			break;
    		    		}
    		    		default:
    		    		{
    		    			en_control = 0x00;
    		    			break;
    		    		}
    		    	}
    		delay_ms(1);
    	}

    	if (microwave_start == 1) {

    		motor_delay++;
    		if(motor_delay >= 2){ // Ajusta la velocidad del motor
    			motor_delay = 0;
    			activar_motor(&motor_change);
    		}

    	    time_control_micro++;

    	    if (time_control_micro >= time_micro) { // cada 60 ciclos ≈ 1 segundo
    	        time_control_micro = 0;

    	        if(microwave_clk.min_unidades> 0){
    	        	microwave_clk.min_unidades--;
    	        }else{
    	        	if(microwave_clk.min_decenas > 0){
    	        		microwave_clk.min_decenas--;
    	        		microwave_clk.min_unidades = 9;
    	        	}else{
    	        		if(microwave_clk.h_unidades > 0){
    	        			microwave_clk.h_unidades--;
    	        			microwave_clk.min_decenas = 5;
    	        			microwave_clk.min_unidades = 9;
    	        		}else{
    	        			if(microwave_clk.h_decenas > 0){
    	        				microwave_clk.h_decenas--;
    	        				microwave_clk.h_unidades = 9;
    	        				microwave_clk.min_decenas = 5;
    	        				microwave_clk.min_unidades = 9;
    	        			}else{
    	        	    		lcd_4bcommand(0x01); //limpiar
    	        	    		lcd_data("¡Listo!");
    	        				microwave_insert = 0;
    	        				microwave_start = 0;
    	        			}
    	        		}
    	        	}
    	        }

    	    }
    	}

    	time_control_24++;

    	if(time_control_24 == time_24){
    		time_control_24 = 0;
    		clk_24.min_unidades++;

    		if(clk_24.min_unidades == 10){
    			clk_24.min_unidades = 0;
    			clk_24.min_decenas++;

    			if(clk_24.min_decenas == 6){
    				clk_24.min_decenas = 0;
    				clk_24.h_unidades++;

    				if(clk_24.h_unidades == 10){
    					clk_24.h_unidades = 0;
    					clk_24.h_decenas++;


    				}

    				if(clk_24.h_decenas == 2 && clk_24.h_unidades == 4){
    					clk_24.h_unidades = 0;
    					clk_24.h_decenas = 0;
    				}
    			}
    		}
    	}
    }
}
