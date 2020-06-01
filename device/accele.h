#ifndef __ACCELE_H
#define __ACCELE_H

#include "heyos_type.h"

#define DEV_ACCELE_IOCTL_GET_DEV_ID                1
#define DEV_ACCELE_IOCTL_SET_MODE                  2
#define DEV_ACCELE_IOCTL_SET_SAMPLE_RATE           3

#define DEV_ACCELE_IOCTL_SIMU_RAND_TYPE1           100
#define DEV_ACCELE_IOCTL_SIMU_RAND_TYPE2           101
#define DEV_ACCELE_IOCTL_SIMU_RAND_TYPE3           102

struct accele_data
{
    uint32_t x;
    uint32_t y;
    uint32_t z;
    uint32_t tim;
};

enum accele_type
{
    ACCELE_XYZ = 1,
};



#endif
