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

#include "booz2_vf_float.h"

/*

X = [ z zdot bias ]

temps :
  propagate 86us
  update    46us

*/
/* initial covariance diagonal */
#define INIT_PXX 1.
/* process noise */
#define ACCEL_NOISE 0.5
#define Qzz       ACCEL_NOISE/512./512./2.
#define Qzdotzdot ACCEL_NOISE/512.
#define Qbiasbias 1e-7
#define R 1.

float b2_vff_z;
float b2_vff_bias;
float b2_vff_zdot;
float b2_vff_zdotdot;

float b2_vff_P[B2_VFF_STATE_SIZE][B2_VFF_STATE_SIZE];

float b2_vff_z_meas;

void b2_vff_init(float init_z, float init_zdot, float init_bias) {
  b2_vff_z    = init_z;
  b2_vff_zdot = init_zdot;
  b2_vff_bias = init_bias;
  int i, j;
  for (i=0; i<B2_VFF_STATE_SIZE; i++) {
    for (j=0; j<B2_VFF_STATE_SIZE; j++)
      b2_vff_P[i][j] = 0.;
    b2_vff_P[i][i] = INIT_PXX;
  }

}


/*

 F = [ 1 dt -dt^2/2
       0  1 -dt
       0  0   1     ];

 B = [ dt^2/2 dt 0]';

 Q = [ 0.01  0     0
       0     0.01  0
       0     0     0.001 ];

 Xk1 = F * Xk0 + B * accel;

 Pk1 = F * Pk0 * F' + Q;

*/
void b2_vff_propagate(float accel) {
  /* update state */
  b2_vff_zdotdot = accel + 9.81 - b2_vff_bias;
  b2_vff_z = b2_vff_z + DT_VFILTER * b2_vff_zdot;
  b2_vff_zdot = b2_vff_zdot + DT_VFILTER * b2_vff_zdotdot;
  /* update covariance */
  const float FPF00 = b2_vff_P[0][0] + DT_VFILTER * ( b2_vff_P[1][0] + b2_vff_P[0][1] + DT_VFILTER * b2_vff_P[1][1] );
  const float FPF01 = b2_vff_P[0][1] + DT_VFILTER * ( b2_vff_P[1][1] - b2_vff_P[0][2] - DT_VFILTER * b2_vff_P[1][2] );
  const float FPF02 = b2_vff_P[0][2] + DT_VFILTER * ( b2_vff_P[1][2] );
  const float FPF10 = b2_vff_P[1][0] + DT_VFILTER * (-b2_vff_P[2][0] + b2_vff_P[1][1] - DT_VFILTER * b2_vff_P[2][1] );
  const float FPF11 = b2_vff_P[1][1] + DT_VFILTER * (-b2_vff_P[2][1] - b2_vff_P[1][2] + DT_VFILTER * b2_vff_P[2][2] );
  const float FPF12 = b2_vff_P[1][2] + DT_VFILTER * (-b2_vff_P[2][2] );
  const float FPF20 = b2_vff_P[2][0] + DT_VFILTER * ( b2_vff_P[2][1] );
  const float FPF21 = b2_vff_P[2][1] + DT_VFILTER * (-b2_vff_P[2][2] );
  const float FPF22 = b2_vff_P[2][2];

  b2_vff_P[0][0] = FPF00 + Qzz;
  b2_vff_P[0][1] = FPF01;
  b2_vff_P[0][2] = FPF02;
  b2_vff_P[1][0] = FPF10;
  b2_vff_P[1][1] = FPF11 + Qzdotzdot;
  b2_vff_P[1][2] = FPF12;
  b2_vff_P[2][0] = FPF20;
  b2_vff_P[2][1] = FPF21;
  b2_vff_P[2][2] = FPF22 + Qbiasbias;

}
/*
  H = [1 0 0];
  R = 0.1;
  // state residual
  y = rangemeter - H * Xm;
  // covariance residual
  S = H*Pm*H' + R;
  // kalman gain
  K = Pm*H'*inv(S);
  // update state
  Xp = Xm + K*y;
  // update covariance
  Pp = Pm - K*H*Pm;
*/
static inline void update_z_conf(float z_meas, float conf) {
  b2_vff_z_meas = z_meas;

  const float y = z_meas - b2_vff_z;
  const float S = b2_vff_P[0][0] + conf;
  const float K1 = b2_vff_P[0][0] * 1/S;
  const float K2 = b2_vff_P[1][0] * 1/S;
  const float K3 = b2_vff_P[2][0] * 1/S;

  b2_vff_z    = b2_vff_z    + K1 * y;
  b2_vff_zdot = b2_vff_zdot + K2 * y;
  b2_vff_bias = b2_vff_bias + K3 * y;

  const float P11 = (1. - K1) * b2_vff_P[0][0];
  const float P12 = (1. - K1) * b2_vff_P[0][1];
  const float P13 = (1. - K1) * b2_vff_P[0][2];
  const float P21 = -K2 * b2_vff_P[0][0] + b2_vff_P[1][0];
  const float P22 = -K2 * b2_vff_P[0][1] + b2_vff_P[1][1];
  const float P23 = -K2 * b2_vff_P[0][2] + b2_vff_P[1][2];
  const float P31 = -K3 * b2_vff_P[0][0] + b2_vff_P[2][0];
  const float P32 = -K3 * b2_vff_P[0][1] + b2_vff_P[2][1];
  const float P33 = -K3 * b2_vff_P[0][2] + b2_vff_P[2][2];

  b2_vff_P[0][0] = P11;
  b2_vff_P[0][1] = P12;
  b2_vff_P[0][2] = P13;
  b2_vff_P[1][0] = P21;
  b2_vff_P[1][1] = P22;
  b2_vff_P[1][2] = P23;
  b2_vff_P[2][0] = P31;
  b2_vff_P[2][1] = P32;
  b2_vff_P[2][2] = P33;

}

void b2_vff_update(float z_meas) {
  update_z_conf(z_meas, R);
}

void b2_vff_update_z_conf(float z_meas, float conf) {
  update_z_conf(z_meas, conf);
}

/*
  H = [0 1 0];
  R = 0.1;
  // state residual
  yd = vz - H * Xm;
  // covariance residual
  S = H*Pm*H' + R;
  // kalman gain
  K = Pm*H'*inv(S);
  // update state
  Xp = Xm + K*yd;
  // update covariance
  Pp = Pm - K*H*Pm;
*/
static inline void update_vz_conf(float vz, float conf) {
  const float yd = vz - b2_vff_zdot;
  const float S = b2_vff_P[1][1] + conf;
  const float K1 = b2_vff_P[0][1] * 1/S; 
  const float K2 = b2_vff_P[1][1] * 1/S; 
  const float K3 = b2_vff_P[2][1] * 1/S; 
  
  b2_vff_z    = b2_vff_z    + K1 * yd; 
  b2_vff_zdot = b2_vff_zdot + K2 * yd; 
  b2_vff_bias = b2_vff_bias + K3 * yd; 

  const float P11 = -K1 * b2_vff_P[1][0] + b2_vff_P[0][0];
  const float P12 = -K1 * b2_vff_P[1][1] + b2_vff_P[0][1];
  const float P13 = -K1 * b2_vff_P[1][2] + b2_vff_P[0][2];
  const float P21 = (1. - K2) * b2_vff_P[1][0];
  const float P22 = (1. - K2) * b2_vff_P[1][1];
  const float P23 = (1. - K2) * b2_vff_P[1][2];
  const float P31 = -K3 * b2_vff_P[1][0] + b2_vff_P[2][0];
  const float P32 = -K3 * b2_vff_P[1][1] + b2_vff_P[2][1];
  const float P33 = -K3 * b2_vff_P[1][2] + b2_vff_P[2][2];

  b2_vff_P[0][0] = P11;
  b2_vff_P[0][1] = P12;
  b2_vff_P[0][2] = P13;
  b2_vff_P[1][0] = P21;
  b2_vff_P[1][1] = P22;
  b2_vff_P[1][2] = P23;
  b2_vff_P[2][0] = P31;
  b2_vff_P[2][1] = P32;
  b2_vff_P[2][2] = P33;

}

void b2_vff_update_vz_conf(float vz_meas, float conf) {
  update_vz_conf(vz_meas, conf);
}

void b2_vff_realign(float z_meas) {
  b2_vff_z = z_meas;
  b2_vff_zdot = 0;
}
