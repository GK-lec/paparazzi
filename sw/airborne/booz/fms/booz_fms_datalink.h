/*
 * $Id$
 *
 * This is the implementation of the "external interface" to the autopilot.
 * using datalink messages. 
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 */

#ifndef BOOZ2_FMS_DATALINK_H
#define BOOZ2_FMS_DATALINK_H


#ifdef BOOZ_FMS_PHI_THETA_MAX
#define BOOZ_FMS_LIMIT_ATTITUDE(_fms_att) { \
  BoundAbs(_fms_att.phi,BOOZ_FMS_PHI_THETA_MAX); \
  BoundAbs(_fms_att.theta,BOOZ_FMS_PHI_THETA_MAX); \
}
#else
#define BOOZ_FMS_LIMIT_ATTITUDE(_x) {}
#endif

#define BOOZ_FMS_PARSE_DATALINK(_dl_buffer) {				\
    fms.last_msg = 0;						\
    fms.input.h_mode = DL_BOOZ2_FMS_COMMAND_h_mode(_dl_buffer);	\
    fms.input.v_mode = DL_BOOZ2_FMS_COMMAND_v_mode(_dl_buffer);	\
    switch (fms.input.h_mode) {					\
    case BOOZ2_GUIDANCE_H_MODE_KILL:					\
    case BOOZ2_GUIDANCE_H_MODE_RATE :					\
      break;								\
    case BOOZ2_GUIDANCE_H_MODE_ATTITUDE :				\
      {									\
	      fms.input.h_sp.attitude.phi   = DL_BOOZ2_FMS_COMMAND_h_sp_1(_dl_buffer);			\
	      fms.input.h_sp.attitude.theta = DL_BOOZ2_FMS_COMMAND_h_sp_2(_dl_buffer);			\
	      fms.input.h_sp.attitude.psi   = DL_BOOZ2_FMS_COMMAND_h_sp_3(_dl_buffer);			\
        ANGLE_REF_NORMALIZE(fms.input.h_sp.attitude.psi); \
        BOOZ_FMS_LIMIT_ATTITUDE(fms.input.h_sp.attitude); \
      }									\
      break;  \
    case BOOZ2_GUIDANCE_H_MODE_HOVER :					\
      {									\
	      fms.input.h_sp.pos.x   = DL_BOOZ2_FMS_COMMAND_h_sp_1(_dl_buffer);			\
	      fms.input.h_sp.pos.y   = DL_BOOZ2_FMS_COMMAND_h_sp_2(_dl_buffer);			\
      }									\
      break;  \
    case BOOZ2_GUIDANCE_H_MODE_NAV :					\
      { \
        fms.input.h_sp.speed.x = DL_BOOZ2_FMS_COMMAND_h_sp_1(_dl_buffer); \
        fms.input.h_sp.speed.y = DL_BOOZ2_FMS_COMMAND_h_sp_2(_dl_buffer); \
        fms.input.h_sp.speed.z = DL_BOOZ2_FMS_COMMAND_h_sp_3(_dl_buffer); \
      } \
      break;								\
    }									\
    switch (fms.input.v_mode) {					\
    case BOOZ2_GUIDANCE_V_MODE_KILL:					\
    case BOOZ2_GUIDANCE_V_MODE_RC_DIRECT:				\
    case BOOZ2_GUIDANCE_V_MODE_RC_CLIMB:				\
      break;								\
    case BOOZ2_GUIDANCE_V_MODE_CLIMB :					\
      fms.input.v_sp.climb = DL_BOOZ2_FMS_COMMAND_v_sp(_dl_buffer); \
      break;								\
    case BOOZ2_GUIDANCE_V_MODE_HOVER :					\
      fms.input.v_sp.height = DL_BOOZ2_FMS_COMMAND_v_sp(_dl_buffer); \
      break;								\
    case BOOZ2_GUIDANCE_V_MODE_NAV :					\
      fms.input.v_sp.climb = DL_BOOZ2_FMS_COMMAND_v_sp(_dl_buffer); \
      break;								\
    }									\
  }

#endif /* BOOZ2_FMS_DATALINK_H */
