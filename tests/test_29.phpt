--TEST--
BBCODE Limit Args Nested
--EXTENSIONS--
bbcode
--FILE--
<?php
$text = <<< MSG
[b][i][b]foo[/b][/i]
[i][b]bar[/b][/i][/b]
[i][b]Foo[/b] - Bar[/i]
[i]Test[b]2[/b][/i]
MSG;

$BBCode = array(
	'i' => array('type'=> BBCODE_TYPE_NOARG, 'max'=>2, 'open_tag'=>'<i>', 'close_tag'=>'</i>'),
	'b' => array('type'=> BBCODE_TYPE_NOARG, 'max'=>2, 'open_tag'=>'<b>', 'close_tag'=>'</b>'),
);

echo bbcode_parse(bbcode_create($BBCode), $text);
?>
--EXPECT--
[b]<i><b>foo</b></i>
<i><b>bar</b></i>[/b]
[i][b]Foo[/b] - Bar[/i]
[i]Test[b]2[/b][/i]

