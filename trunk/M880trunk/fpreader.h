#ifndef _FP485READER_H_
#define _FP485READER_H_

#include <minigui/window.h>

void return_msg_to_fpreader(int type);
int is_slavedevice_verification(void);
int identify_finger_by_template(char *InputPin, unsigned int PIN, WPARAM wParam, LPARAM lParam);
int set_fpreader_msg_flag(WPARAM wParam);
int set_fppic_display_type(int type, WPARAM wParam);
void set_verification_type(WPARAM wParam);

#endif
