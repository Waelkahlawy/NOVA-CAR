#ifndef FOTA_TASK_H
#define FOTA_TASK_H



void Fota_MqttCallback(const char *topic, const char *data);
void Fota_Task(void *pvParameters);




#endif