#ifndef KIRU_MESSAGES_H
#define KIRU_MESSAGES_H

#include <SupportDefs.h>

enum {
	MSG_OPEN = 'open',
	MSG_TICK = 'tick',
	MSG_SCRUB = 'scrb',
	MSG_SEEK = 'seek',
	MSG_TOGGLE_PLAY = 'play',
	MSG_BACK_ONE = 'bak1',
	MSG_FORWARD_ONE = 'fwd1',
	MSG_BACK_FIVE = 'bak5',
	MSG_FORWARD_FIVE = 'fwd5',
	MSG_BACK_FRAME = 'bakf',
	MSG_FORWARD_FRAME = 'fwdf',
	MSG_TO_START = 'home',
	MSG_TO_END = 'end_',
	MSG_MARK_IN = 'mrki',
	MSG_MARK_OUT = 'mrko',
	MSG_CUT = 'chop',
	MSG_CUT_FINISHED = 'done'
};

#endif
