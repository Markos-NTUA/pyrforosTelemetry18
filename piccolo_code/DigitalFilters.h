/*
 * DigitalFilters.h
 *
 *  Created on: May 25, 2018
 *      Author: markos
 */

#ifndef DIGITALFILTERS_H_
#define DIGITALFILTERS_H_

#include "DSP28x_Project.h" //has types

/*
 * you can set the types here
 * (be careful of the input type)
 */
typedef double COEFF_TYPE; //coefficient type
typedef double SIGNAL_TYPE; //input signal type (after casted from ADC)
/*
 * definitions
 */
#define STAGES 2

/*
 * Direct form II Notch filter structure
 * -contains all the coefficients
 * -arbitrary order
 */
struct NotchFilter_DF2 {
    uint8_t stages;

    //Coefficient accuracy can be changed
    const COEFF_TYPE a[STAGES][3];
    const COEFF_TYPE b[STAGES][3];
    const COEFF_TYPE g[STAGES]; //GAIN
    COEFF_TYPE w[STAGES][3];    //MEMORY
};


/*
 * calculate filters response y[n] given  an input x[n]
 */
SIGNAL_TYPE stepNotch(struct NotchFilter_DF2 * f,SIGNAL_TYPE input);


/*
 * Actual filters that will auto-generated by an external script
 */
//notch filter for current 1
static struct NotchFilter_DF2 NotchC1 = {
    .a = {{1,2,3},
          {3,4,3}},
    .b = {{1,2,3},
          {3,4,3}},
    .g = 0.0023043,
    .w={0}
};

//notch filter for current 1
static struct NotchFilter_DF2 NotchC2 = {
    .a = {{1,2,3},
          {3,4,3}},
    .b = {{1,2,3},
          {3,4,3}},
    .g = 0.0023043,
    .w={0}
};

//notch filter for current 1
static struct NotchFilter_DF2 NotchV1 = {
    .a = {{1,2,3},
          {3,4,3}},
    .b = {{1,2,3},
          {3,4,3}},
    .g = 0.0023043,
    .w={0}
};

//notch filter for current 1
static struct NotchFilter_DF2 NotchV2 = {
    .a = {{1,2,3},
          {3,4,3}},
    .b = {{1,2,3},
          {3,4,3}},
    .g = 0.0023043,
    .w={0}
};
#endif


