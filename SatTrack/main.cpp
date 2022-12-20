#include <stdio.h>
#include <math.h>
#include "pico/stdlib.h"
#include "pico/util/queue.h"
#include "pico/multicore.h"
#include "hardware/adc.h"

const uint8_t AZ_R = 21, AZ_L = 20, EL_U = 19, EL_D = 18; //AZ/EL Pins

void MotorControl(){
    uint32_t buffer = 1; //UART buffer

    float azel[2] = {10, 10}; //true AZ/EL angle

    float tgt_azel[2] = {1800,900}; //target AZ/EL angle

    float err_azel[2] = {0, 0}; //error AZ/EL angel
    
    const float minERR = 50; //minimal tolerated error angle: 5deg
    const float slowERR = 100; //error angle when to compensate for voltagedrop: 10deg
    const float maxaz = 3360, maxel = 3520; //max ADC value for max angle -> calibration

    while(true){
        // write data to UART
        printf("%f/%f/%u\n", azel[0], azel[1], buffer);
        fflush(NULL); //flush buffer
        scanf("%u\r\n", &buffer); //read data from UART

        //Parse AZEL
        //Eg. DATA = 18000450
        //AZ = DATA/10000 -> 180 because is integer and everything after the comma is "cut out".
        //EL = DATA - AZ*10000 -> 45
        tgt_azel[0] = (buffer/10000);
        tgt_azel[1] = (buffer - (tgt_azel[0] * 10000));

        //calculate error AZ/EL angle
        err_azel[0] = tgt_azel[0] - azel[0];
        err_azel[1] = tgt_azel[1] - azel[1];

        if(std::abs(err_azel[0]) > slowERR || std::abs(err_azel[1]) > slowERR){ // if error bigger the slowERR -> because of voltage drop
            //read ADC and convert to angle
            adc_select_input(0);
            azel[1] = (((float)adc_read()/maxel)*1800);
            adc_select_input(1);
            azel[0] = ((float)adc_read()/maxaz)*3600;
        }else{ //if smaller than slowERR
            //shutoff all Motors to stop voltage drop
            gpio_put(AZ_R, 0);
            gpio_put(AZ_L, 0);
            gpio_put(EL_U, 0);
            gpio_put(EL_D, 0);

            sleep_ms(400); //wait for motor halt and voltage settling

            //read ADC and convert to error
            adc_select_input(0);
            azel[1] = (((float)adc_read()/maxel)*1800);
            adc_select_input(1);
            azel[0] = ((float)adc_read()/maxaz)*3600;
        }

        //if error is bigger than the minimal error move motors
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

        sleep_ms(600);//sleep for 0.6s -> enough movement that motor does no oscillate with the error resolution.s
    }
}



int main() {
    //initalize stdlib
    stdio_init_all();
    //initalize adc
    adc_init();

    //initalize ADC_GPIO
    adc_gpio_init(26);
    adc_gpio_init(27);

    //initalize GPIO
    gpio_init(AZ_R);
    gpio_init(AZ_L);
    gpio_init(EL_U);
    gpio_init(EL_D);

    //set GPIO function
    gpio_set_dir(AZ_R, GPIO_OUT);
    gpio_set_dir(AZ_L, GPIO_OUT);
    gpio_set_dir(EL_U, GPIO_OUT);
    gpio_set_dir(EL_D, GPIO_OUT);

    //call MotorControl
    MotorControl();
}
