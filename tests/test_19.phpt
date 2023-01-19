--TEST--
BBCODE No parse Test
--EXTENSIONS--
bbcode
--FILE--
<?php
$arrayBBCode=array(
    'b'=>        array('type'=>BBCODE_TYPE_NOARG, 'open_tag'=>'<b>', 'close_tag'=>'</b>'),
    'u'=>        array('type'=>BBCODE_TYPE_NOARG, 'open_tag'=>'<u>', 'close_tag'=>'</u>'),
    'i'=>        array('type'=>BBCODE_TYPE_NOARG, 'open_tag'=>'<i>', 'close_tag'=>'</i>', 'childs'=>'b'),
);
$text="[i] Parser [b] Auto Correction [/i] at work [/b]\n";
$BBHandler=bbcode_create($arrayBBCode);
echo bbcode_parse($BBHandler,$text);
bbcode_set_flags($BBHandler,BBCODE_CORRECT_REOPEN_TAGS, BBCODE_SET_FLAGS_SET);
echo bbcode_parse($BBHandler,$text);
$text="[i] Parser [b] Auto Correction [/i] at work\n";
echo bbcode_parse($BBHandler,$text);
bbcode_set_flags($BBHandler,BBCODE_CORRECT_REOPEN_TAGS|BBCODE_AUTO_CORRECT, BBCODE_SET_FLAGS_SET);
echo bbcode_parse($BBHandler,$text);
?>
--EXPECT--
<i> Parser <b> Auto Correction </b></i> at work 
<i> Parser <b> Auto Correction </b></i><b> at work </b>
<i> Parser [b] Auto Correction </i> at work
<i> Parser <b> Auto Correction </b></i><b> at work
</b>