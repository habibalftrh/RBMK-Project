<?php
// Koneksi ke database MySQL
$servername = "localhost";
$username = "root";
$password = "password";
$dbname = "esp32_rbmk1";  
$api_key_value = "tPmAT5Ab3j7F99";
$api_key = $

// Proses Dekripsi Chacha20
include("chilkat_9_5_0.php");
$crypt = new CkCrypt2();
$crypt->put_CryptAlgorithm('chacha20');
$crypt->put_KeyLength(256);
$crypt->put_EncodingMode('hex');
$ivHex = '000000000000004a00000000';
$crypt->SetEncodedIV($ivHex,'hex');
$crypt->put_InitialCount(1);
$keyHex = '000102030405060708090a0b0c0d0e0f101112131415161718191a1b1c1d1e1f';
$crypt->SetEncodedKey($keyHex,'hex');
$api_key = $encStr = "";


if($_SERVER["REQUEST_METHOD"] == "POST"){
    $api_key = test_input($_POST["api_key"]);
    if($api_key == $api_key_value) {
        $encStr = test_input($_POST["encStr"]);

        $decStr = $crypt->decryptStringENC($encStr);
        echo $decStr . "\n\n"; 

        // Menerima data JSON dari request POST
        $jsonData = file_get_contents($decStr);

        // Memparse JSON menjadi array asosiatif
        $data = json_decode($jsonData, true);
        
        // Mendapatkan nilai location, temperature, humidity, dan indikator dari array
        $location = $data['location'];
        $temperature = $data['temperature'];
        $humidity = $data['humidity'];
        $indikator = $data['indikator'];
        
        $conn = new mysqli($servername, $username, $password, $dbname);
        if ($conn->connect_error) {
            die("Koneksi gagal: " . $conn->connect_error);
        }
        
        // Memasukkan data ke tabel sensor_monitoring
        $sql = "INSERT INTO sensor_monitoring (location, temperature, humidity, indikator)
        VALUES ('$location', '$temperature', '$humidity', '$indikator')";

        if ($conn->query($sql) === TRUE) {
            echo "Data berhasil disimpan ke database.";
        } else {
            echo "Error: " . $sql . "<br>" . $conn->error;
        }

        $conn->close();
    }
    else {
            echo "API Salah!";
        }
    } 
else {
        echo "Tidak ada Data yang dikirim";
}

function test_input($data) {
    $data = trim($data);
    $data = stripslashes($data);
    $data = htmlspecialchars($data);
    return $data;
}

