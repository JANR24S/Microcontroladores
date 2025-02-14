#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "driver/gpio.h"

#define LED_PIN GPIO_NUM_13
#define LSC_PIN GPIO_NUM_32
#define LSA_PIN GPIO_NUM_33
#define FTC_PIN GPIO_NUM_34
#define PP_PIN GPIO_NUM_35
#define SC_PIN GPIO_NUM_25

// Definición de estados
typedef enum {
    ESTADO_ABRIENDO,
    ESTADO_ABIERTA,
    ESTADO_CERRADA,
    ESTADO_CERRANDO,
    ESTADO_DETENIDA,
    ESTADO_DESPANELADO
} Estado;

// Variables globales
Estado estado_actual = ESTADO_DETENIDA;
int LSC = 0, LSA = 0, FTC = 0, PP = 0, SC = 0;
int led_estado = 0;

// Función de inicialización de pines
void inicializar_pines() {
    gpio_reset_pin(LED_PIN);
    gpio_set_direction(LED_PIN, GPIO_MODE_OUTPUT);
    
    gpio_reset_pin(LSC_PIN);
    gpio_set_direction(LSC_PIN, GPIO_MODE_INPUT);
    gpio_set_pull_mode(LSC_PIN, GPIO_PULLUP_ONLY);
    
    gpio_reset_pin(LSA_PIN);
    gpio_set_direction(LSA_PIN, GPIO_MODE_INPUT);
    gpio_set_pull_mode(LSA_PIN, GPIO_PULLUP_ONLY);
    
    gpio_reset_pin(FTC_PIN);
    gpio_set_direction(FTC_PIN, GPIO_MODE_INPUT);
    gpio_set_pull_mode(FTC_PIN, GPIO_PULLUP_ONLY);
    
    gpio_reset_pin(PP_PIN);
    gpio_set_direction(PP_PIN, GPIO_MODE_INPUT);
    gpio_set_pull_mode(PP_PIN, GPIO_PULLUP_ONLY);
    
    gpio_reset_pin(SC_PIN);
    gpio_set_direction(SC_PIN, GPIO_MODE_INPUT);
    gpio_set_pull_mode(SC_PIN, GPIO_PULLUP_ONLY);
}

// Función de actualización de entradas
void leer_entradas() {
    LSC = !gpio_get_level(LSC_PIN);
    LSA = !gpio_get_level(LSA_PIN);
    FTC = !gpio_get_level(FTC_PIN);
    PP = !gpio_get_level(PP_PIN);
    SC = !gpio_get_level(SC_PIN);
}

// Función de transición de estados
void actualizar_estado() {
    leer_entradas();
    
    switch (estado_actual) {
        case ESTADO_ABIERTA:
            if (PP) estado_actual = ESTADO_ABRIENDO;
            break;
        
        case ESTADO_ABRIENDO:
            if (LSA) estado_actual = ESTADO_ABIERTA;
            else if (FTC) estado_actual = ESTADO_DETENIDA;
            gpio_set_level(LED_PIN, !gpio_get_level(LED_PIN));
            vTaskDelay(pdMS_TO_TICKS(50));
            break;
        
        case ESTADO_CERRANDO:
            if (LSC) estado_actual = ESTADO_CERRADA;
            else if (FTC) estado_actual = ESTADO_DETENIDA;
            break;

        case ESTADO_CERRADA:
            if (PP) estado_actual = ESTADO_ABIERTA;
            gpio_set_level(LED_PIN, !gpio_get_level(LED_PIN));
            vTaskDelay(pdMS_TO_TICKS(50));
            break;
        
        case ESTADO_DETENIDA:
            if (PP) estado_actual = ESTADO_CERRANDO;
            break;
        
        case ESTADO_DESPANELADO:
            if (!SC) estado_actual = ESTADO_DETENIDA;
            break;
    }
}

// Tarea principal de la máquina de estados
void tarea_estado(void *pvParameters) {
    while (1) {
        actualizar_estado();
        vTaskDelay(pdMS_TO_TICKS(100)); // Pequeña pausa para estabilidad
    }
}

// Función principal
void app_main() {
    inicializar_pines();
    xTaskCreate(&tarea_estado, "Tarea Estado", 2048, NULL, 5, NULL);
}
