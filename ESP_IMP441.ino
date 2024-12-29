#include "SD_MMC.h"
#include <EEPROM.h>
#include "driver/i2s.h"

#define SD_MMC_CMD 38 // CMD pin for SD_MMC
#define SD_MMC_CLK 39 // CLK pin for SD_MMC
#define SD_MMC_D0  40 // D0 pin for SD_MMC

#define I2S_MIC_CHANNEL I2S_CHANNEL_FMT_ONLY_LEFT
#define I2S_MIC_SERIAL_CLOCK GPIO_NUM_16
#define I2S_MIC_LEFT_RIGHT_CLOCK GPIO_NUM_15
#define I2S_MIC_SERIAL_DATA GPIO_NUM_17

#define SAMPLE_RATE 16000  // Sampling rate (16 kHz)
#define BITS_PER_SAMPLE I2S_BITS_PER_SAMPLE_16BIT  // 16 bits per sample
#define BUFFER_SIZE 1024  // Buffer size

#define VOLUME_SCALE 0.2  // Volume scaling factor (0.0 — silence, 1.0 — original level)
#define DURATION_SEC 10


// Global buffers for recording
int32_t buffer32[BUFFER_SIZE];  // Buffer for 32-bit data from the microphone
int16_t buffer16[BUFFER_SIZE];  // Buffer for 16-bit data to be saved to the file

void setup() {
    Serial.begin(115200);

    // Initialize the SD card
    SD_MMC.setPins(SD_MMC_CLK, SD_MMC_CMD, SD_MMC_D0);
    if (!SD_MMC.begin("/sdcard", true, true, SDMMC_FREQ_DEFAULT, 5)) {
        Serial.println("Card Mount Failed");
        while (1); // Halt if the SD card is not connected
    }
    Serial.println("SD Card was connected");

    // I2S configuration
    i2s_config_t i2s_config = {
        .mode = (i2s_mode_t)(I2S_MODE_MASTER | I2S_MODE_RX),
        .sample_rate = SAMPLE_RATE,
        .bits_per_sample = I2S_BITS_PER_SAMPLE_32BIT,
        .channel_format = I2S_CHANNEL_FMT_ONLY_LEFT,
        .communication_format = I2S_COMM_FORMAT_I2S,
        .intr_alloc_flags = ESP_INTR_FLAG_LEVEL1,
        .dma_buf_count = 4,
        .dma_buf_len = 1024,
        .use_apll = false,
        .tx_desc_auto_clear = false,
        .fixed_mclk = 0};

// Pin configuration
    i2s_pin_config_t pin_config = {
    .bck_io_num = I2S_MIC_SERIAL_CLOCK,
    .ws_io_num = I2S_MIC_LEFT_RIGHT_CLOCK,
    .data_out_num = I2S_PIN_NO_CHANGE,
    .data_in_num = I2S_MIC_SERIAL_DATA};

    if (i2s_driver_install(I2S_NUM_0, &i2s_config, 0, NULL) != ESP_OK) {
        Serial.println("Error installing I2S driver");
        while (1);
    }

    if (i2s_set_pin(I2S_NUM_0, &pin_config) != ESP_OK) {
        Serial.println("Error configuring I2S pins");
        while (1);
    }

    // File name for saving
    String concat_filename = "/audio_" + String(random(10000, 99999)) + ".wav";
    const char *fileName = concat_filename.c_str();

    // Start recording
    Serial.println("Recording sound...");
    recordAudio(fileName, DURATION_SEC); 
    Serial.println("Recording finished to file: " + concat_filename);
}

void loop() {
    // Empty
}

// Record sound in WAV format
void recordAudio(const char *path, uint32_t duration) {
    File file = SD_MMC.open(path, FILE_WRITE);
    if (!file) {
        Serial.println("Failed to open file for writing");
        return;
    }

    // WAV file header
    writeWAVHeader(file, SAMPLE_RATE, 1, BITS_PER_SAMPLE);

    uint32_t samples = SAMPLE_RATE * duration;  // Number of samples

    while (samples > 1024) {
        size_t bytesRead = 0;
        // Read data from the I2S microphone
        i2s_read(I2S_NUM_0, buffer32, BUFFER_SIZE * sizeof(int32_t), &bytesRead, portMAX_DELAY);

        // Convert 32-bit data to 16-bit
        for (int i = 0; i < bytesRead / sizeof(int32_t); i++) {
            buffer16[i] = (int16_t)((buffer32[i] >> 8) * VOLUME_SCALE);  // Apply volume scaling
        }

        // Write data to the file
        file.write((uint8_t *)buffer16, bytesRead / 2);
        samples -= bytesRead / sizeof(int32_t);
    }

    // Update the WAV file header
    updateWAVHeader(file);
    file.close();
}

// Function to write the WAV header
void writeWAVHeader(File &file, uint32_t sampleRate, uint16_t channels, uint16_t bitsPerSample) {
    uint32_t byteRate = sampleRate * channels * (bitsPerSample / 8);
    uint16_t blockAlign = channels * (bitsPerSample / 8);

    file.write((const uint8_t *)"RIFF", 4);  // ChunkID
    uint32_t chunkSize = 0; // Size will be updated later
    file.write((const uint8_t *)&chunkSize, 4);  // ChunkSize
    file.write((const uint8_t *)"WAVE", 4);  // Format
    file.write((const uint8_t *)"fmt ", 4);  // Subchunk1ID
    uint32_t subChunk1Size = 16;
    file.write((const uint8_t *)&subChunk1Size, 4);  // Subchunk1Size
    uint16_t audioFormat = 1;  // PCM
    file.write((const uint8_t *)&audioFormat, 2);  // AudioFormat
    file.write((const uint8_t *)&channels, 2);  // NumChannels
    file.write((const uint8_t *)&sampleRate, 4);  // SampleRate
    file.write((const uint8_t *)&byteRate, 4);  // ByteRate
    file.write((const uint8_t *)&blockAlign, 2);  // BlockAlign
    file.write((const uint8_t *)&bitsPerSample, 2);  // BitsPerSample
    file.write((const uint8_t *)"data", 4);  // Subchunk2ID
    uint32_t subChunk2Size = 0; // Size will be updated later
    file.write((const uint8_t *)&subChunk2Size, 4);  // Subchunk2Size
}

// Function to update the WAV header
void updateWAVHeader(File &file) {
    uint32_t fileSize = file.size();
    uint32_t dataSize = fileSize - 44;

    file.seek(4);  // Move to the ChunkSize field
    uint32_t chunkSize = fileSize - 8;
    file.write((const uint8_t *)&chunkSize, 4);

    file.seek(40);  // Move to the Subchunk2Size field
    file.write((const uint8_t *)&dataSize, 4);
}
