package com.example.iot_control.domain

import com.example.iot_control.core.Constants.WS_URL

data class ConnectionConfig(
    val serverUrl: String = WS_URL,
    val reconnectEnabled: Boolean = true
)
