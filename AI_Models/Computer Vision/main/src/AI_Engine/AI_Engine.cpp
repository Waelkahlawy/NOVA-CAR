#include "AI_Engine.h"
#include <Arduino.h>
#include <SD_MMC.h>
#include <FS.h>


static const char* kLabels[] = {
    "safe driving", "texting - right", "talking on the phone - right", 
    "texting - left", "talking on the phone - left", "operating the radio", 
    "Drinking", "reaching behind", "hair and makeup", "talking to passenger"
};

bool AIEngine::init() {
    if (!loadModelData()) return false;

    
    tensor_arena = (uint8_t*) heap_caps_malloc(CFG_AI_ARENA_SIZE, MALLOC_CAP_SPIRAM);
    if (!tensor_arena) {
        Serial.println("AI: Failed to allocate arena");
        return false;
    }

   
    static tflite::MicroMutableOpResolver<16> resolver;
    resolver.AddConv2D();
    resolver.AddDepthwiseConv2D();
    resolver.AddFullyConnected();
    resolver.AddSoftmax();
    resolver.AddReshape();
    resolver.AddMaxPool2D();
    resolver.AddAveragePool2D();
    resolver.AddQuantize();
    resolver.AddDequantize();
    resolver.AddRelu();
    resolver.AddRelu6();
    resolver.AddAdd();
    resolver.AddMul();
    resolver.AddPad();
    resolver.AddMean();

    
    static tflite::MicroInterpreter static_interpreter(
        model, resolver, tensor_arena, CFG_AI_ARENA_SIZE);
    interpreter = &static_interpreter;

    
    if (interpreter->AllocateTensors() != kTfLiteOk) {
        Serial.println("AI: AllocateTensors failed");
        return false;
    }

    input = interpreter->input(0);
    output = interpreter->output(0);
    
    Serial.printf("AI: Model Input Shape: %dx%d\n", input->dims->data[2], input->dims->data[1]);
    return true;
}

bool AIEngine::loadModelData() {
   
    if (!SD_MMC.begin(CFG_SD_MOUNT_POINT, true)) {
        Serial.println("AI: SD Mount Failed");
        return false;
    }
    
    File file = SD_MMC.open(CFG_MODEL_FILENAME);
    if (!file) {
        Serial.printf("AI: File %s not found\n", CFG_MODEL_FILENAME);
        return false;
    }

    size_t size = file.size();
    model_data = (uint8_t*) heap_caps_malloc(size, MALLOC_CAP_SPIRAM);
    if (!model_data) { 
        Serial.println("AI: Failed to allocate memory for model");
        file.close(); 
        return false; 
    }

    file.read(model_data, size);
    file.close();

    model = tflite::GetModel(model_data);
    if (model->version() != TFLITE_SCHEMA_VERSION) {
        Serial.println("AI: Model Schema Mismatch");
        return false;
    }

    return true;
}

void AIEngine::preprocessImage(camera_fb_t* fb) {
    
    int h = input->dims->data[1];
    int w = input->dims->data[2];
    
    
    uint8_t* in_ptr_uint8 = nullptr;
    int8_t* in_ptr_int8 = nullptr;
    float* in_ptr_float = nullptr;

    if (input->type == kTfLiteUInt8) in_ptr_uint8 = input->data.uint8;
    else if (input->type == kTfLiteInt8) in_ptr_int8 = input->data.int8;
    else if (input->type == kTfLiteFloat32) in_ptr_float = input->data.f;

    
    int min_side = (fb->width < fb->height) ? fb->width : fb->height;
    int crop_x = (fb->width - min_side) / 2;
    int crop_y = (fb->height - min_side) / 2;

    for (int y = 0; y < h; y++) {
        int src_y = crop_y + (y * min_side / h);
        for (int x = 0; x < w; x++) {
            int src_x = crop_x + (x * min_side / w);
            
           
            if (src_y >= fb->height) src_y = fb->height - 1;
            if (src_x >= fb->width) src_x = fb->width - 1;

            int idx = (src_y * fb->width + src_x) * 2;
            
            
            uint8_t high = fb->buf[idx];
            uint8_t low = fb->buf[idx + 1];
            uint16_t pixel = (high << 8) | low;

            
            uint8_t r = ((pixel >> 11) & 0x1F) * 255 / 31;
            uint8_t g = ((pixel >> 5) & 0x3F) * 255 / 63;
            uint8_t b = (pixel & 0x1F) * 255 / 31;

            int tensor_idx = (y * w + x) * 3;

            
            if (in_ptr_uint8) {
                in_ptr_uint8[tensor_idx] = r;
                in_ptr_uint8[tensor_idx+1] = g;
                in_ptr_uint8[tensor_idx+2] = b;
            } else if (in_ptr_int8) {
               
                in_ptr_int8[tensor_idx] = (int8_t)((int)r - 128);
                in_ptr_int8[tensor_idx+1] = (int8_t)((int)g - 128);
                in_ptr_int8[tensor_idx+2] = (int8_t)((int)b - 128);
            } else if (in_ptr_float) {
                
                in_ptr_float[tensor_idx] = r / 255.0f;
                in_ptr_float[tensor_idx+1] = g / 255.0f;
                in_ptr_float[tensor_idx+2] = b / 255.0f;
            }
        }
    }
}

AIResult AIEngine::runModel(camera_fb_t* fb) {
    if (fb == nullptr) return {"No Frame", 0.0};

    
    preprocessImage(fb);
    
    
    if (interpreter->Invoke() != kTfLiteOk) {
        Serial.println("AI: Invoke failed");
        return {"Error", 0.0};
    }

    
    float max_score = 0;
    int best_idx = -1;
    TfLiteQuantizationParams params = output->params;
    
   
    if (output->type == kTfLiteUInt8) {
        uint8_t* results = output->data.uint8;
        for (int i = 0; i < 10; i++) {
            float score = (results[i] - params.zero_point) * params.scale;
            if (score > max_score) { max_score = score; best_idx = i; }
        }
    } else if (output->type == kTfLiteInt8) {
        int8_t* results = output->data.int8;
        for (int i = 0; i < 10; i++) {
            float score = (results[i] - params.zero_point) * params.scale;
            if (score > max_score) { max_score = score; best_idx = i; }
        }
    } else { // Float
         float* results = output->data.f;
         for (int i = 0; i < 10; i++) {
            if (results[i] > max_score) { max_score = results[i]; best_idx = i; }
         }
    }

    if (best_idx >= 0 && max_score > CFG_CONFIDENCE_THR) {
        return {kLabels[best_idx], max_score};
    }
    
    return {"Unsure", max_score};
}