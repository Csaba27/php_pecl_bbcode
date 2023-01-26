# PHP - PECL bbcode extension<br><br>
Compatibile with php 7.x, 8.x, PHP >= 8.2 (it also supports the latest versions)<br>
You can find the documenation here http://php.adamharvey.name/manual/en/ref.bbcode.php <br>
Original version can be found here https://pecl.php.net/package/bbcode <br>
<br>
<br />
# Installation on Linux <br />
* (recommended ubuntu 20.04 and above)


```
$ apt install php-pear php8.2-dev
$ git clone https://github.com/Csaba27/php_pecl_bbcode_new.git
$ cd bbcode && phpize8.2
$ ./configure --with-php-config=/usr/bin/php-config8.2
$ make clean && make && make install
$ echo "extension=bbcode.so" > /etc/php/8.2/mods-available/bbcode.ini

# Symlink to cli and fpm version
$ ln -s /etc/php/8.2/mods-available/bbcode.ini /etc/php/8.2/fpm/conf.d/20-bbcode.ini
$ ln -s /etc/php/8.2/mods-available/bbcode.ini /etc/php/8.2/cli/conf.d/20-bbcode.ini
```

# Test
```
$ php8.2 -d extension=bbcode.so -i | grep "BBCode Extension Version"
```

# Buy me a coffe
https://www.paypal.com/donate/?hosted_button_id=Z4L2Q8NE2JPRS
