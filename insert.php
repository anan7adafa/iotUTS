<?php
$servername = "localhost"; // atau IP database
$username = "root";
$password = "";
$dbname = "iotuts";

// Ambil data dari ESP32
$temperature = $_POST['temperature'];
$humidity = $_POST['humidity'];

// Buat koneksi
$conn = new mysqli($servername, $username, $password, $dbname);

// Cek koneksi
if ($conn->connect_error) {
  die("Connection failed: " . $conn->connect_error);
}

// Masukkan ke database
$sql = "INSERT INTO dht_data (temperature, humidity, timestamp) VALUES ('$temperature', '$humidity', NOW())";

if ($conn->query($sql) === TRUE) {
  echo "New record created successfully";
} else {
  echo "Error: " . $sql . "<br>" . $conn->error;
}

$conn->close();
?>
