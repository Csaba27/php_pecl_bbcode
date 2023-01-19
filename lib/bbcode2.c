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
#include <stdlib.h>
#include <stdio.h>
#ifdef __APPLE__
#include <sys/malloc.h>
#else
#include <malloc.h>
#endif
#include "bbcode2.h"
#include "bstrlib.h"

/*---------------------------*
 *        Public API		 *
 *---------------------------*/
/* Create and init a parser  */
bbcode_parser_p bbcode_parser_create() {
	bbcode_parser_p parser;
	parser=(bbcode_parser_p)malloc(sizeof(bbcode_parser));
	parser->options=0;
	parser->bbcodes=bbcode_list_create();
	parser->smileys=bbcode_smileys_list_create();
	parser->argument_parser=NULL;
	parser->content_replace = bfromcstr("{CONTENT}");
	parser->arg_replace = bfromcstr("{PARAM}");
	return parser;
}

/* Destroy a parser and associated resources */
void bbcode_parser_free(bbcode_parser_p parser) {
	bbcode_list_free(parser->bbcodes);
	bbcode_smileys_list_free(parser->smileys);
	bdestroy(parser->content_replace);
	bdestroy(parser->arg_replace);
	free(parser);
}

/* Set the argument Parser */
void bbcode_parser_set_arg_parser(bbcode_parser_p parser,
		bbcode_parser_p arg_parser) {
	parser->argument_parser=arg_parser;
}

/* Constructs and add a bbcode_element to the parser */
void bbcode_parser_add_ruleset(bbcode_parser_p parser, long type, long flags,
		char *tag, int tag_size,
		char *open_tag, int open_tag_size, char *close_tag, int close_tag_size,
		char *default_arg, int default_arg_size, char *parent_list,
		int parent_list_size, char *child_list, int child_list_size, long max_parsed,
		int (*param_handling_func)(bstring content, bstring param, void *func_data), 
		int (*content_handling_func)(bstring content, bstring param, void *func_data),
		void *param_handling_func_data, void *content_handling_func_data) {
	bbcode_p entry=NULL;
	if (tag_size==0){
		entry=parser->bbcodes->root;
	} else {
		entry=bbcode_entry_create();
	}
	entry->type=type;
	entry->max_parsed=max_parsed;
	entry->flags=flags;
	entry->tag=blk2bstr(tag, tag_size);
	entry->open_tag=blk2bstr(open_tag, open_tag_size);
	entry->close_tag=blk2bstr(close_tag, close_tag_size);
	entry->default_arg=blk2bstr(default_arg, default_arg_size);
	entry->parent_list=blk2bstr(parent_list, parent_list_size);
	entry->child_list=blk2bstr(child_list, child_list_size);
	entry->param_handling_func_data=param_handling_func_data;
	entry->content_handling_func_data=content_handling_func_data;
	entry->param_handling_func=param_handling_func;
	entry->content_handling_func=content_handling_func;
	if (tag_size!=0){
		bbcode_list_add(parser->bbcodes, entry);
	}
}

/* Construct and add a smiley to the parser */
void bbcode_parser_add_smiley(bbcode_parser_p parser, char *smiley_search,
		int smiley_search_size, char *smiley_replace, int smiley_replace_size) {
	bstring search = NULL;
	bstring replace = NULL;
	search=blk2bstr(smiley_search, smiley_search_size);
	replace=blk2bstr(smiley_replace, smiley_replace_size);
	bbcode_smileys_add(parser->smileys, search, replace);
}

/* Parse a BBCoded string to is treated equivalent */
char *bbcode_parse(bbcode_parser_p parser, char *string, int string_size,
		int *result_size) {
	char *return_value;
	int i;

	bstring to_parse = NULL;
	bstring parsed = NULL;
	to_parse=bfromcstr("");
	parsed=bfromcstr("");
	/* Init */
	if (parser->options & BBCODE_DISABLE_TREE_BUILD) {
		/* No BBCode Parsing */
		if (parser->options & BBCODE_FORCE_SMILEYS_OFF) {
			char *return_value=(char *)malloc(string_size * sizeof(char));

			/* No Smiley Treatment */
			*result_size=string_size;
			return memcpy(return_value, string, string_size);
		}
		/* Prepare Datas for smiley */
		balloc(to_parse, string_size+5);
		to_parse->slen=string_size;
		to_parse->data=memcpy(to_parse->data, string, string_size);
		/* Smiley Parsing */
		bbcode_parse_smileys(to_parse, parser->smileys);
	} else {
		/*  starting the tree */
		bbcode_parse_tree_p tree = bbcode_tree_create();

		/* Prepare Datas for parsing */
		balloc(to_parse, string_size+5);
		to_parse->slen=string_size;
		memcpy(to_parse->data, string, string_size);

		/* Preparing tag_list if needed */
		bbcode_prepare_tag_list(parser);
		/* Build the BBCode Tree from the string */
		bbcode_build_tree(parser, to_parse, tree);
		/* Correct Tree to match restrictions */
		bbcode_correct_tree(parser, tree, BBCODE_TREE_ROOT_TAGID, 0);
		/* Reset the working string */
		bassigncstr(to_parse, "");
		/* Apply the Output Rules */
		parser->tag_counter=(long *)malloc(bbcode_array_length(parser->bbcodes->bbcodes)*sizeof(long));
		for (i=0; i<bbcode_array_length(parser->bbcodes->bbcodes); ++i) {
			parser->tag_counter[i]=0;
		}
		bbcode_apply_rules(parser, tree, to_parse);
		free(parser->tag_counter);
		parser->tag_counter=NULL;
		/* Destroy Tree */
		bbcode_tree_free(tree);
	}
	/* Getting the return string */
	*result_size=to_parse->slen;
	return_value=(char *)malloc(*result_size * sizeof(char)+1);
	return_value=memcpy(return_value, to_parse->data, to_parse->slen+1);
	bdestroy(to_parse);
	bdestroy(parsed);
	/* Return Value */
	return return_value;
}

/*bbcode_validation_p bbcode_validate(bbcode_parser_p parser, char *string, int string_size){
	
}*/

/* Get current options of the bbcode_parser */
long bbcode_parser_get_flags(bbcode_parser_p parser) {
	return parser->options;
}

/* Set options for the bbcode_parser */
void bbcode_parser_set_flags(bbcode_parser_p parser, long flags) {
	parser->options=flags;
	parser->bbcodes->options &= ~BBCODE_LIST_IS_READY;
	parser->smileys->ci=0;
	if (flags & BBCODE_SMILEYS_CASE_INSENSITIVE) {
		parser->smileys->ci=1;
	}
}

/* ----------------------------*
 * 		  Internal API         *
 * ----------------------------*/
/* Parse nesting rules and optimize datas */
void bbcode_prepare_tag_list(bbcode_parser_p parser) {
	bbcode_list_p list = NULL;
	bbcode_p bbcode = NULL;
	struct bstrList *bsplited;
	char accept_smileys;
	char default_smileys;
	int i, j, max;

	accept_smileys=1;
	if (parser->options & BBCODE_FORCE_SMILEYS_OFF) {
		accept_smileys=0;
	}
	default_smileys=0;
	if (parser->options & BBCODE_DEFAULT_SMILEYS_ON) {
		default_smileys=1;
	}
	
	max=0;
	list=parser->bbcodes;
	if (list->options & BBCODE_LIST_IS_READY ) {
		return;
	}
	/* Resolve cache preparation */
	for (i=0; i<bbcode_array_length(list->bbcodes); i++) {
		bbcode=bbcode_get_bbcode(parser, i);
		if (blength(bbcode->tag) > max) {
			max=blength(bbcode->tag);
		}
	}
	if (list->bbcode_max_size !=0) {
		for (i=0; i<=list->bbcode_max_size; i++) {
			if (list->search_cache[i] !=NULL) {
				free(list->search_cache[i]);
				list->num_cache[i]=0;
			}
		}
		free(list->num_cache);
		list->num_cache=NULL;
		free(list->search_cache);
		list->search_cache=NULL;
	}
	list->bbcode_max_size=max;
	list->num_cache = (long*) malloc(sizeof(long) * (max+1));
	list->search_cache = (bbcode_search_pp) malloc(sizeof(bbcode_search_p) * (max+1));
	for (i=0; i<max+1; i++) {
		list->num_cache[i]=0;
		list->search_cache[i]=NULL;
	}
	for (i=0; i<bbcode_array_length(list->bbcodes); i++) {
		int slen;
		bbcode_search temp;

		bbcode=bbcode_get_bbcode(parser, i);
		slen=blength(bbcode->tag);
		if (list->search_cache[slen]==NULL) {
			list->search_cache[slen]
					=(bbcode_search_p) malloc(sizeof(bbcode_search));
		} else {
			list->search_cache[slen]=(bbcode_search_p) realloc(
					list->search_cache[slen], sizeof(bbcode_search) * (list->num_cache[slen]+1));
		}

		temp.tag_name=bbcode->tag;
		temp.tag_id=i;
		list->search_cache[slen][list->num_cache[slen]]=temp;
		(list->num_cache[slen])++;
	}
	/* Root Preparation */
	list->root->speed_cache= 0;
	if (accept_smileys && default_smileys) {
		list->root->speed_cache |=BBCODE_CACHE_ACCEPT_SMILEYS;
	}
	list->root->parents->type=BBCODE_ALLOW_LIST_TYPE_ALL;
	list->root->childs->type=BBCODE_ALLOW_LIST_TYPE_ALL;
	/* Root Childs */
	if (list->root->child_list==NULL || blength(list->root->child_list)) {
		if (list->root->child_list==NULL || biseqcstr(list->root->child_list, "all")) {
			/* All Accepted */
			list->root->childs->type=BBCODE_ALLOW_LIST_TYPE_ALL;
		} else {
			long find;
			bstring work = bstrcpy(list->root->child_list);
			if ((bchar(list->root->child_list, 0) == '!') == 1) {
				list->root->childs->type=BBCODE_ALLOW_LIST_TYPE_EXCLUDE;
				/* Remove the ! */
				bdelete(work,0,1);
			} else {
				list->root->childs->type=BBCODE_ALLOW_LIST_TYPE_LISTED;
			}
			/* We add all entries */
			bsplited=bsplit (work, ',');

			bbcode_allow_list_check_size(list->root->childs, bsplited->qty);
			for (j=0; j<bsplited->qty; j++) {
				find = bbcode_get_tag_id (parser, bsplited->entry[j], -1);
				if (find>=0) {
					bbcode_allow_list_add(list->root->childs, find);
				}
			}
			bdestroy(work);
			bstrListDestroy(bsplited);
		}
	} else {
		/* None Accepted */
		list->root->childs->type=BBCODE_ALLOW_LIST_TYPE_NONE;
	}
	/* End Root Childs */
	for (i=0; i<bbcode_array_length(list->bbcodes); i++) {
		bbcode=bbcode_get_bbcode(parser, i);
		bbcode->speed_cache=0;
		if (bbcode->type == BBCODE_TYPE_ARG|| bbcode->type== BBCODE_TYPE_OPTARG) {
			bbcode->speed_cache |=BBCODE_CACHE_ACCEPT_ARG;
		}
		if (bbcode->type == BBCODE_TYPE_NOARG|| bbcode->type
				== BBCODE_TYPE_SINGLE|| bbcode->type == BBCODE_TYPE_OPTARG) {
			bbcode->speed_cache |=BBCODE_CACHE_ACCEPT_NOARG;
		}
		if (bstrchr(bbcode->open_tag, '{')!=BSTR_ERR) {
			bbcode->speed_cache |=BBCODE_CACHE_START_HAS_BRACKET_OPEN;
		}
		if (bstrchr(bbcode->close_tag, '{')!=BSTR_ERR) {
			bbcode->speed_cache |=BBCODE_CACHE_END_HAS_BRACKET_OPEN;
		}
		if (accept_smileys && ( (bbcode->flags & BBCODE_FLAGS_SMILEYS_ON)
				|| ((default_smileys) && ((bbcode->flags
						& BBCODE_FLAGS_SMILEYS_OFF)==0)))) {
			bbcode->speed_cache |=BBCODE_CACHE_ACCEPT_SMILEYS;
		}
		bbcode->parents->size=0;
		bbcode->childs->size=0;
		/* parents */
		if (blength(bbcode->parent_list)) {
			if (biseqcstr(bbcode->parent_list, "all")) {
				/* All Accepted */
				bbcode->parents->type=BBCODE_ALLOW_LIST_TYPE_ALL;
			} else {
				long find;

				bstring work = bstrcpy(bbcode->parent_list);
				if (bchar(bbcode->parent_list, 0) == '!') {
					bbcode->parents->type=BBCODE_ALLOW_LIST_TYPE_EXCLUDE;
					bdelete(work,0,1);
				} else {
					bbcode->parents->type=BBCODE_ALLOW_LIST_TYPE_LISTED;
				}
				/* We add all entries */
				bsplited=bsplit (work, ',');

				bbcode_allow_list_check_size(bbcode->parents, bsplited->qty);
				for (j=0; j<bsplited->qty; j++) {
					find = bbcode_get_tag_id (parser, bsplited->entry[j], -1);
					if (find>=0) {
						bbcode_allow_list_add(bbcode->parents, find);
					}
				}
				bdestroy(work);
				bstrListDestroy(bsplited);
			}
		} else {
			/* None Accepted */
			bbcode->parents->type=BBCODE_ALLOW_LIST_TYPE_NONE;
		}
		/* Childs */
		if (blength(bbcode->child_list)) {
			if (biseqcstr(bbcode->child_list, "all")) {
				/* All Accepted */
				bbcode->childs->type=BBCODE_ALLOW_LIST_TYPE_ALL;
			} else {
				long find;

				bstring work=bstrcpy(bbcode->child_list);
				if (bchar(bbcode->child_list, 0) == '!') {
					bbcode->childs->type=BBCODE_ALLOW_LIST_TYPE_EXCLUDE;
					bdelete(work,0,1);
				} else {
					bbcode->childs->type=BBCODE_ALLOW_LIST_TYPE_LISTED;
				}
				/* We add all entries */
				bsplited=bsplit (work, ',');

				bbcode_allow_list_check_size(bbcode->childs, bsplited->qty);
				for (j=0; j<bsplited->qty; j++) {
					find = bbcode_get_tag_id (parser, bsplited->entry[j], -1);
					if (find>=0) {
						bbcode_allow_list_add(bbcode->childs, find);
					}
				}
				bdestroy(work);
				bstrListDestroy(bsplited);
			}
		} else {
			/* None Accepted */
			bbcode->childs->type=BBCODE_ALLOW_LIST_TYPE_NONE;
		}
	}
}

/* This reparse nesting rules and optimize datas */
void bbcode_build_tree(bbcode_parser_p parser, bstring string,
		bbcode_parse_tree_p tree) {
	/* INIT */
	bstring end_quote = NULL, end_double = NULL, end_single = NULL,
			end_html = NULL, html_quote = NULL, argument = NULL,
			tag = NULL;
	char no_quote;
	char quote_double=parser->options & BBCODE_ARG_DOUBLE_QUOTE;
	char quote_single=parser->options & BBCODE_ARG_SINGLE_QUOTE;
	char quote_html=parser->options & BBCODE_ARG_HTML_QUOTE;
	char escaping_enabled=parser->options & BBCODE_ARG_QUOTE_ESCAPING;
	char added=0;
	int offset, end, next_equal, next_close, string_length;
	long tag_id;
	int quote_enabled=(quote_double || quote_single || quote_html);
	bbcode_parse_tree_array_p work_stack = NULL, close_stack = NULL;

	string_length=blength(string);
	tag_id=end=next_equal=next_close=0;
	end_double=bfromcstr("\"]");
	end_single=bfromcstr("\']");
	end_html=bfromcstr("&quot;]");
	html_quote=bfromcstr("&quot;");
	/* END INIT */
	offset=bstrchr(string, '[');
	bbcode_tree_push_string_child(tree, bmidstr(string, 0, offset), offset);
	work_stack=bbcode_parse_stack_create();
	bbcode_parse_stack_push_element(work_stack,tree);
	close_stack=bbcode_parse_stack_create();
	parser->current_node=tree;
	do {
		added=0;
		if (bchar(string, offset)=='[') {
			if (bchar(string, offset+1)!='/') {
				/* Equal */
				bbcode_find_next(next_equal, string, offset, '=');
				/* Close */
				bbcode_find_next(next_close, string, offset, ']');
				if (next_close!=BSTR_ERR && next_close<string_length) {
					/* With Arg */
					if (next_equal<next_close) {
						tag = bmidstr(string, offset+1, next_equal-offset-1);
						/* Finding tag_id */
						if (BBCODE_ERR!=(tag_id=bbcode_get_tag_id(parser, tag,
										1))) {
							/* Quotes */
							if (quote_enabled) {
								int diff=0;

								end=next_close;
								no_quote=0;

								if (quote_single && bchar(string, next_equal+1)
									=='\'') {
									end_quote=end_single;
								} else if (quote_double && bchar(string,
										next_equal+1)=='"') {
									end_quote=end_double;
									
								} else {
									/* support HTML Quotes */
									if (quote_html) {
										bstring to_comp=bmidstr(string, next_equal+1,
												blength(html_quote));
										if (0==bstricmp(html_quote, to_comp)){
											end_quote=end_html;
											diff=5;
										} else {
											argument=bmidstr(string, next_equal+1,
													next_close-next_equal-1);
											no_quote=1;
										}
										bdestroy(to_comp);
									} else {
										argument=bmidstr(string, next_equal+1,
												next_close-next_equal-1);
										no_quote=1;
									}
								}
								/* We have a quote, delete the ending quote */
								if (!no_quote) {
									int next_pos=next_equal;
									do {
										next_pos=end=binstrcaseless(string, next_pos+1, end_quote);
									} while (escaping_enabled && bbcode_is_escaped(string,end));
									if (end != BSTR_ERR) {
										argument=bmidstr(string, next_equal+2+diff,
												end++ - next_equal - 2-diff);
										/* removing escaping from argument */
										if (escaping_enabled){
											bbcode_strip_escaping(argument);
										}
										next_close=end+diff;
									} else {
										/* No Ending Quote, go to next tag */
										end=next_equal+diff;
									}
								}
							} else {
								/* Quotes are not supported */
								argument=bmidstr(string, next_equal+1,
										next_close-next_equal-1);
							}
							/* We have an argument */
							if (argument!=NULL) {
								if (bbcode_allow_list_no_child(bbcode_get_bbcode(parser,tag_id)->childs)) {
									BBCODE_SPECIAL_CASE_NO_CHILD(argument)
								} else {
									bbcode_tree_push_tree_child(parser, tree,
											work_stack, close_stack, bmidstr(
													string, offset, end-offset
															+1), tag_id,
											argument, offset);
									bdestroy(argument);
									argument=NULL;
									end=next_close;
									added=1;
								}
							}
						} else {
							/* No tag found */
							end=next_close;
						}
					} else {
						/* Without Args */
						tag=bmidstr(string, offset+1, next_close-offset-1);
						end=next_close;
						if (BBCODE_ERR!=(tag_id=bbcode_get_tag_id(parser, tag,
										0))) {
							if (bbcode_allow_list_no_child(bbcode_get_bbcode(parser,tag_id)->childs)) {
								BBCODE_SPECIAL_CASE_NO_CHILD(NULL)
							} else {
								bbcode_tree_push_tree_child(parser, tree,
										work_stack, close_stack, bmidstr(
												string, offset, end-offset+1),
										tag_id, NULL, offset);
								added=1;
							}
						}
					}
					bdestroy(tag);
				}
			} else {
				/* Close */
				bbcode_find_next(next_close, string, offset, ']');
				if (next_close!=BSTR_ERR && next_close<string_length) { 
					tag=bmidstr(string, offset+2, next_close-offset-2);
					end=next_close;
					if (BBCODE_ERR!=(tag_id=bbcode_get_tag_id(parser, tag, -1))) {
						bbcode_close_tag(parser, tree, work_stack, close_stack,
								tag_id, bmidstr(string, offset, end-offset+1), 1, offset);
						added=1;
					}
					bdestroy(tag);
				}
			}
		}
		if (!added) {
			/* No element added to the tree, we add raw string */
			end=bstrchrp(string, '[', offset+1);
			if (end<0) {
				end=string_length;
			} else {
				--end;
			}
			if (end-offset+1>0){
				bbcode_tree_push_string_child(parser->current_node, 
						(bmidstr(string, offset, end-offset+1)), offset);
			}
		}
		offset=end+1;
	} while (offset<string_length);
	/* Freeing resources */
	bdestroy(end_html);
	bdestroy(end_double);
	bdestroy(end_single);
	bdestroy(html_quote);
	bbcode_parse_stack_free(work_stack);
	bbcode_parse_stack_free(close_stack);
}

/* This closes an active tag */
void bbcode_close_tag(bbcode_parser_p parser, bbcode_parse_tree_p tree,
		bbcode_parse_tree_array_p work, bbcode_parse_tree_array_p close,
		long tag_id, bstring close_string, int true_close, int offset) {
	long id;
	int i,j;
	char in_close=0;
	bbcode_parse_tree_array_p conditions = NULL;
	bbcode_parse_tree_array_p local_closes = NULL;
	/* Check if on close List */
	for (i=0; i<bbcode_array_length(close); i++) {
		if (bbcode_array_element(close,i)->tag_id==tag_id) {
			in_close=1;
			break;
		}
	}
	if (in_close) {
		/* Mark Element as closed */
		bbcode_tree_mark_element_closed((close->element[i]));
		/* Tag Allready closed, droping silently */
		bbcode_parse_drop_element_at(close, i);
		bdestroy(close_string);
	} else {
		/* Check If Opened */
		char opened=0;
		for (i=0; i<work->size; i++) {
			if (bbcode_array_element(work,i)->tag_id==tag_id) {
				opened=1;
				break;
			}
		}
		if (opened) {
			/* It's allready opened */
			char searching=1;
			local_closes = bbcode_parse_stack_create();
			do {
				if ((bbcode_get_bbcode(parser, parser->current_node->tag_id)->flags
						& BBCODE_FLAGS_DENY_REOPEN_CHILD)!=0){
					/* Empty local_closes if BBCODE_FLAGS_DENY_REOPEN_CHILD */
					local_closes->size=0;
				}
				if (parser->current_node->tag_id == tag_id) {
					/* It's the searched tag */
					searching=0;
					bbcode_tree_mark_element_closed(parser->current_node);
					parser->current_node->close_string=close_string;
					if (!true_close) {
						bbcode_parse_stack_push_element(close,
								parser->current_node);
						bbcode_parse_stack_push_element(local_closes,
								parser->current_node);
					}
				} else {
					/* It's not the searched tag */
					/* Is this tag putting conditions on tag_id nesting ? */
					if (bbcode_allow_list_check_access(bbcode_get_bbcode(parser, bbcode_get_cn(parser)->tag_id)->childs, tag_id)) {
						/* Fixme
						 * conditions[cond_size]=bbcode_get_cn(parser);
						 * cond_size++; */
					}
					parser->current_node->close_string=NULL;
					if (bbcode_get_bbcode(parser,bbcode_get_cn(parser)->tag_id)->flags
					& BBCODE_FLAGS_ONE_OPEN_PER_LEVEL) {
						bbcode_tree_mark_element_closed(parser->current_node);
					} else {
						bbcode_parse_stack_push_element(close,
								parser->current_node);
						bbcode_parse_stack_push_element(local_closes,
								parser->current_node);
					}
				}
				bbcode_parse_stack_pop_element_loose(work);
				parser->current_node = bbcode_array_element(work,
						bbcode_array_length(work)-1);
			} while (searching);
			/* Reopening incorrectly nested & closed tags */
			if (parser->options & BBCODE_CORRECT_REOPEN_TAGS) {
				bbcode_parse_tree_p tmp_tree = NULL;
				for (i=local_closes->size-1; i>=0; --i) {
					/* First Multipart Element */
					if (bbcode_array_element(local_closes,i)->multiparts == NULL){
						if (bbcode_array_element(local_closes,i)->flags
						& BBCODE_TREE_FLAGS_MULTIPART_FIRST_NODE) {
							bbcode_array_element(local_closes,i)->multiparts=bbcode_parse_stack_create();
							bbcode_parse_stack_push_element(bbcode_array_element(local_closes,i)->multiparts,bbcode_array_element(local_closes,i));
						} else {
							/* The multipart Tree is common to all multipart elements of a set
							 * no changes needed */
						}
					}
					bbcode_array_element(local_closes,i)->flags|=BBCODE_TREE_FLAGS_MULTIPART;
					tmp_tree=bbcode_tree_create();
					bbcode_parse_stack_push_element(bbcode_array_element(local_closes,i)->multiparts,tmp_tree);
					tmp_tree->tag_id=bbcode_array_element(local_closes,i)->tag_id;
					tmp_tree->flags=BBCODE_TREE_FLAGS_MULTIPART;
					tmp_tree->multiparts=bbcode_array_element(local_closes,i)->multiparts;
					tmp_tree->open_string=NULL;
					tmp_tree->close_string=NULL;
					bbcode_tree_push_tree_raw(parser, bbcode_get_cn(parser), tmp_tree, work);
				}
				for (i=bbcode_array_length(local_closes)-1; i>=0; i--) {
					id=local_closes->element[i]->tag_id;
					for (j=bbcode_array_length(close)-1; j>=0; j--){
						if (bbcode_array_element(close,j)->tag_id==id) {
							bbcode_parse_drop_element_at(close, i);
							break;
						}
					}
				}
				local_closes->size=0;
			}
		} else {
			bbcode_tree_push_string_child(tree, close_string, offset);
		}
	}
	if (conditions!=NULL){
		bbcode_parse_stack_free(conditions);
	}
	if (local_closes!=NULL){
		bbcode_parse_stack_free(local_closes);
	}
}

/* This make some basic corrections to a given tree */
int bbcode_correct_tree(bbcode_parser_p parser, bbcode_parse_tree_p tree,
		long parent_id, char force_false) {
	int autocorrect, orig_parent, ret;
	long i,j;
	bbcode_parse_tree_child_p child = NULL;

	bbcode_p tag= bbcode_get_bbcode(parser,tree->tag_id);
	/* Options */
	autocorrect = parser->options & BBCODE_AUTO_CORRECT;
	if (bbcode_allow_list_check_access(tag->parents, parent_id)==0) {
		force_false = 1;
	}
	if (force_false) {
		bbcode_apply_flag_to_parts(tree->multiparts, BBCODE_TREE_FLAGS_PAIRED, 0)
		tree->flags &= ~BBCODE_TREE_FLAGS_PAIRED;
	}
	if (tree->tag_id == BBCODE_TREE_ROOT_TAGID) {
		tree->flags |= BBCODE_TREE_FLAGS_PAIRED;
	}
	if (tag->flags & BBCODE_FLAGS_ONE_OPEN_PER_LEVEL) {
		bbcode_apply_flag_to_parts(tree->multiparts, BBCODE_TREE_FLAGS_PAIRED, 1)
		tree->flags |= BBCODE_TREE_FLAGS_PAIRED;
	}
	orig_parent=parent_id;
	parent_id=force_false ? parent_id : tree->tag_id;
	if (tree->conditions != NULL) {
		for (i=0; i<tree->conditions->size; i++){
			if ((tree->conditions->element[i])->flags & BBCODE_TREE_FLAGS_PAIRED){
				bbcode_apply_flag_to_parts(tree->multiparts, BBCODE_TREE_FLAGS_PAIRED, 0)
				tree->flags &= ~BBCODE_TREE_FLAGS_PAIRED;
				break;
			}
		}
	}

	for (i = 0; i < tree->childs.size ; i++) {
		child=(tree->childs.element[i]);
		if (child->type==BBCODE_TREE_CHILD_TYPE_TREE) {
			bbcode_parse_tree_p child_tree = NULL;
			child_tree=child->tree;
			if (((child_tree->flags & BBCODE_TREE_FLAGS_MULTIPART) !=0)
					&& ((child_tree->flags & BBCODE_TREE_FLAGS_MULTIPART_DONE) ==0)) {
				for (j=0; j<child_tree->multiparts->size; j++){
					if (((child_tree->multiparts->element[j])->tag_id == BBCODE_TREE_ROOT_TAGID)) {
						continue;
					}
					if ((tree->flags & BBCODE_TREE_FLAGS_ROOT) == 0){
						bbcode_parse_tree_p parent_tree = NULL; 
						parent_tree=tree->parent_node;
						do {
							if (parent_tree==NULL){
								break;
							}
							if (parent_tree!=NULL && ((parent_tree->flags & BBCODE_TREE_FLAGS_PAIRED) == 0)){
								bbcode_apply_flag_to_parts(tree->multiparts, BBCODE_TREE_FLAGS_PAIRED, 0)
								tree->flags &= ~BBCODE_TREE_FLAGS_PAIRED;
								parent_tree=parent_tree->parent_node;
								break;
							}
							parent_tree=parent_tree->parent_node;
						} while (parent_tree!=NULL && ((parent_tree->flags & BBCODE_TREE_FLAGS_MULTIPART) == 0));
					}
					if (bbcode_allow_list_check_access(bbcode_get_bbcode( parser, parent_id)->childs, child_tree->tag_id)) {
						if (bbcode_allow_list_check_access(bbcode_get_bbcode(parser,child_tree->tag_id)->childs, parent_id)) {
							continue;
						}
						bbcode_apply_flag_to_parts(tree->multiparts, BBCODE_TREE_FLAGS_PAIRED, 0)
						tree->flags &= ~BBCODE_TREE_FLAGS_PAIRED;
						break;
					}
				}
				bbcode_apply_flag_to_parts(tree->multiparts, BBCODE_TREE_FLAGS_MULTIPART_DONE, 1)
				tree->flags |= BBCODE_TREE_FLAGS_MULTIPART_DONE;
				if (!autocorrect && ((tree->flags & BBCODE_TREE_FLAGS_PAIRED) ==0)) {
					force_false = 1;
				}
				parent_id = force_false ? orig_parent : tree->tag_id;
				/* Make the allow list checking */
				if (bbcode_allow_list_check_access(bbcode_get_bbcode(parser, parent_id)->childs, child_tree->tag_id)) {
					ret = bbcode_correct_tree(parser, child_tree, parent_id, 0);
				} else {
					ret = bbcode_correct_tree(parser, child_tree, parent_id, 1);
				}
			} else {
				if (bbcode_allow_list_check_access(bbcode_get_bbcode(parser, parent_id)->childs, child_tree->tag_id)) {
					ret = bbcode_correct_tree(parser, child_tree, parent_id, 0);
				} else {
					ret = bbcode_correct_tree(parser, child_tree, parent_id, 1);
				}
			}
			if (ret){
				int offset=0;
				int offset_start=0;
				bbcode_parse_tree_p tmp_tree=bbcode_tree_create();
				/* Removes the child */
				bbcode_tree_move_childs( tree, tmp_tree, i, 1, 0);
				/* Add Closing String */
				if (blength(child_tree->close_string)) {
					bbcode_tree_push_string_child(tmp_tree, child_tree->close_string, child->offset);
					bbcode_tree_move_childs( tmp_tree, tree, 1, 1, i);
					offset++;
				} else {
					bdestroy(child_tree->close_string);
				}
				child_tree->close_string= NULL;
				/* Prepend Opening String */
				if (blength(child_tree->open_string)){
					bbcode_tree_push_string_child(tmp_tree, child_tree->open_string, child->offset);
					bbcode_tree_move_childs( tmp_tree, tree, 1, 1, i);
					offset++;
					offset_start++;
				} else {
					bdestroy(child_tree->open_string);
				}
				child_tree->open_string= NULL;
				/* move elements from child to tree */
				bbcode_tree_move_childs( child_tree, tree, 0 , child_tree->childs.size, i+offset_start);
				/* Add child_tree.size+i to i */
				i+=(child_tree->childs.size) + offset-1;
				bbcode_tree_free(tmp_tree);
			}
		}
	}
	if (!force_false && ( (tree->flags & BBCODE_TREE_FLAGS_PAIRED) || autocorrect)){
		return 0;
	} else {
		if (force_false) {
			bbcode_apply_flag_to_parts(tree->multiparts, BBCODE_TREE_FLAGS_PAIRED, 0)
			tree->flags &= ~BBCODE_TREE_FLAGS_PAIRED;
		}
		return 1;
	}

}

/* This apply the BBCode rules to generate the final string */
void bbcode_apply_rules(bbcode_parser_p parser, bbcode_parse_tree_p tree,
		bstring parsed) {
	bbcode_p tag= bbcode_get_bbcode(parser,tree->tag_id);
	bstring working_string = NULL;
	bstring last_string = NULL;
	bstring tmp_string = NULL;
	long i;
	bbcode_parse_tree_p to_drop = NULL;
	bstring arg, content;

	last_string=bfromcstr("");
	tmp_string=bfromcstr("");
	working_string=bfromcstr("");
	/* Dropped elements */

	to_drop=bbcode_tree_create();

	content=arg=NULL;
	/* Multipart Merging */
	for (i=0 ; i < tree->childs.size; i++) {
		if ((tree->childs.element[i])->type==BBCODE_TREE_CHILD_TYPE_TREE){
			long j;
			for (j=i+1; j < tree->childs.size; j++){
				/* Only treat tree elements */
				bbcode_parse_tree_child_p tmp_child=(tree->childs.element[j]);
				if ((tmp_child)->type==BBCODE_TREE_CHILD_TYPE_TREE){
					/* If next tree is not multipart, no mergin possible */
					if (tree->flags & BBCODE_TREE_FLAGS_MULTIPART){
						/* Same Multipart element */
						if (((tree->childs.element[i])->tree->tag_id 
								== (tree->childs.element[j])->tree->tag_id) && 
								((tree->childs.element[i])->tree->multiparts == 
								(tree->childs.element[j])->tree->multiparts)) {
							/* Moving datas from 2° element to first one */
							bbcode_tree_move_childs(
									tree->childs.element[j]->tree,
									tree->childs.element[i]->tree, 0,
									tree->childs.element[j]->tree->childs.size,
									tree->childs.element[i]->tree->childs.size);
							/* Dropping child */
							bbcode_tree_move_childs(tree, to_drop, j, 1, 0);
							/* Prepare merging (appending interpart elements to first tree) */
							bbcode_tree_move_childs(
									tree,
									tree->childs.element[i]->tree,
									i + 1,
									j - i - 1,
									tree->childs.element[i]->tree->childs.size - 1);
						}
					} else { 
						break;
					}
				} else {
					break;
				}
			}
		}
	}
	/* Freeing memory */
	bbcode_tree_free(to_drop);
	/* Applying rules */
	for (i=0 ; i < tree->childs.size; i++) {
		if ((tree->childs.element[i])->type==BBCODE_TREE_CHILD_TYPE_TREE) {
			/* Parsing smileys for inner elements (grouped) */
			if (blength(last_string) > 0) {
				if (tag->speed_cache & BBCODE_CACHE_ACCEPT_SMILEYS) {
					bbcode_parse_smileys(last_string, parser->smileys);
				}
				bconcat(working_string, last_string);
				bdelete(last_string, 0, blength(last_string));
			}
			bbcode_apply_rules(parser, (tree->childs.element[i])->tree, tmp_string);
			bconcat (working_string, tmp_string);
			bdelete(tmp_string, 0, blength(tmp_string));
		} else {
			bconcat (last_string, (tree->childs.element[i])->string);
		}
	}
	/* Parsing smileys for last elements (grouped) */
	if (blength(last_string)>0) {
		if (tag->speed_cache & BBCODE_CACHE_ACCEPT_SMILEYS) {
			bbcode_parse_smileys(last_string, parser->smileys);
		}
		bconcat(working_string, last_string);
		bdelete(last_string, 0, blength(last_string));
	}
	if (!((tag->flags & BBCODE_FLAGS_REMOVE_IF_EMPTY) !=0 &&
			blength(working_string)==0)) {
		/* If element is not paired And no other special cases */
		if ((parser->options & BBCODE_AUTO_CORRECT)==0 
				&& (tree->flags & BBCODE_TREE_FLAGS_PAIRED)==0
				&& (tag->flags	& BBCODE_FLAGS_ONE_OPEN_PER_LEVEL)==0) {
			/* Preparing Return */
			bassign(parsed, tree->open_string);
			bconcat(parsed, working_string);
		} else {
			if (tree->tag_id>=0 && tag->max_parsed>0 && (parser->tag_counter[tree->tag_id])>=tag->max_parsed) {
				bassign(parsed, tree->open_string);
				bconcat(parsed, working_string);
				bconcat(parsed, tree->close_string);
			} else {
				bassign(parsed, tag->open_tag);
				arg=bfromcstr("");
				if (tree->tag_id>=0) {
					++parser->tag_counter[tree->tag_id];
				}
				if (tag->speed_cache & BBCODE_CACHE_ACCEPT_ARG){
					if (blength(tree->argument)>0){
						bassign(arg,tree->argument);
					} else {
						bassign(arg, tag->default_arg);
					}
					if (tag->flags & BBCODE_FLAGS_ARG_PARSING){
						bbcode_parser_p arg_parser;
						char *string_output;
						int string_size;
						int j;
						long *tag_counter=NULL;
						if (parser->argument_parser != NULL){
							arg_parser=parser->argument_parser;
						} else {
							arg_parser=parser;
							tag_counter=parser->tag_counter;
							arg_parser->tag_counter=(long *)malloc(bbcode_array_length(arg_parser->bbcodes->bbcodes)*sizeof(long));
							for (j=0; j<bbcode_array_length(arg_parser->bbcodes->bbcodes); ++j) {
								arg_parser->tag_counter[j]=0;
							}
						}
						string_output=bbcode_parse(arg_parser, arg->data, arg->slen, &string_size);
						if (parser->argument_parser != NULL) {
							free(arg_parser->tag_counter);
						} else {
							free(arg_parser->tag_counter);
							parser->tag_counter=tag_counter;
							tag_counter=NULL;
						}
						bdestroy(arg);
						arg=blk2bstr(string_output, string_size);
						free(string_output);
					}
				}
				/* Callbacks - 1/ Content_callback */
				if (tag->content_handling_func != NULL){
					tag->content_handling_func(working_string, arg, tag->content_handling_func_data);
				}
				/* Callbacks - 2/ Param callback */
				if (tag->param_handling_func != NULL){
					tag->param_handling_func(working_string, arg, tag->param_handling_func_data);
				}
			
				/* Replacing {ARG} by $arg and {CONTENT} by $string in arg & start */
				if (blength(arg)){
					bfindreplace(arg, parser->content_replace, working_string,0);
				}
				if (tag->speed_cache & BBCODE_CACHE_START_HAS_BRACKET_OPEN){
					bfindreplace(parsed, parser->content_replace, working_string, 0);
					bfindreplace(parsed, parser->arg_replace, arg, 0);
				}
				/* Replacing {ARG} by $arg in string & end */
				bfindreplace(working_string, parser->arg_replace, arg,0);
				bassign(tmp_string, tag->close_tag);
				if (tag->speed_cache & BBCODE_CACHE_END_HAS_BRACKET_OPEN){
					bfindreplace(tmp_string, parser->arg_replace, arg, 0);
				}
				/* Concat everything */
				bconcat(parsed, working_string);
				bconcat(parsed, tmp_string);
			}
		}
	}
	/* Freeing resources */
	bdestroy(last_string);
	bdestroy(working_string);
	bdestroy(arg);
	bdestroy(tmp_string);
	/* Returning */
	return;
}

/* Search a tag_id from the string */
long bbcode_get_tag_id(bbcode_parser_p parser, bstring value, long has_arg) {
	int taglen=blength(value);
	bbcode_list_p bbcode_list = parser->bbcodes;
	if (taglen <= bbcode_list->bbcode_max_size) {
		if (bbcode_list->num_cache[taglen]==0) {
			/* Tag does not exists */
			return BBCODE_ERR;
		} else {
			/* tags of this size exists, start binary search
			 * First, get the count of elements & the elements */
			long count=bbcode_list->num_cache[taglen];
			bbcode_search_p list=bbcode_list->search_cache[taglen];
			if (count<20) {
				/* We use linear search, should be faster */
				long i;
				for (i=0; i<count; i++) {
					if (!bstricmp(value, list[i].tag_name)) {
						long pos=list[i].tag_id;
						if (has_arg==1) {
							if (bbcode_get_bbcode(parser,pos)->speed_cache
							& BBCODE_CACHE_ACCEPT_ARG) {
								return pos; /* Arg */
							}
						} else if(has_arg==0) {
							if (bbcode_get_bbcode(parser,pos)->speed_cache
							& BBCODE_CACHE_ACCEPT_NOARG) {
								return pos; /* No Arg */
							}
						} else {
							return pos; /* Close tag */
						}
					}
				}
			} else {
				long left=0;
				long right=count-1;
				long i=count/2;
				long equal, pos;

				bstring lower_tag = NULL;
				lower_tag=bstrcpy(value);
				btolower(lower_tag);
				/* We start true binary */

				while (1) {
					equal=bstrcmp(lower_tag, list[i].tag_name);
					if (equal==0) {
						/* We have found the entry */
						pos=list[i].tag_id;
						if (has_arg==1) {
							if (bbcode_get_bbcode(parser,pos)->speed_cache
							& BBCODE_CACHE_ACCEPT_ARG) {
								return pos;
							}
						} else if (has_arg==0) {
							if (bbcode_get_bbcode(parser,pos)->speed_cache
							& BBCODE_CACHE_ACCEPT_NOARG) {
								return pos;
							}
						} else {
							return pos;
						}
						return BBCODE_ERR;
					} else if (equal<0) {
						/* the searched entry is greater than this pos */
						left=i;
						i=((right+i)>>1);
						if (left==i) {
							break;
						}
					} else {
						/* the searched entry is smaller than this pos */
						right=i;
						i=((left+i)>>1);
						if (right==i) {
							break;
						}
					}
				}
			}
		}
	}
	return BBCODE_ERR;
}

/* Translate Smileys */
void bbcode_parse_smileys(bstring string, bbcode_smiley_list_p list) {
	long i;
	if (list->ci){
		for (i=0; i<list->size; i++) {
			bfindreplacecaseless(string, list->smileys[i].search, list->smileys[i].replace,
					0);
		}
	} else {
		for (i=0; i<list->size; i++) {
			bfindreplace(string, list->smileys[i].search, list->smileys[i].replace,
					0);
		}
	}
}

/*---------------------------
 Smiley Manipulation API
 ---------------------------*/
/* Initialize a smiley list */
bbcode_smiley_list_p bbcode_smileys_list_create() {
	bbcode_smiley_list_p list = NULL;
	list=malloc(sizeof(bbcode_smiley_list));
	list->size=0;
	list->msize=0;
	list->smileys = NULL;
	list->ci=0;
	return list;
}

/* Free a smiley list */
void bbcode_smileys_list_free(bbcode_smiley_list_p list) {
	if (list->msize>0) {
		long i;
		for (i=0; i<list->size; i++) {
			bdestroy(list->smileys[i].search);
			bdestroy(list->smileys[i].replace);
		}
		free(list->smileys);
	}
	free(list);
}

/* Check if we can add an entry */
void bbcode_smiley_list_check_size(bbcode_smiley_list_p list, long size) {
	if (list->msize<=size) {
		list->smileys=realloc(list->smileys, (size+BBCODE_BUFFER)
				*sizeof(bbcode_smiley));
		list->msize=size+BBCODE_BUFFER;
	}
}

/* adds a smiley to the list */
void bbcode_smileys_add(bbcode_smiley_list_p list, bstring search,
		bstring replace) {
	bbcode_smiley_list_check_size(list, list->size+1);
	list->smileys[list->size].search=search;
	list->smileys[list->size].replace=replace;
	list->size++;
}

/*---------------------------
 BBCode List Manipulation API
 ---------------------------*/
/* creates a BBcode list and init it */
bbcode_list_p bbcode_list_create() {
	bbcode_list_p list = NULL;
	list=malloc(sizeof(bbcode_list));
	list->root=bbcode_entry_create();
	list->bbcodes=bbcode_array_create();
	list->options=0;
	list->bbcode_max_size=0;
	list->num_cache=NULL;
	list->search_cache=NULL;
	return list;
}

/* free resources for a BBCode list */
void bbcode_list_free(bbcode_list_p list) {
	if (list->root!=NULL) {
		bbcode_entry_free(list->root);
	}
	if (list->bbcode_max_size >0) {
		long i;
		for (i=0; i<=list->bbcode_max_size; i++) {
			if (list->search_cache[i] !=NULL) {
				free(list->search_cache[i]);
				list->num_cache[i]=0;
			}
		}
		free(list->num_cache);
		list->num_cache=NULL;
		free(list->search_cache);
		list->search_cache=NULL;
	}
	bbcode_array_free(list->bbcodes);
	free(list);
}

/* Check if there is room for a bbcode entry */
void bbcode_list_check_size(bbcode_list_p list, long size) {
	if (list->bbcodes->msize<=size) {
		list->bbcodes->element =realloc(list->bbcodes->element, (size+BBCODE_BUFFER)
				*sizeof(bbcode_p));
		list->bbcodes->msize=size+BBCODE_BUFFER;
	}
}

/* Insert the special entry "Root" */
void bbcode_list_set_root(bbcode_list_p list, bbcode_p root) {
	list->root=root;
	list->options |= BBCODE_LIST_HAS_ROOT;
}

/* add a bbcode to a list */
void bbcode_list_add(bbcode_list_p list, bbcode_p to_add) {
	bbcode_list_check_size(list, list->bbcodes->size+1);
	list->bbcodes->element[list->bbcodes->size]=to_add;
	list->bbcodes->size++;
	list->options &= ~BBCODE_LIST_IS_READY;
}

/*---------------------------
 BBCode Array Manipulation API
 ---------------------------*/
/* creates a BBcode array and init it */
bbcode_array_p bbcode_array_create() {
	bbcode_array_p array = NULL;
	array=malloc(sizeof(bbcode_array));
	array->size=0;
	array->msize=0;
	array->element=NULL;
	return array;
}

/* Free a BBCode array */
void bbcode_array_free(bbcode_array_p array) {
	if (array->msize>0) {
		long i;
		for (i=0; i<array->size; i++) {
			bbcode_entry_free(array->element[i]);
		}
		free(array->element);
	}
	free(array);
}

/* Check if we can add an entry */
void bbcode_array_check_size(bbcode_array_p array, long size) {
	if (array->msize<=size) {
		array->element=realloc(array->element, (size+BBCODE_BUFFER)
				*sizeof(bbcode_p));
		array->msize=size+BBCODE_BUFFER;
	}
}

/* adds a bbcode_rule to the list */
void bbcode_array_add(bbcode_array_p array, bbcode_p bbcode) {
	bbcode_array_check_size(array, array->size+1);
	array->element[array->size]=bbcode;
}

/*---------------------------
 BBCode Entry Manipulation API
 ---------------------------*/
/* Malloc a bbcode entry and init it */
bbcode_p bbcode_entry_create() {
	bbcode_p bbcode = NULL;
	bbcode=malloc(sizeof(struct _bbcode));
	/* Init values */
	bbcode->type = bbcode->speed_cache = bbcode->flags = 0;
	bbcode->tag = bbcode->open_tag = bbcode->close_tag = bbcode->default_arg = bbcode->child_list = bbcode->parent_list = NULL;
	bbcode->param_handling_func_data = bbcode->content_handling_func_data = NULL;
	bbcode->param_handling_func = bbcode->content_handling_func = NULL;
	/* Starting Up allow_lists */
	bbcode->parents=bbcode_allow_list_create();
	bbcode->childs=bbcode_allow_list_create();
	return bbcode;
}

/* Free a bbcode entry resources */
void bbcode_entry_free(bbcode *entry) {
	/* Freeing automaticaly started datas; */
	/* All other stored datas must be freed by user */
	bdestroy(entry->tag);
	bdestroy(entry->open_tag);
	bdestroy(entry->close_tag);
	bdestroy(entry->default_arg);
	bdestroy(entry->child_list);
	bdestroy(entry->parent_list);

	bbcode_allow_list_free(entry->parents);
	bbcode_allow_list_free(entry->childs);
	free(entry);
}

/*---------------------------
 BBCode Allow Manipulation API
 ---------------------------*/
/* Malloc a bbcode_allow_list and init it */
bbcode_allow_list_p bbcode_allow_list_create() {
	bbcode_allow_list_p list = NULL;
	list=malloc(sizeof(bbcode_allow_list));
	list->size=list->msize=list->type=0;
	list->id_list=NULL;
	list->type=BBCODE_ALLOW_LIST_TYPE_ALL;
	return list;
}

/* Free the resources taken by an allow list */
void bbcode_allow_list_free(bbcode_allow_list_p list) {
	if (list->msize > 0) {
		free(list->id_list);
	}
	free(list);
}

/* Check for the size of an allow list */
void bbcode_allow_list_check_size(bbcode_allow_list_p list, long size) {
	if (list->msize<=size) {
		list->id_list=realloc(list->id_list, (size+BBCODE_BUFFER)*sizeof(long));
		list->msize=size+BBCODE_BUFFER;
	}

}

/* Add an element to the list */
void bbcode_allow_list_add(bbcode_allow_list_p list, long element) {
	bbcode_allow_list_check_size(list, list->size+1);
	list->id_list[list->size]=element;
	(list->size)++;
}

/* Check if a given id is autorized */
long bbcode_allow_list_check_access(bbcode_allow_list_p list, long tag_id) {
	if (tag_id < 0){
		return 1;
	}
	/* The easy ones */
	if (list->type==BBCODE_ALLOW_LIST_TYPE_ALL) {
		return 1;
	} else if (list->type==BBCODE_ALLOW_LIST_TYPE_NONE) {
		return 0;
	}
	/* And Now, We run */
	else {
		long i;
		for (i=0; i<list->size; i++) {
			if (list->id_list[i]==tag_id) {
				/* Found, we check if its a include or exclude list */
				if (list->type==BBCODE_ALLOW_LIST_TYPE_LISTED) {
					return 1;
				} else {
					return 0;
				}
			}
		}
		/* Not Found, checking if we're on include or exclude list */
		if (list->type==BBCODE_ALLOW_LIST_TYPE_LISTED) {
			return 0;
		} else {
			return 1;
		}
	}
}

/* return 1 if no_childs are accepted return 0 oterwise */
long bbcode_allow_list_no_child(bbcode_allow_list_p list) {
	/* Most of tags have no restrictions so exit ASAP */
	if (list->type==BBCODE_ALLOW_LIST_TYPE_ALL) {
		return 0;
	} else if (list->type==BBCODE_ALLOW_LIST_TYPE_NONE) { /* No childs */
		return 1;
	} else if (list->type==BBCODE_ALLOW_LIST_TYPE_LISTED) { /* Only Listed Childs and size=0 */
		if (list->size==0) {
			return 1;
		}
	}
	/* Undetermined Case: If exclude list with everything in it, (people using it must be considered insane -_-") */
	return 0;
}

/*---------------------------
 Tree Manipulation API
 ---------------------------*/
/* Malloc and init a bbcode tree  */
bbcode_parse_tree_p bbcode_tree_create() {
	bbcode_parse_tree_p tree = NULL;
	tree = malloc(sizeof(bbcode_parse_tree));
	tree->tag_id = BBCODE_TREE_ROOT_TAGID;
	tree->flags = 0;
	tree->childs.size = 0;
	tree->childs.msize = BBCODE_BUFFER;
	tree->childs.element = malloc(sizeof(bbcode_parse_tree_child_p) * BBCODE_BUFFER);
	tree->multiparts = NULL; 
	tree->conditions = bbcode_parse_stack_create();
	tree->parent_node = NULL;
	tree->argument = tree->open_string = tree->close_string = NULL;
	return tree;
}

/* Free the resources taken by a tree */
void bbcode_tree_free(bbcode_parse_tree_p tree) {
	long i;
	for (i=0; i < tree->childs.size; i++) {
		if (tree->childs.element[i]->type==BBCODE_TREE_CHILD_TYPE_TREE) {
			bbcode_tree_free(tree->childs.element[i]->tree);
		} else {
			bdestroy(tree->childs.element[i]->string);
		}
		bbcode_tree_child_destroy(tree->childs.element[i]);
	}
	if (tree->childs.element != NULL){
		free(tree->childs.element);
	}
	if (tree->argument != NULL){
		bdestroy(tree->argument);
	}
	if (tree->open_string != NULL) {
		bdestroy(tree->open_string);
	}
	if (tree->close_string != NULL) {
		bdestroy(tree->close_string);
	}
	if (tree->multiparts != NULL) {
		if (tree->multiparts->size==1){
			bbcode_parse_stack_free(tree->multiparts);
		} else {
			for (i=0; i<tree->multiparts->size; i++){
				if (tree->multiparts->element[i] == tree){
					bbcode_parse_drop_element_at(tree->multiparts,i);
					break;
				}
			}
		}
	}
	if (tree->conditions != NULL) {
		bbcode_parse_stack_free(tree->conditions);
	}
	free (tree);
}

/* Check if there is sufficient space in child array */
void bbcode_tree_check_child_size(bbcode_parse_tree_p tree, long size) {
	if (tree->childs.msize < size) {
		tree->childs.element=(bbcode_parse_tree_child_pp) realloc(
				tree->childs.element, sizeof(bbcode_parse_tree_child_p)*(size
						+BBCODE_BUFFER));
		tree->childs.msize=size+BBCODE_BUFFER;
		tree->childs.element[tree->childs.size]=NULL;
	}
}

/* adds a child to the current list (sub_tree) */
void bbcode_tree_push_tree_child(bbcode_parser_p parser,
		bbcode_parse_tree_p tree, bbcode_parse_tree_array_p work,
		bbcode_parse_tree_array_p close, bstring open_string, long tag_id,
		bstring argument, long offset) {
	bbcode_parse_tree_p tmp_tree = NULL;
	/* Tree creation */
	if (((bbcode_get_bbcode(parser,tag_id)->flags) & BBCODE_FLAGS_ONE_OPEN_PER_LEVEL) && ((bbcode_get_cn(parser)->tag_id) == tag_id) ){
		bstring empty=bfromcstr("");
		bbcode_close_tag(parser,tree, work, close, tag_id, empty, 1, offset);
	}
	tmp_tree=bbcode_tree_create();
	tmp_tree->tag_id=tag_id;
	tmp_tree->flags=BBCODE_TREE_FLAGS_MULTIPART_FIRST_NODE;
	tmp_tree->open_string=open_string;
	if (argument == NULL) {
		tmp_tree->argument=NULL;
	} else {
		tmp_tree->argument = bstrcpy(argument);
	}
	tmp_tree->parent_node=bbcode_array_element(work,bbcode_array_length(work)-1);

	/* Adding tree to complete structure */
	bbcode_tree_check_child_size(bbcode_get_cn(parser), bbcode_get_cn(parser)->childs.size+1);
	tmp_tree->parent_node->childs.element[bbcode_get_cn(parser)->childs.size] = bbcode_tree_child_create();
	tmp_tree->parent_node->childs.element[bbcode_get_cn(parser)->childs.size]->tree=tmp_tree;
	tmp_tree->parent_node->childs.element[bbcode_get_cn(parser)->childs.size]->type
			=BBCODE_TREE_CHILD_TYPE_TREE;
	tmp_tree->parent_node->childs.element[bbcode_get_cn(parser)->childs.size]->offset=offset;
	tmp_tree->parent_node->childs.size++;
	bbcode_parse_stack_push_element(work,tmp_tree);
	parser->current_node=tmp_tree;
}

/* adds a child to the current list (string_leaf) */
void bbcode_tree_push_string_child(bbcode_parse_tree_p tree, bstring string, long offset) {
	bbcode_tree_check_child_size(tree, tree->childs.size+1);
	if (blength(string)==0){
		bdestroy(string);
		return;
	}
	tree->childs.element[tree->childs.size] = bbcode_tree_child_create();
	tree->childs.element[tree->childs.size]->string=string;
	tree->childs.element[tree->childs.size]->type=BBCODE_TREE_CHILD_TYPE_STRING;
	tree->childs.element[tree->childs.size]->offset=offset;
	tree->childs.size++;
}

/* adds a tree to the current list (raw) */
void bbcode_tree_push_tree_raw(bbcode_parser_p parser, bbcode_parse_tree_p tree,
		bbcode_parse_tree_p tmp_tree, bbcode_parse_tree_array_p work) {
	bbcode_tree_check_child_size(tree, tree->childs.size+1);
	tree->childs.element[tree->childs.size]=bbcode_tree_child_create();
	tree->childs.element[tree->childs.size]->type=BBCODE_TREE_CHILD_TYPE_TREE;
	tree->childs.element[tree->childs.size]->tree=tmp_tree;
	bbcode_parse_stack_push_element(work, tmp_tree);
	tmp_tree->parent_node=tree;
	parser->current_node=tmp_tree;
	tree->childs.size++;
}

/* Get the last child and removes it from the list */
void bbcode_tree_pop_child(bbcode_parse_tree_p tree,
		bbcode_parse_tree_child_p bbcode_parse_tree_child) {
	bbcode_parse_tree_child=tree->childs.element[tree->childs.size];
	tree->childs.size--;
}

/* Insert a given child on a given position */
void bbcode_tree_insert_child_at(bbcode_parse_tree_p tree,
		bbcode_parse_tree_child_p bbcode_parse_tree_child, long pos) {
	long size;
	bbcode_tree_check_child_size(tree, tree->childs.size+1);
	size=sizeof(bbcode_parse_tree_child_p);
	memmove(&(tree->childs.element[pos+1]), 
			&(tree->childs.element[pos]), size*(tree->childs.size-pos-1));
	tree->childs.element[pos]=bbcode_parse_tree_child;
	tree->childs.size+=1;
}

/* Mark an element closed, (and also multipart elements) */
void bbcode_tree_mark_element_closed(bbcode_parse_tree_p tree) {
	bbcode_apply_flag_to_parts(tree->multiparts, BBCODE_TREE_FLAGS_PAIRED, 1)
	tree->flags |=BBCODE_TREE_FLAGS_PAIRED;
}

/* Move a child set from a parent to another */
void bbcode_tree_move_childs(bbcode_parse_tree_p from, bbcode_parse_tree_p to,
		long offset_from, long count, long offset_to) {
	long i;
	if ( from->childs.size - offset_from < count ) {
		count=from->childs.size-offset_from;
	}
	if ( count == 0 ) {
		return;
	}
	bbcode_tree_check_child_size(to, to->childs.size + count);
	if ( to->childs.size > offset_to ) {
		/* We First Move the current childs to leave space */
		for ( i = to->childs.size -1; i >= offset_to; i-- ) {
			to->childs.element[ i + count ] = to->childs.element[ i ];
		}
	}
	/* Setting the sizes to correct values */
	to->childs.size += count;
	from->childs.size -= count;
	/* Copying Childs From Old Position to new */
	for ( i = 0; i < count; i++) {
		to->childs.element[ offset_to + i ] = from->childs.element[ offset_from + i ];
		if ((to->childs.element[ offset_to + i ])->type == BBCODE_TREE_CHILD_TYPE_TREE){
			(to->childs.element[ offset_to + i ])->tree->parent_node = to;
		}
	}
	/* Reducing Child Set In From Elements */
	for (i = offset_from; i < from->childs.size; i++) {
		from->childs.element[ i ] = from->childs.element[ i + count ];
	}
}

/*---------------------------
 Parse Tree array Manipulation API
 ---------------------------*/
/* Create a Tree array */
bbcode_parse_tree_array_p bbcode_parse_stack_create() {
	bbcode_parse_tree_array_p array = NULL;
	array=malloc(sizeof(bbcode_parse_tree_array));
	array->size=0;
	array->msize=0;
	array->element=NULL;
	return array;
}

/* Free resource used by a Tree array */
void bbcode_parse_stack_free(bbcode_parse_tree_array_p stack) {
	if (stack->element != NULL) {
		free(stack->element);
		stack->size=0;
		stack->msize=0;
		stack->element = NULL;
	}
	free(stack);
}

/* Check if there is room for adding elements */
void bbcode_parse_stack_check_size(bbcode_parse_tree_array_p stack, long size) {
	if (stack->msize<size) {
		stack->element=(bbcode_parse_tree_pp) realloc(stack->element,
				(BBCODE_BUFFER+size)*sizeof(bbcode_parse_tree_p));
		stack->msize=(BBCODE_BUFFER+size);
	}
}

/* Add element to the Tree array */
void bbcode_parse_stack_push_element(bbcode_parse_tree_array_p stack,
		bbcode_parse_tree_p element) {
	bbcode_parse_stack_check_size(stack, stack->size+1);
	stack->element[stack->size]=element;
	stack->size++;
}

bbcode_parse_tree_p bbcode_parse_stack_pop_elemen(
		bbcode_parse_tree_array_p stack) {
	stack->size--;
	return stack->element[stack->size];
}

/* Remove element from the Tree array  without giving it back */
void bbcode_parse_stack_pop_element_loose(bbcode_parse_tree_array_p stack) {
	stack->size--;
}

/* Remove element from the Tree array @ index */
void bbcode_parse_drop_element_at(bbcode_parse_tree_array_p stack, long index) {
	if (index<stack->size) {
		long i;
		stack->size--;

		for(i=index;i<stack->size;i++){
			stack->element[i]=stack->element[i+1];
		}
	}
}

/* Init a tree child */
bbcode_parse_tree_child_p bbcode_tree_child_create(){
	bbcode_parse_tree_child_p child;
	child=(bbcode_parse_tree_child_p)malloc(sizeof(bbcode_parse_tree_child));
	return child;
}

/* Free a tree child */
void bbcode_tree_child_destroy(bbcode_parse_tree_child_p child){
	free(child);
}
/* Check if a char is escaped */
int bbcode_is_escaped(bstring string, int pos){
	return bchar(string, pos--)=='\\' && !bbcode_is_escaped(string,pos);
}
/* Remove escaped strings */
int bbcode_strip_escaping(bstring string){
	int i;
	for (i=0; i<blength(string); ++i){
		if (bchar(string,i)=='\\'){
			bdelete(string, i, 1);
		}
	}
}

/* void main() {
	bbcode_parser_p parser = bbcode_parser_create();
	bbcode_parser_set_flags(parser, BBCODE_AUTO_CORRECT|BBCODE_ARG_DOUBLE_QUOTE|BBCODE_ARG_SINGLE_QUOTE);
	bbcode_parser_add_ruleset(parser, BBCODE_TYPE_NOARG, 0, "b", 1, "<b>", 3, "</b>", 4, "", 0, "all", 3, "all", 3, NULL, NULL, NULL, NULL);
	bbcode_parser_add_ruleset(parser, BBCODE_TYPE_NOARG, 0, "i", 1, "<i>", 3, "</i>", 4, "", 0, "all", 3, "all", 3, NULL, NULL, NULL, NULL);
	bbcode_parser_add_ruleset(parser, BBCODE_TYPE_ARG, 0, "url", 1, "<a href=\"{PARAM}\">", 18, "</a>", 4, "", 0, "all", 3, "all", 3, NULL, NULL, NULL, NULL);
	char *ret;
	long i;
	char *string="[b], [i]Test, [/b] [url='http://www.bmco.be/']Coucou[/url][/i] Blug";
	bbcode_parser_add_smiley(parser, ":D", 2, "replaced", 8);
	ret = bbcode_parse(parser, string, strlen(string), &i);
	bbcode_parser_free(parser);
	printf(ret);
	free(ret);
	printf("\n");
} */
/*---------------------------
 Built-in callbacks
 ---------------------------*/
