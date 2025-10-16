package com.example.iot_control.core

import android.content.Context
import android.util.Log
import com.hivemq.client.mqtt.MqttClient
import com.hivemq.client.mqtt.datatypes.MqttQos
import com.hivemq.client.mqtt.mqtt3.Mqtt3AsyncClient
import kotlinx.coroutines.flow.MutableStateFlow
import kotlinx.coroutines.flow.StateFlow
import kotlinx.coroutines.flow.asStateFlow
import java.nio.charset.StandardCharsets

class MqttManager(
    private val brokerHost: String,
    private val brokerPort: Int,
    private val clientId: String = "AndroidClient_${System.currentTimeMillis()}"
) {
    private var mqttClient: Mqtt3AsyncClient? = null

    private val _isConnected = MutableStateFlow(false)
    val isConnected: StateFlow<Boolean> = _isConnected.asStateFlow()

    var onMessageReceived: ((topic: String, message: String) -> Unit)? = null

    fun connect(username: String? = null, password: String? = null) {
        val builder = MqttClient.builder()
            .useMqttVersion3()
            .identifier(clientId)
            .serverHost(brokerHost)
            .serverPort(brokerPort)

        mqttClient = builder.buildAsync()

        val connectBuilder = mqttClient!!.connectWith()
            .cleanSession(true)
            .keepAlive(60)

        if (username != null) {
            connectBuilder.simpleAuth()
                .username(username)
                .password((password ?: "").toByteArray())
                .applySimpleAuth()
        }

        connectBuilder.send()
            .whenComplete { _, throwable ->
                if (throwable != null) {
                    _isConnected.value = false
                    Log.e("MQTT", "Connection failed: ${throwable.message}")
                } else {
                    _isConnected.value = true
                    Log.d("MQTT", "Connected to $brokerHost:$brokerPort")
                    setupMessageListener()
                }
            }
    }

    private fun setupMessageListener() {
        mqttClient?.publishes(com.hivemq.client.mqtt.MqttGlobalPublishFilter.ALL) { publish ->
            val topic = publish.topic.toString()
            val message = String(publish.payloadAsBytes, StandardCharsets.UTF_8)
            Log.d("MQTT", "Message from $topic: $message")
            onMessageReceived?.invoke(topic, message)
        }
    }

    fun subscribe(topic: String, qos: MqttQos = MqttQos.AT_LEAST_ONCE) {
        mqttClient?.subscribeWith()
            ?.topicFilter(topic)
            ?.qos(qos)
            ?.send()
            ?.whenComplete { _, throwable ->
                if (throwable != null) {
                    Log.e("MQTT", "Subscribe failed: ${throwable.message}")
                } else {
                    Log.d("MQTT", "Subscribed to $topic")
                }
            }
    }

    fun publish(
        topic: String,
        message: String,
        qos: MqttQos = MqttQos.AT_LEAST_ONCE,
        retained: Boolean = false
    ) {
        mqttClient?.publishWith()
            ?.topic(topic)
            ?.payload(message.toByteArray(StandardCharsets.UTF_8))
            ?.qos(qos)
            ?.retain(retained)
            ?.send()
            ?.whenComplete { _, throwable ->
                if (throwable != null) {
                    Log.e("MQTT", "Publish failed: ${throwable.message}")
                } else {
                    Log.d("MQTT", "Published to $topic")
                }
            }
    }

    fun unsubscribe(topic: String) {
        mqttClient?.unsubscribeWith()
            ?.topicFilter(topic)
            ?.send()
    }

    fun disconnect() {
        mqttClient?.disconnect()?.whenComplete { _, _ ->
            _isConnected.value = false
            Log.d("MQTT", "Disconnected")
        }
    }
}