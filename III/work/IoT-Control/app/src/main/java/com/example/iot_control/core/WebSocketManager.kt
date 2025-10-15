
package com.example.iot_control.core

import android.util.Log
import com.example.iot_control.domain.AlertType
import kotlinx.serialization.json.Json
import okhttp3.OkHttpClient
import okhttp3.Request
import okhttp3.Response
import okhttp3.WebSocket
import okhttp3.WebSocketListener
import java.util.concurrent.TimeUnit

class WebSocketManager(private val url: String) {
    private var webSocket: WebSocket? = null
    private val client = OkHttpClient.Builder()
        .pingInterval(15, TimeUnit.SECONDS)
        .readTimeout(30, TimeUnit.SECONDS)
        .build()

    var onMessageReceived: ((String) -> Unit)? = null
    var onConnectionChange: ((Boolean) -> Unit)? = null
    private var isConnected = false

    fun connect() {
        if (isConnected) return

        val request = Request.Builder().url(url).build()
        Log.d("WebSocket", "Connecting to $url")
        webSocket = client.newWebSocket(request, object : WebSocketListener() {
            override fun onOpen(webSocket: WebSocket, response: Response) {
                isConnected = true
                onConnectionChange?.invoke(true)
            }

            override fun onMessage(webSocket: WebSocket, text: String) {
                Log.d("WebSocket", "Detected: $text")
                val alert = Json.decodeFromString<AlertType>(text)

                onMessageReceived?.invoke(alert.type)
            }

            override fun onFailure(webSocket: WebSocket, t: Throwable, response: Response?) {
                isConnected = false
                Log.e("WebSocket", "Connection failed: ${t.message}")
                onConnectionChange?.invoke(false)
            }

            override fun onClosed(webSocket: WebSocket, code: Int, reason: String) {
                isConnected = false
                Log.d("WebSocket", "Closed: $reason")
                onConnectionChange?.invoke(false)
            }
        })
    }

    fun disconnect() {
        webSocket?.close(1000, "User disconnect")
    }
}