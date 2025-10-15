package com.example.iot_control.service


import android.content.Context
import android.content.Intent
import android.util.Log

class WebSocketServiceManager(private val context: Context) {
    private var isRunning = false
    fun startService(serverUrl: String) {
        isRunning = true
        Log.d("wsmngr", "started")
        val intent = Intent(context, WebSocketForegroundService::class.java).apply {
            putExtra("server_url", serverUrl)
        }
        context.startForegroundService(intent)
    }

    fun stopService() {
        isRunning = false
        val intent = Intent(context, WebSocketForegroundService::class.java)
        context.stopService(intent)
    }

    fun isServiceRunning(): Boolean = isRunning
}