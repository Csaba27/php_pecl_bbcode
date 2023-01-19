--TEST--
BBCODE Content Callback
--EXTENSIONS--
bbcode
--FILE--
<?php
Class Foo{
	static function bar($content,$param){
		return "Foo.bar ".$content;
	}
	function __construct(){
	}
	function foo_cb($content, $param) {
		return "Foo.foo ".$content;
	}
}
function callback($content, $param){
	return "Callback ".$content;
}
function f_empty(){}
$foo=new Foo;
$arrayBBCode=array(
    'i'=>        array('type'=>BBCODE_TYPE_NOARG, 'open_tag'=>'<i>',
                    'close_tag'=>'</i>', 'content_handling'=>array('Foo','bar')),
    'b'=>        array('type'=>BBCODE_TYPE_NOARG, 'open_tag'=>'<b>',
                    'close_tag'=>'</b>', 'content_handling'=>array($foo,'foo_cb')),
    'u'=>        array('type'=>BBCODE_TYPE_NOARG, 'open_tag'=>'<u>',
                    'close_tag'=>'</u>', 'content_handling'=>'callback'),
    'e'=>        array('type'=>BBCODE_TYPE_NOARG, 'open_tag'=>'<e>',
                    'close_tag'=>'</e>', 'content_handling'=>'f_empty'),

);
$text=<<<EOF
[u]Underlined Text[/u]
[i]Italic Text[/i]
[b]Bold Text[/b]
[e]Underlined Text[/e]
EOF;
$BBHandler=bbcode_create($arrayBBCode);
echo bbcode_parse($BBHandler,$text);
?>
--EXPECT--
<u>Callback Underlined Text</u>
<i>Foo.bar Italic Text</i>
<b>Foo.foo Bold Text</b>
<e></e>
