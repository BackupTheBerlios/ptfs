#include <glib.h>
#include <parse_tree.h>
#define FUSE_USE_VERSION 25
#include <fuse.h>

#define isDir 1
#define isText 2
#define unknown 0

GHashTable* files;
tree tr;

struct pars_obj {
	char* input;
	unsigned int in_size;
	tree tr;
};

char* grammar;
char* map_file_to_str (char* pathname, int* size);
int parse_file(char* file_name);
int get_node(const char* path, tree* ret_tr, struct pars_obj** ret_pars_obj);
unsigned char* compile(char*, int*);


int ptfs_getattr(const char *path, struct stat *stbuf);
int ptfs_readdir(const char *path, void *buf, fuse_fill_dir_t filler,
                  off_t offset, struct fuse_file_info *fi);
int ptfs_read(const char *path, char *buf, size_t size, off_t offset,
                      struct fuse_file_info *fi);

struct fuse_operations ptfs_ops;
