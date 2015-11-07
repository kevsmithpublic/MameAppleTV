
#include "minimal.h"

struct usbjoy * joys [4];
int num_of_joys = 0;

void gp2x_usbjoy_init (void) {
	int i;

	for (i=0; i<4; i++) {
		joys[i] = joy_open (i+1);
		if (joy_buttons(joys[i])>0) { num_of_joys++; }
	}

	printf ("\n\nFound %d Joystick(s)\n",num_of_joys);
	for (i=0; i < num_of_joys; i++) {
		printf ("+-Joystick %d: %s\n",i+1,joy_name(joys[i]));
	}
}

void gp2x_usbjoy_close (void) {
	int i;
	for (i=0; i<num_of_joys; i++) {
		joy_close(joys[i]);
	}
}

int gp2x_usbjoy_check (struct usbjoy * joy) {
 int q, joyExKey = 0;
 if (joy != NULL) {
	/* Update USB Joypad Information */
	joy_update(joy);
	
	if (joy_getaxe(JOYUP, joy))    { joyExKey |= GP2X_UP; }
	if (joy_getaxe(JOYDOWN, joy))  { joyExKey |= GP2X_DOWN; }
	if (joy_getaxe(JOYLEFT, joy))  { joyExKey |= GP2X_LEFT; }
	if (joy_getaxe(JOYRIGHT, joy)) { joyExKey |= GP2X_RIGHT; }

	/* loop through joy buttons to check if they are pushed */
	for (q=0; q<joy_buttons (joy); q++) {
		if (joy_getbutton (q, joy)) {
			if (q == 0)  { joyExKey |= GP2X_Y; }
			if (q == 1)  { joyExKey |= GP2X_B; }
			if (q == 2)  { joyExKey |= GP2X_X; }
			if (q == 3)  { joyExKey |= GP2X_A; }

			if (q == 4)  { joyExKey |= GP2X_L; }
			if (q == 5)  { joyExKey |= GP2X_R; }
			if (q == 6)  { joyExKey |= GP2X_L; } /* left shoulder button 2 */
			if (q == 7)  { joyExKey |= GP2X_R; } /* right shoulder button 2 */
			if (q == 8)  { joyExKey |= GP2X_SELECT; }
			if (q == 9)  { joyExKey |= GP2X_START; }

			if (q == 10) { joyExKey |= GP2X_PUSH; }
			if (q == 11) { joyExKey |= GP2X_PUSH; }
		}
	}
	return joyExKey;
 } else {
	joyExKey = 0;
	return joyExKey;
 }
}
