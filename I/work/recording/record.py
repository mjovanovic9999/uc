import serial
import wave
import struct
import sys

# Configuration
SERIAL_PORT = '/dev/ttyACM0'  # Change to your port (COM3, /dev/ttyUSB0, etc.)
BAUD_RATE = 921600
SAMPLE_RATE = 16000
CHANNELS = 1
SAMPLE_WIDTH = 2  # 16-bit
OUTPUT_FILE = 'silence3.wav'
DURATION_SECONDS = 40  # Recording duration

def record_audio():
    print(f"Opening serial port {SERIAL_PORT}...")
    ser = serial.Serial(SERIAL_PORT, BAUD_RATE, timeout=1)

    # Calculate total samples needed
    total_samples = SAMPLE_RATE * DURATION_SECONDS
    samples_collected = 0
    audio_data = []

    print(f"Recording for {DURATION_SECONDS} seconds...")

    try:
        while samples_collected < total_samples:
            # Read 2 bytes (one 16-bit sample)
            data = ser.read(2)
            if len(data) == 2:
                audio_data.extend(data)
                samples_collected += 1

                # Progress indicator
                if samples_collected % SAMPLE_RATE == 0:
                    print(f"Recorded {samples_collected // SAMPLE_RATE} seconds...")

    except KeyboardInterrupt:
        print("\nRecording stopped by user")

    finally:
        ser.close()

    # Save to WAV file
    print(f"Saving to {OUTPUT_FILE}...")
    with wave.open(OUTPUT_FILE, 'wb') as wav_file:
        wav_file.setnchannels(CHANNELS)
        wav_file.setsampwidth(SAMPLE_WIDTH)
        wav_file.setframerate(SAMPLE_RATE)
        wav_file.writeframes(bytes(audio_data))

    print(f"Done! Recorded {samples_collected / SAMPLE_RATE:.1f} seconds")
    print(f"File saved as {OUTPUT_FILE}")

if __name__ == "__main__":
    try:
        record_audio()
    except serial.SerialException as e:
        print(f"Serial port error: {e}")
        print("Make sure the ESP32 is connected and the port is correct")
        sys.exit(1)
