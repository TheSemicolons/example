/*
 * Copyright (c) 2024 James Anderson <thesemicolons@protonmail.com>
 *
 * Permission to use, copy, modify, and distribute this software for any purpose
 * with or without fee is hereby granted, provided that the above copyright
 * notice and this permission notice appear in all copies.
 *
 * THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES WITH
 * REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF MERCHANTABILITY
 * AND FITNESS. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY SPECIAL, DIRECT,
 * INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES WHATSOEVER RESULTING FROM
 * LOSS OF USE, DATA OR PROFITS, WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE OR
 * OTHER TORTIOUS ACTION, ARISING OUT OF OR IN CONNECTION WITH THE USE OR
 * PERFORMANCE OF THIS SOFTWARE.
 */
// gcc test.c -o test -lrpcsvc -lutil

#include <sys/stat.h>
#include <sys/types.h>
#include <sys/uio.h>

#include <err.h>
#include <errno.h>
#include <pwd.h>
#include <readpassphrase.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <util.h>

int
main(int argc, char *argv[])
{
	struct passwd	*pw;
	char		 oldpass[1024];
	char		*p;
	char		*username;

	username = getlogin();
	if (username == NULL) {
		(void)fprintf(stderr, "who are you??\n");

		exit(1);
	}

	if (!(pw = getpwnam_shadow(username))) {
		(void)fprintf(stderr, "Unknown user: %s\n", username);

		return(1);
	}

	p = readpassphrase("Old password:", oldpass, sizeof(oldpass), RPP_ECHO_OFF);

	if (p == NULL || *p == '\0') {
		explicit_bzero(oldpass, sizeof(oldpass));

		(void)fprintf(stderr, "%s\n", "Password unchanged");

		pw_abort();

		exit(p == NULL ? 1 : 0);
	}

	if (crypt_checkpass(p, pw->pw_passwd) != 0) {
		explicit_bzero(oldpass, sizeof(oldpass));

		errno = EACCES;

		(void)fprintf(stderr, "%s (errno: %d)\n", strerror(errno), errno);
	}

	if (crypt_checkpass(p, pw->pw_passwd) == 0) {
		explicit_bzero(oldpass, sizeof(oldpass));

		(void)printf("%s\n", "Success");

		exit(0);
	}

	explicit_bzero(oldpass, sizeof(oldpass));

	return 2;
}
