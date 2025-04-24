#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/queue.h"
#include "driver/gpio.h"
#include "esp_timer.h"

#define LED_GPIO    GPIO_NUM_2     // LED onboard
#define BTN_GPIO    GPIO_NUM_0     // Botón (GPIO0 normalmente está en el ESP32 devkit)

TaskHandle_t blink_task_handle = NULL;

volatile int64_t press_start_time = 0;
volatile int64_t press_duration = 0;

// ISR del botón
static void IRAM_ATTR button_isr_handler(void* arg) {
    static bool pressed = false;

    if (!pressed && gpio_get_level(BTN_GPIO) == 0) {
        // Botón presionado
        press_start_time = esp_timer_get_time(); // en microsegundos
        gpio_set_level(LED_GPIO, 1); // Enciende LED
        pressed = true;
    } else if (pressed && gpio_get_level(BTN_GPIO) == 1) {
        // Botón liberado
        press_duration = esp_timer_get_time() - press_start_time;
        gpio_set_level(LED_GPIO, 0); // Apaga LED

        BaseType_t xHigherPriorityTaskWoken = pdFALSE;
        vTaskNotifyGiveFromISR(blink_task_handle, &xHigherPriorityTaskWoken);
        pressed = false;

        if (xHigherPriorityTaskWoken) {
            portYIELD_FROM_ISR();
        }
    }
}

// Tarea para parpadeo del LED
void blink_task(void *arg) {
    while (1) {
        ulTaskNotifyTake(pdTRUE, portMAX_DELAY);  // Espera notificación desde ISR

        int64_t duration = press_duration;
        int64_t elapsed = 0;

        while (elapsed < duration) {
            gpio_set_level(LED_GPIO, 1);
            vTaskDelay(pdMS_TO_TICKS(200));
            gpio_set_level(LED_GPIO, 0);
            vTaskDelay(pdMS_TO_TICKS(200));
            elapsed += 400 * 1000;  // 400ms en microsegundos
        }
    }
}

void app_main() {
    gpio_config_t io_conf = {
        .pin_bit_mask = (1ULL << LED_GPIO) | (1ULL << BTN_GPIO),
        .mode = GPIO_MODE_INPUT_OUTPUT,
        .pull_up_en = GPIO_PULLUP_ENABLE,
        .intr_type = GPIO_INTR_ANYEDGE
    };
    gpio_config(&io_conf);

    gpio_set_direction(LED_GPIO, GPIO_MODE_OUTPUT);
    gpio_set_direction(BTN_GPIO, GPIO_MODE_INPUT);
    gpio_set_level(LED_GPIO, 0);  // Asegura apagado

    // Crear tarea de parpadeo
    xTaskCreate(blink_task, "blink_task", 2048, NULL, 10, &blink_task_handle);

    // Configurar interrupción del botón
    gpio_install_isr_service(0);
    gpio_isr_handler_add(BTN_GPIO, button_isr_handler, NULL);
}
