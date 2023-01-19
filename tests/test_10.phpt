--TEST--
BBCODE No parse Test
--EXTENSIONS--
bbcode
--FILE--
<?php
$arrayBBCode=array(
    ''=>        array('type'=>BBCODE_TYPE_ROOT,  'childs'=>'!i'),
    'a'=>        array('type'=>BBCODE_TYPE_NOARG, 'open_tag'=>'<b>', 'close_tag'=>'</b>'),
    'b'=>        array('type'=>BBCODE_TYPE_NOARG, 'open_tag'=>'<u>', 'close_tag'=>'</u>'),
    'c'=>        array('type'=>BBCODE_TYPE_NOARG, 'open_tag'=>'<c>', 'close_tag'=>'</c>', 'childs'=>'b'),
    'd'=>        array('type'=>BBCODE_TYPE_NOARG, 'open_tag'=>'<b>', 'close_tag'=>'</b>'),
    'e'=>        array('type'=>BBCODE_TYPE_NOARG, 'open_tag'=>'<u>', 'close_tag'=>'</u>'),
    'f'=>        array('type'=>BBCODE_TYPE_NOARG, 'open_tag'=>'<i>', 'close_tag'=>'</i>', 'childs'=>'b'),
    'g'=>        array('type'=>BBCODE_TYPE_NOARG, 'open_tag'=>'<b>', 'close_tag'=>'</b>'),
    'h'=>        array('type'=>BBCODE_TYPE_NOARG, 'open_tag'=>'<u>', 'close_tag'=>'</u>'),
    'i'=>        array('type'=>BBCODE_TYPE_NOARG, 'open_tag'=>'<i>', 'close_tag'=>'</i>', 'childs'=>'b'),
    'abbbbbbb'=>        array('type'=>BBCODE_TYPE_NOARG, 'open_tag'=>'<b>', 'close_tag'=>'</b>'),
    'bbbbbbbb'=>        array('type'=>BBCODE_TYPE_NOARG, 'open_tag'=>'<u>', 'close_tag'=>'</u>'),
    'cbbbbbbb'=>        array('type'=>BBCODE_TYPE_NOARG, 'open_tag'=>'<c>', 'close_tag'=>'</c>', 'childs'=>'b'),
    'dbbbbbbb'=>        array('type'=>BBCODE_TYPE_NOARG, 'open_tag'=>'<b>', 'close_tag'=>'</b>'),
    'ebbbbbbb'=>        array('type'=>BBCODE_TYPE_NOARG, 'open_tag'=>'<ebbbbbbb>', 'close_tag'=>'</ebbbbbbb>'),
    'fbbbbbbb'=>        array('type'=>BBCODE_TYPE_NOARG, 'open_tag'=>'<i>', 'close_tag'=>'</i>', 'childs'=>'b'),
    'gbbbbbbb'=>        array('type'=>BBCODE_TYPE_NOARG, 'open_tag'=>'<b>', 'close_tag'=>'</b>'),
    'hbbbbbbb'=>        array('type'=>BBCODE_TYPE_NOARG, 'open_tag'=>'<u>', 'close_tag'=>'</u>'),
    'ibbbbbbb'=>        array('type'=>BBCODE_TYPE_NOARG, 'open_tag'=>'<i>', 'close_tag'=>'</i>', 'childs'=>'b'),
);
$text="[c] Testing [/c]
[ebbbbbbb] Text [/ebbbbbbb] ";
$BBHandler=bbcode_create($arrayBBCode);
echo bbcode_parse($BBHandler,$text);
?>
--EXPECT--
<c> Testing </c>
<ebbbbbbb> Text </ebbbbbbb>