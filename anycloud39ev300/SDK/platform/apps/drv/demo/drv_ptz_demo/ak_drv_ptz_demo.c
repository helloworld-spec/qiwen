
#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include "ak_common.h"
#include "ak_drv_ptz.h"


int main (int argc, char **argv) {

    assert (ak_drv_ptz_open() == AK_SUCCESS);

    /// Setup the Step Range Parameter.
//    assert (ak_drv_ptz_setup_step_param (PTZ_DEV_H, 1024, 1000, -1) == AK_SUCCESS);
//    assert (ak_drv_ptz_setup_step_param (PTZ_DEV_V, 512, 500, -1) == AK_SUCCESS);

	ak_drv_ptz_set_angle_rate(24/24.0, 21/21.0);
	ak_drv_ptz_set_degree (350, 130);
	ak_drv_ptz_check_self(PTZ_FEEDBACK_PIN_NONE);

	while (1) {

		int hdg = rand () % 350;
		int vdg = rand () % 130;

		ak_drv_ptz_turn_to_pos (hdg, vdg);
		ak_print_normal ("Turn to Degree (%d, %d)\r\n", hdg, vdg);

		getchar ();

		ak_print_normal ("Position H=%d V=%d\r\n",
				ak_drv_ptz_get_step_pos (PTZ_DEV_H), ak_drv_ptz_get_step_pos (PTZ_DEV_V));

	}

	ak_print_normal ("Double Clicked to Continue.\r\n");
	getchar ();
	getchar ();

    ak_drv_ptz_close();

	return 0;
}
