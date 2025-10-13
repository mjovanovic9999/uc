package com.example.iot_control.core

import android.util.Log
import okhttp3.OkHttpClient
import okhttp3.Request
import okhttp3.Response
import okhttp3.WebSocket
import okhttp3.WebSocketListener
import java.util.concurrent.TimeUnit

class WebSocketManager(private val url: String) {
    private var webSocket: WebSocket? = null
    private val client = OkHttpClient.Builder()
        .readTimeout(3, TimeUnit.SECONDS)
        .build()

    var onMessageReceived: ((String) -> Unit)? = null
    var onConnectionChange: ((Boolean) -> Unit)? = null

    fun connect() {
        val request = Request.Builder().url(url).build()
        Log.d("WebSocket", "Connecting to $url")
        webSocket = client.newWebSocket(request, object : WebSocketListener() {
            override fun onOpen(webSocket: WebSocket, response: Response) {
                Log.d("WebSocket", "Connected ✅")
                onConnectionChange?.invoke(true)
            }

            override fun onMessage(webSocket: WebSocket, text: String) {
                Log.d("WebSocket", "Message: $text")
                onMessageReceived?.invoke(text)
            }

            override fun onFailure(webSocket: WebSocket, t: Throwable, response: Response?) {
                Log.e("WebSocket", "Connection failed ❌: ${t.message}")
                onConnectionChange?.invoke(false)
            }

            override fun onClosed(webSocket: WebSocket, code: Int, reason: String) {
                Log.d("WebSocket", "Closed: $reason")
                onConnectionChange?.invoke(false)
            }
        })
    }

    fun sendMessage(message: String) {
        webSocket?.send(message)
    }

    fun disconnect() {
        webSocket?.close(1000, "User disconnect")
    }
}