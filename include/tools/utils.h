/**
 * @file
 */

#ifndef __UTILS_H
#define __UTILS_H

#ifndef __countof
#	define __countof(x) (sizeof(x) / sizeof(x[0]))
#endif /* __countof */

/**
 * @brief make strong password
 * @param [out] s NULL-terminated string with password
 * @param [in] len length of password
 * @return pointer to @p s or NULL, if failed
 *
 * This function use random(), so before using you should initialize
 * random seed by some value, for example by this call srandom(time(NULL))
 */
char* mkpasswd(char *s, size_t len);

#endif /* __UTILS_H */
