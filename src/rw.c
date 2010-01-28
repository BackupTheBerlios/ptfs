/* 
 LICENSE: GNU General Public License version 3 
          as published by the Free Software Foundation.
          See the file 'gpl-3.0.txt' for details.
*/

/* This file contains reuse notes. See the file 'REUSE' for details. */

#include <glib.h>
#include <sys/types.h>

#include "ptfs.h"

static size_t put_to_buf (const char* from, size_t len, 
			  struct rw_op_context* read_op_context) 
{
	char* buf = read_op_context->buf;
	size_t size = read_op_context->size;
	off_t offset = read_op_context->offset;

	if (offset < len) {
		if (offset + size > len)
			size = len - offset;
		memcpy(buf, from + offset, size);
	} 
	else
		size = 0;
	return size;
}

int ptfs_read_from_parsed(struct rw_op_context* read_op_context)
{
	int my_len = 0;
	int my_offset = 0;
	char fparsed_buf[fparsed_size];
	GList* flist = g_hash_table_get_values(files);
	for(; flist; flist=flist->next) {
		struct pars_obj* pars_obj = flist->data;
		my_len = pars_obj->full_path_size;
		memcpy(fparsed_buf+my_offset, pars_obj->full_path, my_len);
		my_offset += my_len;
		fparsed_buf[my_offset] = '\n';
		my_offset += 1;
	}
	return put_to_buf(fparsed_buf, fparsed_size, read_op_context);
}

int ptfs_read_from_text(struct rw_op_context* read_op_context)
{
	char* path = read_op_context->path;
	size_t len;
	int s_begin;
	tree cur_tr;
	struct pars_obj* pars_obj;
	int status = get_node(path, &cur_tr, &pars_obj);
	substring* text;

	if(cur_tr->t_element.t_node.n_attributes) {
		text = &(cur_tr->t_element.t_node.n_attributes[0].a_value);
		len = text->s_end - text->s_begin;
		s_begin = text->s_begin;
	}
	else {
		len = pars_obj->in_size;
		s_begin = 0;
	}

	char substr_buf[len];
	memcpy(substr_buf, pars_obj->input + s_begin, len);

	int retval = put_to_buf(substr_buf, len, read_op_context);
	return retval;
}


int ptfs_write_to_parsed(struct rw_op_context* write_op_context)
{
	char* buf = write_op_context->buf;	
	size_t size = write_op_context->size;

	char fname[size];
	memcpy(fname, buf, size);
	fname[size-1] = '\0';
	parse_file(fname);
	return size;
}

int ptfs_write_to_unparse(struct rw_op_context* write_op_context)
{
	char* buf = write_op_context->buf;	
	size_t size = write_op_context->size;

	char fname[size];
	memcpy(fname, buf, size);
	fname[size-1] = '\0';
	unparse_file(fname);
	return size;
}

void update(tree cur_tr, int dlt, int border)
{
	substring* text;
	tree r_tr;
	tree u_tr;
	if(!cur_tr->t_parent)
		return;
	for(r_tr = cur_tr; r_tr; r_tr=r_tr->t_sibling) {
		node_t* node = &(r_tr->t_element.t_node);
		text = &(node->n_attributes[0].a_value);
		//printf("%d, %d\n", text, r_tr->t_parenti);
		if(text->s_begin > border)
			text->s_begin += dlt;
		text->s_end += dlt;
		u_tr = r_tr->t_parent;
		while(u_tr) {
			update(u_tr, dlt, border);
			u_tr = u_tr->t_parent;
		}
	}
}

int ptfs_write_to_text(struct rw_op_context* write_op_context)
{
	char* buf = write_op_context->buf;	
	char* path = write_op_context->path;
	size_t size = write_op_context->size;
	struct pars_obj* pars_obj;
	tree cur_tr;
	tree tr;
	node_t* node;
	int i,j;
	substring* text;
	size_t len, str_len;
	int border;
	char* new_input;

	int status = get_node(path, &cur_tr, &pars_obj);
	char* cur_input = pars_obj->input;
	
	node_t* cur_node = &(cur_tr->t_element.t_node);
	if(cur_node->n_attributes) {
		text = &(cur_node->n_attributes[0].a_value);
		int dlt = (size -1) - (text->s_end - text->s_begin);
		int new_size = pars_obj->in_size - (text->s_end - text->s_begin) + size -1;
		new_input = malloc(new_size + 1);

		for(i=0; i<text->s_begin; i++)
			new_input[i] = cur_input[i];
		for(i=text->s_begin, j=0; i<text->s_begin+size-1; i++, j++)
			new_input[i] = buf[j];
		for(i = text->s_begin+size-1, j = text->s_end; 
		    j < pars_obj->in_size; j++, i++)
			new_input[i] = cur_input[j];

		border = text->s_end;
		//text->s_end += dlt;
		
		tree r_tr;
		node_t* r_node;
		for(r_tr = cur_tr; r_tr; r_tr=r_tr->t_sibling) {
			r_node = &(cur_tr->t_element.t_node);
			r_node->n_children = NULL;
		}

		update(cur_tr, dlt, border);


		new_input[new_size]='\0';
		pars_obj->input = new_input;
		pars_obj->in_size = new_size;
		free(cur_input);
		//for(i=text->s_begin; i<text->s_begin+size; i++) {
		//}
		//len = text->s_end - text->s_begin;
		//text->s_end -= 1;
	}
	else {
		new_input = malloc(size);
		memcpy(new_input, buf, size);

		pars_obj->input = new_input;
		pars_obj->in_size = size-1;
		free(cur_input);

		cur_node->n_children = NULL;
	}


	char fname[size];
	memcpy(fname, buf, size);
	fname[size-1] = '\0';

	printf("%s\n", fname);
	return size;
}
