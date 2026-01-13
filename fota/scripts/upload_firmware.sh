#!/bin/bash

FIRMWARE_FILE=$1
VERSION=$2
VPS_IP="18.197.149.97" 
FIRMWARE_DIR="/home/ubuntu/NOVA_CAR1/fota/firmware"

if [ -z "$FIRMWARE_FILE" ] || [ -z "$VERSION" ]; then
    echo "Usage: $0 <firmware.bin> <version>"
    exit 1
fi

if [ ! -f "$FIRMWARE_FILE" ]; then
    echo "Error: File $FIRMWARE_FILE not found!"
    exit 1
fi

SIZE=$(stat -c%s "$FIRMWARE_FILE")
MD5=$(md5sum "$FIRMWARE_FILE" | awk '{print $1}")

cat > "$FIRMWARE_DIR/version.json" << JSON
{
  "version": "$VERSION",
  "url": "http://18.197.149.97:8080/firmware/$(basename $FIRMWARE_FILE)",
  "size": $SIZE,
  "md5": "$MD5",
  "date": "$(date)"
}
JSON

echo "Done: $VERSION created."

docker exec -it nova_car_mosquitto1 mosquitto_pub \
  -u "asm_user" \
  -P "543as" \
  -t "NOVA_CAR/ESP1/FOTA_Update/check" \
  -m "check"


