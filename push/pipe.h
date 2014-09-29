#ifndef _PIPE_H
#define _PIPE_H

int pipe_init(void);
int pipe_write_to_parent_cmd(char cmd);
int pipe_write_to_parent(void *buf, int len);
int pipe_write_to_child(void *buf, int len);
int pipe_write_to_child_cmd(char cmd);
int pipe_read_from_parent(void *buf, int len);
int pipe_read_from_child(void *buf, int len);
void pipe_childend_close(void);
void pipe_parentend_close(void);
void pipe_free(void);
void pipe_clean_parent_cmd(void);
#endif

