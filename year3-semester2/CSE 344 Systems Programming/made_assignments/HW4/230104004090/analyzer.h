#ifndef ANALYZER_H
#define ANALYZER_H

#include "shm.h"

void analyzer_process_main(int level_idx, shm_handle_t *shm, const config_t *cfg);

#endif /* ANALYZER_H */
