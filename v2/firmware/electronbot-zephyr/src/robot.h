/*
 * Robot orchestration - ties the USB transport, display and servo bus together.
 *
 * Mirrors the original head firmware's per-frame flow: render each received
 * stripe to the LCD, and after a full frame push the 6 joint set-points (with
 * per-joint limits) to the servo bus, while returning measured angles back to
 * the host on the IN handshake.
 */
#ifndef ELECTRONBOT_ROBOT_H_
#define ELECTRONBOT_ROBOT_H_

#include "protocol.h"

/* Per-joint mechanical limits (deg), j1..j6. */
extern const float eb_joint_limit[EB_JOINT_COUNT];
extern const int   eb_joint_invert[EB_JOINT_COUNT];

int  eb_robot_init(void);
void eb_robot_run(void); /* never returns */

/* Map a host "model angle" to a raw servo angle (clamp + invert). */
float eb_robot_map_angle(int joint_index, float model_angle);

#endif /* ELECTRONBOT_ROBOT_H_ */
