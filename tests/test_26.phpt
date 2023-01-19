--TEST--
BBCODE Param and Content Callbacks
--EXTENSIONS--
bbcode
--FILE--
<?php
Class Foo_C{
	static function bar($content,$param){
		return "Foo_C::bar ".$content;
	}
	function foo($content, $param) {
		return "Foo_C->foo_cb() ".$content;
	}
}
function C_callback($content, $param){
	return "C_Callback ".$content;
}
Class Foo_P{
	static function bar($content,$param){
		return "Foo_P::bar ".$param;
	}
	function foo($content, $param) {
		return "Foo_P->foo() ".$param;
	}
}
function P_callback($content, $param){
	return "P_Callback ".$param;
}
function f_empty(){}

$foo_C = new Foo_C;
$foo_P = new Foo_P;

$arrayBBCode=array(
	'i'=>		array('type' => BBCODE_TYPE_ARG, 'open_tag' => '<i {PARAM}>', 'close_tag' => '</i>',
					'content_handling' => array('Foo_C', 'bar'),
					'param_handling'   => array('Foo_P', 'bar'),
				),
	'b'=>		array('type' => BBCODE_TYPE_ARG, 'open_tag' => '<b {PARAM}>', 'close_tag' => '</b>',
					'content_handling' => array($foo_C, 'foo'),
					'param_handling'   => array($foo_P, 'foo'),
				),
	'u'=>		array('type' => BBCODE_TYPE_ARG, 'open_tag' => '<u {PARAM}>', 'close_tag' => '</u>',
					'content_handling' => 'C_callback',
					'param_handling'   => 'P_callback',
				),
	'e'=>		array('type' => BBCODE_TYPE_ARG, 'open_tag' => '<e {PARAM}>', 'close_tag' => '</e>',
					'content_handling' => 'f_empty',
					'param_handling'   => 'f_empty',
				),
);
$text=<<<EOF
[u=aa]Underlined Text[/u]
[i=aa]Italic Text[/i]
[b=aa]Bold Text[/b]
[e=aa]Emptied Text[/e]
EOF;
$BBHandler = bbcode_create($arrayBBCode);
echo "== Run #1 ==\n", bbcode_parse($BBHandler,$text);
echo "\n";
echo "== Run #2 ==\n", bbcode_parse($BBHandler,$text);
echo "\n";
echo "== Run #3 ==\n", bbcode_parse($BBHandler,$text);
bbcode_destroy($BBHandler);
?>
--EXPECT--
== Run #1 ==
<u P_Callback aa>C_Callback Underlined Text</u>
<i Foo_P::bar aa>Foo_C::bar Italic Text</i>
<b Foo_P->foo() aa>Foo_C->foo_cb() Bold Text</b>
<e ></e>
== Run #2 ==
<u P_Callback aa>C_Callback Underlined Text</u>
<i Foo_P::bar aa>Foo_C::bar Italic Text</i>
<b Foo_P->foo() aa>Foo_C->foo_cb() Bold Text</b>
<e ></e>
== Run #3 ==
<u P_Callback aa>C_Callback Underlined Text</u>
<i Foo_P::bar aa>Foo_C::bar Italic Text</i>
<b Foo_P->foo() aa>Foo_C->foo_cb() Bold Text</b>
<e ></e>
