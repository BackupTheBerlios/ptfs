/* 
 LICENSE: GNU General Public License version 3 
          as published by the Free Software Foundation.
          See the file 'gpl-3.0.txt' for details.
*/

/* This file contains reuse notes. See the file 'REUSE' for details. */


#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <errno.h>
#include <stddef.h>
#include <string.h>

#define FUSE_USE_VERSION 25
#include <errno.h>
#include <fuse.h>

#include "ptfs.h"

const char* program_name = "ptfs";

static struct options {
	char* grammar;
	char* input;
	int help;
	int version;
} options;


#define PTFS_OPT_KEY(t, p, v) { t, offsetof(struct options, p), v }

static struct fuse_opt ptfs_opts[] = {
	PTFS_OPT_KEY("-h", help, 1),
	PTFS_OPT_KEY("-help", help, 1),
	PTFS_OPT_KEY("-v", version, 1),
	PTFS_OPT_KEY("--version", version, 1),
	PTFS_OPT_KEY("-g %s", grammar, 0),
	PTFS_OPT_KEY("--grammar=%s", grammar, 0),
	PTFS_OPT_KEY("-i %s", input, 0),
	PTFS_OPT_KEY("--input=%s", input, 0),
	FUSE_OPT_END
};

void print_usage(FILE *F, int exit_code)
{
	fprintf(F,"\
Usage: %s -g GRAMMARFILE -i INPUTFILE  mountpoint\n\
  -h, --help			Print help\n\
  -v, --version			Print version\n\
  -g, --grammar=GRAMMARFILE	Set file name with a grammar\n\
  -i, --input=INPUTFILE		Set file name with an input text\n", 
    program_name);
	
	exit(exit_code);
}

void print_version()
{
	printf("ptfs version 0.0.2.1\n");
	exit(0);
}


char* map_file_to_str (char* pathname, int* size)
{
	GError* error = NULL;
	GMappedFile* mf = g_mapped_file_new(pathname, FALSE, &error);
	char* data = g_mapped_file_get_contents(mf);
	if(!data) {
		if(error)
			fprintf(stderr, "%s\n", error->message);
		else
			fprintf(stderr, 
				"map_file_to_str %s: unkown error\n", 
				pathname);
		exit(1);
	}

	if(size)
		*size = g_mapped_file_get_length(mf); 

	return data;
}

int cmp(char* s1, char* s2)
{
	return !strcmp(s1, s2);
}

int main(int argc, char** argv)
{
	caml_startup(argv);

	files = g_hash_table_new(g_str_hash, cmp);

	struct fuse_args args = FUSE_ARGS_INIT(argc, argv);
	memset(&options, 0, sizeof(struct options));
	if (fuse_opt_parse(&args, &options, ptfs_opts, NULL) == -1)
		print_usage(stderr, -1);
	
	if(options.version)
		print_version();
	
	if(options.help)
		print_usage(stdout, 0);

	if(options.grammar && options.input) {
		/* read grammar */
		grammar = map_file_to_str(options.grammar, NULL);
		/* read input */
		parse_file(options.input);
	}
	else
		print_usage(stderr, -1);

	fuse_main(args.argc, args.argv, &ptfs_ops);
	fuse_opt_free_args(&args);
	return 0;
}
