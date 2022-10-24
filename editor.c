#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <errno.h>
#include "editor.h"
#include "sv.h"

#define EDITOR_INIT_CAPACITY  128

bool line_insert_text(tline *line, size_t col, const char *text, size_t len)
{
	if(col > line->len) col = line->len;
	if(len > line->size - line->len) {
		if(line->size == 0) line->size = 1024;
		line->buf = realloc(line->buf, line->size * 2);
		line->size *= 2;
	}
	memmove(line->buf + col + len, line->buf + col, line->len - col);
	memcpy(line->buf + col, text, len);
	line->len += len;
	return true;
}

bool line_append_text(tline *line, const char *text, size_t len)
{
	size_t col = line->len;
	return line_insert_text(line, col, text, len);
}

bool line_delete_after_text(tline *line, size_t col, size_t len)
{
	if(col <= line->len) {
		if(col + len > line->len) len = line->len - col;
		memmove(line->buf + col, line->buf + col + len, len);
		line->len -= len;
	}
	return true;
}
bool line_delete_back_text(tline *line, size_t col, size_t len)
{
	if(len > col) len = col;
	if(len <= line->len) {
		memmove(line->buf + col - len, line->buf + col, line->len - col);
		line->len -= len;
	}
	return true;
}

bool editor_init(teditor *editor)
{
	editor->cursor_row = 0;
	editor->cursor_col = 0;
	editor->size = 100;
	editor->lines = calloc(sizeof(tline), editor->size);
	return true;
}
bool editor_deinit(teditor *editor)
{
	for(size_t i = 0; i < editor->size; i++) {
		free(editor->lines[i].buf);
	}
	if(editor->lines) free(editor->lines);
	return true;
}

void editor_grow(teditor *editor, size_t n)
{
    size_t new_capacity = editor->capacity;

    assert(new_capacity >= editor->size);
    while (new_capacity - editor->size < n) {
        if (new_capacity == 0) {
            new_capacity = EDITOR_INIT_CAPACITY;
        } else {
            new_capacity *= 2;
        }
    }

    if (new_capacity != editor->capacity) {
        editor->lines = realloc(editor->lines, new_capacity * sizeof(editor->lines[0]));
        editor->capacity = new_capacity;
    }
}
bool editor_create_first_new_line(teditor *editor)
{
	if(editor->cursor_row >= editor->size)
	{
		if(editor->size > 0)
		{
			editor->cursor_row = editor->size - 1;
		}
		else
		{
			editor_grow(editor, 1);
			memset(&editor->lines[editor->size], 0, sizeof(editor->lines[0]));
			editor->size += 1;
		}
	}
	return true;
}

bool editor_load_from_file(teditor *editor, const char *file_path)
{
	assert(editor->lines == NULL && "You can only load files into an empty editor");
	FILE *f = fopen(file_path, "r");
	if(NULL == f)
	{
		fprintf(stderr, "ERROR: could not open file `%s` : %s\n", file_path, strerror(errno));
		exit(1);
	}
	editor_create_first_new_line(editor);

	static char chunk[640 * 1024];
	while(!feof(f))
	{
		size_t n = fread(chunk, 1, sizeof(chunk), f);
		String_View chunk_sv = { .data = chunk, .count = n };
		while(chunk_sv.count > 0)
		{
			String_View chunk_line = {0};
			tline *line = &editor->lines[editor->size - 1];
			if(sv_try_chop_by_delim(&chunk_sv, '\n', &chunk_line))
			{
				line_append_text(line, chunk_line.data, chunk_line.count);
				editor_insert_newline(editor);
			}
			else
			{
				line_append_text(line, chunk_sv.data, chunk_sv.count);
				chunk_sv = SV_NULL;
			}
		}
	}
	editor->cursor_row = 0;
	fclose(f);
	return true;
}
bool editor_save_to_file(teditor *editor, const char *file_path)
{
	FILE *f = fopen(file_path, "w");
	if(f == NULL) {
		fprintf(stderr, "open %s to write failed: %s\n", file_path, strerror(errno));
		return false;
	}
	for(size_t i = 0; i < editor->size; i++) {
		tline *line = &editor->lines[i];
		fwrite(line->buf, 1, line->len, f);
		fwrite("\n", 1, 1, f);
	}
	fclose(f);
	return true;
}
bool editor_insert_newline(teditor *editor)
{
	if(editor->size + 1 > editor->capacity) {
		editor->lines = realloc(editor->lines, editor->capacity* 2 * sizeof(editor->lines[0]));
		memset(&editor->lines[editor->capacity], 0, editor->capacity * sizeof(editor->lines[0]));
		editor->capacity *= 2;
	}
	if(editor->cursor_row > editor->size)
	{
		editor->cursor_row = editor->size;
	}
//	editor_grow(editor, 1);

	memmove(editor->lines + editor->cursor_row + 1, editor->lines + editor->cursor_row, editor->size - editor->cursor_row - 1);
	memset(&editor->lines[editor->cursor_row + 1], 0, sizeof(editor->lines[0]));
	editor->cursor_row++;
	editor->cursor_col = 0;
	editor->size++;
	return true;
}
bool editor_cursorCanRight(teditor *editor)
{
	if(editor->cursor_col < editor->lines[editor->cursor_row].len)
		return true;
	else
		return false;
}
bool editor_cursorCanLeft(teditor *editor)
{
	if(editor->cursor_col)
		return true;
	else
		return false;
}
bool editor_insert_text(teditor *editor, const char *text, size_t len)
{
	size_t row = editor->cursor_row;
	size_t col = editor->cursor_col;

	tline *line = &editor->lines[row];
	line_insert_text(line, col, text, len);
	editor->cursor_col += len;

	return true;
}
bool editor_delete_after_text(teditor *editor, size_t len)
{
	size_t row = editor->cursor_row;
	size_t col = editor->cursor_col;
	if(row < editor->size ) {
		tline *line = &editor->lines[row];
		line_delete_after_text(line, col, len);
	}
	return true;
}
bool editor_delete_back_text(teditor *editor, size_t len)
{
	size_t row = editor->cursor_row;
	size_t col = editor->cursor_col;
	if(row < editor->size) {
		tline *line = &editor->lines[row];
		line_delete_back_text(line, col, len);
		if(editor->cursor_col < len) editor->cursor_col = 0;
		else editor->cursor_col -= len;
	}
	return true;
}


#define SV_IMPLEMENTATION
#include "sv.h"
