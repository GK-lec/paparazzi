/*
 * $Id$
 *
 * Copyright (C) 2009 Pascal Brisset <pascal.brisset@gmail.com>,
 *                    Antoine Drouin <poinix@gmail.com>,
 *                    Gautier Hattenberger <gautier.hattenberger@laas.fr>
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
 *
 *
 * Adaptation bloc of the vertical guidance
 * This is a dimension one kalman filter estimating
 * the ratio of vertical acceleration over thrust command ( ~ invert of the mass )
 * needed by the invert dynamic model to produce a nominal command
 */

#ifndef BOOZ2_GUIDANCE_V_ADPT
#define BOOZ2_GUIDANCE_V_ADPT

extern int32_t b2_gv_adapt_X;
extern int32_t b2_gv_adapt_P;
extern int32_t b2_gv_adapt_Xmeas;


#ifdef B2_GUIDANCE_V_C

/* Our State
   Q13.18
*/
int32_t b2_gv_adapt_X;
#define B2_GV_ADAPT_X_FRAC 18

/* Our covariance
   Q13.18
*/
int32_t b2_gv_adapt_P;
#define B2_GV_ADAPT_P_FRAC 18

/* Our measurement */
int32_t b2_gv_adapt_Xmeas;


/* Initial State and Covariance    */
#define B2_GV_ADAPT_X0_F 0.15
#define B2_GV_ADAPT_X0 BFP_OF_REAL(B2_GV_ADAPT_X0_F, B2_GV_ADAPT_X_FRAC)
#define B2_GV_ADAPT_P0_F 0.5
#define B2_GV_ADAPT_P0 BFP_OF_REAL(B2_GV_ADAPT_P0_F, B2_GV_ADAPT_P_FRAC)

/* System and Measuremement noises */
#define B2_GV_ADAPT_SYS_NOISE_F 0.00005
#define B2_GV_ADAPT_SYS_NOISE  BFP_OF_REAL(B2_GV_ADAPT_SYS_NOISE_F, B2_GV_ADAPT_P_FRAC)

#ifndef USE_ADAPT_HOVER

#define B2_GV_ADAPT_MEAS_NOISE_F 2.0
#define B2_GV_ADAPT_MEAS_NOISE BFP_OF_REAL(B2_GV_ADAPT_MEAS_NOISE_F, B2_GV_ADAPT_P_FRAC)

#else /* USE_ADAPT_HOVER */

#define B2_GV_ADAPT_MEAS_NOISE_HOVER_F 10.0
#define B2_GV_ADAPT_MEAS_NOISE_HOVER BFP_OF_REAL(B2_GV_ADAPT_MEAS_NOISE_HOVER_F, B2_GV_ADAPT_P_FRAC)
#define B2_GV_ADAPT_MEAS_NOISE_F 50.0
#define B2_GV_ADAPT_MEAS_NOISE BFP_OF_REAL(B2_GV_ADAPT_MEAS_NOISE_F, B2_GV_ADAPT_P_FRAC)

#define B2_GV_ADAPT_MAX_ACCEL ACCEL_BFP_OF_REAL(4.0)
#define B2_GV_ADAPT_HOVER_ACCEL ACCEL_BFP_OF_REAL(1.0)
#define B2_GV_ADAPT_MAX_CMD 180
#define B2_GV_ADAPT_MIN_CMD 20
#define B2_GV_ADAPT_HOVER_MAX_CMD 120
#define B2_GV_ADAPT_HOVER_MIN_CMD 60
#endif

static inline void b2_gv_adapt_init(void) {
  b2_gv_adapt_X = B2_GV_ADAPT_X0;
  b2_gv_adapt_P = B2_GV_ADAPT_P0;
}

/*
  zdd_meas : INT32_ACCEL_FRAC
  thrust_applied : controller input [2-200]
*/
#define K_FRAC 12
static inline void b2_gv_adapt_run(int32_t zdd_meas, int32_t thrust_applied) {

  /* Do you really think we want to divide by zero ? */
  if (thrust_applied == 0) return;
  /* We don't propagate state, it's constant !       */
  /* We propagate our covariance                     */
  b2_gv_adapt_P =  b2_gv_adapt_P + B2_GV_ADAPT_SYS_NOISE;
  /* Compute our measurement. If zdd_meas is in the range +/-5g, meas is less than 24 bits */
  const int32_t g_m_zdd = ((int32_t)BFP_OF_REAL(9.81, INT32_ACCEL_FRAC) - zdd_meas)<<(B2_GV_ADAPT_X_FRAC - INT32_ACCEL_FRAC);
#ifdef USE_ADAPT_HOVER
  /* Update only if accel and commands are in a valid range */
  if (thrust_applied < B2_GV_ADAPT_MIN_CMD || thrust_applied > B2_GV_ADAPT_MAX_CMD
      || zdd_meas < -B2_GV_ADAPT_MAX_ACCEL || zdd_meas > B2_GV_ADAPT_MAX_ACCEL)
    return;
#endif
  if ( g_m_zdd > 0)
    b2_gv_adapt_Xmeas = (g_m_zdd + (thrust_applied>>1)) / thrust_applied;
  else
    b2_gv_adapt_Xmeas = (g_m_zdd - (thrust_applied>>1)) / thrust_applied;
  /* Compute a residual */
  int32_t residual = b2_gv_adapt_Xmeas - b2_gv_adapt_X;
  /* Covariance Error   */
  int32_t E = 0;
#ifdef USE_ADAPT_HOVER
  if ((thrust_applied > B2_GV_ADAPT_HOVER_MIN_CMD && thrust_applied < B2_GV_ADAPT_HOVER_MAX_CMD) ||
      (zdd_meas > -B2_GV_ADAPT_HOVER_ACCEL && zdd_meas < B2_GV_ADAPT_HOVER_ACCEL))
    E = b2_gv_adapt_P + B2_GV_ADAPT_MEAS_NOISE_HOVER;
  else
#endif
    E = b2_gv_adapt_P + B2_GV_ADAPT_MEAS_NOISE;
  /* Kalman gain        */
  int32_t K = (b2_gv_adapt_P<<K_FRAC) / E;
  /* Update Covariance */
  b2_gv_adapt_P = b2_gv_adapt_P - ((K * b2_gv_adapt_P)>>K_FRAC);
  /* Don't let covariance climb over initial value */
  if (b2_gv_adapt_P > B2_GV_ADAPT_P0) b2_gv_adapt_P = B2_GV_ADAPT_P0;
  /* Update State */
  b2_gv_adapt_X = b2_gv_adapt_X + ((K * residual)>>K_FRAC);
  /* Again don't let it climb over a value that would
     give less than zero throttle and don't let it down to zero.
     30254 = MAX_ACCEL*B2_GV_ADAPT_X_FRAC/MAX_THROTTLE
     aka
     30254 = 3*9.81*2^8/255
     2571632 = 9.81*2^18
  */
  Bound(b2_gv_adapt_X, 10000, 2571632);
}


#endif /* B2_GUIDANCE_V_C */

#endif /* BOOZ2_GUIDANCE_V_ADPT */
