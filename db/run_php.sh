#!/bin/sh
PHP_FCGI_MAX_REQUESTS=0
export PHP_FCGI_MAX_REQUESTS
daemon php-cgi -b 127.0.0.1:5000

