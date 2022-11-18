#include <stdio.h>
#include "pico/stdlib.h"
#include "pico/util/queue.h"
#include "pico/multicore.h"
#include "hardware/adc.h"

queue_t adc_data;
queue_t pc_data;

void readADC(){

    float az = 10;
    float el = 10;

    float is_az = 0;
    float is_el = 0;

    float err_az = 0;
    float err_el = 0;

    uint32_t buffer = 100;

    while(true){
        adc_select_input(0);
        az = ((float)adc_read()/4096)*360;
        adc_select_input(1);
        el = ((float)adc_read()/4096)*360;

        printf("%f/%f/%u\n", az, el, buffer);
        scanf("%u\r\n", &buffer);

        is_az = (buffer/10000)/10;
        is_el = (buffer - (is_az * 100000))/10;

        err_az = is_az - az;
        err_el = is_el - el;

        sleep_ms(500);
    }
}

void blink(){





    
    while(true){



        gpio_put(25, 0);
        sleep_ms(250);
        gpio_put(25, 1);
        sleep_ms(250);
    }
}

int main() {
    stdio_init_all();
    adc_init();

    adc_gpio_init(26);
    adc_gpio_init(27);

    gpio_init(25);
    gpio_set_dir(25, GPIO_OUT);

    queue_init(&adc_data, 2 * sizeof(float), 1);
    queue_init(&pc_data, sizeof(int32_t), 1);

    multicore_launch_core1(readADC);
    blink();
}