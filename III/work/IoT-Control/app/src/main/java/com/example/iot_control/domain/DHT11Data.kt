package com.example.iot_control.domain

import kotlinx.serialization.Serializable

@Serializable
data class DHT11Data(
    val temperature: Float? = null,
    val humidity: Float? = null,
)
