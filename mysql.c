/*
 * Copyright (c) 2022 James Anderson <thesemicolons@protonmail.com>
 *
 * Permission to use, copy, modify, and distribute this software for any
 * purpose with or without fee is hereby granted, provided that the
 * above copyright notice and this permission notice appear in all
 * copies.
 *
 * THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL
 * WARRANTIES WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL THE
 * AUTHOR BE LIABLE FOR ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL
 * DAMAGES OR ANY DAMAGES WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR
 * PROFITS, WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE OR OTHER
 * TORTIOUS ACTION, ARISING OUT OF OR IN CONNECTION WITH THE USE OR
 * PERFORMANCE OF THIS SOFTWARE.
 */

#include <stdio.h>

#include <mysql/mysql.h>

static char		*opt_host_name = "localhost";
static char		*opt_user_name = "root";
static char		*opt_password = "password";
static unsigned int	 opt_port_num = 0;
static char		*opt_socket_name = NULL;
static char		*opt_db_name = "test";
static unsigned int	 opt_flags = 0;

unsigned int i = 0;

int
main(int argc, char *argv[])
{
	static MYSQL	*conn;
	MYSQL_RES	*res_set;
	MYSQL_ROW	 row;

	conn = mysql_init(NULL);

	if (conn == NULL) {
		fprintf(stdout, "%s\n", "Mysql connection failed.");

		return 1;
	}

	if (mysql_real_connect(conn, opt_host_name, opt_user_name, opt_password, opt_db_name, opt_port_num, opt_socket_name, opt_flags) == NULL) {
		fprintf(stdout, "%s\n", "mysql_real_connect() failed.");

		mysql_close(conn);

		return 1;
	}

	if (mysql_query(conn, "SELECT * FROM user") != 0) {
		fprintf(stdout, "%s\n", "Query failed.");
	} else {
		fprintf(stdout, "%s\n", "Query succeeded.");

		res_set = mysql_store_result(conn);

		if (res_set == NULL) {
			fprintf(stdout, "%s\n", "No results.");
		} else {
			while ((row = mysql_fetch_row(res_set)) != NULL) {
				for(i = 0; i < mysql_num_fields(res_set); i++) {
					if (i > 0) {
						fputc('\t', stdout);
					}

					fprintf(stdout, "%s", row[i] ? row[i]: "NULL");
				}

				fputc('\t', stdout);
			}

			mysql_free_result(res_set);
		}
	}

	mysql_close(conn);

	return 0;
}

