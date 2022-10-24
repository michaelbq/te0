#ifndef _EDITOR_H
#define _EDITOR_H

#include <stdio.h>
#include <stdbool.h>

typedef struct {
	size_t size;
	size_t len;
	char *buf;
}tline;

typedef struct {
	size_t capacity;
	size_t size;
	size_t cursor_row;
	size_t cursor_col;
	tline *lines;
}teditor;

bool line_insert_text(tline *line, size_t col, const char *text, size_t len);
bool line_delete_after_text(tline *line, size_t col, size_t len);
bool line_delete_back_text(tline *line, size_t col, size_t len);

bool editor_create_first_new_line(teditor *editor);
bool editor_init(teditor *editor);
bool editor_deinit(teditor *editor);
bool editor_load_from_file(teditor *editor, const char *file_path);
bool editor_save_to_file(teditor *editor, const char *file_path);
bool editor_insert_newline(teditor *editor);
bool editor_cursorCanRight(teditor *editor);
bool editor_cursorCanLeft(teditor *editor);
bool editor_insert_text(teditor *editor, const char *text, size_t len);
bool editor_delete_after_text(teditor *editor, size_t len);
bool editor_delete_back_text(teditor *editor, size_t len);

#endif // _EDITOR_H
