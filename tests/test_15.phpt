--TEST--
BBCODE List Parsing Test
--EXTENSIONS--
bbcode
--FILE--
<?php
$arrayBBCode=array(
    ''=>         array('type'=>BBCODE_TYPE_ROOT,  'childs'=>'!i'),
    'b'=>        array('type'=>BBCODE_TYPE_NOARG, 'open_tag'=>'<b>', 'close_tag'=>'</b>'),
    'u'=>        array('type'=>BBCODE_TYPE_NOARG, 'open_tag'=>'<u>', 'close_tag'=>'</u>', 'flags'),
    'i'=>        array('type'=>BBCODE_TYPE_NOARG, 'open_tag'=>'<i>', 'close_tag'=>'</i>', 'childs'=>'b'),
    'list'=>	 array('type'=>BBCODE_TYPE_OPTARG,'open_tag'=>'<ol type="{PARAM}">',							'close_tag'=>'</ol>',		'default_arg'=>'1'),
	'*'	=>		 array('type'=>BBCODE_TYPE_NOARG, 'flags'=>BBCODE_FLAGS_ONE_OPEN_PER_LEVEL, 'parents'=>'list',	'open_tag'=>'<li>',					'close_tag'=>'</li>',),
);
$text=<<<EOF
[list][*]Coucou[/*]
[*]Blug[*]Blog[/list]
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
<ol type="1"><li>Coucou</li>
<li>Blug</li><li>Blog</li></ol>