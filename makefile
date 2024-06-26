#
# Copyright (c) 2024 James Anderson <thesemicolons@protonmail.com>
#
# Permission to use, copy, modify, and distribute this software for any purpose
# with or without fee is hereby granted, provided that the above copyright
# notice and this permission notice appear in all copies.
#
# THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES WITH
# REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF MERCHANTABILITY
# AND FITNESS. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY SPECIAL, DIRECT,
# INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES WHATSOEVER RESULTING FROM
# LOSS OF USE, DATA OR PROFITS, WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE OR
# OTHER TORTIOUS ACTION, ARISING OUT OF OR IN CONNECTION WITH THE USE OR
# PERFORMANCE OF THIS SOFTWARE.
#

.POSIX:
.SUFFIXES:

CC        = clang
CFLAGS    = -I/usr/include -I/usr/local/include -O3
LDFLAGS   = -L/usr/local/lib
MANPREFIX = $(PREFIX)/man
PREFIX    = /usr/local
VERSION   = 0.1

all: lex_test metar mysql spin struct threads

lex_test:
	lex lex.l
	$(CC) lex.yy.c -o lex_test -lfl
metar:
	$(CC) $(CFLAGS) $(LDFLAGS) -lcrypto -lcurl -lkcgi -lnghttp2 -lnghttp3 -lngtcp2 -lngtcp2_crypto_quictls -lpthread -lssl -lz -static -o metar metar.c

mysql:
	$(CC) $(CFLAGS) $(LDFLAGS) `mysql_config --cflags --libs` -o mysql mysql.c

spin:
	$(CC) -o spin spin.c

struct:
	$(CC) -o struct struct.c

threads:
	$(CC) -pthread -o threads threads.c

clean:
	rm -f *.o *.core *.yy.c

nuke: clean
	rm -f lex_test metar mysql spin struct threads
