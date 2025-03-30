package com.example.dronegpsmonitor;

import android.Manifest;
import android.annotation.SuppressLint;
import android.bluetooth.*;
import android.bluetooth.le.BluetoothLeScanner;
import android.bluetooth.le.ScanCallback;
import android.bluetooth.le.ScanResult;
import android.content.pm.PackageManager;
import android.os.Bundle;
import android.os.Handler;
import android.os.Looper;
import android.util.Log;
import android.view.View;
import android.widget.Button;
import android.widget.TextView;
import android.widget.Toast;
import androidx.annotation.NonNull;
import androidx.annotation.RequiresPermission;
import androidx.appcompat.app.AppCompatActivity;
import androidx.core.app.ActivityCompat;
import androidx.core.content.ContextCompat;
import com.google.gson.Gson;
import com.google.gson.annotations.SerializedName;
import java.nio.charset.StandardCharsets;
import java.util.UUID;

public class MainActivity extends AppCompatActivity {
    // UUIDs deben coincidir con el ESP32
    private static final UUID SERVICE_UUID = UUID.fromString("4fafc201-1fb5-459e-8fcc-c5c9c331914b");
    private static final UUID CHARACTERISTIC_UUID = UUID.fromString("beb5483e-36e1-4688-b7f5-ea07361b26a8");
    private static final UUID DESCRIPTOR_UUID = UUID.fromString("00002902-0000-1000-8000-00805f9b34fb");
    private static final int PERMISSION_REQUEST_CODE = 1001;

    // Views
    private Button btnScan;
    private TextView tvStatus;
    private TextView tvLat;
    private TextView tvLon;
    private TextView tvAlt;
    private TextView tvSpeed;
    private TextView tvSatellites;
    private TextView tvRawData;

    // BLE
    private BluetoothAdapter bluetoothAdapter;
    private BluetoothGatt bluetoothGatt;
    private boolean connected = false;
    private final Gson gson = new Gson();

    // Manejo de chunks BLE
    private final StringBuilder jsonBuffer = new StringBuilder();
    private final Handler handler = new Handler(Looper.getMainLooper());
    private static final long JSON_TIMEOUT_MS = 200; // Tiempo para considerar mensaje completo

    // Modelo de datos para JSON abreviado
    public static class GPSData {
        @SerializedName("la")
        public Double latitude;  // latitud

        @SerializedName("lo")
        public Double longitude; // longitud

        @SerializedName("al")
        public Double altitude;  // altitud

        @SerializedName("sp")
        public Double speed;    // velocidad

        @SerializedName("sat")
        public Integer satellites; // satélites
    }

    // Variables para manejo de delimitadores
    private static final String DELIMITER = "---";
    private boolean lookingForDelimiter = true;

    // Modificar processJsonRunnable para resetear estado
    private final Runnable processJsonRunnable = new Runnable() {
        @Override
        public void run() {
            if (jsonBuffer.length() > 0) {
                try {
                    String completeJson = jsonBuffer.toString();
                    jsonBuffer.setLength(0);
                    processGPSData(completeJson);
                } catch (Exception e) {
                    Log.e("GPS", "Error processing JSON", e);
                }
            }
            lookingForDelimiter = true; // Volver a buscar delimitador
        }
    };

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.activity_main);

        initViews();
        initBluetooth();
        setupButton();
    }

    private void initViews() {
        btnScan = findViewById(R.id.btn_scan);
        tvStatus = findViewById(R.id.tv_status);
        tvLat = findViewById(R.id.tv_lat);
        tvLon = findViewById(R.id.tv_lon);
        tvAlt = findViewById(R.id.tv_alt);
        tvSpeed = findViewById(R.id.tv_speed);
        tvSatellites = findViewById(R.id.tv_satellites);
        tvRawData = findViewById(R.id.tv_raw_data);
    }

    private void initBluetooth() {
        BluetoothManager bluetoothManager = (BluetoothManager) getSystemService(BLUETOOTH_SERVICE);
        if (bluetoothManager != null) {
            bluetoothAdapter = bluetoothManager.getAdapter();
        }

        if (bluetoothAdapter == null) {
            Toast.makeText(this, "Bluetooth no disponible", Toast.LENGTH_SHORT).show();
            finish();
        }
    }

    private void setupButton() {
        btnScan.setOnClickListener(v -> {
            if (checkPermissions()) {
                scanDevices();
            } else {
                requestPermissions();
            }
        });
    }

    @SuppressLint("MissingPermission")
    private void scanDevices() {
        if (bluetoothAdapter == null || !bluetoothAdapter.isEnabled()) {
            Toast.makeText(this, "Active Bluetooth primero", Toast.LENGTH_SHORT).show();
            return;
        }

        BluetoothLeScanner scanner = bluetoothAdapter.getBluetoothLeScanner();
        if (scanner == null) {
            Toast.makeText(this, "Error al obtener scanner BLE", Toast.LENGTH_SHORT).show();
            return;
        }

        ScanCallback scanCallback = new ScanCallback() {
            @Override
            public void onScanResult(int callbackType, ScanResult result) {
                super.onScanResult(callbackType, result);
                BluetoothDevice device = result.getDevice();
                if (device != null && "DroneGPS_BLE".equals(device.getName())) {
                    scanner.stopScan(this);
                    connectToDevice(device);
                }
            }

            @Override
            public void onScanFailed(int errorCode) {
                super.onScanFailed(errorCode);
                runOnUiThread(() ->
                        Toast.makeText(MainActivity.this, "Escaneo fallido: " + errorCode, Toast.LENGTH_SHORT).show());
            }
        };

        scanner.startScan(scanCallback);
        tvStatus.setText("Buscando dispositivo...");
        Toast.makeText(this, "Buscando DroneGPS_BLE...", Toast.LENGTH_SHORT).show();
    }

    @SuppressLint("MissingPermission")
    private void connectToDevice(BluetoothDevice device) {
        if (bluetoothGatt != null) {
            bluetoothGatt.close();
        }

        bluetoothGatt = device.connectGatt(this, false, gattCallback);
        tvStatus.setText("Conectando...");
    }

    private final BluetoothGattCallback gattCallback = new BluetoothGattCallback() {
        @RequiresPermission(Manifest.permission.BLUETOOTH_CONNECT)
        @Override
        public void onConnectionStateChange(BluetoothGatt gatt, int status, int newState) {
            super.onConnectionStateChange(gatt, status, newState);

            if (newState == BluetoothProfile.STATE_CONNECTED) {
                connected = true;
                runOnUiThread(() -> {
                    tvStatus.setText("Conectado");
                    Toast.makeText(MainActivity.this, "Conectado al dispositivo", Toast.LENGTH_SHORT).show();
                });
                gatt.discoverServices();
            } else if (newState == BluetoothProfile.STATE_DISCONNECTED) {
                connected = false;
                runOnUiThread(() -> {
                    tvStatus.setText("Desconectado");
                    Toast.makeText(MainActivity.this, "Desconectado", Toast.LENGTH_SHORT).show();
                });
            }
        }

        @SuppressLint("MissingPermission")
        @Override
        public void onServicesDiscovered(BluetoothGatt gatt, int status) {
            super.onServicesDiscovered(gatt, status);

            if (status == BluetoothGatt.GATT_SUCCESS) {
                BluetoothGattService service = gatt.getService(SERVICE_UUID);
                if (service != null) {
                    BluetoothGattCharacteristic characteristic =
                            service.getCharacteristic(CHARACTERISTIC_UUID);

                    if (characteristic != null) {
                        gatt.setCharacteristicNotification(characteristic, true);

                        BluetoothGattDescriptor descriptor =
                                characteristic.getDescriptor(DESCRIPTOR_UUID);

                        if (descriptor != null) {
                            descriptor.setValue(BluetoothGattDescriptor.ENABLE_NOTIFICATION_VALUE);
                            gatt.writeDescriptor(descriptor);
                        }
                    }
                }
            }
        }

        @Override
        public void onCharacteristicChanged(BluetoothGatt gatt, BluetoothGattCharacteristic characteristic) {
            super.onCharacteristicChanged(gatt, characteristic);

            byte[] data = characteristic.getValue();
            if (data != null && data.length > 0) {
                String chunk = new String(data, StandardCharsets.UTF_8);
                handleIncomingData(chunk);
            }
        }

        @Override
        public void onDescriptorWrite(BluetoothGatt gatt, BluetoothGattDescriptor descriptor, int status) {
            super.onDescriptorWrite(gatt, descriptor, status);
            if (status == BluetoothGatt.GATT_SUCCESS) {
                Log.d("BLE", "Descriptor configurado correctamente");
            }
        }
    };

    /*private void handleIncomingData(String chunk) {
        runOnUiThread(() -> {
            // Agregar chunk al buffer
            jsonBuffer.append(chunk);

            // Reiniciar temporizador para procesamiento
            handler.removeCallbacks(processJsonRunnable);
            handler.postDelayed(processJsonRunnable, JSON_TIMEOUT_MS);

            // Mostrar datos crudos (opcional)
            tvRawData.setText("Recibido: " + chunk + "\n\nBuffer: " + jsonBuffer.toString());
        });
    }*/
    private void handleIncomingData(String chunk) {
        runOnUiThread(() -> {
            if (lookingForDelimiter) {
                // Estamos buscando el delimitador de inicio
                if (chunk.contains(DELIMITER)) {
                    // Encontramos el delimitador, comenzamos nuevo mensaje
                    jsonBuffer.setLength(0);
                    lookingForDelimiter = false;

                    // Extraer cualquier dato después del delimitador
                    int delimiterEnd = chunk.indexOf(DELIMITER) + DELIMITER.length();
                    if (delimiterEnd < chunk.length()) {
                        String dataAfterDelimiter = chunk.substring(delimiterEnd);
                        jsonBuffer.append(dataAfterDelimiter);
                    }
                }
            } else {
                // Estamos recibiendo datos del mensaje
                if (chunk.contains(DELIMITER)) {
                    // Encontramos delimitador de fin
                    int delimiterPos = chunk.indexOf(DELIMITER);
                    jsonBuffer.append(chunk.substring(0, delimiterPos));

                    // Procesar el mensaje completo
                    handler.post(processJsonRunnable);

                    // Prepararse para el siguiente mensaje
                    lookingForDelimiter = true;
                } else {
                    // Añadir chunk al buffer actual
                    jsonBuffer.append(chunk);
                }
            }

            // Mostrar estado actual
            tvRawData.setText("Estado: " + (lookingForDelimiter ? "Buscando delimitador" : "Recibiendo datos") +
                    "\nBuffer: " + jsonBuffer.toString());
        });
    }

    private void processGPSData(String jsonString) {
        try {
            // Limpiar JSON (puede haber caracteres nulos o basura)
            jsonString = jsonString.replaceAll("\u0000", "").trim();

            // Verificar si es un JSON completo
            if (!jsonString.startsWith("{") || !jsonString.endsWith("}")) {
                Log.w("GPS", "JSON incompleto recibido: " + jsonString);
                return;
            }

            GPSData gpsData = gson.fromJson(jsonString, GPSData.class);

            // Actualizar UI con los datos
            String finalJsonString = jsonString;
            runOnUiThread(() -> {
                if (gpsData.latitude != null) {
                    tvLat.setText(String.format("Latitud: %.6f", gpsData.latitude));
                }

                if (gpsData.longitude != null) {
                    tvLon.setText(String.format("Longitud: %.6f", gpsData.longitude));
                }

                if (gpsData.altitude != null) {
                    tvAlt.setText(String.format("Altitud: %.1f m", gpsData.altitude));
                }

                if (gpsData.speed != null) {
                    tvSpeed.setText(String.format("Velocidad: %.1f km/h", gpsData.speed));
                }

                if (gpsData.satellites != null) {
                    tvSatellites.setText(String.format("Satélites: %d", gpsData.satellites));
                }

                // Mostrar JSON completo en raw data
                tvRawData.setText("JSON completo:\n" + finalJsonString);
            });
        } catch (Exception e) {
            Log.e("GPS", "Error parsing GPS data: " + jsonString, e);
            String finalJsonString1 = jsonString;
            runOnUiThread(() -> {
                tvRawData.setText("Error en JSON: " + finalJsonString1 + "\n\n" + e.getMessage());
                Toast.makeText(MainActivity.this, "Error en formato JSON", Toast.LENGTH_SHORT).show();
            });
        }
    }

    private boolean checkPermissions() {
        return ContextCompat.checkSelfPermission(this, Manifest.permission.ACCESS_FINE_LOCATION) == PackageManager.PERMISSION_GRANTED &&
                ContextCompat.checkSelfPermission(this, Manifest.permission.BLUETOOTH_SCAN) == PackageManager.PERMISSION_GRANTED &&
                ContextCompat.checkSelfPermission(this, Manifest.permission.BLUETOOTH_CONNECT) == PackageManager.PERMISSION_GRANTED;
    }

    private void requestPermissions() {
        ActivityCompat.requestPermissions(
                this,
                new String[]{
                        Manifest.permission.ACCESS_FINE_LOCATION,
                        Manifest.permission.BLUETOOTH_SCAN,
                        Manifest.permission.BLUETOOTH_CONNECT
                },
                PERMISSION_REQUEST_CODE
        );
    }

    @Override
    public void onRequestPermissionsResult(int requestCode, @NonNull String[] permissions, @NonNull int[] grantResults) {
        super.onRequestPermissionsResult(requestCode, permissions, grantResults);
        if (requestCode == PERMISSION_REQUEST_CODE) {
            boolean allGranted = true;
            for (int result : grantResults) {
                if (result != PackageManager.PERMISSION_GRANTED) {
                    allGranted = false;
                    break;
                }
            }
            if (allGranted) {
                scanDevices();
            } else {
                Toast.makeText(this, "Se necesitan todos los permisos", Toast.LENGTH_SHORT).show();
            }
        }
    }

    @SuppressLint("MissingPermission")
    @Override
    protected void onDestroy() {
        super.onDestroy();
        handler.removeCallbacks(processJsonRunnable);

        if (bluetoothGatt != null) {
            bluetoothGatt.disconnect();
            bluetoothGatt.close();
            bluetoothGatt = null;
        }
    }
}