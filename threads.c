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

/*
 * Compile with -pthread
 */
#include <stdio.h>
#include <string.h>

#include <pthread.h>
#include <unistd.h>

#define NUM_THREADS 15

void	*taskCode(void *);

int
main(int argc, char *argv[])
{
	pthread_t	 threads[NUM_THREADS];
	int		 rc = 0;
	long		 i = 0;

	for (i = 0; i < NUM_THREADS; i++) {
		rc = pthread_create(&threads[i], NULL, taskCode, (void *)i);

		if (rc) {
			fprintf(stderr, "Error creating thread: %ld\n", i);

			return -1;
		}
	}

	for (i = 0; i < NUM_THREADS; i++) {
		rc = pthread_join(threads[i], NULL);
	}

	fprintf(stdout, "%s\n", "All threads completed successfully, exiting.");

	pthread_exit(NULL);

	return 0;
}

void *
taskCode(void *threadID)
{
	long	 tid = 0;

	tid = (long)threadID;

	printf("Thread %ld reporting for duty!\n", tid);

	sleep (tid + 4);

	printf("Thread %ld has completed.\n", tid);

	pthread_exit(NULL);
}

