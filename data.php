<?php
$db = new SQLite3('/home/gabriel/Desktop/Noise Monitoring Prototype/ns3-simulation/ns-3.44/simulation_data.db');
$device = 'NMS_0001';
$statusTable = $device . '_Status';

$noise = $db->query("SELECT * FROM $device ORDER BY ID DESC LIMIT 1")->fetchArray(SQLITE3_ASSOC);
$status = $db->query("SELECT * FROM $statusTable ORDER BY ID DESC LIMIT 1")->fetchArray(SQLITE3_ASSOC);

if (!$noise || !$status) {
    echo json_encode(['error' => 'No data found']);
    exit;
}

echo json_encode([
    'table' => $device,
    'timestamp' => $noise['TIMESTAMP'],
    'value' => $noise['SOUND_PRESSURE'],
    'status' => [
        'timestamp' => $status['TIMESTAMP'],
        'battery_voltage' => $status['BATTERY_VOLTAGE'],
        'battery_current' => $status['BATTERY_CURRENT'],
        'temperature' => $status['TEMPERATURE'],
        'latitude' => $status['LATITUDE'],
        'longitude' => $status['LONGITUDE']
    ]
]);
?>

