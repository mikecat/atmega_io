#ifndef PROGRESS_BAR_GUARD_CBCC953C_E435_48E0_B174_1FE5D7EBE567
#define PROGRESS_BAR_GUARD_CBCC953C_E435_48E0_B174_1FE5D7EBE567

typedef struct {
	int bar_max;
	int now_bar;
} progress_t;

/* プログレスバーの表示を開始する */
void init_progress(progress_t *progress, int max);
/* プログレスバーを進める */
void update_progress(progress_t *progress, int now);

#endif
