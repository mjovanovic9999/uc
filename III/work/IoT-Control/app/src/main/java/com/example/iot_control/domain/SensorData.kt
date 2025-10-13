package com.example.iot_control.domain

data class SensorData(
    val timestamp: String,
    val topic: String,
    val temperature: Float? = null,
    val humidity: Float? = null,
    val gasLevel: Int? = null,
    val alertType: String? = null
)
