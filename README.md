# PHP - PECL bbcode extension<br><br>
Compatibile with php 7.x, 8.x, PHP >= 8.2 (it also supports the latest versions)<br>
You can find the documenation here http://php.adamharvey.name/manual/en/ref.bbcode.php <br>
Original version can be found here https://pecl.php.net/package/bbcode <br>
<br>
<br />
# Installation on Linux <br />
* (recommended ubuntu 20.04 and above)


```
apt install php-pear php8.2-dev

git clone https://github.com/Csaba27/php_pecl_bbcode.git

cd php_pecl_bbcode && phpize8.2

./configure --with-php-config=/usr/bin/php-config8.2

make clean && make && make install

echo "extension=bbcode.so" > /etc/php/8.2/mods-available/bbcode.ini

# Symlink to fpm version
ln -s /etc/php/8.2/mods-available/bbcode.ini /etc/php/8.2/fpm/conf.d/20-bbcode.ini

# Symlink to cli version
ln -s /etc/php/8.2/mods-available/bbcode.ini /etc/php/8.2/cli/conf.d/20-bbcode.ini

# Restart fpm service for apply changes
service php8.2-fpm restart
```

# Test
```
php8.2 -d extension=bbcode.so -i | grep "BBCode Extension Version"
```

# Uninstall
```
rm /etc/php/8.2/mods-available/bbcode.ini && rm (php-config8.2 --extension-dir)/bbcode.so

service php8.2-fpm restart
```

# Buy me a coffe

[![paypal](https://www.paypalobjects.com/en_US/i/btn/btn_donateCC_LG.gif)](https://www.paypal.com/donate/?hosted_button_id=Z4L2Q8NE2JPRS)
