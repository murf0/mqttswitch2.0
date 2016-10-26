<?PHP
file_put_contents('file.txt', print_r($_SERVER,true));
file_put_contents('file.txt', print_r($_POST,true),FILE_APPEND);
file_put_contents('file.txt', print_r($_GET,true),FILE_APPEND);
file_put_contents('file.txt', print_r($_FILES,true),FILE_APPEND);

header('Content-type: text/plain; charset=utf8', true);

function check_header($name, $value = false) {
    if(!isset($_SERVER[$name])) {
        return false;
    }
    if($value && $_SERVER[$name] != $value) {
        return false;
    }
    return true;
}

function sendFile($path) {
    header($_SERVER["SERVER_PROTOCOL"].' 200 OK', true, 200);
    header('Content-Type: application/octet-stream', true);
    header('Content-Disposition: attachment; filename='.basename($path));
    header('Content-Length: '.filesize($path), true);
    header('x-MD5: '.md5_file($path), true);
    readfile($path);
}

if(!check_header('HTTP_USER_AGENT', 'ESP8266-http-Update')) {
    header($_SERVER["SERVER_PROTOCOL"].' 403 Forbidden', true, 403);
    echo "only for ESP8266 updater!\n";
    file_put_contents('file.txt', "FAILED at HTTP_USER_AGENTi \n",FILE_APPEND);
    exit();
}

if(
    !check_header('HTTP_X_ESP8266_STA_MAC') ||
    !check_header('HTTP_X_ESP8266_AP_MAC') ||
    !check_header('HTTP_X_ESP8266_FREE_SPACE') ||
    !check_header('HTTP_X_ESP8266_SKETCH_SIZE') ||
    !check_header('HTTP_X_ESP8266_CHIP_SIZE') ||
    !check_header('HTTP_X_ESP8266_SDK_VERSION')
) {
    header($_SERVER["SERVER_PROTOCOL"].' 403 Forbidden', true, 403);
    echo "only for ESP8266 updater! (header)\n";
    file_put_contents('file.txt', "FAILED at HTTP_ESP8266 \n",FILE_APPEND);
    exit();
}
$files = scandir('bin', SCANDIR_SORT_DESCENDING);
$newest_file = $files[0];
$latestVersion = substr(substr($newest_file, 14),0,-4);
echo $latestVersion;
if($_GET["firmware"] == "mqttSwitch2.0") {
    file_put_contents('file.txt', "Firmware \n",FILE_APPEND);
    if($_GET["version"] != $latestVersion)  {
        $file="./bin/".$_GET['firmware']."_".$latestVersion.".bin";
        file_put_contents('file.txt', "sending New binary ".$file."\n",FILE_APPEND);
        sendFile($file);
    } else {
        file_put_contents('file.txt', "304 Same image\n",FILE_APPEND);
        header($_SERVER["SERVER_PROTOCOL"].' 304 Not Modified', true, 304);
    }
    exit();
}
header($_SERVER["SERVER_PROTOCOL"].' 500 no version for ESP MAC', true, 500);

?>