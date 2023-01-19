#ifndef PHP_BBCODE_H
#define PHP_BBCODE_H

#define PHP_BBCODE_VERSION "2.0.1"
#define PHP_BBCODE_EXTNAME "bbcode"
#define PHP_BBCODE_RES_NAME "BBCode resource"

#define BBCODE_SET_FLAGS_SET	0
#define BBCODE_SET_FLAGS_ADD	1
#define BBCODE_SET_FLAGS_REMOVE 2

extern zend_module_entry bbcode_module_entry;
#define phpext_bbcode_ptr &bbcode_module_entry

#endif
