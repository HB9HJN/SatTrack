#include <stdio.h>
#include <math.h>
#include "pico/stdlib.h"
#include "pico/util/queue.h"
#include "pico/multicore.h"
#include "hardware/adc.h"

const uint8_t AZ_R = 21, AZ_L = 20, EL_U = 19, EL_D = 18;

void MotorControl(){
    uint32_t buffer = 1;

    float azel[2] = {10, 10};

    float tgt_azel[2] = {1800,900};

    float err_azel[2] = {0, 0};
    
    const float minERR = 50;
    const float slowERR = 100;

    const float maxaz = 3360, maxel = 3520;

    while(true){
        if(std::abs(err_azel[0]) > slowERR || std::abs(err_azel[1]) > slowERR){
            adc_select_input(0);
            azel[1] = (((float)adc_read()/maxel)*1800);
            //azel[1] = adc_read();
            adc_select_input(1);
            azel[0] = ((float)adc_read()/maxaz)*3600;
            //azel[0] = adc_read();
        }else{
            gpio_put(AZ_R, 0);
            gpio_put(AZ_L, 0);
            gpio_put(EL_U, 0);
            gpio_put(EL_D, 0);

            sleep_ms(400);

            adc_select_input(0);
            azel[1] = (((float)adc_read()/maxel)*1800);
            adc_select_input(1);
            azel[0] = ((float)adc_read()/maxaz)*3600;
        }

        err_azel[0] = tgt_azel[0] - azel[0];
        err_azel[1] = tgt_azel[1] - azel[1];

        if(std::abs(err_azel[0]) > minERR){
            if (err_azel[0] <= 0){
                gpio_put(AZ_R, 0);
                gpio_put(AZ_L, 1);
            }else if(err_azel[0] > 0){
                gpio_put(AZ_R, 1);
                gpio_put(AZ_L, 0);
            }
        }else{
            gpio_put(AZ_R, 0);
            gpio_put(AZ_L, 0);
        }

        

       if(std::abs(err_azel[1]) > minERR){
            if (err_azel[1] <= 0){
                gpio_put(EL_U, 1);
                gpio_put(EL_D, 0);
            }else if(err_azel[1] > 0){
                gpio_put(EL_U, 0);
                gpio_put(EL_D, 1);
            }
        }else{
            gpio_put(EL_U, 0);
            gpio_put(EL_D, 0);
        }

        printf("%f/%f/%u\n", azel[0], azel[1], buffer);
        fflush(NULL);
        scanf("%u\r\n", &buffer);

        tgt_azel[0] = (buffer/10000);
        tgt_azel[1] = (buffer - (tgt_azel[0] * 10000));

        sleep_ms(600);
    }
}



int main() {
    stdio_init_all();
    adc_init();

    adc_gpio_init(26);
    adc_gpio_init(27);

    gpio_init(AZ_R);
    gpio_init(AZ_L);
    gpio_init(EL_U);
    gpio_init(EL_D);
    gpio_set_dir(AZ_R, GPIO_OUT);
    gpio_set_dir(AZ_L, GPIO_OUT);
    gpio_set_dir(EL_U, GPIO_OUT);
    gpio_set_dir(EL_D, GPIO_OUT);


    MotorControl();
}