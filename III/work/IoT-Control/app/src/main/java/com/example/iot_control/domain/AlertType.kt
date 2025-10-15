package com.example.iot_control.domain

import kotlinx.serialization.Serializable

@Serializable
data class AlertType(
    val type: String = ""
)