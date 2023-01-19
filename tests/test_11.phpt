--TEST--
BBCODE Param Callback
--EXTENSIONS--
bbcode
--FILE--
<?php
Class Foo{
	static function bar($content,$param){
		return "Foo.bar ".$param;
	}
	function __construct(){
	}
	function foo_cb($content, $param) {
		return "Foo.foo ".$param;
	}
}
function callback($content, $param){
	return "Callback ".$param;
}
function f_empty(){}
$foo=new Foo;
$arrayBBCode=array(
    'i'=>        array('type'=>BBCODE_TYPE_ARG, 'open_tag'=>'<i {PARAM}>',
                    'close_tag'=>'</i>', 'param_handling'=>array('Foo','bar')),
    'b'=>        array('type'=>BBCODE_TYPE_ARG, 'open_tag'=>'<b {PARAM}>',
                    'close_tag'=>'</b>', 'param_handling'=>array($foo,'foo_cb')),
    'u'=>        array('type'=>BBCODE_TYPE_ARG, 'open_tag'=>'<u {PARAM}>',
                    'close_tag'=>'</u>', 'param_handling'=>'callback'),
    'e'=>        array('type'=>BBCODE_TYPE_ARG, 'open_tag'=>'<e {PARAM}>',
                    'close_tag'=>'</e>', 'param_handling'=>'f_empty'),

);
$text=<<<EOF
[u=aa]Underlined Text[/u]
[i=aa]Italic Text[/i]
[b=aa]Bold Text[/b]
[e=aa]Emptied Text[/e]
EOF;
$BBHandler=bbcode_create($arrayBBCode);
echo bbcode_parse($BBHandler,$text);
?>
--EXPECT--
<u Callback aa>Underlined Text</u>
<i Foo.bar aa>Italic Text</i>
<b Foo.foo aa>Bold Text</b>
<e >Emptied Text</e>
