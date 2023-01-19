--TEST--
BBCODE Limit Args
--EXTENSIONS--
bbcode
--FILE--
<?php
$text = <<< MSG
[i]foo[/i]
[i]bar[/i]
[i]Foo - Bar[/i]
MSG;

$BBCode = array(
	'i' => array('type'=> BBCODE_TYPE_NOARG, 'max'=>2, 'open_tag'=>'<i>', 'close_tag'=>'</i>'),
);

echo bbcode_parse(bbcode_create($BBCode), $text);
?>
--EXPECT--
<i>foo</i>
<i>bar</i>
[i]Foo - Bar[/i]
