--TEST--
BBCODE Deny Reopen TAG FLAG
--EXTENSIONS--
bbcode
--FILE--
<?php
$arrayBBCode=array(
    'b'=>        array('type'=>BBCODE_TYPE_NOARG, 'flags'=>BBCODE_FLAGS_DENY_REOPEN_CHILD, 'open_tag'=>'<b>', 'close_tag'=>'</b>'),
    'u'=>        array('type'=>BBCODE_TYPE_NOARG, 'open_tag'=>'<u>', 'close_tag'=>'</u>'),
    'i'=>        array('type'=>BBCODE_TYPE_NOARG, 'open_tag'=>'<i>', 'close_tag'=>'</i>'),
);
$text="[u][b] [i]Simple [/b] [/u]Test [/i]";
$BBHandler=bbcode_create($arrayBBCode);
bbcode_set_flags($BBHandler,BBCODE_CORRECT_REOPEN_TAGS);
echo bbcode_parse($BBHandler,$text);
?>
--EXPECT--
<u><b> <i>Simple </i></b> </u>Test 