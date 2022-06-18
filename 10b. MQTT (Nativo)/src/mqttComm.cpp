#include "mqttComm.h"
#include "wificonfig.h"
#include <QuickDebug.h>
#include "mqtt_client.h"

static const char* MQTT_TAG = "MQTT";

const char* mqtt_server = "192.168.5.120";
const char* ledTopic = "led/set";
const char* ledStateTopic = "led/state";
const char* buttonTopic = "led/set";

extern bool ledChanged;
extern bool ledOn;


esp_mqtt_client_handle_t client;
String clientId;

void manageMqttData (char* topic, size_t topicLen, char* payload, int len) {
    int msg_id;
    if (len > 0 && strncmp (topic, ledTopic, topicLen) == 0) {
        if (strncmp (payload, "1", 1) == 0) {
            DEBUG_INFO (MQTT_TAG, "Led on");
            ledChanged = true;
            ledOn = true;
        } else if (strncmp (payload, "0", 1) == 0) {
            DEBUG_INFO (MQTT_TAG, "Led off");
            ledChanged = true;
            ledOn = false;
        }
    } else if (len > 0 && strncmp (topic, ledStateTopic, topicLen) == 0) {
        if (strncmp (payload, "?", 1) == 0) {
            DEBUG_INFO (MQTT_TAG, "Led state request");
            msg_id = esp_mqtt_client_publish (client, ledStateTopic, ledOn ? "1" : "0", 1, 0, 0);
            DEBUG_INFO (MQTT_TAG, "sent publish successful, msg_id=%d", msg_id);
        }
    }

}

static void mqtt_event_handler (void* handler_args, esp_event_base_t base, int32_t event_id, void* event_data) {
    ESP_LOGD (TAG, "Event dispatched from event loop base=%s, event_id=%d", base, event_id);
    esp_mqtt_event_handle_t event = (esp_mqtt_event_handle_t) event_data;
    esp_mqtt_client_handle_t client = event->client;
    int msg_id;
    switch ((esp_mqtt_event_id_t)event_id) {
    case MQTT_EVENT_CONNECTED:
        DEBUG_INFO (MQTT_TAG, "MQTT_EVENT_CONNECTED");
        msg_id = esp_mqtt_client_publish (client, "led/client", clientId.c_str (), clientId.length (), 1, 0);
        DEBUG_INFO (MQTT_TAG, "sent publish successful, msg_id=%d", msg_id);

        msg_id = esp_mqtt_client_subscribe (client, ledTopic, 0);
        DEBUG_INFO (MQTT_TAG, "sent subscribe successful, msg_id=%d", msg_id);

        msg_id = esp_mqtt_client_subscribe (client, ledStateTopic, 0);
        DEBUG_INFO (MQTT_TAG, "sent subscribe successful, msg_id=%d", msg_id);

        break;
    case MQTT_EVENT_DISCONNECTED:
        DEBUG_INFO (MQTT_TAG, "MQTT_EVENT_DISCONNECTED");
        break;

    case MQTT_EVENT_SUBSCRIBED:
        DEBUG_INFO (MQTT_TAG, "MQTT_EVENT_SUBSCRIBED, msg_id=%d", event->msg_id);
        break;
    case MQTT_EVENT_UNSUBSCRIBED:
        DEBUG_INFO (MQTT_TAG, "MQTT_EVENT_UNSUBSCRIBED, msg_id=%d", event->msg_id);
        break;
    case MQTT_EVENT_PUBLISHED:
        DEBUG_INFO (MQTT_TAG, "MQTT_EVENT_PUBLISHED, msg_id=%d", event->msg_id);
        break;
    case MQTT_EVENT_DATA:
        DEBUG_INFO (MQTT_TAG, "MQTT_EVENT_DATA");
        DEBUG_INFO (MQTT_TAG, "Topic: %.*s", event->topic_len, event->topic);
        DEBUG_INFO (MQTT_TAG, "Data: %.*s", event->data_len, event->data);
        manageMqttData(event->topic, event->topic_len, event->data, event->data_len);
        break;
    case MQTT_EVENT_ERROR:
        DEBUG_INFO (MQTT_TAG, "MQTT_EVENT_ERROR");
        if (event->error_handle->error_type == MQTT_ERROR_TYPE_TCP_TRANSPORT) {
            LOG_ERROR_IF_NON_ZERO (MQTT_TAG, event->error_handle->esp_tls_last_esp_err, "reported from esp-tls");
            LOG_ERROR_IF_NON_ZERO (MQTT_TAG, event->error_handle->esp_tls_stack_err, "reported from tls stack");
            LOG_ERROR_IF_NON_ZERO (MQTT_TAG, event->error_handle->esp_transport_sock_errno, "captured as transport's socket errno");
            DEBUG_INFO (MQTT_TAG, "Last errno string (%s)", strerror (event->error_handle->esp_transport_sock_errno));
        }
        break;
    default:
        DEBUG_INFO (MQTT_TAG, "Other event id:%d", event->event_id);
        break;
    }
}

void initMqttComm () {
    DEBUG_INFO (MQTT_TAG, "initMqttComm");
    
    String brokerURL = "mqtt://";
    brokerURL += MQTT_USER;
    brokerURL += ":";
    brokerURL += MQTT_PASSWORD;
    brokerURL += "@";
    brokerURL += mqtt_server;

    DEBUG_INFO (MQTT_TAG, "Server: %s", brokerURL.c_str ());

    String clientId_conn = "ESP8266Client-";
    clientId_conn += String ((int32_t)(ESP.getEfuseMac () & 0xFFFFFFFF), HEX);
    clientId_conn += String ((int32_t)(ESP.getEfuseMac () >> 32), HEX);

    DEBUG_INFO (MQTT_TAG, "Server: %s clientId: %s", brokerURL.c_str (), clientId_conn.c_str ());
    
    esp_mqtt_client_config_t mqtt_cfg = {
        .uri = brokerURL.c_str (),
        .client_id = clientId_conn.c_str (),
    };

    DEBUG_INFO (MQTT_TAG, "Configuración creada: URI: %s, Client ID: %s", mqtt_cfg.uri, mqtt_cfg.client_id);

    clientId = clientId_conn;

    client = esp_mqtt_client_init (&mqtt_cfg);

    esp_mqtt_client_register_event (client, (esp_mqtt_event_id_t)ESP_EVENT_ANY_ID, mqtt_event_handler, NULL);
    esp_mqtt_client_start (client);

    DEBUG_INFO (MQTT_TAG, "MQTT client started");
}

void sendButtonMqtt (bool state) {
    DEBUG_INFO (MQTT_TAG, "Send button State: %d", state);
    String payload = state ? "1" : "0";
    int msg_id = esp_mqtt_client_publish (client, buttonTopic, payload.c_str (), payload.length (), 0, 0);
    DEBUG_INFO (MQTT_TAG, "sent publish successful, msg_id=%d", msg_id);
}