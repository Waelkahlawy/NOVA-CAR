#ifndef AI_ENGINE_H
#define AI_ENGINE_H

#include "esp_camera.h"
#include "tensorflow/lite/micro/micro_interpreter.h"
#include "tensorflow/lite/micro/micro_log.h"
#include "tensorflow/lite/micro/micro_mutable_op_resolver.h"
#include "tensorflow/lite/schema/schema_generated.h"
#include "../Config.h"

struct AIResult {
    const char* label;
    float score;
};

class AIEngine {
public:
    bool init();
    AIResult runModel(camera_fb_t* fb);

private:
    const tflite::Model* model = nullptr;
    tflite::MicroInterpreter* interpreter = nullptr;
    TfLiteTensor* input = nullptr;
    TfLiteTensor* output = nullptr;
    uint8_t *tensor_arena = nullptr;
    uint8_t *model_data = nullptr;
    
    bool loadModelData();
    void preprocessImage(camera_fb_t* fb);
};

#endif