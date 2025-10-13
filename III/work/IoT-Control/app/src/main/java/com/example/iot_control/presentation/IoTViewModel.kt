package com.example.iot_control.presentation

import android.util.Log
import androidx.lifecycle.ViewModel
import com.example.iot_control.core.WebSocketManager
import com.example.iot_control.domain.ConnectionConfig
import com.example.iot_control.domain.SensorData
import kotlinx.coroutines.flow.MutableStateFlow
import kotlinx.coroutines.flow.StateFlow
import kotlinx.coroutines.flow.asStateFlow
import org.json.JSONObject
import java.text.SimpleDateFormat
import java.util.Date
import java.util.Locale

class IoTViewModel : ViewModel() {
    private val _sensorDataList = MutableStateFlow<List<SensorData>>(emptyList())
    val sensorDataList: StateFlow<List<SensorData>> = _sensorDataList.asStateFlow()

    private val _isConnected = MutableStateFlow(false)
    val isConnected: StateFlow<Boolean> = _isConnected.asStateFlow()

    private val _config = MutableStateFlow(ConnectionConfig())
    val config: StateFlow<ConnectionConfig> = _config.asStateFlow()

    private val _alerts = MutableStateFlow<List<String>>(emptyList())
    val alerts: StateFlow<List<String>> = _alerts.asStateFlow()

    private var wsManager: WebSocketManager? = null

    fun connect() {
        wsManager = WebSocketManager(_config.value.serverUrl).apply {
            onMessageReceived = { message ->
                Log.d("IoTViewModel", "Message received: $message")
                parseMessage(message)
            }
            onConnectionChange = { connected ->
                Log.d("IoTViewModel", "Connection state changed: $connected")
                _isConnected.value = connected
            }
            connect()
        }
    }

    fun disconnect() {
        wsManager?.disconnect()
        _isConnected.value = false
    }

    fun updateServerUrl(url: String) {
        _config.value = _config.value.copy(serverUrl = url)
    }

    fun sendCommand(command: String) {
        wsManager?.sendMessage(command)
    }

    private fun parseMessage(message: String) {
        try {
            val json = JSONObject(message)
            val topic = json.optString("topic", "unknown")
            val timestamp = SimpleDateFormat("HH:mm:ss", Locale.getDefault()).format(Date())

            val sensorData = when {
                topic.contains("dht11") -> SensorData(
                    timestamp = timestamp,
                    topic = "DHT11",
                    temperature = json.optDouble("temperature", 0.0).toFloat(),
                    humidity = json.optDouble("humidity", 0.0).toFloat()
                )

                topic.contains("mq2") -> SensorData(
                    timestamp = timestamp,
                    topic = "MQ2",
                    gasLevel = json.optInt("gas_level", 0)
                )

                topic.contains("alert") -> {
                    val alertMsg = json.optString("message", "Alert detected")
                    _alerts.value = listOf(alertMsg) + _alerts.value.take(9)
                    SensorData(
                        timestamp = timestamp,
                        topic = "ALERT",
                        alertType = alertMsg
                    )
                }

                else -> null
            }

            sensorData?.let {
                _sensorDataList.value = listOf(it) + _sensorDataList.value.take(49)
            }
        } catch (e: Exception) {
            e.printStackTrace()
        }
    }

    override fun onCleared() {
        super.onCleared()
        disconnect()
    }
}