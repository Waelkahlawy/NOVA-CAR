#!/usr/bin/env python3
"""
ESP32 Simulator - WiFi and MQTT Publisher
Simulates two ESP32 nodes (ESP1 and ESP2) connecting to WiFi and MQTT broker
Publishes sensor data to multiple MQTT topics in JSON format for InfluxDB and Grafana
"""

import time
import json
import socket
import paho.mqtt.client as mqtt
from typing import Optional
import random
import math

# ==================== Configuration ====================
# WiFi Configuration
WIFI_SSID = "we#100"  
WIFI_PASSWORD = "#50006000#"       

# MQTT Configuration
MQTT_BROKER_URI = "18.197.149.97"
MQTT_BROKER_PORT = 1883
ESP_CLIENT_ID="NOVA_CAR"
MQTT_USERNAME = "asm_user"
MQTT_PASSWORD = "543as"

# Node and Sub-Node IDs - ESP1
ESP_CLIENT_ID = "NOVA_CAR"
ESP1_NODE_ID = "ESP1"
ESP1_SUB_CABIN_DH11 = "DH11"
ESP1_SUB_GPS = "GPS"
ESP1_SUB_LDR = "LDR"
ESP1_SUB_ULTRASONIC = "ULTRASONIC"
ESP1_SUB_IMU = "IMU"
ESP1_SUB_GSM = "GSM"
ESP1_SUB_PREDICTION = "Prediction_Maintenance"
ESP1_SUB_FOTA = "FOTA"

# Node and Sub-Node IDs - ESP2
ESP_CLIENT_ID = "NOVA_CAR"
ESP2_NODE_ID = "ESP2"
ESP2_SUB_CV_DRIVER = "CV_Driver"

# MQTT Topics Structure: {ESP_CLIENT_ID}/{ESP1_NODE_ID}/{sub_ESP1_NODE_ID}/{topic}
# ESP1 Topics
MQTT_TOPIC_ESP1_CABIN_TEMP = f"{ESP_CLIENT_ID}/{ESP1_NODE_ID}/{ESP1_SUB_CABIN_DH11}/temperature"
MQTT_TOPIC_ESP1_CABIN_HUMIDITY = f"{ESP_CLIENT_ID}/{ESP1_NODE_ID}/{ESP1_SUB_CABIN_DH11}/humidity"
MQTT_TOPIC_ESP1_GPS_LATITUDE = f"{ESP_CLIENT_ID}/{ESP1_NODE_ID}/{ESP1_SUB_GPS}/latitude"
MQTT_TOPIC_ESP1_GPS_LONGITUDE = f"{ESP_CLIENT_ID}/{ESP1_NODE_ID}/{ESP1_SUB_GPS}/longitude"
MQTT_TOPIC_ESP1_GPS_SPEED = f"{ESP_CLIENT_ID}/{ESP1_NODE_ID}/{ESP1_SUB_GPS}/speed"
MQTT_TOPIC_ESP1_LDR = f"{ESP_CLIENT_ID}/{ESP1_NODE_ID}/{ESP1_SUB_LDR}/light_level"
MQTT_TOPIC_ESP1_ULTRASONIC = f"{ESP_CLIENT_ID}/{ESP1_NODE_ID}/{ESP1_SUB_ULTRASONIC}/distance"
MQTT_TOPIC_ESP1_IMU_TEMP = f"{ESP_CLIENT_ID}/{ESP1_NODE_ID}/{ESP1_SUB_IMU}/temperature_engine"
MQTT_TOPIC_ESP1_IMU_RMS = f"{ESP_CLIENT_ID}/{ESP1_NODE_ID}/{ESP1_SUB_IMU}/rms"
MQTT_TOPIC_ESP1_GSM = f"{ESP_CLIENT_ID}/{ESP1_NODE_ID}/{ESP1_SUB_GSM}/message"
MQTT_TOPIC_ESP1_PREDICTION_CONFIDENCE = f"{ESP_CLIENT_ID}/{ESP1_NODE_ID}/{ESP1_SUB_PREDICTION}/confidence"
MQTT_TOPIC_ESP1_PREDICTION_STATUS = f"{ESP_CLIENT_ID}/{ESP1_NODE_ID}/{ESP1_SUB_PREDICTION}/index_status"
MQTT_TOPIC_ESP1_ULTRASONIC_ALARM = f"{ESP_CLIENT_ID}/{ESP1_NODE_ID}/{ESP1_SUB_ULTRASONIC}/alarm_status"
MQTT_TOPIC_ESP1_GPS_TIMESTAMP = f"{ESP_CLIENT_ID}/{ESP1_NODE_ID}/{ESP1_SUB_GPS}/timestamp"
MQTT_TOPIC_ESP1_FOTA_CHECK = f"{ESP_CLIENT_ID}/{ESP1_NODE_ID}/{ESP1_SUB_FOTA}/check"
MQTT_TOPIC_ESP1_FOTA_STATUS = f"{ESP_CLIENT_ID}/{ESP1_NODE_ID}/{ESP1_SUB_FOTA}/status"

# ESP2 Topics
MQTT_TOPIC_ESP2_CV_CONFIDENCE = f"{ESP_CLIENT_ID}/{ESP2_NODE_ID}/{ESP2_SUB_CV_DRIVER}/confidence1"
MQTT_TOPIC_ESP2_CV_STATUS = f"{ESP_CLIENT_ID}/{ESP2_NODE_ID}/{ESP2_SUB_CV_DRIVER}/index_status1"

# Publishing interval (30 seconds)
PUBLISH_INTERVAL = 30

# Value ranges for realistic simulation - Updated according to requirements
GPS_LAT_MIN = 22.0
GPS_LAT_MAX = 31.0
GPS_LON_MIN = 25.0
GPS_LON_MAX = 37.0
GPS_SPEED_MIN = 50
GPS_SPEED_MAX = 120
GPS_SPEED_GAP = 5

# Thresholds
TEMP_CABIN_THRESHOLD = 45.0
HUMIDITY_THRESHOLD = 80.0
TEMP_ENGINE_THRESHOLD = 200.0
IMU_RMS_THRESHOLD = 12000.0
LDR_THRESHOLD = 30
DISTANCE_THRESHOLD = 10
PREDICTION_FAULTY_THRESHOLD =0.60 # 60% of engine
CV_THRESHOLD = 0.30 # 0.30f 

# Sensor value ranges
TEMP_CABIN_MIN = 10.0
TEMP_CABIN_MAX = 40.0
TEMP_CABIN_GAP = 0.5

HUMIDITY_MIN = 70.0
HUMIDITY_MAX = 170.0
HUMIDITY_GAP = 2.0

TEMP_ENGINE_MIN = 40.0
TEMP_ENGINE_MAX = 85.0
TEMP_ENGINE_GAP = 2.0

DISTANCE_MIN = 1
DISTANCE_MAX = 40
DISTANCE_GAP = 1

LDR_MIN = 0
LDR_MAX = 100

IMU_RMS_MIN = 2000.0
IMU_RMS_MAX = 16000.0
IMU_RMS_GAP = 100.0

# Prediction maintenance confidence range
PREDICTION_CONFIDENCE_MIN = 55
PREDICTION_CONFIDENCE_MAX = 95

# CV Driver confidence range
CV_CONFIDENCE_MIN = 80
CV_CONFIDENCE_MAX = 98

# Prediction maintenance messages
PREDICTION_MESSAGES = {
    0: "faulty engine",
    1: "healthy engine"
}

# GSM Alert Messages
GSM_ALERT_MESSAGES = {
    "ultrasonic": "ALERT: Object near car door!",
    "vibration": "ALERT: Engine vibration!",
    "engine_temp": "ALERT: Engine over temperature!",
    "cabin_temp": "ALERT: Cabin temperature high!",
    "cabin_humidity": "ALERT: Cabin humidity high!",
    "gps_lost": "WARNING: GPS lost!"
}

# CV Driver risk levels (0-9)
CV_RISK_LABELS = [
    "safe driving",
    "texting - right",
    "talking on the phone - right",
    "texting - left",
    "talking on the phone - left", 
    "operating the radio",
    "drinking",
    "reaching behind",
    "hair and makeup",
    "talking to passenger"

]


# ==================== Global Variables ====================
mqtt_client: Optional[mqtt.Client] = None
is_connected = False

# State variables for generating different values
gps_lat_counter = 0
gps_lon_counter = 0
gps_speed_counter = 0
temp_cabin_counter = 0
humidity_counter = 0
temp_engine_counter = 0
distance_counter = 0
imu_rms_counter = 0
fota_check = 0  # FOTA check status
fota_status = 0  # FOTA status: "checking" (string) or 0 (up_to_date) or 1 (failed)

# ==================== WiFi Simulation ====================
def check_wifi_connection() -> bool:
    """Simulates WiFi connection by checking network connectivity."""
    try:
        socket.create_connection(("8.8.8.8", 53), timeout=3)
        print("[WIFI] ✓ WiFi connected")
        return True
    except OSError:
        print("[WIFI] ✗ WiFi connection failed")
        return False

def wifi_init_sta():
    """Initializes WiFi station mode and connects to network."""
    print("[WIFI] Initializing WiFi Station mode...")
    print(f"[WIFI] SSID: {WIFI_SSID}")
    time.sleep(1)
    if check_wifi_connection():
        print("[WIFI] ✓ WiFi initialized successfully")
        return True
    else:
        print("[WIFI] ✗ WiFi initialization failed")
        return False

# ==================== MQTT Functions ====================
def on_connect(client, userdata, flags, rc):
    """Callback when MQTT client connects to broker."""
    global is_connected
    if rc == 0:
        is_connected = True
        print(f"[MQTT] ✓ Connected to MQTT broker ({MQTT_BROKER_URI}:{MQTT_BROKER_PORT})")
    else:
        is_connected = False
        print(f"[MQTT] ✗ Connection failed with code {rc}")

def on_disconnect(client, userdata, rc):
    """Callback when MQTT client disconnects from broker."""
    global is_connected
    is_connected = False
    print(f"[MQTT] ✗ Disconnected from MQTT broker")

def on_publish(client, userdata, mid):
    """Callback when message is published."""
    pass

def mqtt_init():
    """
    Simulates Mqtt_Init() from mqtt.c
    Initializes MQTT client and connects to broker.
    """
    global mqtt_client
    
    print("[MQTT] Initializing MQTT client...")
    print(f"[MQTT] Broker: {MQTT_BROKER_URI}:{MQTT_BROKER_PORT}")
    print(f"[MQTT] Client ID: {ESP_CLIENT_ID}")
    print(f"[MQTT] Username: {MQTT_USERNAME}")
    
    # Create MQTT client
    mqtt_client = mqtt.Client(client_id=ESP_CLIENT_ID)
    
    # Set callbacks
    mqtt_client.on_connect = on_connect
    mqtt_client.on_disconnect = on_disconnect
    mqtt_client.on_publish = on_publish
    
    # Set username and password (if provided)
    if MQTT_USERNAME and MQTT_PASSWORD:
        mqtt_client.username_pw_set(MQTT_USERNAME, MQTT_PASSWORD)
    
    try:
        # Connect to broker
        mqtt_client.connect(MQTT_BROKER_URI, MQTT_BROKER_PORT, keepalive=60)
        
        # Start network loop (non-blocking)
        mqtt_client.loop_start()
        
        # Wait for connection
        timeout = 5
        start_time = time.time()
        while not is_connected and (time.time() - start_time) < timeout:
            time.sleep(0.1)
        
        if is_connected:
            print("[MQTT] ✓ MQTT initialized successfully")
            return True
        else:
            print("[MQTT] ✗ MQTT connection timeout")
            return False
            
    except Exception as e:
        print(f"[MQTT] ✗ MQTT initialization failed: {e}")
        return False

def mqtt_publish(topic: str, data: str, qos: int = 1, retain: int = 0):
    """Publishes data to MQTT topic."""
    global mqtt_client, is_connected
    
    if not mqtt_client or not is_connected:
        print(f"[MQTT] ✗ Cannot publish: MQTT client not connected")
        return False
    
    try:
        result = mqtt_client.publish(topic, data, qos=qos, retain=retain)
        if result.rc == mqtt.MQTT_ERR_SUCCESS:
            return True
        else:
            print(f"[MQTT] ✗ Failed to publish to {topic}")
            return False
    except Exception as e:
        print(f"[MQTT] ✗ Error publishing to {topic}: {e}")
        return False

# ==================== Sensor Data Generation Functions ====================
def generate_gps_latitude():
    """Generate GPS latitude in range 22-31"""
    return round(random.uniform(GPS_LAT_MIN, GPS_LAT_MAX), 5)

def generate_gps_longitude():
    """Generate GPS longitude in range 25-37"""
    return round(random.uniform(GPS_LON_MIN, GPS_LON_MAX), 5)

def generate_gps_speed():
    """Generate GPS speed"""
    global gps_speed_counter
    speed = GPS_SPEED_MIN + (gps_speed_counter * GPS_SPEED_GAP)
    if speed > GPS_SPEED_MAX:
        gps_speed_counter = 0
        speed = GPS_SPEED_MIN
    else:
        gps_speed_counter += 1
    return speed

def generate_cabin_temp():
    """Generate cabin temperature"""
    global temp_cabin_counter
    temp = TEMP_CABIN_MIN + (temp_cabin_counter * TEMP_CABIN_GAP)
    if temp > TEMP_CABIN_MAX:
        temp_cabin_counter = 0
        temp = TEMP_CABIN_MIN
    else:
        temp_cabin_counter += 1
    return round(temp, 1)
    
def generate_humidity():
    """Generate humidity"""
    global humidity_counter
    humidity = HUMIDITY_MIN + (humidity_counter * HUMIDITY_GAP)
    if humidity > HUMIDITY_MAX:
        humidity_counter = 0
        humidity = HUMIDITY_MIN
    else:
        humidity_counter += 1
    return round(humidity, 1)

def generate_engine_temp():
    """Generate engine temperature"""
    global temp_engine_counter
    temp = TEMP_ENGINE_MIN + (temp_engine_counter * TEMP_ENGINE_GAP)
    if temp > TEMP_ENGINE_MAX:
        temp_engine_counter = 0
        temp = TEMP_ENGINE_MIN
    else:
        temp_engine_counter += 1
    return round(temp, 1)

def generate_distance():
    """Generate ultrasonic distance"""
    global distance_counter
    distance = DISTANCE_MIN + (distance_counter * DISTANCE_GAP)
    if distance > DISTANCE_MAX:
        distance_counter = 0
        distance = DISTANCE_MIN
    else:
        distance_counter += 1
    return distance

def generate_light_level():
    """Generate LDR light level (0-100)"""
    return random.randint(LDR_MIN, LDR_MAX)

def generate_imu_rms():
    """Generate IMU RMS value in range 2000-16000"""
    global imu_rms_counter
    rms = IMU_RMS_MIN + (imu_rms_counter * IMU_RMS_GAP)
    if rms > IMU_RMS_MAX:
        imu_rms_counter = 0
        rms = IMU_RMS_MIN
    else:
        imu_rms_counter += 1
    return round(rms, 1)

def generate_prediction_confidence():
    """Generate prediction maintenance confidence (55-95%)"""
    return random.randint(PREDICTION_CONFIDENCE_MIN, PREDICTION_CONFIDENCE_MAX)

def generate_prediction_index_status():
    """Generate prediction maintenance index_status (0 or 1)"""
    return random.randint(0, 1)

def generate_cv_confidence():
    """Generate CV driver confidence (80-98%)"""
    return random.randint(CV_CONFIDENCE_MIN, CV_CONFIDENCE_MAX)

def generate_cv_index_status():
    """Generate CV driver index_status (0-9)"""
    return random.randint(0, 9)

def generate_gsm_message(temp_cabin, humidity, temp_engine, rms_value, distance, gps_valid=True):
    """Generate GSM message based on sensor conditions with specific alert messages"""
    alerts = []
    
    # Check ultrasonic distance threshold (10 cm)
    if distance <= DISTANCE_THRESHOLD:
        alerts.append(GSM_ALERT_MESSAGES["ultrasonic"])
    
    # Check engine vibration threshold (12000)
    if rms_value >= IMU_RMS_THRESHOLD:
        alerts.append(GSM_ALERT_MESSAGES["vibration"])
    
    # Check engine temperature threshold (200)
    if temp_engine >= TEMP_ENGINE_THRESHOLD:
        alerts.append(GSM_ALERT_MESSAGES["engine_temp"])
    
    # Check cabin temperature threshold (45)
    if temp_cabin >= TEMP_CABIN_THRESHOLD:
        alerts.append(GSM_ALERT_MESSAGES["cabin_temp"])
    
    # Check cabin humidity threshold (80)
    if humidity >= HUMIDITY_THRESHOLD:
        alerts.append(GSM_ALERT_MESSAGES["cabin_humidity"])
    
    # Check GPS status
    if not gps_valid:
        alerts.append(GSM_ALERT_MESSAGES["gps_lost"])
    
    if alerts:
        message = "; ".join(alerts)
    else:
        message = "All systems normal"
    
    return message

def generate_alarm_status(distance):
    """Generate alarm status based on ultrasonic distance threshold"""
    return 1 if distance <= DISTANCE_THRESHOLD else 0

def generate_fota_check():
    """Generate FOTA check value (0 or 1)"""
    global fota_check
    fota_check = random.randint(0, 1)
    return fota_check

def generate_fota_status():
    """Generate FOTA status based on check value
    When check == 0: status can be "checking", then 0 (up_to_date) or 1 (failed)
    When check == 1: status is 0 (up_to_date)
    """
    global fota_status, fota_check
    if fota_check == 0:
        # If check == 0, status can be "checking", then 0 (up_to_date) or 1 (failed)
        status_choice = random.choice(["checking", 0, 1])
        if status_choice == "checking":
            fota_status = "checking"
        elif status_choice == 0:
            fota_status = 0  # up_to_date
        else:
            fota_status = 1  # failed
    else:
        # If check == 1, status is 0 (up_to_date)
        fota_status = 0  # up_to_date
    return fota_status

# ==================== ESP1 Task ====================
def esp1_task():
    """ESP1 Node - Publishes sensor data to MQTT topics"""
    iter_count = 0
    
    print("\n" + "="*60)
    print("Starting ESP1 Node - Sensor Data Publisher")
    print("="*60)
    print(f"Client ID: {ESP_CLIENT_ID}")
    print(f"Node ID: {ESP1_NODE_ID}")
    print("Sub-Nodes: cabin, gps, LDR, ultrasonic, imu, GSM, prediction_maintenance, FOTA")
    print("="*60 + "\n")
    
    while True:
        print(f"\n[ESP1] === Publishing Sensor Data - Iteration {iter_count} ===")
        
        if is_connected:
            timestamp = int(time.time())
            
            # 1. Cabin (DHT11) - Temperature
            temp_cabin = generate_cabin_temp()
            cabin_temp_data = {
                "ESP_CLIENT_ID": ESP_CLIENT_ID,
                "ESP1_NODE_ID": ESP1_NODE_ID,
                "SUB_ESP1_NODE_ID": ESP1_SUB_CABIN_DH11,
                "temperature": temp_cabin
                
            }
            mqtt_publish(MQTT_TOPIC_ESP1_CABIN_TEMP, json.dumps(cabin_temp_data), 1, 0)
            print(f"[ESP1/CABIN] Temperature: {temp_cabin}°C")
            
            # 2. Cabin (DHT11) - Humidity
            humidity = generate_humidity()
            cabin_humidity_data = {
                "ESP_CLIENT_ID": ESP_CLIENT_ID,
                "ESP1_NODE_ID": ESP1_NODE_ID,
                "SUB_ESP1_NODE_ID": ESP1_SUB_CABIN_DH11,
                "humidity": humidity
                
            }
            mqtt_publish(MQTT_TOPIC_ESP1_CABIN_HUMIDITY, json.dumps(cabin_humidity_data), 1, 0)
            print(f"[ESP1/CABIN] Humidity: {humidity}%")
            
            # 3. GPS - Latitude
            latitude = generate_gps_latitude()
            gps_lat_data = {
                "ESP_CLIENT_ID": ESP_CLIENT_ID,
                "ESP1_NODE_ID": ESP1_NODE_ID,
                "SUB_ESP1_NODE_ID": ESP1_SUB_GPS,
                "latitude": latitude
              
            }
            mqtt_publish(MQTT_TOPIC_ESP1_GPS_LATITUDE, json.dumps(gps_lat_data), 1, 0)
            print(f"[ESP1/GPS] Latitude: {latitude}")
            
            # 4. GPS - Longitude
            longitude = generate_gps_longitude()
            gps_lon_data = {
                "ESP_CLIENT_ID": ESP_CLIENT_ID,
                "ESP1_NODE_ID": ESP1_NODE_ID,
                "SUB_ESP1_NODE_ID": ESP1_SUB_GPS,
                "longitude": longitude
               
            }
            mqtt_publish(MQTT_TOPIC_ESP1_GPS_LONGITUDE, json.dumps(gps_lon_data), 1, 0)
            print(f"[ESP1/GPS] Longitude: {longitude}")
            
            # 5. GPS - Speed
            speed = generate_gps_speed()
            gps_speed_data = {
                "ESP_CLIENT_ID": ESP_CLIENT_ID,
                "ESP1_NODE_ID": ESP1_NODE_ID,
                "SUB_ESP1_NODE_ID": ESP1_SUB_GPS,
                "speed": speed
               
            }
            mqtt_publish(MQTT_TOPIC_ESP1_GPS_SPEED, json.dumps(gps_speed_data), 1, 0)
            print(f"[ESP1/GPS] Speed: {speed} km/h")
            
            # 5b. GPS - Timestamp
            gps_timestamp_data = {
                "ESP_CLIENT_ID": ESP_CLIENT_ID,
                "ESP1_NODE_ID": ESP1_NODE_ID,
                "SUB_ESP1_NODE_ID": ESP1_SUB_GPS,
                "timestamp": timestamp
              
            }
            mqtt_publish(MQTT_TOPIC_ESP1_GPS_TIMESTAMP, json.dumps(gps_timestamp_data), 1, 0)
            print(f"[ESP1/GPS] Timestamp: {timestamp}")
            
            # 6. LDR - Light Level
            light_level = generate_light_level()
            ldr_data = {
                "ESP_CLIENT_ID": ESP_CLIENT_ID,
                "ESP1_NODE_ID": ESP1_NODE_ID,
                "SUB_ESP1_NODE_ID": ESP1_SUB_LDR,
                "light_level": light_level
               
            }
            mqtt_publish(MQTT_TOPIC_ESP1_LDR, json.dumps(ldr_data), 1, 0)
            print(f"[ESP1/LDR] Light Level: {light_level}")
            
            # 7. Ultrasonic - Distance
            distance = generate_distance()
            ultrasonic_data = {
                "ESP_CLIENT_ID": ESP_CLIENT_ID,
                "ESP1_NODE_ID": ESP1_NODE_ID,
                "SUB_ESP1_NODE_ID": ESP1_SUB_ULTRASONIC,
                "distance": distance
            }
            mqtt_publish(MQTT_TOPIC_ESP1_ULTRASONIC, json.dumps(ultrasonic_data), 1, 0)
            print(f"[ESP1/ULTRASONIC] Distance: {distance} cm")
            
            # 7b. Ultrasonic - Alarm Status
            alarm_status = generate_alarm_status(distance)
            ultrasonic_alarm_data = {
                "ESP_CLIENT_ID": ESP_CLIENT_ID,
                "ESP1_NODE_ID": ESP1_NODE_ID,
                "SUB_ESP1_NODE_ID": ESP1_SUB_ULTRASONIC,
                "alarm_status": alarm_status
                
            }
            mqtt_publish(MQTT_TOPIC_ESP1_ULTRASONIC_ALARM, json.dumps(ultrasonic_alarm_data), 1, 0)
            print(f"[ESP1/ULTRASONIC] Alarm Status: {alarm_status}")
            
            # 8. IMU - Temperature Engine
            temp_engine = generate_engine_temp()
            imu_temp_data = {
                "ESP_CLIENT_ID": ESP_CLIENT_ID,
                "ESP1_NODE_ID": ESP1_NODE_ID,
                "SUB_ESP1_NODE_ID": ESP1_SUB_IMU,
                "temperature_engine": temp_engine
                
            }
            mqtt_publish(MQTT_TOPIC_ESP1_IMU_TEMP, json.dumps(imu_temp_data), 1, 0)
            print(f"[ESP1/IMU] Temperature Engine: {temp_engine}°C")
            
            # 9. IMU - RMS Value (Vibration)
            rms_value = generate_imu_rms()
            imu_rms_data = {
                "ESP_CLIENT_ID": ESP_CLIENT_ID,
                "ESP1_NODE_ID": ESP1_NODE_ID,
                "SUB_ESP1_NODE_ID": ESP1_SUB_IMU,
                "rms": rms_value
              
            }
            mqtt_publish(MQTT_TOPIC_ESP1_IMU_RMS, json.dumps(imu_rms_data), 1, 0)
            print(f"[ESP1/IMU] RMS: {rms_value}")
            
            # 10. GSM - Message
            gsm_message = generate_gsm_message(temp_cabin, humidity, temp_engine, rms_value, distance, gps_valid=True)
            gsm_data = {
                "ESP_CLIENT_ID": ESP_CLIENT_ID,
                "ESP1_NODE_ID": ESP1_NODE_ID,
                "ESP1_SUB_GSM": ESP1_SUB_GSM,
                "SUB_ESP1_NODE_ID": ESP1_SUB_GSM,
                "message": gsm_message
               
            }
            mqtt_publish(MQTT_TOPIC_ESP1_GSM, json.dumps(gsm_data), 1, 0)
            print(f"[ESP1/GSM] Message: {gsm_message}")
            
            # 11. Prediction Maintenance - Confidence
            pred_confidence = generate_prediction_confidence()
            pred_conf_data = {
                "ESP_CLIENT_ID": ESP_CLIENT_ID,
                "ESP1_NODE_ID": ESP1_NODE_ID,
                "SUB_ESP1_NODE_ID": ESP1_SUB_PREDICTION,
                "confidence": pred_confidence
               
            }
            mqtt_publish(MQTT_TOPIC_ESP1_PREDICTION_CONFIDENCE, json.dumps(pred_conf_data), 1, 0)
            print(f"[ESP1/PREDICTION] Confidence: {pred_confidence}%")
            
            # 12. Prediction Maintenance - Index Status
            pred_index = generate_prediction_index_status()
            pred_status_data = {
                "ESP_CLIENT_ID": ESP_CLIENT_ID,
                "ESP1_NODE_ID": ESP1_NODE_ID,
                "SUB_ESP1_NODE_ID": ESP1_SUB_PREDICTION,
                "index_status": pred_index,
                "message": PREDICTION_MESSAGES[pred_index],
                "faulty_threshold": PREDICTION_FAULTY_THRESHOLD
               
            }
            mqtt_publish(MQTT_TOPIC_ESP1_PREDICTION_STATUS, json.dumps(pred_status_data), 1, 0)
            print(f"[ESP1/PREDICTION] Index Status: {pred_index} - {PREDICTION_MESSAGES[pred_index]}")
            
            # 13. FOTA - Check
            fota_check_value = generate_fota_check()
            fota_check_data = {
                "ESP_CLIENT_ID": ESP_CLIENT_ID,
                "ESP1_NODE_ID": ESP1_NODE_ID,
                "SUB_ESP1_NODE_ID": ESP1_SUB_FOTA,
                "check": fota_check_value
               
            }
            mqtt_publish(MQTT_TOPIC_ESP1_FOTA_CHECK, json.dumps(fota_check_data), 1, 0)
            print(f"[ESP1/FOTA] Check: {fota_check_value}")
            
            # 14. FOTA - Status
            fota_status_value = generate_fota_status()
            fota_status_data = {
                "ESP_CLIENT_ID": ESP_CLIENT_ID,
                "ESP1_NODE_ID": ESP1_NODE_ID,
                "SUB_ESP1_NODE_ID": ESP1_SUB_FOTA,
                "status": fota_status_value,
                "check": fota_check_value
                
            }
            mqtt_publish(MQTT_TOPIC_ESP1_FOTA_STATUS, json.dumps(fota_status_data), 1, 0)
            print(f"[ESP1/FOTA] Status: {fota_status_value}")
            
            print("[ESP1] ✓ All sensor data published successfully!")
        else:
            print("[ESP1] ✗ MQTT disabled — no publish (not connected)")
        
        iter_count += 1
        time.sleep(PUBLISH_INTERVAL)

# ==================== ESP2 Task ====================
def esp2_task():
    """ESP2 Node - Publishes CV Driver data to MQTT topics"""
    iter_count = 0
    
    print("\n" + "="*60)
    print("Starting ESP2 Node - CV Driver Data Publisher")
    print("="*60)
    print(f"Client ID: {ESP_CLIENT_ID}")
    print(f"Node ID: {ESP2_NODE_ID}")
    print("Sub-Nodes: CV_Driver")
    print("="*60 + "\n")
    
    while True:
        print(f"\n[ESP2] === Publishing CV Driver Data - Iteration {iter_count} ===")
        
        if is_connected:
            timestamp = int(time.time())
            
            # 1. CV_Driver - Confidence
            cv_confidence = generate_cv_confidence()
            cv_conf_data = {
                "ESP_CLIENT_ID": ESP_CLIENT_ID,
                "ESP2_NODE_ID": ESP2_NODE_ID,
                "SUB_ESP2_NODE_ID": ESP2_SUB_CV_DRIVER,
                "confidence": cv_confidence,
                "threshold_CV": CV_THRESHOLD
              
            }
            mqtt_publish(MQTT_TOPIC_ESP2_CV_CONFIDENCE, json.dumps(cv_conf_data), 1, 0)
            print(f"[ESP2/CV_DRIVER] Confidence: {cv_confidence}% (Threshold: {CV_THRESHOLD})")
            
            # 2. CV_Driver - Index Status (0-9)
            cv_index = generate_cv_index_status()
            cv_status_data = {
                "ESP_CLIENT_ID": ESP_CLIENT_ID,
                "ESP2_NODE_ID": ESP2_NODE_ID,
                "SUB_ESP2_NODE_ID": ESP2_SUB_CV_DRIVER,
                "index_status": cv_index,
                "message": CV_RISK_LABELS[cv_index],
                "confidence": cv_confidence,
                "threshold_CV": CV_THRESHOLD
              
            }
            mqtt_publish(MQTT_TOPIC_ESP2_CV_STATUS, json.dumps(cv_status_data), 1, 0)
            print(f"[ESP2/CV_DRIVER] Index Status: {cv_index} - {CV_RISK_LABELS[cv_index]} (Confidence: {cv_confidence}%)")
            
            print("[ESP2] ✓ All CV Driver data published successfully!")
        else:
            print("[ESP2] ✗ MQTT disabled — no publish (not connected)")
        
        iter_count += 1
        time.sleep(PUBLISH_INTERVAL)

# ==================== Main Function ====================
def main():
    """Main function - Initializes WiFi and MQTT, then starts both ESP tasks"""
    import threading
    
    print("\n" + "="*60)
    print("ESP32 Simulator - Dual Node MQTT Publisher")
    print("="*60 + "\n")
    
    # Initialize WiFi
    if not wifi_init_sta():
        print("\n[ERROR] WiFi initialization failed. Exiting...")
        return
    
    time.sleep(2)
    
    # Initialize MQTT
    if not mqtt_init():
        print("\n[ERROR] MQTT initialization failed. Exiting...")
        return
    
    time.sleep(1)
    
    # Start ESP1 and ESP2 tasks in separate threads
    print("\n[MAIN] Starting ESP1 and ESP2 tasks...")
    esp1_thread = threading.Thread(target=esp1_task, daemon=True)
    esp2_thread = threading.Thread(target=esp2_task, daemon=True)
    
    esp1_thread.start()
    time.sleep(0.5)  # Small delay between starts
    esp2_thread.start()
    
    try:
        # Keep main thread alive
        while True:
            time.sleep(1)
    except KeyboardInterrupt:
        print("\n\n[MAIN] Shutting down...")
        if mqtt_client:
            mqtt_client.loop_stop()
            mqtt_client.disconnect()
        print("[MAIN] Goodbye!")

if __name__ == "__main__":
    main()
