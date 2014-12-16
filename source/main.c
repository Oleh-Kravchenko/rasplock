#include <stdio.h>
#include <limits.h>
#include <string.h>
#include <unistd.h>
#include <jansson.h>

#include <liblog/log.h>

#include <tools/file.h>
#include <tools/trim.h>

#include "config.h"

/*------------------------------------------------------------------------*/

void gpio_trigger(unsigned i)
{
	char direction[PATH_MAX];
	char value[PATH_MAX];
	char gpio[PATH_MAX];

	_INFO("gpio trigger %d", i);

	snprintf(direction, sizeof(direction), "/sys/class/gpio/gpio%d/direction", i);
	snprintf(value, sizeof(value), "/sys/class/gpio/gpio%d/value", i);
	snprintf(gpio, sizeof(gpio), "%d", i);

	put_file_contents("/sys/class/gpio/export", gpio, strlen(gpio));
	put_file_contents(direction, "out", strlen("out"));
	put_file_contents(value, "1", strlen("1"));
	sleep(2);
	put_file_contents(value, "0", strlen("0"));
	put_file_contents("/sys/class/gpio/unexport", gpio, strlen(gpio));
}

/*------------------------------------------------------------------------*/

int main(int argc, char **argv)
{
	struct rasplock_config cfg;


	liblog_init();

	if (config_load(&cfg)) {
		return (EXIT_FAILURE);
	}


	/* dump configuration */
	_INFO("gpio: %u", cfg.gpio);
	_INFO("locks: %u", cfg.unlocks.count);

	for (unsigned i = 0; i < cfg.unlocks.count; ++ i) {
		_INFO("%s", cfg.unlocks.s[i]);
	}

	if (getuid()) {
		_WARNING("root permissions required");
	}


	unsigned state = 0;

	for (;;) {
		char s[PATH_MAX];
		unsigned new_state = 0;

		fgets(s, sizeof(s), stdin);
		trim(s);

		for (unsigned i = 0; i < cfg.unlocks.count; ++ i) {
			if (!strncmp(s, cfg.unlocks.s[i], sizeof(s))) {
				new_state |= (1 << i);

				break;
			}
		}

		state = new_state ? (state | new_state) : 0;

		if (state == ((1 << cfg.unlocks.count) - 1)) {
			gpio_trigger(cfg.gpio);
		}
	}


	/* free memory */
	for (unsigned i = 0; i < cfg.unlocks.count; ++ i) {
		free(cfg.unlocks.s[i]);
	}
	free(cfg.unlocks.s);


	return (EXIT_SUCCESS);
}
