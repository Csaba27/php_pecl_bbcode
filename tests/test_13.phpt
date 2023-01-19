--TEST--
BBCODE Param Quoting
--EXTENSIONS--
bbcode
--FILE--
<?php
$arrayBBCode=array(
    'center'=>    array('type'=>BBCODE_TYPE_NOARG,                        'open_tag'=>'<div style="text-align:center;">',                'close_tag'=>'</div>'),
    'strike'=>    array('type'=>BBCODE_TYPE_NOARG,                        'open_tag'=>'<span style="text-decoration:line-through;">',    'close_tag'=>'</span>'),
    'color'=>    array('type'=>BBCODE_TYPE_ARG,                            'open_tag'=>'<span style="color:{PARAM}">',                    'close_tag'=>'</span>'),
);
$text=<<<EOF
[color=navy]Navy Text[/color]
[color='red']Red Text[/color]
[color="green"]Green Text[/color]
[color=&quot;blue&quot;]Blue Text[/color]
EOF;
$BBHandler=bbcode_create($arrayBBCode);
echo bbcode_parse($BBHandler,$text);
?>
--EXPECT--
<span style="color:navy">Navy Text</span>
<span style="color:red">Red Text</span>
<span style="color:green">Green Text</span>
<span style="color:blue">Blue Text</span>