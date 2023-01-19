--TEST--
BBCODE Smileys Test
--EXTENSIONS--
bbcode
--FILE--
<?php
$arrayBBCode=array(
    ''=>        array('type'=>BBCODE_TYPE_ROOT,  'childs'=>'!i'),
    'b'=>        array('type'=>BBCODE_TYPE_NOARG, 'open_tag'=>'<b>', 'close_tag'=>'</b>'),
    'u'=>        array('type'=>BBCODE_TYPE_NOARG, 'open_tag'=>'<u>', 'close_tag'=>'</u>', 'flags'=>BBCODE_FLAGS_SMILEYS_OFF),
    'i'=>        array('type'=>BBCODE_TYPE_NOARG, 'open_tag'=>'<i>', 'close_tag'=>'</i>', 'childs'=>'b'),
);
$text=<<<EOF
[i] No parse Test [/i] :)
[b] Parsed, with smiley :( [/b]
[u] Parsed, with no smiley :D [/u]
EOF;
$BBHandler=bbcode_create($arrayBBCode);
bbcode_add_smiley($BBHandler, ":)", "<img src=\"smiley.gif\" alt=\":)\" />");
bbcode_add_smiley($BBHandler, ":(", "<img src=\"sad.gif\" alt=\":(\" />");
bbcode_add_smiley($BBHandler, ":D", "<img src=\"happy.gif\" alt=\":D\" />");
bbcode_add_smiley($BBHandler, ":p", "<img src=\"tong.gif\" alt=\":p\" />");
bbcode_add_smiley($BBHandler, ":|", "<img src=\"special.gif\" alt=\":|\" />");
bbcode_add_smiley($BBHandler, ":6:", "<img src=\"six.gif\" alt=\":6:\" />");
echo bbcode_parse($BBHandler,$text);
?>
--EXPECT--
[i] No parse Test [/i] <img src="smiley.gif" alt=":)" />
<b> Parsed, with smiley <img src="sad.gif" alt=":(" /> </b>
<u> Parsed, with no smiley :D </u>