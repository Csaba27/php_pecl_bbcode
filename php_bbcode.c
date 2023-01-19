#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include <stdbool.h>
#include "php.h"
#include "ext/standard/info.h"
#include "php_bbcode.h"
#include "lib/bbcode2.h"
#include "lib/bstrlib.h"

static int le_bbcode;

#define PHP_BBCODE_CONTENT_CB 1
#define PHP_BBCODE_PARAM_CB 2

/* {{{ arginfo */

#if PHP_VERSION_ID >= 80000
ZEND_BEGIN_ARG_INFO_EX(arginfo_bbcode_create, 0, 0, 1)
	ZEND_ARG_TYPE_INFO_WITH_DEFAULT_VALUE(0, bbcode_initial_tags, IS_ARRAY, 0, "NULL")
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(arginfo_bbcode_add_element, 0, 3, _IS_BOOL, 0)
	ZEND_ARG_INFO(0, bbcode_container)
	ZEND_ARG_TYPE_INFO(0, tag_name, IS_STRING, 0)
	ZEND_ARG_TYPE_INFO(0, tag_rules, IS_ARRAY, 0)
ZEND_END_ARG_INFO()	

ZEND_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(arginfo_bbcode_destroy, 0, 1, _IS_BOOL, 0)
	ZEND_ARG_INFO(0, bbcode_container)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(arginfo_bbcode_parse, 0, 2, IS_STRING, 0)
	ZEND_ARG_INFO(0, bbcode_container)
	ZEND_ARG_TYPE_INFO(0, to_parse, IS_STRING, 0)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(arginfo_bbcode_add_smiley, 0, 3, _IS_BOOL, 0)
	ZEND_ARG_INFO(0, bbcode_container)
	ZEND_ARG_TYPE_INFO(0, smiley, IS_STRING, 0)
	ZEND_ARG_TYPE_INFO(0, replace_by, IS_STRING, 0)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(arginfo_bbcode_set_flags, 0, 2, _IS_BOOL, 0)
	ZEND_ARG_INFO(0, bbcode_container)
	ZEND_ARG_TYPE_INFO(0, flags, IS_LONG, 0)
	ZEND_ARG_TYPE_INFO_WITH_DEFAULT_VALUE(0, mode, IS_LONG, 0, "BBCODE_SET_FLAGS_SET")
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(arginfo_bbcode_set_arg_parser, 0, 2, _IS_BOOL, 0)
	ZEND_ARG_INFO(0, bbcode_container)
	ZEND_ARG_INFO(0, bbcode_arg_parser)
ZEND_END_ARG_INFO()
#else
ZEND_BEGIN_ARG_INFO_EX(arginfo_bbcode_create, 0, 0, 1)
	ZEND_ARG_INFO(0, bbcode_initial_tags)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfo_bbcode_add_element, 0, 0, 3)
	ZEND_ARG_INFO(0, bbcode_container)
	ZEND_ARG_INFO(0, tag_name)
	ZEND_ARG_INFO(0, tag_rules)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfo_bbcode_destroy, 0, 0, 1)
	ZEND_ARG_INFO(0, bbcode_container)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfo_bbcode_parse, 0, 0, 2)
	ZEND_ARG_INFO(0, bbcode_container)
	ZEND_ARG_INFO(0, to_parse)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfo_bbcode_add_smiley, 0, 0, 3)
	ZEND_ARG_INFO(0, bbcode_container)
	ZEND_ARG_INFO(0, smiley)
	ZEND_ARG_INFO(0, replace_by)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfo_bbcode_set_flags, 0, 0, 2)
	ZEND_ARG_INFO(0, bbcode_container)
	ZEND_ARG_INFO(0, flags)
	ZEND_ARG_INFO(0, mode)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfo_bbcode_set_arg_parser, 0, 0, 2)
	ZEND_ARG_INFO(0, bbcode_container)
	ZEND_ARG_INFO(0, bbcode_arg_parser)
ZEND_END_ARG_INFO()
#endif
/* }}} */

/* {{{ _php_bbcode_callback_handler
	Common code for content and parameter handlers */
static int _php_bbcode_callback_handler(int cb_type, bstring content, bstring param, zval *func_name)
{
	zval retval;
	zend_string *callable = NULL;
	int i, res;
	bstring target;

	switch (cb_type) {
		case PHP_BBCODE_CONTENT_CB:
			target = content;
			break;

		case PHP_BBCODE_PARAM_CB:
			target = param;
			break;
	}

	zval* zargs = emalloc(sizeof(zval) * 2);
	ZVAL_STRINGL(&zargs[0], bdata(content), blength(content));
	ZVAL_STRINGL(&zargs[1], bdata(param), blength(param));

#if PHP_VERSION_ID >= 80000
	res = call_user_function(EG(function_table), NULL, func_name, &retval, 2, zargs);
#else
	res = call_user_function_ex(EG(function_table), NULL, func_name, &retval, 2, zargs, 1, NULL);
#endif

	if (res != SUCCESS) {
		if (!zend_is_callable(func_name, 0, &callable)) {
			php_error_docref(NULL, E_WARNING, "function `%s' is not callable", ZSTR_VAL(callable));
		} else {
			php_error_docref(NULL, E_WARNING, "callback function %s() failed", ZSTR_VAL(callable));
		}
		zend_string_release(callable);
	} else {
		convert_to_string_ex(&retval);
		if (Z_STRVAL(retval)) {
			bassignblk(target, Z_STRVAL(retval), Z_STRLEN(retval));
		} else {
			bdelete(target, 0, blength(target));
		}
		zval_ptr_dtor(&retval);
	}

	/* Free zargs */
	for (i = 0; i < 2; i++) {
		zval_ptr_dtor(&zargs[i]);
	}
	efree(zargs);

	return 0;
}
/* }}} */

/* {{{ _php_bbcode_content_handler */
static int _php_bbcode_content_handler(bstring content, bstring param, void *func_data)
{
	return _php_bbcode_callback_handler(PHP_BBCODE_CONTENT_CB, content, param, func_data);
}
/* }}} */

/* {{{ _php_bbcode_param_handler */
static int _php_bbcode_param_handler(bstring content, bstring param, void *func_data)
{
	return _php_bbcode_callback_handler(PHP_BBCODE_PARAM_CB, content, param, func_data);
}
/* }}} */

/* {{{ _php_bbcode_callable_lookup */
static bool _php_bbcode_callable_lookup(zval *l) 
{
	if(Z_TYPE_P(l) == IS_ARRAY) 
	{
		zval *classref;
		zval *method;
		zend_string *name, *lcname;
		if (((classref = zend_hash_index_find(Z_ARRVAL_P(l), 0)) == NULL)
			|| ((method = zend_hash_index_find(Z_ARRVAL_P(l), 1)) == NULL))
		{
			php_error_docref(NULL, E_WARNING, "Expected array($object, $method) or array($classname, $method)");
			return false;
		}

		zend_class_entry *ce = NULL;
		if (Z_TYPE_P(classref) != IS_OBJECT) {
			name = zval_get_string(classref);
			if (UNEXPECTED(!name))
			{
				return false;
			}

			if ((ce = zend_lookup_class(name)) == NULL) 
			{
				php_error_docref(NULL, E_WARNING, "Class \"%s\" does not exist", ZSTR_VAL(name));
				zend_string_release(name);
				return false;
			}
			zend_string_release(name);
		}
		else 
		{
			ce = Z_OBJCE_P(classref);
		}

		name = zval_get_string(method);
		if (UNEXPECTED(!name)) {
			return false;
		}

		lcname = zend_string_tolower(name);
		if (zend_hash_find_ptr(&ce->function_table, lcname) == NULL) 
		{
			php_error_docref(NULL, E_WARNING, "Method %s::%s() does not exis", ZSTR_VAL(ce->name), ZSTR_VAL(name));	
			zend_string_release(name);
			zend_string_release(lcname);
			return false;
		}
		zend_string_release(name);
		zend_string_release(lcname);
	}
	else if(Z_TYPE_P(l) == IS_STRING)
	{
		zend_function *fptr;
		zend_string *lcname = zend_string_tolower(Z_STR_P(l));
		fptr = zend_hash_find_ptr(EG(function_table), lcname);
		zend_string_release(lcname);
		if (!fptr) 
		{
			php_error_docref(NULL, E_WARNING, "Function %s() does not exist", Z_STRVAL_P(l));	
			return false;
		}
		// ce = fptr->common.scope;
	}
	return true;
}
/* }}} */

/* {{{ _php_bbcode_add_element
	Fills a bbcode_container */
static void _php_bbcode_add_element(bbcode_parser_p parser, char *tag_name, int tag_name_len, zval *content)
{
	zval *e;
	long type;
	zend_string* callback_name;
	char empty[] = "";
	char all[] = "all";
	int (*content_handling_func)(bstring content, bstring param, void *func_data) = NULL;
	int (*param_handling_func)(bstring content, bstring param, void *func_data) = NULL;
	HashTable *ht = NULL;
	long flags = 0;
	char *childs = all;
	int childs_len = 3;
	char *parents = all;
	int parents_len = 3;
	char *open_tag, *close_tag, *default_arg;
	long max_parsed=-1;
	int open_tag_len = 0, close_tag_len = 0, default_arg_len = 0;
	zval *content_handling = NULL;
	zval *param_handling = NULL;

	if (Z_TYPE_P(content) == IS_ARRAY) {
		ht = HASH_OF(content);
	}

	/* flags */
	if ((e = zend_hash_str_find(ht, "flags", strlen("flags"))) != NULL && Z_TYPE_P(e) == IS_LONG) {
		flags = Z_LVAL_P(e);
	}

	/* type */
	if ((e = zend_hash_str_find(ht, "type", strlen("type"))) != NULL && Z_TYPE_P(e) == IS_LONG) {
		type = Z_LVAL_P(e);
	} else {
		php_error_docref(NULL, E_WARNING, "No type specified for tag [%s]", tag_name);
		return;
	}

	/* open_tag */
	if ((e = zend_hash_str_find(ht, "open_tag", strlen("open_tag"))) != NULL &&
		Z_TYPE_P(e) == IS_STRING && Z_STRLEN_P(e)
	) {
		open_tag = Z_STRVAL_P(e);
		open_tag_len = Z_STRLEN_P(e);
	} else {
		open_tag = empty;
	}

	/* close_tag */
	if ((e = zend_hash_str_find(ht, "close_tag", strlen("close_tag"))) != NULL &&
		(Z_TYPE_P(e) == IS_STRING) && Z_STRLEN_P(e)
	) {
		close_tag = Z_STRVAL_P(e);
		close_tag_len = Z_STRLEN_P(e);
	} else {
		close_tag = empty;
	}

	/* default_arg */
	if ((e = zend_hash_str_find(ht, "default_arg", strlen("default_arg"))) != NULL &&
		Z_TYPE_P(e) == IS_STRING && Z_STRLEN_P(e)
	) {
		default_arg = Z_STRVAL_P(e);
		default_arg_len = Z_STRLEN_P(e);
	} else {
		default_arg = empty;
	}

	/* max_parsed */
	if ((e = zend_hash_str_find(ht, "max", strlen("max"))) != NULL && Z_TYPE_P(e) == IS_LONG) {
		max_parsed = Z_LVAL_P(e);
	}

	/* content_handling */
	if ((e = zend_hash_str_find(ht, "content_handling", strlen("content_handling"))) != NULL &&
		((Z_TYPE_P(e) == IS_STRING && Z_STRLEN_P(e)) || Z_TYPE_P(e) == IS_ARRAY)
	) {
		// SEPARATE_ZVAL(e);
		if (Z_TYPE_P(e) != IS_STRING && Z_TYPE_P(e) != IS_ARRAY)
		{
			convert_to_string_ex(e);
		}

		if(_php_bbcode_callable_lookup(e))
		{
			if (!zend_is_callable(e, 0, &callback_name)) 
			{
				php_error_docref(NULL, E_WARNING, "First argument is expected to be a valid callback, '%s' was given", ZSTR_VAL(callback_name));
				zend_string_release(callback_name);
				return;
			}
			zend_string_release(callback_name);
			content_handling = e;
			content_handling_func = _php_bbcode_content_handler;
		}

	} else {
		content_handling_func = NULL;
		content_handling = NULL;
	}

	/* param_handling */
	if ((e = zend_hash_str_find(ht, "param_handling", strlen("param_handling"))) != NULL &&
		((Z_TYPE_P(e) == IS_STRING && Z_STRLEN_P(e)) || Z_TYPE_P(e) == IS_ARRAY)
	) {
		// SEPARATE_ZVAL(e);
		if (Z_TYPE_P(e) != IS_STRING && Z_TYPE_P(e) != IS_ARRAY)
		{
			convert_to_string_ex(e);
		}

		if(_php_bbcode_callable_lookup(e))
		{
			if (!zend_is_callable(e, 0, &callback_name)) 
			{
				php_error_docref(NULL, E_WARNING, "First argument is expected to be a valid callback, '%s' was given", ZSTR_VAL(callback_name));
				zend_string_release(callback_name);
				return;
			}
			zend_string_release(callback_name);
			param_handling = e;
			param_handling_func = _php_bbcode_param_handler;
		}
	} else {
		param_handling_func = NULL;
		param_handling = NULL;
	}

	/* childs */
	if ((e = zend_hash_str_find(ht, "childs", strlen("childs"))) != NULL && Z_TYPE_P(e) == IS_STRING) {
		childs = Z_STRVAL_P(e);
		childs_len = Z_STRLEN_P(e);
	}

	/* parents */
	if ((e = zend_hash_str_find(ht, "parents", strlen("parents"))) != NULL && Z_TYPE_P(e) == IS_STRING && Z_STRLEN_P(e)) {
		parents = Z_STRVAL_P(e);
		parents_len = Z_STRLEN_P(e);
	}

	bbcode_parser_add_ruleset(parser, type, flags,
		tag_name, tag_name_len,
		open_tag, open_tag_len,
		close_tag, close_tag_len,
		default_arg, default_arg_len,
		parents, parents_len,
		childs, childs_len, max_parsed,
		param_handling_func,
		content_handling_func,
		(void *)param_handling,
		(void *)content_handling
	);
}
/* }}} */

/* {{{ Resource destructor */
ZEND_RSRC_DTOR_FUNC(php_bbcode_dtor)
{
	if (res->ptr) {
		bbcode_parser_free((bbcode_parser_p)res->ptr);
		res->ptr = NULL;
	}
}
/* }}} */

/* {{{ Module INIT / SHUTDOWN */
PHP_MINIT_FUNCTION(bbcode)
{
	le_bbcode = zend_register_list_destructors_ex(php_bbcode_dtor, NULL, PHP_BBCODE_RES_NAME, module_number);

	/* BBCODE Types */
	REGISTER_LONG_CONSTANT("BBCODE_TYPE_NOARG",					BBCODE_TYPE_NOARG, CONST_CS|CONST_PERSISTENT);
	REGISTER_LONG_CONSTANT("BBCODE_TYPE_SINGLE",				BBCODE_TYPE_SINGLE, CONST_CS|CONST_PERSISTENT);
	REGISTER_LONG_CONSTANT("BBCODE_TYPE_ARG",					BBCODE_TYPE_ARG, CONST_CS|CONST_PERSISTENT);
	REGISTER_LONG_CONSTANT("BBCODE_TYPE_OPTARG",				BBCODE_TYPE_OPTARG, CONST_CS|CONST_PERSISTENT);
	REGISTER_LONG_CONSTANT("BBCODE_TYPE_ROOT",					BBCODE_TYPE_ROOT, CONST_CS|CONST_PERSISTENT);

	/* BBCODE Flags */
	REGISTER_LONG_CONSTANT("BBCODE_FLAGS_ARG_PARSING",			BBCODE_FLAGS_ARG_PARSING, CONST_CS|CONST_PERSISTENT);
	REGISTER_LONG_CONSTANT("BBCODE_FLAGS_CDATA_NOT_ALLOWED",	BBCODE_FLAGS_CDATA_NOT_ALLOWED, CONST_CS|CONST_PERSISTENT);
	REGISTER_LONG_CONSTANT("BBCODE_FLAGS_SMILEYS_ON",			BBCODE_FLAGS_SMILEYS_ON, CONST_CS|CONST_PERSISTENT);
	REGISTER_LONG_CONSTANT("BBCODE_FLAGS_SMILEYS_OFF",			BBCODE_FLAGS_SMILEYS_OFF, CONST_CS|CONST_PERSISTENT);
	REGISTER_LONG_CONSTANT("BBCODE_FLAGS_ONE_OPEN_PER_LEVEL",	BBCODE_FLAGS_ONE_OPEN_PER_LEVEL, CONST_CS|CONST_PERSISTENT);
	REGISTER_LONG_CONSTANT("BBCODE_FLAGS_REMOVE_IF_EMPTY",		BBCODE_FLAGS_REMOVE_IF_EMPTY, CONST_CS|CONST_PERSISTENT);
	REGISTER_LONG_CONSTANT("BBCODE_FLAGS_DENY_REOPEN_CHILD",	BBCODE_FLAGS_DENY_REOPEN_CHILD, CONST_CS|CONST_PERSISTENT);

	/* Parser Flags */

	/* Quotes styles */
	REGISTER_LONG_CONSTANT("BBCODE_ARG_DOUBLE_QUOTE",			BBCODE_ARG_DOUBLE_QUOTE, CONST_CS|CONST_PERSISTENT);
	REGISTER_LONG_CONSTANT("BBCODE_ARG_SINGLE_QUOTE",			BBCODE_ARG_SINGLE_QUOTE, CONST_CS|CONST_PERSISTENT);
	REGISTER_LONG_CONSTANT("BBCODE_ARG_HTML_QUOTE",				BBCODE_ARG_HTML_QUOTE, CONST_CS|CONST_PERSISTENT);
	REGISTER_LONG_CONSTANT("BBCODE_ARG_QUOTE_ESCAPING",			BBCODE_ARG_QUOTE_ESCAPING, CONST_CS|CONST_PERSISTENT);

	/* Parsing Options */
	REGISTER_LONG_CONSTANT("BBCODE_AUTO_CORRECT",				BBCODE_AUTO_CORRECT, CONST_CS|CONST_PERSISTENT);
	REGISTER_LONG_CONSTANT("BBCODE_CORRECT_REOPEN_TAGS",		BBCODE_CORRECT_REOPEN_TAGS, CONST_CS|CONST_PERSISTENT);
	REGISTER_LONG_CONSTANT("BBCODE_DISABLE_TREE_BUILD",			BBCODE_DISABLE_TREE_BUILD, CONST_CS|CONST_PERSISTENT);

	/* Smiley Options */
	REGISTER_LONG_CONSTANT("BBCODE_DEFAULT_SMILEYS_ON",			BBCODE_DEFAULT_SMILEYS_ON, CONST_CS|CONST_PERSISTENT);
	REGISTER_LONG_CONSTANT("BBCODE_DEFAULT_SMILEYS_OFF",		BBCODE_DEFAULT_SMILEYS_OFF, CONST_CS|CONST_PERSISTENT);
	REGISTER_LONG_CONSTANT("BBCODE_FORCE_SMILEYS_OFF",			BBCODE_FORCE_SMILEYS_OFF, CONST_CS|CONST_PERSISTENT);
	REGISTER_LONG_CONSTANT("BBCODE_SMILEYS_CASE_INSENSITIVE",	BBCODE_SMILEYS_CASE_INSENSITIVE, CONST_CS|CONST_PERSISTENT);

	/* FLAG SET / ADD / REMOVE */
	REGISTER_LONG_CONSTANT("BBCODE_SET_FLAGS_SET",				BBCODE_SET_FLAGS_SET, CONST_CS|CONST_PERSISTENT);
	REGISTER_LONG_CONSTANT("BBCODE_SET_FLAGS_ADD",				BBCODE_SET_FLAGS_ADD, CONST_CS|CONST_PERSISTENT);
	REGISTER_LONG_CONSTANT("BBCODE_SET_FLAGS_REMOVE",			BBCODE_SET_FLAGS_REMOVE, CONST_CS|CONST_PERSISTENT);

	return SUCCESS;
}
/* }}} */

#if Void_0
typedef struct _bbcode_object {
	zend_object	std;
	int			rsrc_id;
} bbcode_object;

/* {{{ TODO: Not finished yet! */
static void _php_bbcode_object_free_storage(void *object)
{
	bbcode_object *intern = (bbcode_object *)object;
	zval *pointer;
	zval *return_value;

	zend_object_std_dtor(&intern->std);

	bbcode_container *container;

	if (intern->rsrc_id) {
			ALLOC_INIT_ZVAL(pointer);
			ALLOC_INIT_ZVAL(return_value);
			Z_TYPE_P(return_value) = IS_LONG;
			Z_TYPE_P(pointer) = IS_LONG;
			Z_LVAL_P(pointer) = intern->rsrc_id;
			ZEND_FETCH_RESOURCE(container, bbcode_container*, &pointer, -1, PHP_BBCODE_RES_NAME, le_bbcode);
			bbcode_destroy_tag_stack(container);
			efree(return_value);
			zval_ptr_dtor(&pointer);
			zend_list_delete(intern->rsrc_id);
	}
	efree(object);
}
static void _php_bbcode_object_new(zend_class_entry *class_type, zend_object_handlers *handlers, zend_object_value *retval)
{
	bbcode_object *intern;
	zval *tmp;

	intern = emalloc(sizeof(bbcode_object));
	memset(intern, 0, sizeof(bbcode_object));

	zend_object_std_init(&intern->std, class_type);
	zend_hash_copy(intern->std.properties, &class_type->default_properties, (copy_ctor_func_t) zval_add_ref, (void *) &tmp, sizeof(zval *));

	retval->handle = zend_objects_store_put(intern,
									(zend_objects_store_dtor_t)zend_objects_destroy_object,
									(zend_objects_free_object_storage_t) _php_bbcode_object_free_storage,
									NULL);
	retval->handlers = handlers;
}
/* }}} */
#endif

/* {{{ proto resource bbcode_container bbcode_create([array initial_tags])
	Creates a new bbcode_container and adds elements given in the array to the resource. */
PHP_FUNCTION(bbcode_create)
{
	zval *bbcode_entry = NULL;
	bbcode_parser_p parser = NULL;

	if (zend_parse_parameters(ZEND_NUM_ARGS(), "|a", &bbcode_entry) == FAILURE) {
		RETURN_NULL();
	}

	/* Container init */
	parser = bbcode_parser_create();
	if (parser == NULL) {	
		php_error_docref(NULL, E_ERROR, "Unable to allocate memory for tag_stack");
	}
	bbcode_parser_set_flags(parser, BBCODE_AUTO_CORRECT|BBCODE_ARG_DOUBLE_QUOTE|BBCODE_ARG_SINGLE_QUOTE|BBCODE_ARG_HTML_QUOTE|BBCODE_DEFAULT_SMILEYS_ON);

	/* If array given initialisation */
	if (bbcode_entry != NULL) {
		int i;
		HashTable *myht = NULL;

		if (Z_TYPE_P(bbcode_entry) == IS_ARRAY) {
			myht = HASH_OF(bbcode_entry);
		}

		i = myht ? zend_hash_num_elements(myht) : 0;
		if (i > 0) {
			zend_string *key;
			zval *data;
			HashTable *tmp_ht;

			/* Array walking */
			ZEND_HASH_FOREACH_STR_KEY_VAL(myht, key, data) {
				if (key && data != NULL) {
					tmp_ht = HASH_OF(data);
					if (tmp_ht) {
#if PHP_VERSION_ID >= 70300
						GC_PROTECT_RECURSION(tmp_ht);
#else
						tmp_ht->u.v.nApplyCount++;
#endif
					}
					_php_bbcode_add_element(parser, ZSTR_VAL(key), ZSTR_LEN(key), data);
					if (tmp_ht) {
#if PHP_VERSION_ID >= 70300
						GC_UNPROTECT_RECURSION(tmp_ht);
#else
						tmp_ht->u.v.nApplyCount--;
#endif
					}
				}
			} ZEND_HASH_FOREACH_END();
		}
	}

	/* Register resource and return it */
	RETURN_RES(zend_register_resource(parser, le_bbcode));
}
/* }}} */

/* {{{ proto boolean bbcode_add_element(resource bbcode_container, string tag_name, array tag_definition)
	Adds a new tag to the given bbcode_container. */
PHP_FUNCTION(bbcode_add_element)
{
	zval *z_bbcode_parser;
	zval *bbcode_entry;
	bbcode_parser_p parser = NULL;
	char *tag_name;
	size_t tag_name_len;

	/* tag_definition is an array with these elements:

	- required:
		o 'type' => Type of the tag (view BBCODE_TYPE_* to view available tags)

	- optional:
		o 'flags' =0 => features of the tag (view BBCODE_FLAGS_* to view capacities)
		o 'open_tag' = '' => the replacement open_tag (1)
		o 'close_tag' = '' => the replacement close_tag (1)
		o 'default_argument' = '' => For Optionnal argument tags argument taken if not given [1]
		o 'content_handling' = '' => Function name called as callback for content ( string new_content content_function ( content, param ); )
		o 'param_handling' = '' => Function name called as callback for content ( string new_content content_function ( content, param ); )
		o 'childs' = 'all' => Comma separated list of accepted childs (use !list to make a list of non accepted childs, no space between tag_names)
		o 'parents' = 'all' => Comma separated list of tags of which this tag could be a child

		[1] note that the {CONTENT} string is automatically replaced by the content of the tag and {PARAM} by the parameter
	*/

	if (zend_parse_parameters(ZEND_NUM_ARGS(), "rsa", &z_bbcode_parser, &tag_name, &tag_name_len, &bbcode_entry) == FAILURE) {
		RETURN_NULL();
	}

	if ((parser = (bbcode_parser_p)zend_fetch_resource_ex(z_bbcode_parser, PHP_BBCODE_RES_NAME, le_bbcode)) == NULL) {
		RETURN_NULL();
	}

	_php_bbcode_add_element(parser, tag_name, tag_name_len, bbcode_entry);
	RETURN_TRUE;
}
/* }}} */

/* {{{ proto boolean bbcode_destroy(resource bbcode_container)
	Frees memory resources of the given bbcode_container. */
PHP_FUNCTION(bbcode_destroy)
{
	zval *z_bbcode_parser;

	if (zend_parse_parameters(ZEND_NUM_ARGS(), "r", &z_bbcode_parser) == FAILURE) {
		RETURN_NULL();
	}

	RETURN_BOOL(zend_list_delete(Z_RES_P(z_bbcode_parser)) == SUCCESS);
}
/* }}} */

/* {{{ proto string bbcode_parse(resource bbcode_container, string to_parse)
	Returns the bbcode parsed value of the to_parse string with definitions given by bbcode_container. */
PHP_FUNCTION(bbcode_parse)
{
	zval *z_bbcode_parser;
	bbcode_parser_p parser;
	char *string;
	size_t str_len;
	char *ret_string;
	int ret_size;

	if (zend_parse_parameters(ZEND_NUM_ARGS(), "rs", &z_bbcode_parser, &string, &str_len) == FAILURE) {
		RETURN_NULL();
	}

	if ((parser = (bbcode_parser_p)zend_fetch_resource_ex(z_bbcode_parser, PHP_BBCODE_RES_NAME, le_bbcode)) == NULL) {
		RETURN_NULL();
	}

	/* convert string for bbcode_parse_string usage */
	ret_string = bbcode_parse(parser, string, str_len, &ret_size);

	RETVAL_STRINGL(ret_string, ret_size);
	free(ret_string);
}
/* }}} */

/* {{{ proto boolean bbcode_add_smiley(resource bbcode_container, string find, string replace)
	Adds a smiley to find and replace ruleset. */
PHP_FUNCTION(bbcode_add_smiley)
{
	zval *z_bbcode_parser;
	char *search, *replace;
	size_t s_len, r_len;
	bbcode_parser_p parser = NULL;

	if (zend_parse_parameters(ZEND_NUM_ARGS(), "rss", &z_bbcode_parser, &search, &s_len, &replace, &r_len) == FAILURE) {
		RETURN_NULL();
	}

	if ((parser = (bbcode_parser_p)zend_fetch_resource_ex(z_bbcode_parser, PHP_BBCODE_RES_NAME, le_bbcode)) == NULL) {
		RETURN_NULL();
	}

	bbcode_parser_add_smiley(parser, search, s_len, replace, r_len);
	RETURN_BOOL(SUCCESS);
}
/* }}} */

/* {{{ proto boolean bbcode_set_flags(resource bbcode_container, long flag, long mode)
	Set flags on parser using BBCODE_SET_FLAGS_* constants. */
PHP_FUNCTION(bbcode_set_flags)
{
	zval *z_bbcode_parser;
	long new_flags;
	long mode = 0;
	long flags;
	bbcode_parser_p parser=NULL;

	if (zend_parse_parameters(ZEND_NUM_ARGS(), "rl|l", &z_bbcode_parser, &new_flags, &mode) == FAILURE) {
		RETURN_NULL();
	}

	if ((parser = (bbcode_parser_p)zend_fetch_resource_ex(z_bbcode_parser, PHP_BBCODE_RES_NAME, le_bbcode)) == NULL) {
		RETURN_NULL();
	}

	flags = bbcode_parser_get_flags(parser);

	switch (mode) {
		case BBCODE_SET_FLAGS_ADD:
			bbcode_parser_set_flags(parser, flags | new_flags);
			break;

		case BBCODE_SET_FLAGS_REMOVE:
			bbcode_parser_set_flags(parser, flags & (~new_flags));
			break;

		default:
		case BBCODE_SET_FLAGS_SET:
			bbcode_parser_set_flags(parser, new_flags);
			break;
	}

	RETURN_BOOL(SUCCESS);
}
/* }}} */

/* {{{ proto boolean bbcode_set_arg_parser(resource bbcode_container, resource bbcode_child)
	Defines a bbcode_argument_parser. */
PHP_FUNCTION(bbcode_set_arg_parser)
{
	zval *z_bbcode_parser;
	zval *z_bbcode_parser_child;
	bbcode_parser_p parser = NULL;
	bbcode_parser_p arg_parser = NULL;

	if (zend_parse_parameters(ZEND_NUM_ARGS(), "rr", &z_bbcode_parser, &z_bbcode_parser_child) == FAILURE) {
		RETURN_NULL();
	}

	if ((parser = (bbcode_parser_p)zend_fetch_resource_ex(z_bbcode_parser, PHP_BBCODE_RES_NAME, le_bbcode)) == NULL) {
		RETURN_NULL();
	}
	if ((arg_parser = (bbcode_parser_p)zend_fetch_resource_ex(z_bbcode_parser_child, PHP_BBCODE_RES_NAME, le_bbcode)) == NULL) {
		RETURN_NULL();
	}

	bbcode_parser_set_arg_parser(parser, arg_parser);

	RETURN_BOOL(SUCCESS);
}
/* }}} */

/* {{{ Module Info */
PHP_MINFO_FUNCTION(bbcode)
{
	php_info_print_table_start();
	php_info_print_table_header(2, "BBCode support", "enabled");
	php_info_print_table_row(2, "BBCode Extension Version", PHP_BBCODE_VERSION);
	php_info_print_table_row(2, "BBCode Library Version", BBCODE_LIB_VERSION);
	php_info_print_table_row(2, "BBCode Max Stack Size", "Dynamic");
	php_info_print_table_row(2, "BBCode Max Elements", "No Limit");
	php_info_print_table_end();
	php_info_print_box_start(0);
	PUTS("This extension makes use of Bstrlib available at http://bstrlib.sf.net");
	php_info_print_box_end();
}
/* }}} */

static const zend_function_entry bbcode_functions[] = { /* {{{ */
	PHP_FE(bbcode_create,			arginfo_bbcode_create)
	PHP_FE(bbcode_add_element,		arginfo_bbcode_add_element)
	PHP_FE(bbcode_destroy,			arginfo_bbcode_destroy)
	PHP_FE(bbcode_parse,			arginfo_bbcode_parse)
	PHP_FE(bbcode_add_smiley,		arginfo_bbcode_add_smiley)
	PHP_FE(bbcode_set_flags,		arginfo_bbcode_set_flags)
	PHP_FE(bbcode_set_arg_parser,	arginfo_bbcode_set_arg_parser)
	PHP_FE_END
};
/* }}} */

#if Void_0
/* Object Part, working on it */
static const zend_function_entry bbcode_funcs_object[] = {
	PHP_ME_MAPPING(__construct, bbcode_create,		NULL, 0)
	PHP_ME_MAPPING(addElement, 	bbcode_add_element,	NULL, 0)
	PHP_ME_MAPPING(destroy,		bbcode_destroy,		NULL, 0)
	PHP_ME_MAPPING(parse,		bbcode_parse,		NULL, 0)
	PHP_ME_MAPPING_END
};
#endif

zend_module_entry bbcode_module_entry = {
#if ZEND_MODULE_API_NO >= 20010901
	STANDARD_MODULE_HEADER,
#endif
	PHP_BBCODE_EXTNAME,
	bbcode_functions,
	PHP_MINIT(bbcode),
	NULL,
	NULL,
	NULL,
	PHP_MINFO(bbcode),
#if ZEND_MODULE_API_NO >= 20010901
	PHP_BBCODE_VERSION,
#endif
	STANDARD_MODULE_PROPERTIES
};

#ifdef COMPILE_DL_BBCODE
ZEND_GET_MODULE(bbcode)
#endif
