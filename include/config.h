#ifndef __CONFIG_H
#define __CONFIG_H

struct rasplock_config {
	struct {
		log_func_t type;

		unsigned verbose;
	} log;

	unsigned gpio;

	struct {
		unsigned count;

		char **s;
	} unlocks;
};

int config_load(struct rasplock_config *cfg);

#endif /* __CONFIG_H */
