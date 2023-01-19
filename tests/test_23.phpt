--TEST--
BBCODE case insensitive smileys
--EXTENSIONS--
bbcode
--FILE--
<?php
$arrayBBCode=array(
    'b'=>        array('type'=>BBCODE_TYPE_NOARG, 'flags'=>BBCODE_FLAGS_DENY_REOPEN_CHILD, 'open_tag'=>'<b>', 'close_tag'=>'</b>'),
    'u'=>        array('type'=>BBCODE_TYPE_NOARG, 'open_tag'=>'<u>', 'close_tag'=>'</u>'),
    'i'=>        array('type'=>BBCODE_TYPE_NOARG, 'open_tag'=>'<i>', 'close_tag'=>'</i>'),
);
$text=":P";
$BBHandler=bbcode_create($arrayBBCode);
bbcode_add_smiley($BBHandler,":p", "REPLACED");
bbcode_set_flags($BBHandler,BBCODE_DISABLE_TREE_BUILD, BBCODE_SET_FLAGS_ADD);
echo bbcode_parse($BBHandler,$text)."\n";
bbcode_set_flags($BBHandler,BBCODE_SMILEYS_CASE_INSENSITIVE, BBCODE_SET_FLAGS_ADD);
echo bbcode_parse($BBHandler,$text);
?>
--EXPECT--
:P
REPLACED