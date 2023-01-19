/*
 * This source file is part of the bbcode library.
 * Written and maintained by Xavier De Cock 2006-2010
 * Licensed under the BSD License Terms
 * Refer to the accompanying documentation for details on usage and license.
 * See also: Company Website: http://www.nxdomain.be/
 * See also: Hosted on pecl: http://pecl.php.net/
 * Leave this header As Is, add your name as maintainer, and please, contribute
 * enhancement back to the community
 * Revision : $Id$
 */
#ifndef BBCODE_H_
#define BBCODE_H_
#include "bstrlib.h"
#define BBCODE_LIB_VERSION "2.0"

#define BBCODE_BUFFER 4

#define BBCODE_TYPE_NOARG               1
#define BBCODE_TYPE_SINGLE              2
#define BBCODE_TYPE_ARG                 3
#define BBCODE_TYPE_OPTARG              4
#define BBCODE_TYPE_ROOT                5

#define BBCODE_FLAGS_ARG_PARSING        0x1
#define BBCODE_FLAGS_CDATA_NOT_ALLOWED  0x2
#define BBCODE_FLAGS_SMILEYS_ON         0x4
#define BBCODE_FLAGS_SMILEYS_OFF        0x8
#define BBCODE_FLAGS_ONE_OPEN_PER_LEVEL 0x10
#define BBCODE_FLAGS_REMOVE_IF_EMPTY	0x20
#define BBCODE_FLAGS_DENY_REOPEN_CHILD	0x40

#define BBCODE_ARG_DOUBLE_QUOTE    0x1
#define BBCODE_ARG_SINGLE_QUOTE    0x2
#define BBCODE_ARG_HTML_QUOTE      0x4
#define BBCODE_ARG_QUOTE_ESCAPING  0x10
#define BBCODE_AUTO_CORRECT        0x100
#define BBCODE_CORRECT_REOPEN_TAGS 0x200
#define BBCODE_DEFAULT_SMILEYS_ON  0x400
#define BBCODE_DEFAULT_SMILEYS_OFF 0x800
#define BBCODE_FORCE_SMILEYS_OFF   0x1000
#define BBCODE_DISABLE_TREE_BUILD  0x2000
#define BBCODE_SMILEYS_CASE_INSENSITIVE 0x4000

#define BBCODE_CACHE_ACCEPT_ARG             0x01
#define BBCODE_CACHE_ACCEPT_NOARG           0x02
#define BBCODE_CACHE_START_HAS_BRACKET_OPEN 0x04
#define BBCODE_CACHE_END_HAS_BRACKET_OPEN   0x08
#define BBCODE_CACHE_ACCEPT_SMILEYS         0x10

#define BBCODE_ALLOW_LIST_TYPE_ALL     0
#define BBCODE_ALLOW_LIST_TYPE_NONE    1
#define BBCODE_ALLOW_LIST_TYPE_LISTED  2
#define BBCODE_ALLOW_LIST_TYPE_EXCLUDE 3

#define BBCODE_TREE_CHILD_TYPE_TREE 0
#define BBCODE_TREE_CHILD_TYPE_STRING 1

#define BBCODE_LIST_IS_READY    1
#define BBCODE_LIST_HAS_ROOT    2

#define BBCODE_TREE_FLAGS_PAIRED 0x1
#define BBCODE_TREE_FLAGS_MULTIPART 0x2
#define BBCODE_TREE_FLAGS_MULTIPART_FIRST_NODE 0x4
#define BBCODE_TREE_FLAGS_MULTIPART_LAST_NODE 0x8
#define BBCODE_TREE_FLAGS_MULTIPART_DONE 0x10
#define BBCODE_TREE_FLAGS_ROOT 0x20
#define BBCODE_TREE_ROOT_TAGID -1

#define BBCODE_ERR -2

#define bbcode_get_bbcode(parser, pos)      ((pos == BBCODE_TREE_ROOT_TAGID)?(parser)->bbcodes->root : (parser)->bbcodes->bbcodes->element[(pos)])
#define bbcode_get_cn(parser)               ((parser)->current_node)
#define bbcode_array_length(array)          (((array) == (void *)0 || (array)->size < 0) ? (long)0 : ((long)(array)->size))
#define bbcode_array_element(array, pos)    ((((unsigned)(pos)) < (unsigned)bbcode_array_length(array)) ? ((array)->element[(pos)]) : NULL)
#define bbcode_find_next(to_update,string,offset,char) if (to_update <= offset){ if (0>(to_update = bstrchrp( string, char, offset))){ to_update = blength( string )+5; } }
#define BBCODE_SPECIAL_CASE_NO_CHILD(argument) \
    int sc_offset; \
    bstring close_tag=bfromcstr("[/"); \
    bconcat(close_tag,tag); \
    bcatcstr(close_tag,"]"); \
    sc_offset=binstrcaseless(string, next_close, close_tag); \
    if (sc_offset!=BSTR_ERR){ \
        bbcode_tree_push_tree_child(parser, bbcode_get_cn(parser), work_stack, close_stack, bmidstr(string, offset, end-offset+1),tag_id, argument, offset); \
        bbcode_tree_push_string_child(bbcode_get_cn(parser), bmidstr(string,next_close+1,sc_offset-next_close-1), offset+next_close+1); \
        bbcode_close_tag(parser, bbcode_get_cn(parser), work_stack, close_stack, tag_id, bmidstr(string, sc_offset, blength(close_tag)),1, sc_offset); \
        added=1; \
        end=next_close=sc_offset+blength(close_tag)-1; \
    } \
    bdestroy(close_tag);
#define bbcode_apply_flag_to_parts(multipart, flag, add) { \
	long i; \
	if (multipart!=NULL) { \
		if (add) { \
			for (i=0; i<multipart->size; i++) { \
				multipart->element[i]->flags |= flag; \
			} \
		} else { \
			for (i=0; i<multipart->size; i++) { \
				multipart->element[i]->flags &= ~flag; \
			} \
		} \
	} \
}
#define bbcode_max(a,b) ((a>b)?a:b) 

typedef struct _bbcode_smiley bbcode_smiley;
typedef struct _bbcode_smiley * bbcode_smiley_p;
typedef struct _bbcode_smiley_array bbcode_smiley_list;
typedef struct _bbcode_smiley_array * bbcode_smiley_list_p;
typedef struct _bbcode_allow_list bbcode_allow_list;
typedef struct _bbcode_allow_list * bbcode_allow_list_p;
typedef struct _bbcode bbcode;
typedef struct _bbcode * bbcode_p;
typedef struct _bbcode ** bbcode_pp;
typedef struct _bbcode_search bbcode_search;
typedef struct _bbcode_search * bbcode_search_p;
typedef struct _bbcode_search ** bbcode_search_pp;
typedef struct _bbcode_array bbcode_array;
typedef struct _bbcode_array * bbcode_array_p;
typedef struct _bbcode_list bbcode_list;
typedef struct _bbcode_list * bbcode_list_p;
typedef struct _bbcode_parser bbcode_parser;
typedef struct _bbcode_parser * bbcode_parser_p;
typedef struct _bbcode_parse_tree bbcode_parse_tree;
typedef struct _bbcode_parse_tree * bbcode_parse_tree_p;
typedef struct _bbcode_parse_tree ** bbcode_parse_tree_pp;
typedef struct _bbcode_parse_tree_array bbcode_parse_tree_array;
typedef struct _bbcode_parse_tree_array* bbcode_parse_tree_array_p;
typedef struct _bbcode_parse_tree_child bbcode_parse_tree_child;
typedef struct _bbcode_parse_tree_child * bbcode_parse_tree_child_p;
typedef struct _bbcode_parse_tree_child ** bbcode_parse_tree_child_pp;
typedef struct _bbcode_parse_tree_child_array bbcode_parse_tree_child_array;
typedef struct _bbcode_tag_id_search bbcode_tag_id_search;
typedef struct _bbcode_tag_id_search * bbcode_tag_id_search_p;
typedef struct _bbcode_validation bbcode_validation;
typedef struct _bbcode_validation * bbcode_validation_p;
typedef struct _bbcode_validation_array bbcode_validation_array;
typedef struct _bbcode_validation_array * bbcode_validation_array_p;
/* This represent a smiley list */
struct _bbcode_smiley_array {
	long size;
	long msize;
	bbcode_smiley_p smileys;
	char ci;
};

/* Represents a set of bbcode rules */
struct _bbcode_array {
	long size;
	long msize;
	bbcode_pp element;
};

/* Represent an array of parse tree */
struct _bbcode_parse_tree_array {
	long size;
	long msize;
	bbcode_parse_tree_pp element;
};

/* Represents an array of parse_tree_child */
struct _bbcode_parse_tree_child_array {
	long size;
	long msize;
	bbcode_parse_tree_child_pp element;
};

/* This represent a single smiley with search / replace */
struct _bbcode_smiley {
	bstring search;
	bstring replace;
};

/* This represent a list of allowed tags */
struct _bbcode_allow_list {
	long *id_list;
	char type;
	long size;
	long msize;
};

/* This represent a BBCode Tag Rule Set */
struct _bbcode {
	char type;
	long flags;
	char speed_cache;
	long max_parsed;
	bstring tag;
	bstring open_tag;
	bstring close_tag;
	bstring default_arg;
	bstring parent_list;
	bstring child_list;
	bbcode_allow_list_p parents;
	bbcode_allow_list_p childs;
	void *param_handling_func_data;
	void *content_handling_func_data;
	int (*param_handling_func)(bstring content, bstring param, void *func_data);
	int (*content_handling_func)(bstring content, bstring param,
			void *func_data);
};

/* This represent a complete BBCode Parsing Rule Set */
struct _bbcode_list {
	long options;
	long bbcode_max_size;
	bbcode_array_p bbcodes;
	bbcode_p root;
	bbcode_search_pp search_cache;
	long *num_cache;
};

/* This is the bbcode parser */
struct _bbcode_parser {
	bbcode_parser_p argument_parser;
	bbcode_smiley_list_p smileys;
	bbcode_list_p bbcodes;
	bbcode_parse_tree_p current_node;
	bstring content_replace;
	bstring arg_replace;
	long options;
	long *tag_counter;
};

/* This is the parse tree temporary data store */
struct _bbcode_parse_tree {
	long tag_id;
	long flags;
	bbcode_parse_tree_child_array childs;
	bbcode_parse_tree_array_p multiparts;
	bbcode_parse_tree_array_p conditions;
	bbcode_parse_tree_p parent_node;
	bstring open_string;
	bstring close_string;
	bstring argument;
};

/* This is a single token found by parsing (in fact a token is often splitted with partial matches) */
struct _bbcode_parse_tree_child {
	union {
		bbcode_parse_tree_p tree;
		bstring string;
	};
	long offset;
	char type;
};

/* The tagId search cache */
struct _bbcode_search {
	bstring tag_name;
	long tag_id;
};

/* BBCode Validation error entries */
struct _bbcode_validation {
	char error_type;
	long tag_id_1;
	long tag_id_2;
	long offset_1;
	long offset_2;
};

/* BBCode Validation array entries */
struct _bbcode_validation_entry {
	long msize;
	long size;
	bbcode_validation_p element;
};

/*---------------------------
 Public API
 ---------------------------*/
/* Create and init a parser */
bbcode_parser_p bbcode_parser_create();

/* Destroy a parser and associated resources */
void bbcode_parser_free(bbcode_parser_p parser);

/* Destroy a parser and associated resources */
void bbcode_parser_set_arg_parser(bbcode_parser_p parser,
		bbcode_parser_p arg_parser);

/* Constructs and add a bbcode_element to the parser */
void bbcode_parser_add_ruleset(bbcode_parser_p parser, long type, long flags,
		char *tag, int tag_size,
		char *open_tag, int open_tag_size, char *close_tag, int close_tag_size,
		char *default_arg, int default_arg_size, char *parent_list,
		int parent_list_size, char *child_list, int child_list_size, long max_parsed,
		int (*param_handling_func)(bstring content, bstring param, void *func_data), 
		int (*content_handling_func)(bstring content, bstring param, void *func_data),
		void *param_handling_func_data, void *content_handling_func_data);

/* Construct and add a smiley to the parser */
void bbcode_parser_add_smiley(bbcode_parser_p parser, char *smiley_search,
		int smiley_search_size, char *smiley_replace, int smiley_replace_size);
	
/* Parse a BBCoded string to is treated equivalent */
char *bbcode_parse(bbcode_parser_p parser, char *string, int string_size,
		int *result_size);

/* Get current options of the bbcode_parser */
long bbcode_parser_get_flags(bbcode_parser_p parser);

/* Set options for the bbcode_parser */
void bbcode_parser_set_flags(bbcode_parser_p parser, long flags);

/*---------------------------
 Internal API
 ---------------------------*/
/* Parse nesting rules and optimize datas */
void bbcode_prepare_tag_list(bbcode_parser_p parser);

/* This reparse nesting rules and optimize datas */
void bbcode_build_tree(bbcode_parser_p parser, bstring string,
		bbcode_parse_tree_p tree);

/* This closes an active tag */
void bbcode_close_tag(bbcode_parser_p parser, bbcode_parse_tree_p tree,
		bbcode_parse_tree_array_p work, bbcode_parse_tree_array_p close,
		long tag_id, bstring close_string, int true_close, int offset);

/* This make some basic corrections to a given tree */
int bbcode_correct_tree(bbcode_parser_p parser, bbcode_parse_tree_p tree,
		long parent_id, char force_false);

/* This apply the BBCode rules to generate the final string */
void bbcode_apply_rules(bbcode_parser_p parser, bbcode_parse_tree_p tree,
		bstring parsed);

/* Search a tag_id from the string */
long bbcode_get_tag_id(bbcode_parser_p parser, bstring value, long has_arg);

/* Translate Smileys */
void bbcode_parse_smileys(bstring string, bbcode_smiley_list_p list);

/*---------------------------
 Smiley Manipulation API
 ---------------------------*/
/* Initialize a smiley list */
bbcode_smiley_list_p bbcode_smileys_list_create();

/* Free a smiley list */
void bbcode_smileys_list_free(bbcode_smiley_list_p list);

/* Check if we can add an entry */
void bbcode_smiley_list_check_size(bbcode_smiley_list_p list, long size);

/* adds a smiley to the list */
void bbcode_smileys_add(bbcode_smiley_list_p list, bstring search,
		bstring replacement);

/*---------------------------
 BBCode List Manipulation API
 ---------------------------*/
/* creates a BBcode list and init it */
bbcode_list_p bbcode_list_create();

/* free resources for a BBCode list */
void bbcode_list_free(bbcode_list_p list);

/* Check if there is room for a bbcode entry */
void bbcode_list_check_size(bbcode_list_p list, long size);

/* Insert the special entry "Root" */
void bbcode_list_set_root(bbcode_list_p list, bbcode_p root);

/* add a bbcode to a list */
void bbcode_list_add(bbcode_list_p list, bbcode_p to_add);

/*---------------------------
 BBCode Array Manipulation API
 ---------------------------*/
/* creates a BBcode array and init it */
bbcode_array_p bbcode_array_create();

/* Free a BBCode array */
void bbcode_array_free(bbcode_array_p array);

/* Check if we can add an entry */
void bbcode_array_check_size(bbcode_array_p array, long size);

/* adds a bbcode_rule to the list */
void bbcode_array_add(bbcode_array_p array, bbcode_p bbcode);

/*---------------------------
 BBCode Entry Manipulation API
 ---------------------------*/
/* Malloc a bbcode entry and init it */
bbcode_p bbcode_entry_create();

/* Free a bbcode entry resources */
void bbcode_entry_free(bbcode_p entry);

/*---------------------------
 BBCode Allow Manipulation API
 ---------------------------*/
/* Malloc a bbcode_allow_list and init it */
bbcode_allow_list_p bbcode_allow_list_create();

/* Free the resources taken by an allow list */
void bbcode_allow_list_free(bbcode_allow_list_p list);

/* Check for the size of an allow list */
void bbcode_allow_list_check_size(bbcode_allow_list_p list, long size);

/* Add an element to the list */
void bbcode_allow_list_add(bbcode_allow_list_p list, long element);

/* Check if a given id is autorized */
long bbcode_allow_list_check_access(bbcode_allow_list_p list, long tag_id);

/* Check if a list does accept any child */
long bbcode_allow_list_no_child(bbcode_allow_list_p list);

/*---------------------------
 Tree Manipulation API
 ---------------------------*/
/* Malloc and init a bbcode tree  */
bbcode_parse_tree_p bbcode_tree_create();

/* Free the resources taken by a tree */
void bbcode_tree_free(bbcode_parse_tree_p tree);

/* Check if there is sufficient space in child array */
void bbcode_tree_check_child_size(bbcode_parse_tree_p tree, long size);

/* adds a child to the current list (sub_tree) */
void bbcode_tree_push_tree_child(bbcode_parser_p parser,
		bbcode_parse_tree_p tree, bbcode_parse_tree_array_p work,
		bbcode_parse_tree_array_p close, bstring open_string, long tag_id,
		bstring argument, long offset);

/* adds a child to the current list (string_leaf) */
void bbcode_tree_push_string_child(bbcode_parse_tree_p tree, bstring string, long offset);

/* adds a tree to the current list (raw) */
void bbcode_tree_push_tree_raw(bbcode_parser_p parser, bbcode_parse_tree_p tree,
		bbcode_parse_tree_p tmp_tree, bbcode_parse_tree_array_p work);

/* Get the last child and removes it from the list */
void bbcode_tree_pop_child(bbcode_parse_tree_p tree,
		bbcode_parse_tree_child_p bbcode_parse_tree_child);

/* Insert a given child on a given position */
void bbcode_tree_insert_child_at(bbcode_parse_tree_p tree,
		bbcode_parse_tree_child_p bbcode_parse_tree_child, long pos);

/* Mark an element closed, (and also multipart elements) */
void bbcode_tree_mark_element_closed(bbcode_parse_tree_p tree);

/* Move a child set from a parent to another */
void bbcode_tree_move_childs(bbcode_parse_tree_p from, bbcode_parse_tree_p to,
		long offset_from, long count, long offset_to);

/*---------------------------
 Parse Stack Manipulation API
 ---------------------------*/
/* Create a Tree array */
bbcode_parse_tree_array_p bbcode_parse_stack_create();

/* Free resource used by a Tree array */
void bbcode_parse_stack_free(bbcode_parse_tree_array_p stack);

/* Check if there is room for adding elements */
void bbcode_parse_stack_check_size(bbcode_parse_tree_array_p stack, long size);

/* Add element to the Tree array */
void bbcode_parse_stack_push_element(bbcode_parse_tree_array_p stack,
		bbcode_parse_tree_p element);

/* Remove element from the Tree array  without giving it back */
void bbcode_parse_stack_pop_element_loose(bbcode_parse_tree_array_p stack);

/* Remove element from the Tree array @ index */
void bbcode_parse_drop_element_at(bbcode_parse_tree_array_p stack, long index);

/* Init a tree child */
bbcode_parse_tree_child_p bbcode_tree_child_create();

/* Free a tree child */
void bbcode_tree_child_destroy(bbcode_parse_tree_child_p child);

/* Check if a string is escaped */
int bbcode_is_escaped(bstring string, int pos);

/* Remove escaped strings */
int bbcode_strip_escaping(bstring string);

/*---------------------------
 Built-in callbacks
 ---------------------------*/
#endif /*BBCODE_H_*/
