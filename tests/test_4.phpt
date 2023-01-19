--TEST--
BBCODE Multiple Level Selective Parse
--EXTENSIONS--
bbcode
--FILE--
<?php
$arrayBBCode=array(
    ''=>         array('type'=>BBCODE_TYPE_ROOT,  'childs'=>'!i'),
    'b'=>        array('type'=>BBCODE_TYPE_NOARG, 'open_tag'=>'<b>', 'close_tag'=>'</b>'),
    'u'=>        array('type'=>BBCODE_TYPE_NOARG, 'open_tag'=>'<u>', 'close_tag'=>'</u>'),
    'i'=>        array('type'=>BBCODE_TYPE_NOARG, 'open_tag'=>'<i>', 'close_tag'=>'</i>', 'childs'=>'b'),
);
$text="[b] [i] [u] Multiple Level Parse [/u] [/i] [/b]";
$BBHandler=bbcode_create($arrayBBCode);
echo bbcode_parse($BBHandler,$text);
?>
--EXPECT--
<b> <i> [u] Multiple Level Parse [/u] </i> </b>