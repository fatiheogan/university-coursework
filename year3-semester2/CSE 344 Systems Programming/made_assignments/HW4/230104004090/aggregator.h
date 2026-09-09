#ifndef AGGREGATOR_H
#define AGGREGATOR_H

#include "shm.h"

void aggregator_process_main(shm_handle_t *shm, const config_t *cfg);

#endif /* AGGREGATOR_H */
