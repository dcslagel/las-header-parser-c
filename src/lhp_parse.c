/*
 * File-Name: lhp_parce.c
 * File-Desc: parsing functions in lh_parse
 * App-Name: lh_parser
 * Project-Name: Las-Header-Parser
 * Copyright: Copyright (c) 2020, DC Slagel
 * License-Identifier: MIT
 */

#include <ctype.h> // iscntrl(), isspace()
#include <stdio.h> // FILE, fclose(), fopen(), fprintf(), perror()
#include <stdlib.h> // malloc()
#include <string.h> // strlen(), strncmp(), strncpy(),


#include "lhp_parse.h"

/* module private variables : start */
/* use enum to make line_max constant */
enum { line_max = 1024 };
enum { record_max = 6 };
static char *lhp_section_type = NULL;
static char *lhp_field_buffer = NULL;
static char *mnemonic_name = NULL;
static char *unit = NULL;
static char *value = NULL;

struct record_fields {
    char *mnemonic_name;
    char *unit;
    char *value;
    char *desc;
};

static struct record_fields record_array[record_max];
// consider replacing record_array with a record_linked_list
// struct record_list {
//    struct record_fields record;
//    record_list *next;
// };

/* module private variables : finis */

/* module private functions : start */
static FILE *open_lhp_file(const char *lhp_filename);
static void clean_up_end_of_line(char *line);
static void parse_section_type(char *line);
/* Unused functions
static void parse_mnemonic_name(char *line_iter, size_t rec_idx);
static void parse_unit(char *line_iter, size_t rec_idx);
static void parse_value(char *line_iter, size_t rec_idx);
static void parse_desc(char *line_iter, size_t rec_idx);
*/
static void free_records(void);
static void display_record(void);
/* module private functions : finis */

void read_las_file(const char *lhp_filename)
{
    FILE *fp;
    char line[line_max];
    /* unused variables
    size_t rec_idx = 0;
    char field_id = '\0';
    char *line_iter;
    */

    fp = open_lhp_file(lhp_filename);

    if (fp == NULL) {
        return;
    }

    // TODO: move fgets() to my_io.c and wrap in my_fgets.c
    while (fgets(line, line_max, fp))
    {

        clean_up_end_of_line(line);

        parse_section_type(line);

        size_t line_len = strlen(line);
        size_t line_idx = 0;

        // Canidate content for fields: name, unit, value, desc
        lhp_field_buffer = malloc(line_len);
        if (lhp_field_buffer == NULL) {
            perror("strncpy");
            exit(EXIT_FAILURE);
        }

        // Skip spaces at the beginning of the string.
        while (isspace(line[line_idx])) {
          line_idx++;
        }

        // ---------------------------------------------------------------------
        // Parse Mnemonic name.
        // ---------------------------------------------------------------------
        size_t mnemonic_idx = 0;
        while (line[line_idx] != '.') {
          lhp_field_buffer[mnemonic_idx] = line[line_idx];
          line_idx++;
          mnemonic_idx++;
        }

        // Move line_idx past the '.' delimiter.
        line_idx++;

        mnemonic_name = malloc(strlen(lhp_field_buffer) + 1);
        strncpy(mnemonic_name, lhp_field_buffer, strlen(lhp_field_buffer) + 1);
        size_t mnemonic_reverse_idx = strlen(mnemonic_name) - 1;
        while (mnemonic_name[mnemonic_reverse_idx] == ' ') {
          mnemonic_name[mnemonic_reverse_idx] = '\0';
          mnemonic_reverse_idx--;
        }

        // ---------------------------------------------------------------------
        // Parse unit.
        // ---------------------------------------------------------------------

        // Clear lhp_field_buffer.
        memset(lhp_field_buffer,0,strlen(lhp_field_buffer));

        size_t unit_idx = 0;
        while (line[line_idx] != ' ') {
          lhp_field_buffer[unit_idx] = line[line_idx];
          line_idx++;
          unit_idx++;
        }

        // Move line_idx past the ' ' delimiter.
        line_idx++;

        unit = malloc(strlen(lhp_field_buffer) + 1);
        strncpy(unit, lhp_field_buffer, strlen(lhp_field_buffer) + 1);

        // ---------------------------------------------------------------------
        // Parse value.
        // ---------------------------------------------------------------------

        // Clear lhp_field_buffer.
        memset(lhp_field_buffer,0,strlen(lhp_field_buffer));

        size_t value_idx = 0;

        // Move past spaces at the beginning of the value field.
        while (line[line_idx] == ' ') {
          printf("LINE-IDX: [%zu]\n", line_idx);
          line_idx++;
        }

        // Save value field into buffer
        while (line[line_idx] != ':') {
          lhp_field_buffer[value_idx] = line[line_idx];
          line_idx++;
          value_idx++;
        }

        // Move line_idx past the ':' delimiter.
        line_idx++;

        value = malloc(value_idx);
        strncpy(value, lhp_field_buffer, value_idx);
        value[value_idx] = '\0';

        // ---------------------------------------------------------------------
        // Display fields
        // ---------------------------------------------------------------------

        printf("#----------------------------------------#\n");
        printf("Record: [%s]\n", line);
        // %zu: size_t
        printf("Record-Size:  [%zu]\n", strlen(line));
        printf("Record-Size:  [%zu]\n", sizeof(line));
        printf("Mnemonic: [%zu]\n", sizeof(mnemonic_name));
        printf("Mnemonic: [%s]\n", mnemonic_name);
        printf("Unit: [%zu]\n", strlen(unit));
        printf("Unit: [%s]\n", unit);
        printf("Value: [%zu]\n", strlen(value));
        printf("Value: [%s]\n", value);
        printf("#----------------------------------------#\n");
        return;

        /*
        // Parse entry fields
        field_id = line[0];
        line_iter = line;
        line_iter++;

        switch (field_id)
        {
            case 'D':
                parse_mnemonic_name(line_iter, rec_idx);
                break;
            case 'U':
                parse_unit(line_iter, rec_idx);
                break;
            case 'T':
                parse_value(line_iter, rec_idx);
                break;
            case 'N':
                parse_desc(line_iter, rec_idx);
                break;
            default:
                break;
        }

        // clear line for next input
        line[0] = '\0';
        */
    }

    if (fclose(fp) == EOF) {
        perror("close");
    }

    display_record();

    free_records();
}

static void parse_section_type(char *line)
{
    size_t line_len = strlen(line);
    // Get the section type
    if (strncmp("~", line, 1) == 0) {
        lhp_section_type = malloc(line_len);

        if (lhp_section_type != NULL) {
            strncpy(lhp_section_type, line, line_len);
            if (lhp_section_type == NULL) {
                perror("strncpy");
                exit(EXIT_FAILURE);
            }
        }
        printf("Section Type: [%s]\n\n", lhp_section_type);
    }
}

/* Unused functions.
void parse_mnemonic_name(char *line_iter, size_t rec_idx)
{
    record_array[rec_idx].mnemonic_name = (char *)malloc(strlen(line_iter));
    strncpy(record_array[rec_idx].mnemonic_name, line_iter, strlen(line_iter));
}

void parse_unit(char *line_iter, size_t rec_idx)
{
    record_array[rec_idx].unit = (char *)malloc(strlen(line_iter));
    strncpy(record_array[rec_idx].unit, line_iter, strlen(line_iter));
}

void parse_value(char *line_iter, size_t rec_idx)
{
    record_array[rec_idx].value = (char *)malloc(strlen(line_iter));
    strncpy(record_array[rec_idx].value, line_iter, strlen(line_iter));
}

void parse_desc(char *line_iter, size_t rec_idx)
{
    record_array[rec_idx].desc = (char *)malloc(strlen(line_iter));
    strncpy(record_array[rec_idx].desc, line_iter, strlen(line_iter));
}
*/



FILE *open_lhp_file(const char *lhp_filename)
{
    FILE *result;
    result = fopen(lhp_filename, "r");

    if (result == NULL)
    {
        fprintf(
            stdout,
            "INFO: config file [%s] not found. Using default settings.\n",
            lhp_filename);
        /* perror("fopen"); */
    }
    return result;
}


static void clean_up_end_of_line(char *line)
{
    // Remove spaces and new-lines from the end of the line
    size_t line_len = strlen(line);
    for (size_t idx = line_len ; idx >= 1; idx--) {
        if (iscntrl(line[idx]) || isspace(line[idx])) {
            line[idx] = '\0';
        }
        else {
            break;
        }
    }
}


void free_records(void)
{
    for (size_t free_idx = 0; free_idx < record_max; free_idx++) {
        // printf("releasing memory for rec_idx [%ld]\n", free_idx);
        free(record_array[free_idx].mnemonic_name);
        free(record_array[free_idx].unit);
        free(record_array[free_idx].value);
        free(record_array[free_idx].desc);
    }
}


void display_record(void)
{
    for (size_t idx = 0; idx < record_max; idx++) {
        // display record
        printf("#----------------------------------------#\n");
        printf("Record: [%zu]\n", idx);
        printf("#----------------------------------------#\n");
        printf("Mnemonic: [%s]\n", record_array[idx].mnemonic_name);
        printf("Unit: [%s]\n", record_array[idx].unit);
        printf("Value: [%s]\n", record_array[idx].value);
        printf("Description: [%s]\n", record_array[idx].desc);
    }
}


