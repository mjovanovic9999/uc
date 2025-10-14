package com.example.iot_control.core

import android.content.Context
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
                Log.d("WebSocket", "Message: $text")
                onMessageReceived?.invoke(text)
//                showNotification(text)
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

    fun sendMessage(message: String) {
        webSocket?.send(message)
    }

    fun disconnect() {
        webSocket?.close(1000, "User disconnect")
    }

    fun isConnected(): Boolean = isConnected

//    private fun showNotification(message: String) {
//        val notificationHandler = NotificationHandler(context)
//        notificationHandler.showSimpleNotification("New Message", message)
//    }
}