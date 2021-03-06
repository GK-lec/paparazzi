/*
 * $Id$
 *  
 * Copyright (C) 2008-2009 Antoine Drouin <poinix@gmail.com>
 *
 * This file is part of paparazzi.
 *
 * paparazzi is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2, or (at your option)
 * any later version.
 *
 * paparazzi is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with paparazzi; see the file COPYING.  If not, write to
 * the Free Software Foundation, 59 Temple Place - Suite 330,
 * Boston, MA 02111-1307, USA. 
 */

#ifndef BOOZ_IMU_H
#define BOOZ_IMU_H

#include "math/pprz_algebra_int.h"

/* must be defined by underlying hardware */
extern void booz_imu_impl_init(void);
extern void booz_imu_periodic(void);

struct BoozImu {
  struct Int32Rates gyro;
  struct Int32Vect3 accel;
  struct Int32Vect3 mag;
  struct Int32Rates gyro_prev;
  struct Int32Vect3 accel_prev;
  struct Int32Rates gyro_neutral;
  struct Int32Vect3 accel_neutral;
  struct Int32Vect3 mag_neutral;
  struct Int32Rates gyro_unscaled;
  struct Int32Vect3 accel_unscaled;
  struct Int32Vect3 mag_unscaled;
  struct Int32Quat  body_to_imu_quat;
  struct Int32RMat  body_to_imu_rmat;
};

/* underlying hardware */
#include BOOZ_IMU_TYPE_H

extern struct BoozImu booz_imu;

extern void booz_imu_init(void);

#define BoozImuScaleGyro() {						\
    RATES_COPY(booz_imu.gyro_prev, booz_imu.gyro);			\
    booz_imu.gyro.p = ((booz_imu.gyro_unscaled.p - booz_imu.gyro_neutral.p)*IMU_GYRO_P_SENS_NUM)/IMU_GYRO_P_SENS_DEN; \
    booz_imu.gyro.q = ((booz_imu.gyro_unscaled.q - booz_imu.gyro_neutral.q)*IMU_GYRO_Q_SENS_NUM)/IMU_GYRO_Q_SENS_DEN; \
    booz_imu.gyro.r = ((booz_imu.gyro_unscaled.r - booz_imu.gyro_neutral.r)*IMU_GYRO_R_SENS_NUM)/IMU_GYRO_R_SENS_DEN; \
  }


#define BoozImuScaleAccel() {						\
    VECT3_COPY(booz_imu.accel_prev, booz_imu.accel);			\
    booz_imu.accel.x = ((booz_imu.accel_unscaled.x - booz_imu.accel_neutral.x)*IMU_ACCEL_X_SENS_NUM)/IMU_ACCEL_X_SENS_DEN; \
    booz_imu.accel.y = ((booz_imu.accel_unscaled.y - booz_imu.accel_neutral.y)*IMU_ACCEL_Y_SENS_NUM)/IMU_ACCEL_Y_SENS_DEN; \
    booz_imu.accel.z = ((booz_imu.accel_unscaled.z - booz_imu.accel_neutral.z)*IMU_ACCEL_Z_SENS_NUM)/IMU_ACCEL_Z_SENS_DEN; \
  }

#if defined IMU_MAG_45_HACK
#define BoozImuScaleMag() {						\
    int32_t msx = ((booz_imu.mag_unscaled.x - booz_imu.mag_neutral.x) * IMU_MAG_X_SENS_NUM) / IMU_MAG_X_SENS_DEN; \
    int32_t msy = ((booz_imu.mag_unscaled.y - booz_imu.mag_neutral.y) * IMU_MAG_Y_SENS_NUM) / IMU_MAG_Y_SENS_DEN; \
    booz_imu.mag.x = msx - msy;					\
    booz_imu.mag.y = msx + msy;					\
    booz_imu.mag.z = ((booz_imu.mag_unscaled.z - booz_imu.mag_neutral.z) * IMU_MAG_Z_SENS_NUM) / IMU_MAG_Z_SENS_DEN; \
  }
#else
#define BoozImuScaleMag() {					\
    booz_imu.mag.x = ((booz_imu.mag_unscaled.x - booz_imu.mag_neutral.x) * IMU_MAG_X_SENS_NUM) / IMU_MAG_X_SENS_DEN; \
    booz_imu.mag.y = ((booz_imu.mag_unscaled.y - booz_imu.mag_neutral.y) * IMU_MAG_Y_SENS_NUM) / IMU_MAG_Y_SENS_DEN; \
    booz_imu.mag.z = ((booz_imu.mag_unscaled.z - booz_imu.mag_neutral.z) * IMU_MAG_Z_SENS_NUM) / IMU_MAG_Z_SENS_DEN; \
  }
#endif


#endif /* BOOZ2_IMU_H */
