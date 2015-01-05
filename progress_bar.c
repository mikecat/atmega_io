#include <stdio.h>
#include "progress_bar.h"

void init_progress(progress_t *progress, int max) {
	if (progress == NULL) return;
	progress->bar_max = max;
	progress->now_bar = 0;
	fputs("--------20--------40--------60--------80-------100 [%]\n", stderr);
}

void update_progress(progress_t *progress, int now) {
	int now_pos;
	if (progress == NULL) return;
	now_pos = now * 50 / progress->bar_max;
	if (now_pos > 50) now_pos = 50;
	if (progress->now_bar < now_pos) {
		for (; progress->now_bar < now_pos; progress->now_bar++) {
			fputc('#', stderr);
		}
	}
}
