package com.example.iot_control.presentation

import android.app.Application
import android.util.Log
import androidx.lifecycle.AndroidViewModel
import androidx.lifecycle.viewModelScope
import com.example.iot_control.core.Constants.BROKER_IP
import com.example.iot_control.core.Constants.BROKER_PORT
import com.example.iot_control.core.Constants.WS_URL
import com.example.iot_control.core.MqttManager
import com.example.iot_control.core.WebSocketManager
import com.example.iot_control.domain.AlertType
import com.example.iot_control.domain.DHT11Data
import com.example.iot_control.domain.MQ2Data
import com.example.iot_control.service.WebSocketServiceManager
import kotlinx.coroutines.cancel
import kotlinx.coroutines.flow.MutableStateFlow
import kotlinx.coroutines.flow.StateFlow
import kotlinx.coroutines.flow.asStateFlow
import kotlinx.coroutines.flow.first
import kotlinx.coroutines.launch
import kotlinx.serialization.json.Json

class DashboardViewModel(application: Application) : AndroidViewModel(application) {
    private val _mcuState = MutableStateFlow("standby")
    val mcuState = _mcuState.asStateFlow()

    private val _measurePeriod = MutableStateFlow(5)
    val measurePeriod = _measurePeriod.asStateFlow()

    private val _subscribedAlerts = MutableStateFlow(setOf("fire", "gas_leak", "high_humidity"))
    val subscribedAlerts = _subscribedAlerts.asStateFlow()

    private val _notificationsEnabled = MutableStateFlow(false)
    val notificationsEnabled = _notificationsEnabled.asStateFlow()

    private val _DHT11DataList = MutableStateFlow<List<DHT11Data>>(emptyList())
    val DHT11DataList: StateFlow<List<DHT11Data>> = _DHT11DataList.asStateFlow()

    private val _MQ2DataList = MutableStateFlow<List<MQ2Data>>(emptyList())
    val MQ2DataList: StateFlow<List<MQ2Data>> = _MQ2DataList.asStateFlow()

    private val _isWSConnected = MutableStateFlow(false)//mozda to delete
    val isWSConnected: StateFlow<Boolean> = _isWSConnected.asStateFlow()

    private val _isMqttConnected = MutableStateFlow(false)
    val isMqttConnected: StateFlow<Boolean> = _isMqttConnected.asStateFlow()

    private val _alerts = MutableStateFlow("")
    val alerts: StateFlow<String> = _alerts.asStateFlow()

    private val wsServiceManager = WebSocketServiceManager(application)
    private var wsManager: WebSocketManager? = null
    private var mqttManager: MqttManager? = null

    init {
        viewModelScope.launch {
            _notificationsEnabled.value= wsServiceManager.isServiceRunning(application)
            
            connectMqtt()

            isMqttConnected.first { it }

            subscribeMqtt("sensors/mq2")
            subscribeMqtt("sensors/dht11")
            subscribeMqtt("alerts")
            Log.d("mqtt", "launched")
        }
    }

    fun connectWS() {
        wsServiceManager.startService(WS_URL)
        _isWSConnected.value = true
    }

    fun disconnectWS() {
        wsManager?.disconnect()
        wsServiceManager.stopService()
        _isWSConnected.value = false
    }

    fun connectMqtt() {
        connectMqtt(BROKER_IP, BROKER_PORT)
    }

    private fun connectMqtt(
        brokerHost: String, brokerPort: Int, username: String? = null, password: String? = null
    ) {
        mqttManager = MqttManager(brokerHost, brokerPort)

        viewModelScope.launch {
            mqttManager?.isConnected?.collect { connected ->
                _isMqttConnected.value = connected
            }
        }

        mqttManager?.onMessageReceived = { topic, message ->
            handleMqttMessage(topic, message)
        }

        mqttManager?.connect(username, password)
    }

    fun subscribeMqtt(topic: String) {
        mqttManager?.subscribe(topic)
    }

    fun publishMqtt(topic: String, message: String) {
        mqttManager?.publish(topic, message)
    }

    fun disconnectMqtt() {
        mqttManager?.disconnect()
    }

    private fun handleMqttMessage(topic: String, message: String) {
        try {
            when (topic) {
                "alerts" -> {
                    val alert = Json.decodeFromString<AlertType>(message)
                    _alerts.value = alert.type;
                }

                "sensors/mq2" -> {
                    val mq2 = Json.decodeFromString<MQ2Data>(message)
                    _MQ2DataList.value = listOf(mq2) + MQ2DataList.value
                }

                "sensors/dht11" -> {
                    val dht11 = Json.decodeFromString<DHT11Data>(message)
                    _DHT11DataList.value = listOf(dht11) + _DHT11DataList.value
                }

                else -> {
                    Log.d("mqtt", "unknown topic")
                }
            }

        } catch (e: Exception) {
            Log.e("MQTT", "Failed to parse message: ${e.message}")
        }
    }

    fun setMcuState(state: String) {
        _mcuState.value = state
        // Publish to MQTT: "mcu/state" with payload state
    }

    fun blinkLed() {
        // Publish to MQTT: "mcu/led/blink"
    }

    fun setMeasurePeriod(seconds: Int) {
        _measurePeriod.value = seconds//NE TU NEGO U SUBSCRIBE AL AJDE MOZD AI MOZE
        // Publish to MQTT: "mcu/period" with payload seconds
    }

    fun toggleAlertSubscription(alert: String) {
        _subscribedAlerts.value = if (_subscribedAlerts.value.contains(alert)) {
            _subscribedAlerts.value - alert
        } else {
            _subscribedAlerts.value + alert
        }
        // Subscribe/unsubscribe from MQTT topic: "alerts/$alert"
    }

    fun toggleNotifications(enabled: Boolean) {
        _notificationsEnabled.value = enabled
        if (enabled) connectWS()
        else disconnectWS()
    }

    override fun onCleared() {
        super.onCleared()
//        disconnectWS()//?????????????
        disconnectMqtt()
        viewModelScope.cancel()

    }
}