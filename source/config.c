#include <limits.h>
#include <string.h>
#include <jansson.h>

#include <liblog/log.h>
#include <liblog/syslog.h>
#include <liblog/stderr.h>

#include "tools/utils.h"
#include "config.h"

/*------------------------------------------------------------------------*/

/* try to locate configuration by this paths */
static const char * const path_configs[] = {
	"./rasplock.conf",
	"/etc/rasplock.conf",
	"/usr/etc/rasplock.conf",
	"/usr/local/etc/rasplock.conf",
};

/*------------------------------------------------------------------------*/

static json_t *json_recursive_node(json_t *root, const char *path)
{
	char *full_path = strdup(path);

	if (!full_path) {
		return (NULL);
	}

	json_t *node = root;
	char *saveptr;
	char *node_name = strtok_r(full_path, ".", &saveptr);

	while (node_name && node) {
		node = json_object_get(node, node_name);

		node_name = strtok_r(NULL, ".", &saveptr);
	}

	free(full_path);

	return (node);
}

/*------------------------------------------------------------------------*/

static char *json_read_string(json_t *root, const char *path, char *s, size_t len)
{
	json_t *node = json_recursive_node(root, path);

	if (!node) {
		return (NULL);
	}

	if (!json_is_string(node)) {
		return (NULL);
	}

	strncpy(s, json_string_value(node), len - 1);
	s[len - 1] = 0;

	return (s);
}

/*------------------------------------------------------------------------*/

static int json_read_int(json_t *root, const char *path, int def)
{
	json_t *node = json_recursive_node(root, path);

	if (!node) {
		return (def);
	}

	if (!json_is_integer(node)) {
		return (def);
	}

	return (json_integer_value(node));
}

/*------------------------------------------------------------------------*/

int config_load(struct rasplock_config *cfg)
{
	json_error_t error;
	json_t *root;

	int rc = -1;


	/* try to load configuration */
	for (size_t i = 0; i < __countof(path_configs); ++ i) {
		if ((root = json_load_file(path_configs[i], 0, &error))) {
			break;
		}
	}

	if (!root) {
		_ERR("can't find suitable config file");

		return (rc);
	}


	/* parse json configuration file */
	do {
		/* retrieve log settings */
		char s[PATH_MAX];

		cfg->log.verbose = json_read_int(root, "log.verbose", __LOG_INFO);

		if (!json_read_string(root, "log.type", s, sizeof(s))) {
			_ERR("missed log.type option");

			break;
		}

		if (!strcmp(s, "syslog")) {
			cfg->log.type = log_syslog;
		} if (!strcmp(s, "stderr")) {
			cfg->log.type = log_stderr;
		} else {
			_ERR("unsupported log type: %s", s);

			break;
		}

		/* configure log */
		liblog_type_set(NULL, cfg->log.type);
		liblog_level_set(NULL, cfg->log.verbose);


		/* retrieve gpio number */
		cfg->gpio = json_read_int(root, "gpio", 23);


		/* retrieve unlock codes */
		json_t *n_unlock = json_object_get(root, "unlocks");

		if (!n_unlock) {
			_ERR("missed unlocks option");

			break;
		}

		if (!json_is_array(n_unlock)) {
			_ERR("unlocks option should be array of strings");

			break;
		}

		cfg->unlocks.count = json_array_size(n_unlock);
		if (!(cfg->unlocks.s = calloc(cfg->unlocks.count, sizeof(char*)))) {
			_ERR("%m");

			break;
		}

		for (size_t i = 0; i < cfg->unlocks.count; ++ i) {
			json_t *node = json_array_get(n_unlock, i);

			if (!node || !json_is_string(node)) {
				_ERR("failed to retrieve unlock");

				continue;
			}

			cfg->unlocks.s[i] = strdup(json_string_value(node));
		}


		rc = 0;
	} while (0);


	json_decref(root);

	return (rc);
}
