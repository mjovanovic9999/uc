package com.example.iot_control.domain

import kotlinx.serialization.Serializable

@Serializable

data class MQ2Data(
    val quality: Float? = null,
)
