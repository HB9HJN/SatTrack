#include <stdio.h>
#include <math.h>
#include "pico/stdlib.h"
#include "pico/util/queue.h"
#include "pico/multicore.h"
#include "hardware/adc.h"

queue_t angle_data;
queue_t tgt_data;


void MotorControl(){

    float azel[2] = {10, 10};

    float tgt_azel[2] = {0,0};

    float err_az = 0;
    float err_el = 0;

    const uint8_t AZ_R = 10, AZ_L = 9, EL_U = 10, EL_D = 9;
    const float minERR = 2;

    while(true){
        adc_select_input(0);
        azel[0] = ((float)adc_read()/4096)*360;
        adc_select_input(1);
        azel[1] = ((float)adc_read()/4096)*360;

        if(queue_is_empty(&angle_data)){
            queue_add_blocking(&angle_data, azel);
        }

        if(queue_is_full(&tgt_data)){
            queue_remove_blocking(&tgt_data, tgt_azel);
        }

        err_az = tgt_azel[0] - azel[0];
        err_el = tgt_azel[1] - azel[1];

        if(std::abs(err_az) > minERR){
            if (err_az <= 0){
                gpio_put(AZ_R, 1);
                gpio_put(AZ_L, 0);
            }else if(err_az > 0){
                gpio_put(AZ_R, 0);
                gpio_put(AZ_L, 1);
            }
        }else{
            gpio_put(AZ_R, 0);
            gpio_put(AZ_L, 0);
        }
        
        if(std::abs(err_el) > minERR){
            if (err_el <= 0){
                gpio_put(EL_U, 1);
                gpio_put(EL_D, 0);
            }else if(err_az > 0){
                gpio_put(EL_U, 0);
                gpio_put(EL_D, 1);
            }
        }else{
            gpio_put(EL_U, 0);
            gpio_put(EL_D, 0);
        }

        sleep_ms(100);
    }
}

void toPC(){
    uint32_t buffer = 100;
    
    float azel[2] = {10, 10};
    float tgt_azel[2] = {0, 0};

    
    while(true){

        if(queue_is_full(&angle_data)){
            queue_remove_blocking(&angle_data, azel);
        }

        if(queue_is_empty(&tgt_data)){
            queue_add_blocking(&tgt_data, tgt_azel);
        }

        printf("%f/%f/%u\n", azel[0], azel[1], buffer);
        scanf("%u\r\n", &buffer);

        tgt_azel[0] = (buffer/10000)/10;
        tgt_azel[1] = (buffer - (tgt_azel[0] * 100000))/10;

        sleep_ms(500);
    }
}

int main() {
    stdio_init_all();
    adc_init();

    adc_gpio_init(26);
    adc_gpio_init(27);

    gpio_init(25);
    gpio_set_dir(25, GPIO_OUT);

    queue_init(&angle_data, 2 * sizeof(float), 1);
    queue_init(&tgt_data, 2 * sizeof(float), 1);
    
    multicore_launch_core1(MotorControl);
    toPC();
}